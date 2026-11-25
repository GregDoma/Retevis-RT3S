#!/usr/bin/env python3
"""Generuje rt3s_flasher/ciphers.py z tablic szyfrów loadera OpenGD77
(MDUV380_firmware/tools/opengd77_stm32_firmware_loader.py). Tablice są tylko odczytywane
(ast), skrypt loadera nie jest uruchamiany.

    python3 scripts/gen_ciphers.py > rt3s_flasher/ciphers.py
"""
import ast
import base64
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
LOADER = HERE.parents[2] / 'MDUV380_firmware' / 'tools' / 'opengd77_stm32_firmware_loader.py'
NAMES = ['MD9600_ENCODE_CIPHER', 'MDUV380_ENCODE_CIPHER', 'MD380_ENCODE_CIPHER', 'DM1701_ENCODE_CIPHER']


def loader_tables(path=LOADER):
    tree = ast.parse(Path(path).read_text(encoding='utf-8'))
    found = {}
    for node in tree.body:
        if isinstance(node, ast.AnnAssign) and isinstance(node.target, ast.Name):
            name, value = node.target.id, node.value
        elif isinstance(node, ast.Assign) and len(node.targets) == 1 and isinstance(node.targets[0], ast.Name):
            name, value = node.targets[0].id, node.value
        else:
            continue
        if name in NAMES or name == 'FW2645_SHA256_Checksum':
            found[name] = ast.literal_eval(value)
    missing = [n for n in NAMES + ['FW2645_SHA256_Checksum'] if n not in found]
    if missing:
        raise SystemExit(f'brak w loaderze: {", ".join(missing)}')
    for n in NAMES:
        if len(found[n]) != 1024:
            raise SystemExit(f'{n}: {len(found[n])} bajtów zamiast 1024')
    return found


def main():
    t = loader_tables()
    out = ['# Wygenerowane przez scripts/gen_ciphers.py z MDUV380_firmware/tools/opengd77_stm32_firmware_loader.py',
           '# (OpenGD77 STM32 FW loader, F1RMB / VK3KYY): szyfry XOR firmware, po 1024 bajty. Nie edytować ręcznie.',
           'import base64', '']
    for n in NAMES:
        b64 = base64.b64encode(bytes(t[n])).decode('ascii')
        out.append(f"{n.replace('_ENCODE', '')} = base64.b64decode(")
        for i in range(0, len(b64), 96):
            out.append(f"    '{b64[i:i + 96]}'")
        out.append(')')
    out += ['', '# SHA-256 oryginalnego firmware MD-9600 V26.45 (dawca kodeka AMBE)',
            f"DONOR_SHA256 = '{t['FW2645_SHA256_Checksum']}'", '']
    sys.stdout.write('\n'.join(out))


if __name__ == '__main__':
    main()
