#!/usr/bin/env python3
# Renders the EMCOM style GPS pages (status, signal, sky plot) for the README, following
# the em*() drawing functions of menuGPS.c. The satellites are an example sky.
#
#   python3 docs/screens/render_gps.py   (writes docs/screens/gps_*.png)
import math
import os
import sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import render_screens as R
import render_sat as S

P = R.DARK
DEG = '\xb0'
STRIP_Y, STRIP_H, KEY_Y = 14, 11, 128 - 11
TOP = STRIP_Y + STRIP_H          # 25, first free row
GPS_C, BD_C = 0x5AC8FA, 0xFF3B30  # THEME_ITEM_FG_GPS_COLOUR / _BD_COLOUR (EMCOM preset)

# (PRN, elevation, azimuth, SNR dB-Hz, 0 / -1 = no signal)
GPS = [(2, 62, 148, 41), (5, 12, 40, 28), (12, 32, 18, 36), (13, 8, 300, 0), (15, 45, 250, 38),
       (20, 33, 129, 30), (24, 70, 80, 45), (25, 61, 339, 44), (29, 63, 203, 42), (26, 7, 220, -1)]
BD = [(1, 47, 140, 35), (3, 34, 313, 31), (4, 44, 22, 26), (6, 20, 190, 22), (8, 55, 260, 0),
      (13, 65, 330, 33)]


def btext(s, x, y, t, font, c, right=False):
    # displayPrintCore(): the character cells are painted in the background colour
    fw = 6 if font in (1, '1b') else 8
    fh = 16 if font == 3 else 8
    if right:
        x -= len(t) * fw
    s.fill(x, y, len(t) * fw, fh, P['bg'])
    return s.text(x, y, t, font, c)


def heard(lst):
    return sum(1 for x in lst if x[3] > 0)


# --- status strip: legend symbol + "GPS 8/10", "BD 5/6" (heard / in view) ------------------------
def sym(s, x, y, kind, c):
    # 7 x 7 symbols, as emSymbol() in menuGPS.c
    if kind == 'bar':            # GPS bar: solid
        s.fill(x, y, 7, 7, c)
    elif kind == 'hatch':        # BeiDou bar: outline + horizontal hatch
        s.rect(x, y, 7, 7, c)
        for j in (2, 4):
            s.fill(x, y + j, 7, 1, c)
    elif kind == 'dot':          # GPS on the sky plot: disc
        disc(s, x + 3, y + 3, 3, c, True)
    elif kind == 'square':       # BeiDou on the sky plot: square
        s.fill(x, y, 7, 7, c)


def disc(s, cx, cy, r, c, filled):
    for j in range(-r, r + 1):
        for i in range(-r, r + 1):
            d = i * i + j * j
            if (filled and d <= r * r + r) or ((not filled) and (r * r - r) < d <= r * r + r):
                s.fill(cx + i, cy + j, 1, 1, c)


def strip(s, gsym, bsym, right):
    S.strip(s)
    x = 2
    if gsym:
        sym(s, x, STRIP_Y + 2, gsym, GPS_C); x += 9
    else:
        x += S.chip(s, x, 'SATS', P['info']) + 4
    x += s.text(x, STRIP_Y + 2, f'GPS {heard(GPS)}/{len(GPS)}', 1, P['barFg']) + 6
    if bsym:
        sym(s, x, STRIP_Y + 2, bsym, BD_C); x += 9
    s.text(x, STRIP_Y + 2, f'BD {heard(BD)}/{len(BD)}', 1, P['barFg'])
    if right:
        s.text(157, STRIP_Y + 2, right, 1, P['mut'], True)


def big_chip(s, x, y, text, c):
    w = len(text) * 8 + 6
    s.fill(x, y, w, 18, c)
    s.text(x + 3, y + 1, text, 3, P['bg'])
    return w


def kv(s, x, y, k, v, c=None, right=False):
    if right:
        w = s.text(x, y, v, 1, c or P['fg'], True)
        s.text(x - w - 4, y, k, 1, P['mut'], True)
    else:
        w = s.text(x, y, k, 1, P['mut']) + 4
        s.text(x + w, y, v, 1, c or P['fg'])


