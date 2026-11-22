#!/usr/bin/env python3
# Checks the EMCOM palettes (uiEmcom.c) and the menu presets (emcomThemePreset()):
#  - WCAG 2 contrast ratio of every text / background pair that the screens draw
#  - the same ratios after a protanopia / deuteranopia simulation (Machado, Oliveira and
#    Fernandes 2009, severity 1.0, applied in linear RGB)
#  - how far apart the RX / TX / SCAN colours stay under that simulation (CIE76 delta E)
#
#   python3 docs/screens/palette_check.py
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import render_screens as R  # noqa: E402

PROTAN = ((0.152286, 1.052583, -0.204868), (0.114503, 0.786281, 0.099216), (-0.003882, -0.048116, 1.051998))
DEUTAN = ((0.367322, 0.860646, -0.227968), (0.280085, 0.672501, 0.047413), (-0.011820, 0.042940, 0.968881))


def lin(v):
    v /= 255.0
    return v / 12.92 if v <= 0.04045 else ((v + 0.055) / 1.055) ** 2.4


def srgb(v):
    v = min(1.0, max(0.0, v))
    return round(255 * (12.92 * v if v <= 0.0031308 else 1.055 * v ** (1 / 2.4) - 0.055))


def split(c):
    return ((c >> 16) & 255, (c >> 8) & 255, c & 255)


def simulate(c, m):
    r, g, b = (lin(v) for v in split(c))
    out = [srgb(m[i][0] * r + m[i][1] * g + m[i][2] * b) for i in range(3)]
    return (out[0] << 16) | (out[1] << 8) | out[2]


def luminance(c):
    r, g, b = (lin(v) for v in split(c))
    return 0.2126 * r + 0.7152 * g + 0.0722 * b


def ratio(a, b):
    la, lb = sorted((luminance(a), luminance(b)), reverse=True)
    return (la + 0.05) / (lb + 0.05)


def lab(c):
    r, g, b = (lin(v) for v in split(c))
    x = (0.4124 * r + 0.3576 * g + 0.1805 * b) / 0.95047
    y = 0.2126 * r + 0.7152 * g + 0.0722 * b
    z = (0.0193 * r + 0.1192 * g + 0.9505 * b) / 1.08883
    f = [t ** (1 / 3) if t > 0.008856 else 7.787 * t + 16 / 116 for t in (x, y, z)]
    return (116 * f[1] - 16, 500 * (f[0] - f[1]), 200 * (f[1] - f[2]))


def delta_e(a, b):
    return sum((p - q) ** 2 for p, q in zip(lab(a), lab(b))) ** 0.5


def main_pairs(P):
    # (name, text, background) as drawn by uiEmcom.c
    return [('text', P['fg'], P['bg']), ('labels', P['mut'], P['bg']), ('freq', P['info'], P['bg']),
            ('RX text', P['rx'], P['bg']), ('TX text', P['tx'], P['bg']), ('status', P['barFg'], P['bar']),
            ('RX chip', P['bar'], P['rx']), ('TX bar', P['txFg'], P['tx']), ('SCAN chip', P['bar'], P['scan'])]


def alert_pairs(P):
    # UI refresh stage 1: card title strips (kind word in the background colour), TX banner, TOT chip
    import render_refresh
    w = render_refresh.warn(P)
    return [('info card', P['bg'], P['info']), ('warn card', P['bg'], w), ('error card', P['bg'], P['tx']),
            ('ask card', P['bg'], P['fg']), ('card text', P['fg'], P['bg']), ('APRS Tx', P['txFg'], P['tx']),
            ('TOT 10 s', P['bg'], w)]


def menu_pairs(M):
    return [('menu text', M['fg'], M['bg']), ('values', M['val'], M['bg']), ('selected', M['bg'], M['sel']),
            ('title', M['name'], M['bar'])]


def report(name, pairs, states):
    print(f'\n{name}')
    print(f'  {"pair":<10} {"normal":>7} {"protan":>7} {"deutan":>7}')
    worst = 99.0
    for label, fg, bg in pairs:
        n = ratio(fg, bg)
        p = ratio(simulate(fg, PROTAN), simulate(bg, PROTAN))
        d = ratio(simulate(fg, DEUTAN), simulate(bg, DEUTAN))
        worst = min(worst, n)
        print(f'  {label:<10} {n:7.2f} {p:7.2f} {d:7.2f}')
    print(f'  lowest normal ratio: {worst:.2f}')
    if states:
        for label, m in (('normal', None), ('protan', PROTAN), ('deutan', DEUTAN)):
            sim = {k: (simulate(v, m) if m else v) for k, v in states.items()}
            de = {f'{a}/{b}': delta_e(sim[a], sim[b]) for a, b in (('rx', 'tx'), ('rx', 'scan'), ('tx', 'scan'))}
            print(f'  state colours dE ({label}): ' + ', '.join(f'{k} {v:.0f}' for k, v in de.items()))


def main():
    import render_access as A
    for name, P in (('EMCOM day', R.DARK), ('EMCOM night', R.NIGHT), ('EMCOM mono day', R.MONO),
                    ('EMCOM mono night', R.MONO_NIGHT), ('EMCOM contrast day', R.CONTRAST),
                    ('EMCOM contrast night', R.CONTRAST_NIGHT)):
        report(name + ' (main screen)', main_pairs(P), {k: P[k] for k in ('rx', 'tx', 'scan')})
        report(name + ' (cards / TX)', alert_pairs(P), None)
    for name, M in (('mono menus day', A.MENU['mono']), ('mono menus night', A.night_menu(A.MENU['mono'])),
                    ('contrast menus day', A.MENU['contrast']), ('contrast menus night', A.night_menu(A.MENU['contrast']))):
        report(name, menu_pairs(M), None)


if __name__ == '__main__':
    main()
