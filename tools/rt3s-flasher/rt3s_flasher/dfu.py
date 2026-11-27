"""Zapis przez DFU do bootloadera TYT / Retevis na STM32 („Digital Radio in USB mode”, 0483:df11).

Ta sama sekwencja poleceń co MDUV380_firmware/tools/opengd77_stm32_firmware_loader.py i flasher
WebUSB (web/js/dfu.js): 0x91 0x01, 0x91 0x31, kasowanie 10 sektorów, zapis bloków 1 KB, skok
pod 0x08000000 i pusty DNLOAD.
"""
import time

from .core import ADDRESSES, write_plan

VID = 0x0483
PID = 0xDF11
INTERFACE = 0
TIMEOUT_MS = 4000
STATUS_TIMEOUT_MS = 20000

DNLOAD, GETSTATUS, CLRSTATUS, ABORT = 1, 3, 4, 6
REQ_OUT = 0x21   # class, interface, host -> urządzenie
REQ_IN = 0xA1    # class, interface, urządzenie -> host

IDLE, DNLOAD_BUSY, DNLOAD_IDLE, MANIFEST, UPLOAD_IDLE, ERROR = 2, 4, 5, 7, 9, 10
STATE_NAMES = {0: 'appIDLE', 1: 'appDETACH', 2: 'dfuIDLE', 3: 'dfuDNLOAD-SYNC', 4: 'dfuDNBUSY',
               5: 'dfuDNLOAD-IDLE', 6: 'dfuMANIFEST-SYNC', 7: 'dfuMANIFEST', 8: 'dfuMANIFEST-WAIT-RESET',
               9: 'dfuUPLOAD-IDLE', 10: 'dfuERROR'}


class DfuError(Exception):
    pass


def _state(s):
    return STATE_NAMES.get(s, str(s))


class Dfu:
    """dev: urządzenie pyusb (albo obiekt z tą samą metodą ctrl_transfer, np. symulator w testach)."""

    def __init__(self, dev, sleep=time.sleep):
        self.dev = dev
        self.sleep = sleep

    # ------------------------------------------------------------------ niski poziom
    def out(self, request, value=0, data=None):
        self.dev.ctrl_transfer(REQ_OUT, request, value, INTERFACE, data, TIMEOUT_MS)

    def status(self):
        try:
            r = self.dev.ctrl_transfer(REQ_IN, GETSTATUS, 0, INTERFACE, 6, STATUS_TIMEOUT_MS)
        except Exception:   # noqa: BLE001 - tak jak loader: brak odpowiedzi = brak stanu
            return None
        return r[4] if len(r) >= 6 else None

    def abort(self):
        self.out(ABORT)

    def clear_status(self):
        self.out(CLRSTATUS)

    def prepare(self):
        """Doprowadza bootloader do stanu dfuIDLE (jak init() w loaderze)."""
        for _ in range(4):
            s = self.status()
            if s == IDLE:
                break
            if s in (DNLOAD_IDLE, UPLOAD_IDLE):
                self.abort()
            else:
                self.clear_status()

    def expect(self, stage, wanted):
        s = self.status()
        # Loader pyta raz; wolniejsze kasowanie sektora dostaje kilka dodatkowych prób (jak web)
        tries = 0
        while s == DNLOAD_BUSY and wanted == DNLOAD_IDLE and tries < 50:
            self.sleep(0.02)
            s = self.status()
            tries += 1
        if s != wanted:
            raise DfuError(f'{stage}: stan {_state(s)}, oczekiwano {_state(wanted)}')

    def wait_for_idle(self):
        s = self.status()
        tries = 0
        while s != IDLE:
            if tries > 40:
                raise DfuError(f'bootloader nie wraca do dfuIDLE (stan {_state(s)})')
            self.clear_status()
            s = self.status()
            tries += 1

    def command(self, payload, stage):
        self.out(DNLOAD, 0, payload)
        self.expect(stage, DNLOAD_BUSY)
        self.expect(stage, DNLOAD_IDLE)

    @staticmethod
    def address_command(code, address):
        return bytes([code]) + address.to_bytes(4, 'little')

    def erase(self, address):
        self.command(self.address_command(0x41, address), f'kasowanie 0x{address:08X}')

    def set_address(self, address):
        self.command(self.address_command(0x21, address), f'adres 0x{address:08X}')

    def write_block(self, number, data):
        self.out(DNLOAD, number, data)
        self.expect(f'zapis bloku {number}', DNLOAD_BUSY)
        self.expect(f'zapis bloku {number}', DNLOAD_IDLE)

    def custom(self, a, b):
        self.out(DNLOAD, 0, bytes([a & 0xFF, b & 0xFF]))
        self.status()
        self.sleep(0.1)
        self.status()
        self.wait_for_idle()

    # ------------------------------------------------------------------ całość
    def flash(self, image, progress=lambda phase, done, total: None):
        """Zapisuje zaszyfrowany obraz (core.build_image().data).
        progress(phase, done, total), phase: 'erase', 'write', 'done'."""
        plan = write_plan(image)
        total = sum(len(s.blocks) for s in plan)
        done = 0

        self.prepare()
        self.wait_for_idle()
        self.custom(0x91, 0x01)
        self.custom(0x91, 0x31)

        for i, address in enumerate(ADDRESSES):
            progress('erase', i, len(ADDRESSES))
            self.erase(address)
        progress('erase', len(ADDRESSES), len(ADDRESSES))

        for sector in plan:
            self.set_address(sector.address)
            for number, data in sector.blocks:
                self.write_block(number, data)
                done += 1
                if done % 4 == 0 or done == total:
                    progress('write', done, total)

        # Wyjście z DFU: adres skoku, potem pusty DNLOAD; radio się restartuje
        self.set_address(0x08000000)
        try:
            self.out(DNLOAD, 0, None)
            self.status()
        except Exception:   # noqa: BLE001
            pass
        progress('done', total, total)


