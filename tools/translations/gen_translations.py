#!/usr/bin/env python3
"""Fork (M0SQQ): generates the Polish translation table of the fork screens.

Input : tools/translations/pl.tsv (UTF-8), one entry per line: ENGLISH<TAB>POLISH
        Both texts are written exactly as C string literal contents (\\n, \\", \\\\ escapes),
        spaces are significant. Lines starting with '//' and empty lines are ignored.
Output: MDUV380_firmware/application/source/user_interface/uiTranslationsPL.c

Checks: duplicate English keys, printf conversions (same specifiers in the same order),
characters the radio font cannot show, every TR("...")/TRN("...") literal in the sources has a
translation (--check also lists translations that are no longer used).

Usage: python3 tools/translations/gen_translations.py [--check]
"""
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
TSV = os.path.join(ROOT, 'tools', 'translations', 'pl.tsv')
APP = os.path.join(ROOT, 'MDUV380_firmware', 'application')
OUT = os.path.join(APP, 'source', 'user_interface', 'uiTranslationsPL.c')

# Polish letters remapped in the firmware font (HX8353E_charset.h), plus Latin-1 for the rest
REMAP = {'Ć': 0x81, 'ć': 0x82, 'Ą': 0x83, 'ą': 0x84, 'Ę': 0x85, 'ę': 0x86, 'Ł': 0x87, 'ł': 0x88,
         'Ń': 0x89, 'ń': 0x8B, 'Ś': 0x8C, 'ś': 0x8D, 'Ź': 0x8F, 'ź': 0x90, 'Ż': 0x91, 'ż': 0x92,
         'Ó': 0xD3, 'ó': 0xF3, '°': 0xB0}

FMT = re.compile(r'%[-+ #0]*(\*|\d+)?(\.(\*|\d+))?(hh|h|ll|l|z|j|t|L)?[diouxXeEfgGcspn%]')


def c_unescape(s):
    out = bytearray()
    i = 0
    while i < len(s):
        ch = s[i]
        if ch == '\\':
            i += 1
            e = s[i]
            if e in 'n':
                out.append(10)
            elif e == 't':
                out.append(9)
            elif e in '"\\\'':
                out.append(ord(e))
            elif e in '01234567':
                j = i
                while j < len(s) and j < i + 3 and s[j] in '01234567':
                    j += 1
                out.append(int(s[i:j], 8))
                i = j - 1
            else:
                raise ValueError('unsupported escape \\' + e)
        else:
            if ch in REMAP:
                out.append(REMAP[ch])
            elif ord(ch) < 128:
                out.append(ord(ch))
            else:
                raise ValueError('character not in the radio font: %r' % ch)
        i += 1
    return bytes(out)


def c_escape(b):
    out = ''
    prev_octal = False
    for x in b:
        if x == 10:
            out += '\\n'
        elif x == 34:
            out += '\\"'
        elif x == 92:
            out += '\\\\'
        elif 32 <= x < 127:
            # a digit right after an octal escape would extend it
            out += ('""' if (prev_octal and chr(x) in '01234567') else '') + chr(x)
        else:
            out += '\\%03o' % x
            prev_octal = True
            continue
        prev_octal = False
    return out


def load():
    entries = {}
    errors = []
    with open(TSV, encoding='utf-8') as f:
        for n, line in enumerate(f, 1):
            line = line.rstrip('\n')
            if not line or line.startswith('//'):
                continue
            if line.count('\t') != 1:
                errors.append('%d: expected exactly one TAB' % n)
                continue
            en, pl = line.split('\t')
            try:
                enb, plb = c_unescape(en), c_unescape(pl)
            except ValueError as e:
                errors.append('%d: %s' % (n, e))
                continue
            if enb in entries:
                if entries[enb][1] != plb:
                    errors.append('%d: duplicate English text with another translation: %s' % (n, en))
                continue
            fe = [m.group(0) for m in FMT.finditer(en)]
            fp = [m.group(0) for m in FMT.finditer(pl)]
            if fe != fp:
                errors.append('%d: printf conversions differ: %s / %s' % (n, fe, fp))
            entries[enb] = (en, plb, n)
    return entries, errors


