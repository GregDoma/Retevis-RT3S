#!/usr/bin/env python3
# Renders the EMCOM style satellite screens for the README, following the em*() drawing
# functions of menuSatelliteScreen.c and the Location page key bar of menuRadioInfos.c.
# The data are examples.
#
#   python3 docs/screens/render_sat.py   (writes docs/screens/sat_*.png)
import math
import os
import sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import render_screens as R

P = R.DARK
DEG = '\xb0'
STRIP_Y, STRIP_H, KEY_Y = 14, 11, 128 - 11
LIST_Y, ROW_H = STRIP_Y + STRIP_H + 2, 14
PCX, PCY, PR = 100, (STRIP_Y + STRIP_H + KEY_Y) // 2, 40


def title(s, text, pos):
    # menuDisplayTitle(), EMCOM style
    s.fill(0, 0, 160, 14, P['bar']); s.fill(0, 0, 3, 13, P['rx']); s.fill(0, 13, 160, 1, P['mut'])
    s.text(7, 3, text.upper(), 2, P['barFg'])
    if pos:
        s.text(157, 4, pos, 1, P['mut'], True)


def strip(s):
    s.fill(0, STRIP_Y, 160, STRIP_H, P['bar'])


def chip(s, x, text, c):
    text = text.upper()
    w = len(text) * 6 + 4
    s.fill(x, STRIP_Y + 1, w, 9, c)
    s.text(x + 2, STRIP_Y + 1, text, '1b', P['bg'])
    return w


def keybar(s, text):
    s.fill(0, KEY_Y, 160, 11, P['bar'])
    s.text(3, KEY_Y + 2, text, 1, P['barFg'])


def sat_list():
    s = R.Screen(P['bg'])
    title(s, 'Satellite', '1/4')
    strip(s)
    x = 2 + chip(s, 2, 'GPS', P['rx']) + 4
    s.text(x, STRIP_Y + 2, '52.23N 21.01E', 1, P['barFg'])
    s.text(157, STRIP_Y + 2, 'UTC', 1, P['mut'], True)
    rows = [('ISS', None, 58, True), ('SO-50', '13:05', 22, False), ('AO-91', '14:37', 71, False),
            ('RS-44', '15:10', 34, False), ('ISS', '17:52', 41, False), ('TEVEL-12', '18:20', 15, False)]
    for i, (name, t, el, vis) in enumerate(rows):
        y = LIST_Y + i * ROW_H
        sel = i == 0
        if sel:
            s.fill(0, y, 160, ROW_H - 1, P['rx'])
        if sel or vis:
            s.fill(0, y + 3, 2, ROW_H - 7, P['bg'] if sel else P['rx'])
        s.text(6, y + 3, name[:8], 2, P['bg'] if sel else P['fg'])
        s.text(78, y + 3, 'NOW' if vis else t, 2, P['bg'] if sel else (P['rx'] if vis else P['info']))
        s.text(157, y + 3, f'{el}{DEG}', 2, P['bg'] if sel else P['fg'], True)
    keybar(s, 'GREEN OPEN * POS RED EXIT')
    return s


def freq_line(s, y, label, f, c):
    s.text(4, y + 5, label, 1, P['mut'])
    s.text(20, y, f, 3, c)


def sat_selected():
    s = R.Screen(P['bg'])
    title(s, 'ISS', '2/4')
    strip(s)
    x = 2 + chip(s, 2, 'Voice', P['info']) + 4
    s.text(x, STRIP_Y + 2, 'DOPPLER', 1, P['barFg'])
    s.text(157, STRIP_Y + 2, '5W', 1, P['barFg'], True)
    y = STRIP_Y + STRIP_H + 5
    s.text(4, y + 5, 'Az', 1, P['mut']); s.text(20, y, f'214{DEG}', 3, P['fg'])
    s.text(84, y + 5, 'El', 1, P['mut']); s.text(100, y, f'38{DEG}', 3, P['rx'])
    y += 23
    freq_line(s, y, 'RX', '437.80345', P['info'])
    y += 19
    freq_line(s, y, 'TX', '145.99000', P['tx'])
    y += 23
    s.fill(0, y - 3, 160, 1, P['mut'])
    s.text(4, y, 'LOS in 5:40', 1, P['rx'])
    s.text(157, y, f'Max 58{DEG}', 1, P['fg'], True)
    s.text(4, y + 10, 'RSSI -103dBm', 1, P['mut'])
    keybar(s, '1-3 QRG  9 SSTV  RED LIST')
    return s


def circle(s, cx, cy, r, c):
    for k in range(360):
        a = math.radians(k)
        s.fill(round(cx + r * math.sin(a)), round(cy - r * math.cos(a)), 1, 1, c)


def sat_polar():
    s = R.Screen(P['bg'])
    title(s, 'SO-50', '3/4')
    strip(s)
    chip(s, 2, 'Pass 1/7', P['info'])
    s.text(157, STRIP_Y + 2, '-121dBm', 1, P['mut'], True)
    keybar(s, 'UP/DN VIEW # NEXT RED LIST')
    for r in (PR, PR // 3 * 2, PR // 3):
        circle(s, PCX, PCY, r, P['mut'])
    s.fill(PCX, PCY - PR - 3, 1, (PR + 3) * 2, P['mut'])
    s.fill(PCX - PR - 3, PCY, (PR + 3) * 2, 1, P['mut'])
    s.text(3, STRIP_Y + STRIP_H + 6, 'AOS in', 1, P['mut'])
    s.text(3, STRIP_Y + STRIP_H + 15, '3:12', 3, P['scan'])
    s.text(3, STRIP_Y + STRIP_H + 36, 'Max', 1, P['mut'])
    s.text(3, STRIP_Y + STRIP_H + 45, f'22{DEG}', 3, P['rx'])
    pts = []
    for k in range(21):
        az = math.radians(200 - k * 7.5)
        el = 22 * math.sin(math.pi * k / 20)
        f = (1 - el / 90) * PR
        pts.append((round(PCX + math.sin(az) * f), round(PCY - math.cos(az) * f)))
    for a, b in zip(pts, pts[1:]):
        s.line(a[0], a[1], b[0], b[1], P['fg'])
    s.fill(pts[0][0] - 3, pts[0][1] - 3, 7, 7, P['rx'])
    h = KEY_Y - (STRIP_Y + STRIP_H) - 2
    s.fill(154, STRIP_Y + STRIP_H + 1, 5, h, P['bar'])
    s.fill(154, STRIP_Y + STRIP_H + 1 + h - 12, 5, 12, P['rx'])
    return s


def location():
    s = R.Screen(P['bg'])
    title(s, 'Location', '4/6')
    entry = '52.2___N ___.____E'  # typing: 3 of the 13 digits
    s.text((160 - len(entry) * 8) // 2, 42, entry, 3, P['fg'])
    lines = ['UP N  DN S  SK2+UP/DN E/W', 'GREEN SAVE RED CLEAR < DEL']
    y = 128 - 2 * 10 - 1
    s.fill(0, y, 160, 128 - y, P['bar'])
    for i, l in enumerate(lines):
        s.text(2, y + 2 + i * 10, l, 1, P['barFg'])
    return s


if __name__ == '__main__':
    for name, fn in (('list', sat_list), ('selected', sat_selected), ('polar', sat_polar), ('location', location)):
        fn().png(os.path.join(HERE, f'sat_{name}.png'))