def status(fix=True):
    s = R.Screen(P['bg'])
    S.title(s, 'GPS', '1/6')
    strip(s, None, None, None)
    y = TOP + 3
    if fix:
        big_chip(s, 2, y, '3D FIX', P['rx'])
        s.text(68, y + 5, 'UTC', 1, P['mut'])
        s.text(158, y + 1, '14:32:07', 3, P['fg'], True)
    else:
        big_chip(s, 2, y, 'NO FIX', P['tx'])
        s.text(158 - 32 - 4, y + 5, 'SEARCH', 1, P['mut'], True)
        s.text(158, y + 1, '3:12', 3, P['scan'], True)
    y += 21
    s.text(2, y + 5, 'LAT', 1, P['mut']); s.text(106, y + 1, 'LOC', 1, P['mut']); s.text(106, y + 21, 'ALT', 1, P['mut'])
    s.text(2, y + 21, 'LON', 1, P['mut'])
    if fix:
        s.text(22, y, '52.23456N', 3, P['fg']); s.text(22, y + 16, '21.01234E', 3, P['fg'])
        s.text(110, y + 10, 'KO02MF', 2, P['info']); s.text(158, y + 21, '112m', 1, P['fg'], True)
    else:
        s.text(22, y, '--', 3, P['mut']); s.text(22, y + 16, '--', 3, P['mut'])
        s.text(110, y + 10, '--', 2, P['mut']); s.text(158, y + 21, '--', 1, P['mut'], True)
    y += 35
    s.fill(0, y, 160, 1, P['mut'])
    y += 3
    kv(s, 2, y, 'HDOP', '0.9 GOOD' if fix else '-- ', P['rx'] if fix else P['mut'])
    kv(s, 158, y, 'SPD', '12km/h' if fix else '--', right=True)
    y += 10
    kv(s, 2, y, 'CRS', '245 WSW' if fix else '--')
    if fix:
        kv(s, 158, y, 'TTFF', '42s', right=True)
    y += 10
    kv(s, 2, y, 'DATE', '2026-11-22')
    S.keybar(s, 'UP/DN PAGE  SK2+RED OFF')
    return s


def gps_off():
    s = R.Screen(P['bg'])
    S.title(s, 'GPS', '1/6')
    S.strip(s)
    y = TOP + 3
    big_chip(s, 2, y, 'GPS OFF', P['mut'])
    s.text(4, y + 30, 'The receiver is off.', 1, P['fg'])
    s.text(4, y + 42, 'SK2+GREEN turns it on', 1, P['mut'])
    s.text(4, y + 52, '(or Options > General).', 1, P['mut'])
    S.keybar(s, 'SK2+GRN GPS ON  RED EXIT')
    return s


# --- signal page --------------------------------------------------------------------------------
CH_X, CH_TOP, CH_BASE = 14, TOP + 10, 98   # bars area x 14..158, 0 dB-Hz at y 98


def snr_y(db):
    return CH_BASE - (min(50, db) * (CH_BASE - CH_TOP)) // 50


