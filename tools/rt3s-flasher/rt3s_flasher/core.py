"""Przygotowanie obrazu firmware dla radii OpenGD77 na STM32 (RT3S / MD-UV380, RT-84 / DM-1701).

Ten sam algorytm co MDUV380_firmware/tools/opengd77_stm32_firmware_loader.py (F1RMB, VK3KYY)
i flasher WebUSB (web/js/flasher-core.js): wklejenie drugiego języka (.gla), wklejenie kodeka
AMBE z oryginalnego firmware MD-9600 V26.45, szyfrowanie XOR, podział na bloki DFU.
Bez USB: moduł działa i jest testowany bez radia.
"""
import hashlib
from dataclasses import dataclass, field

from .ciphers import DM1701_CIPHER, DONOR_SHA256, MD380_CIPHER, MD9600_CIPHER, MDUV380_CIPHER

PLATFORMS = {
    'MD-UV380': MDUV380_CIPHER,   # Retevis RT3S, TYT MD-UV380/390
    'MD-2017': MDUV380_CIPHER,
    'MD-380': MD380_CIPHER,
    'DM-1701': DM1701_CIPHER,     # Retevis RT-84, Baofeng DM-1701
    'MD-9600': MD9600_CIPHER,
}
DEFAULT_PLATFORM = 'MD-UV380'

# Sektory flash zapisywane przez loader, ich rozmiary i numer ostatniego bloku DFU w każdym
ADDRESSES = (0x0800C000, 0x08010000, 0x08020000, 0x08040000, 0x08060000,
             0x08080000, 0x080A0000, 0x080C0000, 0x080E0000, 0x08100000)
SIZES = (0x4000, 0x10000, 0x20000, 0x20000, 0x20000, 0x20000, 0x20000, 0x20000, 0x20000, 0x20000)
BLOCK_ENDS = (0x11, 0x41, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81)
BLOCK_SIZE = 1024
FIRST_BLOCK = 2
MAX_FIRMWARE = sum(SIZES)

OFFICIAL_HEADER = b'OutSecurityBin\0\0'
CODEC_DONOR_OFFSET = 0xC2C7C   # położenie kodeka w zaszyfrowanym pliku MD-9600 V26.45
CODEC_LENGTH = 0x48BB0
CODEC_TARGET = 0x6937C         # offset w obrazie = 0x0807537C (.codec_bin_section_1 w linkerze)


class FirmwareError(Exception):
    """Plik nie nadaje się do wgrania."""


def sha256_hex(data):
    return hashlib.sha256(data).hexdigest()


def is_official_firmware(fw):
    return bytes(fw[:len(OFFICIAL_HEADER)]) == OFFICIAL_HEADER


def is_donor(data):
    return sha256_hex(data) == DONOR_SHA256


def _be32(buf, off):
    return int.from_bytes(buf[off:off + 4], 'big')


def language_version(gla):
    """Wersja tabeli językowej w pliku .gla (None: to nie jest paczka językowa)."""
    return _be32(gla, 8) if bytes(gla[:8]) == b'GD77LANG' and len(gla) >= 12 else None


def _english_table(fw):
    found = fw.find(b'GD77LANG')
    while found > 0:
        if fw.find(b'English', found + 12, found + 19) == found + 12:
            return found
        found = fw.find(b'GD77LANG', found + 8)
    return -1


def firmware_language_version(fw):
    """Wersja tabeli językowej, której oczekuje firmware (z tabeli angielskiej)."""
    found = _english_table(bytes(fw))
    return _be32(fw, found + 8) if found > 0 else None


def merge_language(fw, gla):
    """Kopiuje .gla na miejsce drugiego języka w firmware (fw: bytearray, zmieniany w miejscu).

    Zwraca 'merged', 'ignored' (firmware bez języków, np. JA), 'mismatch', 'badfile' lub 'noslot'.
    """
    if fw.find(b'IGNRLANG') >= 0:
        return 'ignored'
    lang_version = language_version(gla)
    if lang_version is None:
        return 'badfile'
    found = _english_table(fw)
    if found <= 0:
        return 'noslot'
    if _be32(fw, found + 8) != lang_version:
        return 'mismatch'
    slot = fw.find(b'GD77LANG', found + 12)
    if slot <= 0 or slot + len(gla) > len(fw):
        return 'noslot'
    fw[slot:slot + len(gla)] = gla
    return 'merged'


def patch_codec(fw, donor):
    """Wkleja kodek AMBE z (zaszyfrowanego) firmware MD-9600 V26.45 do otwartego firmware."""
    if len(donor) < CODEC_DONOR_OFFSET + CODEC_LENGTH or len(fw) < CODEC_TARGET + CODEC_LENGTH:
        return False
    shift = CODEC_TARGET % 1024
    for j in range(CODEC_LENGTH):
        fw[CODEC_TARGET + j] = donor[CODEC_DONOR_OFFSET + j] ^ MD9600_CIPHER[(j + shift) % 1024]
    return True


def encrypt(fw, platform):
    try:
        cipher = PLATFORMS[platform]
    except KeyError:
        raise FirmwareError(f'nieznany model: {platform}') from None
    # XOR całymi kilobajtami: szyfr ma 1024 bajty
    key = (cipher * (len(fw) // 1024 + 1))[:len(fw)]
    return bytearray((int.from_bytes(fw, 'little') ^ int.from_bytes(key, 'little')).to_bytes(len(fw), 'little'))


@dataclass
class Image:
    data: bytearray
    platform: str
    dmr: bool = False
    language: str = None          # wynik merge_language() albo None (bez pliku .gla)
    notes: list = field(default_factory=list)


def build_image(firmware, platform=DEFAULT_PLATFORM, language=None, donor=None):
    """Buduje zaszyfrowany obraz. firmware / language / donor: bytes (language i donor opcjonalne)."""
    if platform not in PLATFORMS:
        raise FirmwareError(f'nieznany model: {platform}')
    if len(firmware) == 0:
        raise FirmwareError('plik firmware jest pusty')
    if len(firmware) > MAX_FIRMWARE:
        raise FirmwareError(f'plik firmware jest za duży ({len(firmware)} B, maks. {MAX_FIRMWARE} B)')
    if is_official_firmware(firmware):
        raise FirmwareError('to jest oryginalny firmware producenta (dawca kodeka), nie OpenGD77')
    fw = bytearray(firmware)
    lang = merge_language(fw, language) if language is not None else None
    dmr = patch_codec(fw, donor) if donor is not None else False
    return Image(data=encrypt(fw, platform), platform=platform, dmr=dmr, language=lang)


@dataclass
class Sector:
    address: int
    size: int
    blocks: list   # [(numer bloku, 1024 bajty)]


def write_plan(image):
    """Plan zapisu DFU dokładnie jak w loaderze Python: w każdym sektorze bloki od numeru 2,
    po 1024 bajty, ostatni dopełniony 0xFF. Sektory bez danych mają pustą listę bloków
    (loader i tak ustawia ich adres)."""
    sectors = []
    offset = 0
    for address, size, last in zip(ADDRESSES, SIZES, BLOCK_ENDS):
        blocks = []
        written = 0
        number = FIRST_BLOCK
        while offset < len(image) and written < size:
            if number > last:
                raise FirmwareError('przepełnienie numeru bloku')
            chunk = bytes(image[offset:offset + BLOCK_SIZE])
            blocks.append((number, chunk + b'\xff' * (BLOCK_SIZE - len(chunk))))
            offset += BLOCK_SIZE
            written += BLOCK_SIZE
            number += 1
        sectors.append(Sector(address, size, blocks))
    return sectors
