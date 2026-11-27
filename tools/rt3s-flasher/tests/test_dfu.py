"""Zapis przez DFU na symulatorze bootloadera (bez radia)."""
import unittest

from rt3s_flasher import core, dfu

from . import helpers

FLASH_BASE = 0x08000000
FLASH_SIZE = 0x100000 + 0x20000


class FakeBootloader:
    """Uproszczony bootloader STM32 DFU w wersji TYT: polecenia 0x41 (kasuj sektor), 0x21 (adres),
    bloki od numeru 2 zapisywane pod adres + (n - 2) * 1024. Polecenie wykonuje się przy pierwszym
    GETSTATUS (dfuDNBUSY), drugie zwraca dfuDNLOAD-IDLE."""

    def __init__(self, start_state=dfu.IDLE, slow_erase=0, fail_on_block=None):
        self.mem = bytearray(b'\x5a' * FLASH_SIZE)   # stara zawartość, nie 0xFF
        self.state = start_state
        self.pending = None
        self.busy_left = 0
        self.address = None
        self.log = []
        self.erased = []
        self.slow_erase = slow_erase
        self.fail_on_block = fail_on_block
        self.exited = False

    def ctrl_transfer(self, bm, req, value, index, data, timeout):
        assert index == dfu.INTERFACE
        if bm == dfu.REQ_IN and req == dfu.GETSTATUS:
            assert data == 6
            return self.get_status()
        assert bm == dfu.REQ_OUT
        if req == dfu.CLRSTATUS or req == dfu.ABORT:
            self.state = dfu.IDLE
            self.pending = None
            return 0
        assert req == dfu.DNLOAD
        payload = bytes(data or b'')
        self.pending = (value, payload)
        self.state = 3   # dfuDNLOAD-SYNC
        return len(payload)

    def get_status(self):
        if self.pending is not None:
            value, payload = self.pending
            self.pending = None
            self.execute(value, payload)
            if self.state == dfu.ERROR:
                return [0x0A, 0, 0, 0, dfu.ERROR, 0]
            self.state = dfu.DNLOAD_BUSY
            return [0, 0, 0, 0, dfu.DNLOAD_BUSY, 0]
        if self.state == dfu.DNLOAD_BUSY:
            if self.busy_left:
                self.busy_left -= 1
                return [0, 0, 0, 0, dfu.DNLOAD_BUSY, 0]
            self.state = dfu.DNLOAD_IDLE
        return [0, 0, 0, 0, self.state, 0]

    def execute(self, value, payload):
        if value == 0:
            if len(payload) == 0:
                self.log.append('exit')
                self.exited = True
            elif len(payload) == 2:
                self.log.append(f'cmd {payload[0]:02x} {payload[1]:02x}')
            elif payload[0] == 0x41:
                addr = int.from_bytes(payload[1:5], 'little')
                i = core.ADDRESSES.index(addr)
                off = addr - FLASH_BASE
                self.mem[off:off + core.SIZES[i]] = b'\xff' * core.SIZES[i]
                self.erased.append(addr)
                self.busy_left = self.slow_erase
                self.log.append(f'erase {addr:08x}')
            elif payload[0] == 0x21:
                self.address = int.from_bytes(payload[1:5], 'little')
                self.log.append(f'addr {self.address:08x}')
            else:
                self.state = dfu.ERROR
        else:
            if value == self.fail_on_block:
                self.state = dfu.ERROR
                return
            assert len(payload) == core.BLOCK_SIZE
            off = self.address - FLASH_BASE + (value - 2) * core.BLOCK_SIZE
            assert self.mem[off:off + core.BLOCK_SIZE] == b'\xff' * core.BLOCK_SIZE, 'zapis do nieskasowanej pamięci'
            self.mem[off:off + core.BLOCK_SIZE] = payload
            self.log.append(f'write {value}')


def flash(fake, image):
    events = []
    dfu.Dfu(fake, sleep=lambda s: None).flash(image, lambda *e: events.append(e))
    return events


class DfuTest(unittest.TestCase):
    def setUp(self):
        self.image = core.build_image(helpers.firmware(), language=helpers.language(), donor=helpers.donor()).data

    def check_memory(self, fake):
        start = core.ADDRESSES[0] - FLASH_BASE
        padded = len(self.image) + (-len(self.image)) % core.BLOCK_SIZE
        self.assertEqual(bytes(fake.mem[start:start + len(self.image)]), bytes(self.image))
        self.assertEqual(set(fake.mem[start + len(self.image):start + padded]), {0xFF})
        self.assertEqual(fake.mem[:start], b'\x5a' * start, 'bootloader (0x08000000-0x0800BFFF) nienaruszony')

    def test_full_flash(self):
        fake = FakeBootloader()
        events = flash(fake, self.image)
        self.check_memory(fake)
        self.assertEqual(fake.erased, list(core.ADDRESSES))
        self.assertEqual(fake.log[:2], ['cmd 91 01', 'cmd 91 31'])
        self.assertEqual(fake.log[-2:], ['addr 08000000', 'exit'])
        self.assertEqual(events[-1][0], 'done')
        writes = [e for e in events if e[0] == 'write']
        self.assertEqual(writes[-1][1], writes[-1][2])

    def test_same_sequence_as_reference(self):
        fake = FakeBootloader()
        flash(fake, self.image)
        ref = [op if not op.startswith('write') else ' '.join(op.split()[:2]) for op in helpers.our_ops(self.image)]
        ref = ref[:-1] + ['addr 08000000', 'exit']
        self.assertEqual(fake.log, ref)

    def test_recovers_from_error_state_and_slow_erase(self):
        fake = FakeBootloader(start_state=dfu.ERROR, slow_erase=5)
        flash(fake, self.image)
        self.check_memory(fake)

    def test_write_error_is_reported(self):
        fake = FakeBootloader(fail_on_block=10)
        with self.assertRaises(dfu.DfuError):
            flash(fake, self.image)
        self.assertFalse(fake.exited)


if __name__ == '__main__':
    unittest.main()
