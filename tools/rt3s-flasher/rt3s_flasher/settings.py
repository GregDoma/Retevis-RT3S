"""Zapamiętany plik dawcy kodeka (oryginalny firmware MD-9600 V26.45).

Kopia pliku trafia do ~/.local/share/rt3s-flasher/donor.bin (tylko na tym komputerze, nigdy do
repozytorium). Jeśli kopii nie ma, używany jest plik wskazany wcześniej loaderowi OpenGD77
(~/.gd77firmwareloader.ini, SourceSTM32Firmware).
"""
import configparser
import os
import shutil
from pathlib import Path

from .core import is_donor


def data_dir():
    base = os.environ.get('XDG_DATA_HOME') or str(Path.home() / '.local' / 'share')
    return Path(base) / 'rt3s-flasher'


def donor_path():
    return data_dir() / 'donor.bin'


def legacy_donor_path():
    cfg = configparser.ConfigParser()
    try:
        cfg.read(Path.home() / '.gd77firmwareloader.ini')
        p = cfg['GLOBAL']['SourceSTM32Firmware']
    except (KeyError, configparser.Error, OSError):
        return None
    return Path(p) if p else None


def load_donor():
    """Zwraca (ścieżka, dane) poprawnego pliku dawcy albo (None, None)."""
    for p in (donor_path(), legacy_donor_path()):
        if p and p.is_file():
            data = p.read_bytes()
            if is_donor(data):
                return p, data
    return None, None


def remember_donor(path):
    """Sprawdza sumę SHA-256 i zapisuje kopię. Zwraca ścieżkę kopii; ValueError gdy plik zły."""
    data = Path(path).read_bytes()
    if not is_donor(data):
        raise ValueError('To nie jest firmware MD-9600 V26.45 (suma SHA-256 się nie zgadza).')
    target = donor_path()
    target.parent.mkdir(parents=True, exist_ok=True)
    tmp = target.with_suffix('.tmp')
    shutil.copyfile(path, tmp)
    os.chmod(tmp, 0o600)
    tmp.replace(target)
    return target


def forget_donor():
    try:
        donor_path().unlink()
        return True
    except FileNotFoundError:
        return False