def signal(empty=False):
    s = R.Screen(P['bg'])
    S.title(s, 'GPS', '2/6')
    strip(s, 'bar', 'hatch', 'dBHz')
    S.keybar(s, 'UP/DN PAGE  RED EXIT')
    if empty:
        s.text(80 - 4 * 17, 50, 'No satellites yet', 3, P['fg'])
        s.text(80 - 3 * 25, 72, 'Searching the sky: 3:12', 1, P['scan'])
        s.text(80 - 3 * 24, 84, 'Open sky helps (1-5 min)', 1, P['mut'])
        return s
    for db in (10, 20, 30, 40, 50):
        y = snr_y(db)
        for x in range(CH_X, 158, 2 if db == 30 else 4):
            s.fill(x, y, 1, 1, P['mut'])
        if db in (20, 40):
            btext(s, 1, y - 3, str(db), 1, P['mut'])
    s.fill(CH_X - 1, CH_BASE, 160 - CH_X, 1, P['mut'])
    sats = [(snr, prn, 'G') for prn, el, az, snr in GPS] + [(snr, prn, 'B') for prn, el, az, snr in BD]
    sats.sort(key=lambda t: -t[0])
    n = len(sats)
    pitch = min(14, (158 - CH_X) // n)
    bw = pitch - (2 if pitch >= 8 else 1)
    label_w = 6 * len(str(max(t[1] for t in sats)))
    one_row = pitch > label_w
    for i, (snr, prn, k) in enumerate(sats):
        x = CH_X + 1 + i * pitch
        c = GPS_C if k == 'G' else BD_C
        if snr > 0:
            y = snr_y(snr)
            if k == 'G':
                s.fill(x, y, bw, CH_BASE - y, c)
            else:
                s.rect(x, y, bw, CH_BASE - y, c)
                for yy in range(y + 2, CH_BASE, 2):
                    s.fill(x, yy, bw, 1, c)
            if one_row and pitch >= 13:
                btext(s, x + bw // 2 - 6, y - 9, f'{snr}', 1, P['fg'])
        else:
            s.rect(x, CH_BASE - 3, bw, 3, P['mut'])
        lab = f'{prn}'
        lx = x + bw // 2 - len(lab) * 3
        row = 0 if one_row else (i & 1)
        btext(s, max(0, lx), CH_BASE + 2 + row * 8, lab, 1, c if snr > 0 else P['mut'])
    return s


# --- sky plot -----------------------------------------------------------------------------------
CX, R_, CY = 50, 44, (TOP + KEY_Y) // 2


def sky():
    s = R.Screen(P['bg'])
    S.title(s, 'GPS', '3/6')
    strip(s, 'dot', 'square', 'N UP')
    S.keybar(s, 'UP/DN PAGE  RED EXIT')
    for r in (R_, R_ * 2 // 3, R_ // 3):
        S.circle(s, CX, CY, r, P['mut'])
    s.fill(CX, CY - R_, 1, 2 * R_ + 1, P['mut'])
    s.fill(CX - R_, CY, 2 * R_ + 1, 1, P['mut'])
    # cardinals just inside the horizon ring
    btext(s, CX - 2, CY - R_ + 2, 'N', '1b', P['fg'])
    btext(s, CX - 2, CY + R_ - 9, 'S', 1, P['mut'])
    btext(s, CX + R_ - 7, CY - 3, 'E', 1, P['mut'])
    btext(s, CX - R_ + 3, CY - 3, 'W', 1, P['mut'])
    boxes = []
    pts = []
    for lst, k in ((GPS, 'G'), (BD, 'B')):
        for prn, el, az, snr in lst:
            if el < 0:
                continue
            f = (1 - el / 90) * R_
            x = round(CX + math.sin(math.radians(az)) * f)
            y = round(CY - math.cos(math.radians(az)) * f)
            pts.append((snr, prn, k, x, y))
    pts.sort(key=lambda t: -t[0])  # strongest get the labels first
    for snr, prn, k, x, y in reversed(pts):
        c = GPS_C if k == 'G' else BD_C
        if k == 'G':
            if snr > 0:
                disc(s, x, y, 3, c, True)
            else:
                disc(s, x, y, 3, c, False)
        else:
            if snr > 0:
                s.fill(x - 3, y - 3, 7, 7, c)
            else:
                s.rect(x - 3, y - 3, 7, 7, c)
    marks = [(x - 3, y - 3, 7, 7) for _, _, _, x, y in pts]
    # ring elevations on the SE diagonal, else the SW one, where no satellite is
    for el, r in ((30, R_ * 2 // 3), (60, R_ // 3)):
        d = r * 707 // 1000
        for lx in (CX + d + 1, CX - d - 12):
            box = (lx, CY + d - 1, 12, 8)
            if not any(not (box[0] + box[2] <= b[0] or b[0] + b[2] <= box[0] or box[1] + box[3] <= b[1] or b[1] + b[3] <= box[1]) for b in marks):
                btext(s, lx, CY + d - 1, f'{el}', 1, P['mut'])
                boxes.append(box)
                break
    for snr, prn, k, x, y in pts:
        if snr <= 0:
            continue
        lab = f'{prn}'
        w = len(lab) * 6
        for lx in (x + 5, x - 4 - w):
            box = (lx, y - 4, w, 8)
            if lx < 0 or lx + w > 98:
                continue
            if any(not (box[0] + box[2] <= b[0] or b[0] + b[2] <= box[0] or box[1] + box[3] <= b[1] or b[1] + b[3] <= box[1])
                   for b in boxes + [m for m in marks if m != (x - 3, y - 3, 7, 7)]):
                continue
            btext(s, lx, y - 4, lab, 1, P['fg'])
            boxes.append(box)
            break
    # right column
    x0 = 102
    S_chip_y = TOP + 3
    w = len('3D FIX') * 6 + 4
    s.fill(x0, S_chip_y, w, 10, P['rx']); s.text(x0 + 2, S_chip_y + 1, '3D FIX', '1b', P['bg'])
    s.text(x0, TOP + 17, 'HDOP', 1, P['mut'])
    s.text(x0, TOP + 26, '0.9', 3, P['rx'])
    s.text(x0 + 28, TOP + 31, 'GOOD', 1, P['rx'])
    y = TOP + 50
    disc(s, x0 + 3, y + 3, 3, P['fg'], True); s.text(x0 + 10, y, 'signal', 1, P['fg'])
    disc(s, x0 + 3, y + 13, 3, P['fg'], False); s.text(x0 + 10, y + 10, 'no sig', 1, P['mut'])
    s.text(x0, y + 24, 'rings', 1, P['mut'])
    s.text(x0, y + 33, f'0/30/60{DEG}', 1, P['mut'])
    return s


if __name__ == '__main__':
    status().png(os.path.join(HERE, 'gps_status.png'))
    signal().png(os.path.join(HERE, 'gps_signal.png'))
    sky().png(os.path.join(HERE, 'gps_sky.png'))
    if '--all' in sys.argv:
        status(False).png(os.path.join(HERE, 'gps_status_nofix.png'))
        signal(True).png(os.path.join(HERE, 'gps_signal_empty.png'))
        gps_off().png(os.path.join(HERE, 'gps_off.png'))
    print('ok')