def strip_comments(text):
    """Blank out C comments, keep string/char literals and line numbers."""
    out = []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c in '"\'':
            j = i + 1
            while j < n and text[j] != c:
                j += 2 if text[j] == '\\' else 1
            out.append(text[i:j + 1])
            i = j + 1
        elif text.startswith('//', i):
            j = text.find('\n', i)
            i = n if j < 0 else j
        elif text.startswith('/*', i):
            j = text.find('*/', i + 2)
            j = n if j < 0 else j + 2
            out.append('\n' * text.count('\n', i, j))
            i = j
        else:
            out.append(c)
            i += 1
    return ''.join(out)


def source_keys():
    # TR("...") translates at the call site, TRN("...") only marks a literal
    # (e.g. in a static array) that is translated later with TR(array[i]).
    keys = {}
    lit = re.compile(r'\bTRN?\(\s*"((?:[^"\\]|\\.)*)"\s*\)')
    for d, _, files in os.walk(APP):
        for fn in files:
            if fn.endswith(('.c', '.h')) and fn != 'uiTranslationsPL.c':
                p = os.path.join(d, fn)
                with open(p, encoding='latin-1') as f:
                    text = strip_comments(f.read())
                for m in lit.finditer(text):
                    n = text.count('\n', 0, m.start()) + 1
                    keys.setdefault(c_unescape(m.group(1)), '%s:%d' % (os.path.relpath(p, APP), n))
    return keys


def main():
    entries, errors = load()
    keys = source_keys()
    missing = [(v, k) for k, v in keys.items() if k not in entries]
    for where, k in sorted(missing):
        errors.append('no translation for TR("%s") at %s' % (c_escape(k), where))
    if errors:
        print('\n'.join(errors), file=sys.stderr)
        sys.exit(1)
    if '--check' in sys.argv:
        unused = [v[0] for k, v in entries.items() if k not in keys]
        for u in unused:
            print('unused translation: %s' % u)
        print('%d translations, %d TR() texts in the sources' % (len(entries), len(keys)))
        return
    order = sorted(entries.keys())  # bytes order == strcmp() order
    lines = [
        '/*',
        ' * GENERATED FILE, DO NOT EDIT: tools/translations/gen_translations.py from tools/translations/pl.tsv',
        ' *',
        ' * Fork (M0SQQ): Polish texts of the fork screens, see TR() in uiLocalisation.h.',
        ' * Sorted by strcmp() of the English text (binary search). Kept after the codec (.upper_text).',
        ' *',
        ' * Use of this source code or binary releases for commercial purposes is',
        ' * strictly forbidden (see license.txt).',
        ' */',
        "#include <stdint.h>",
        "#include <stddef.h>",
        "#include \"user_interface/uiLocalisation.h\"",
        '',
        '#if defined(PLATFORM_MDUV380) || defined(PLATFORM_MD380) || defined(PLATFORM_RT84_DM1701) || defined(PLATFORM_MD2017)',
        '#define TR_SECTION __attribute__((section(".upper_text")))',
        '#else',
        '#define TR_SECTION',
        '#endif',
        '',
    ]
    for i, k in enumerate(order):
        en, plb, _ = entries[k]
        lines.append('static const char en%d[] TR_SECTION = "%s";' % (i, en))
        lines.append('static const char pl%d[] TR_SECTION = "%s";' % (i, c_escape(plb)))
    lines.append('')
    lines.append('const uiTranslation_t uiTranslationsPL[] TR_SECTION =')
    lines.append('{')
    for i in range(len(order)):
        lines.append('\t{ en%d, pl%d },' % (i, i))
    lines.append('};')
    lines.append('')
    lines.append('const uint16_t uiTranslationsPLCount = %d;' % len(order))
    with open(OUT, 'w', encoding='latin-1', newline='\n') as f:
        f.write('\n'.join(lines) + '\n')
    print('%s: %d entries' % (os.path.relpath(OUT, ROOT), len(order)))


if __name__ == '__main__':
    main()