# ---------------------------------------------------------------------- USB (pyusb)
class UsbProblem(Exception):
    """Błąd po stronie systemu (brak libusb, uprawnienia, brak radia) z podpowiedzią po polsku."""


def _usb():
    try:
        import usb.core
        import usb.util
    except ImportError:
        raise UsbProblem('Brak modułu pyusb. Zainstaluj: pip install pyusb '
                         '(albo pakiet python3-usb z dystrybucji).') from None
    return usb


def find_radios():
    usb = _usb()
    try:
        return list(usb.core.find(find_all=True, idVendor=VID, idProduct=PID))
    except usb.core.NoBackendError:
        raise UsbProblem('Brak biblioteki libusb-1.0. Zainstaluj pakiet libusb-1.0-0 '
                         '(Debian/Ubuntu) lub libusb1 (Fedora/openSUSE/Arch).') from None


def describe(dev):
    usb = _usb()
    names = []
    for idx in (dev.iManufacturer, dev.iProduct):
        try:
            names.append(usb.util.get_string(dev, idx) or '')
        except Exception:   # noqa: BLE001 - bez uprawnień nie da się odczytać opisów
            names.append('?')
    return f'magistrala {dev.bus:03d} urządzenie {dev.address:03d}, {VID:04x}:{PID:04x}, ' \
           f'„{names[1]}” ({names[0]})'


def open_radio():
    """Znajduje jedno radio w trybie aktualizacji i przejmuje interfejs DFU."""
    usb = _usb()
    devices = find_radios()
    if not devices:
        raise UsbProblem('Nie widzę radia w trybie aktualizacji (0483:df11). Wyłącz radio, trzymając '
                         'PTT i boczny przycisk nad nim włącz je (ekran pusty, dioda miga), podłącz kabel.')
    if len(devices) > 1:
        raise UsbProblem('Podłączonych jest kilka urządzeń 0483:df11. Zostaw tylko radio.')
    dev = devices[0]
    try:
        try:
            if dev.is_kernel_driver_active(INTERFACE):
                dev.detach_kernel_driver(INTERFACE)
        except (NotImplementedError, usb.core.USBError):
            pass
        dev.set_configuration()
        usb.util.claim_interface(dev, INTERFACE)
    except usb.core.USBError as e:
        if e.errno == 13:
            raise UsbProblem('Brak uprawnień do radia (USB). Zainstaluj regułę udev: '
                             'sudo ./install.sh --udev, potem odłącz i podłącz kabel.') from None
        if e.errno == 16:
            raise UsbProblem('Radio jest zajęte przez inny program (CPS, przeglądarka z WebUSB, '
                             'inny loader). Zamknij go i spróbuj ponownie.') from None
        raise UsbProblem(f'Nie mogę otworzyć radia: {e}') from None
    return dev


def release(dev):
    try:
        _usb().util.dispose_resources(dev)
    except Exception:   # noqa: BLE001 - radio zwykle już się zrestartowało
        pass
