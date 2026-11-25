"""Wspólne dla wiersza poleceń i okna: wczytanie plików, budowa obrazu, opis po polsku."""
from dataclasses import dataclass, field
from pathlib import Path

from . import core


@dataclass
class Prepared:
    image: core.Image
    firmware: Path
    language: Path = None
    donor: Path = None
    info: list = field(default_factory=list)       # co zostanie wgrane
    warnings: list = field(default_factory=list)   # na co uważać


LANG_MESSAGES = {
    'ignored': 'firmware nie ma tabel językowych (IGNRLANG), plik .gla pominięty',
    'badfile': 'to nie jest plik języka .gla (brak nagłówka GD77LANG), pominięty',
    'noslot': 'w firmware nie ma miejsca na drugi język, plik .gla pominięty',
}


def read_file(path, what):
    try:
        return Path(path).read_bytes()
    except OSError as e:
        raise core.FirmwareError(f'nie mogę odczytać pliku {what} {path}: {e.strerror}') from None


def prepare(firmware, language=None, donor=None, donor_data=None, platform=core.DEFAULT_PLATFORM):
    """firmware, language: ścieżki (language opcjonalny); donor/donor_data: ścieżka i zawartość
    sprawdzonego pliku dawcy albo None (wtedy tylko FM)."""
    firmware = Path(firmware)
    if firmware.suffix.lower() != '.bin':
        raise core.FirmwareError(f'plik firmware musi mieć rozszerzenie .bin, a ma „{firmware.suffix}”')
    fw = read_file(firmware, 'firmware')
    gla = read_file(language, 'języka') if language else None

    image = core.build_image(fw, platform=platform, language=gla, donor=donor_data)
    p = Prepared(image=image, firmware=firmware, language=Path(language) if language else None,
                 donor=Path(donor) if donor else None)

    p.info.append(f'model: {platform}')
    p.info.append(f'firmware: {firmware.name} ({len(fw)} B, SHA-256 {core.sha256_hex(fw)[:16]}…)')
    fw_lang = core.firmware_language_version(fw)
    if gla is None:
        p.info.append('drugi język: brak (tylko angielski)')
    elif image.language == 'merged':
        p.info.append(f'drugi język: {Path(language).stem}')
    elif image.language == 'mismatch':
        p.warnings.append(f'plik {Path(language).name} (wersja 0x{core.language_version(gla):X}) nie pasuje do '
                          f'firmware (wersja 0x{fw_lang:X}), drugi język pominięty')
    else:
        p.warnings.append(LANG_MESSAGES[image.language])

    if image.dmr:
        p.info.append(f'DMR: tak, kodek AMBE z {Path(donor).name}')
    elif donor_data is not None:
        p.warnings.append('firmware jest za krótki na kodek pod 0x0807537C: DMR nie będzie działać')
    else:
        p.warnings.append('brak pliku dawcy (MD-9600 V26.45): radio będzie działać tylko na FM')

    blocks = sum(len(s.blocks) for s in core.write_plan(image.data))
    p.info.append(f'zapis: {blocks} bloków po 1 KB')
    return p
