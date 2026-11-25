"""Pliki syntetyczne do testów (bez prawdziwego firmware i bez pliku dawcy)."""
import hashlib
import importlib
import random
import sys
import types
from pathlib import Path

from rt3s_flasher import core

ROOT = Path(__file__).resolve().parents[3]
LOADER_DIR = ROOT / 'MDUV380_firmware' / 'tools'

LANG_VERSION = 0x0000002A
ENGLISH_AT = 0x20000
SLOT_AT = 0x21000


def rnd(n, seed):
    return random.Random(seed).randbytes(n)


def firmware(size=core.CODEC_TARGET + core.CODEC_LENGTH + 0x3000, lang_version=LANG_VERSION, seed=1):
    fw = bytearray(rnd(size, seed))
    # Pierwsza tabela (angielska) i miejsce na drugi język, jak w buildzie OpenGD77
    fw[ENGLISH_AT:ENGLISH_AT + 19] = b'GD77LANG' + lang_version.to_bytes(4, 'big') + b'English'
    fw[SLOT_AT:SLOT_AT + 19] = b'GD77LANG' + lang_version.to_bytes(4, 'big') + b'English'
    # Losowe dane nie mogą przypadkiem zawierać znaczników
    for tag in (b'IGNRLANG',):
        assert fw.find(tag) < 0
    assert fw.find(b'GD77LANG') == ENGLISH_AT
    return bytes(fw)


def language(version=LANG_VERSION, size=0x900):
    return b'GD77LANG' + version.to_bytes(4, 'big') + b'Polski\0' + rnd(size - 19, 7)


def donor():
    """Udaje zaszyfrowany plik MD-9600 (tylko rozmiar ma znaczenie dla wklejania kodeka)."""
    return rnd(core.CODEC_DONOR_OFFSET + core.CODEC_LENGTH + 0x100, 3)


def reference_loader():
    """Loader OpenGD77 z repo z podstawionym modułem usb (do porównań, bez radia)."""
    usb = types.ModuleType('usb')
    usb.core = types.ModuleType('usb.core')
    usb.util = types.ModuleType('usb.util')
    usb.util.get_string = lambda dev, index: None
    saved = {k: sys.modules.get(k) for k in ('usb', 'usb.core', 'usb.util')}
    sys.modules.update({'usb': usb, 'usb.core': usb.core, 'usb.util': usb.util})
    sys.path.insert(0, str(LOADER_DIR))
    try:
        sys.modules.pop('opengd77_stm32_firmware_loader', None)
        return importlib.import_module('opengd77_stm32_firmware_loader')
    finally:
        sys.path.remove(str(LOADER_DIR))
        for k, v in saved.items():
            if v is None:
                sys.modules.pop(k, None)
            else:
                sys.modules[k] = v


def reference_ops(loader, model, fw_path, lang_path=None, donor_path=''):
    """Lista operacji DFU, które wysłałby loader OpenGD77 (jak web/test/python_reference.py)."""
    ops = []
    loader.wait_for_idle = lambda: None
    loader.send_custom_command = lambda a, b: ops.append(f'cmd {a:02x} {b:02x}')
    loader.page_erase = lambda a: ops.append(f'erase {a:08x}')
    loader.set_address = lambda a: ops.append(f'addr {a:08x}')
    loader.write_firmware_data = lambda n, c: ops.append(f'write {n} {hashlib.sha256(bytes(c)).hexdigest()}')
    loader.exit_dfu = lambda *a: ops.append('exit')
    loader.print = lambda *a, **k: None
    loader.config = {'GLOBAL': {'SourceSTM32Firmware': str(donor_path)}}
    loader.patch_and_download_firmware(str(fw_path), str(lang_path) if lang_path else None,
                                       loader.FWPlatformOutput(loader.FWOutputPlatforms.index(model)))
    return ops


def our_ops(image):
    ops = ['cmd 91 01', 'cmd 91 31'] + [f'erase {a:08x}' for a in core.ADDRESSES]
    for s in core.write_plan(image):
        ops.append(f'addr {s.address:08x}')
        ops += [f'write {n} {hashlib.sha256(d).hexdigest()}' for n, d in s.blocks]
    return ops + ['exit']
