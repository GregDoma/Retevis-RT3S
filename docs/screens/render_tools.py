#!/usr/bin/env python3
# UI refresh, stage 2: mock-ups of the field tool screens in the EMCOM styles (day, night, mono,
# contrast, mono / contrast at night), following the EMCOM views of menuPACE.c, menuRadioCheck.c,
# menuEventLog.c, menuCoverage.c, menuPosition.c, menuSSTV.c and menuNetControl.c and the emcomUi* helpers of
# uiEmcom.c. Colours: the menu theme items of emcomThemePreset(). Example data.
#
#   python3 docs/screens/render_tools.py
#   (writes tools_<screen>.png sheets: one column per style)
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import render_screens as R  # noqa: E402
import render_access as A  # noqa: E402
import render_refresh as F  # noqa: E402

W, H = R.W, R.H
STRIP_Y, STRIP_H = 14, 11
BODY_Y = STRIP_Y + STRIP_H + 2
KEYBAR_H = 11
KEYBAR_Y = H - KEYBAR_H


# --- theme items (emcomThemePreset) ---------------------------------------------------------------
def night_of(T, style):
    def mono(c):
        r = ((c & 0xFF) * 210) // 255
        return (r << 16) | ((r // 5) << 8) | (r // 5)

    def contrast(k, c):
        if c == 0xFFFFFF or k == 'sel':
            return 0xFFA000
        if c == 0xFFFF00:
            return 0xFF4020
        g = c & 0xFF
        return (g << 16) | ((g // 4) << 8)
    out = {}
    for k, c in T.items():
        if not isinstance(c, int) or isinstance(c, bool):
            out[k] = c
        elif style == 'mono':
            out[k] = mono(c)
        else:
            out[k] = contrast(k, c)
    return out


THEMES = [('day', dict(F.THEME['day'])), ('night', dict(F.THEME['night'])), ('mono', dict(F.THEME['mono'])),
          ('contrast', dict(F.THEME['contrast'], large=True))]
THEMES += [('mono night', night_of(F.THEME['mono'], 'mono')),
           ('contrast night', dict(night_of(F.THEME['contrast'], 'contrast'), large=True))]
for _, t in THEMES:
    t['cues'] = t is not THEMES[0][1] and t is not THEMES[1][1]


# --- emcomUi* helpers --------------------------------------------------------------------------------
def fw(font):
    return 16 if font == 4 else (8 if font in (2, 3) else 6)


def txt(s, T, x, y, t, font, c, right=False):
    if font == 1 and T.get('large'):
        font = '1b'
    w = len(t) * fw(font)
    if right:
        x -= w
    s.text(max(0, x), y, t, font, c)
    return w


def chip(s, T, x, y, t, c, outline=False, right=False):
    w = len(t) * 6 + 4
    if right:
        x -= w
    if outline:
        s.rect(x, y - 1, w, 9, c); s.text(x + 2, y, t, '1b', c)
    else:
        s.fill(x, y - 1, w, 9, c); s.text(x + 2, y, t, '1b', T['bg'])
    return w


def kv(s, T, x, y, k, v, c):
    w = txt(s, T, x, y, k, 1, T['deco']) + 4
    return w + txt(s, T, x + w, y, v, 1, c)


def title(s, T, text, counter=None):
    F.title(s, T, text, None)
    if counter:
        txt(s, T, W - 3, 4, counter, 1, T['deco'], True)


def strip(s, T):
    s.fill(0, STRIP_Y, W, STRIP_H, T['hbg'])


def keybar(s, T, keys):
    s.fill(0, KEYBAR_Y, W, KEYBAR_H, T['hbg']); s.text(3, KEYBAR_Y + 2, keys, '1b' if T.get('large') else 1, T['hfg'])


def confirm(s, T, q):
    keybar(s, T, '')
    x = 2 + chip(s, T, 2, KEYBAR_Y + 2, q, T['red']) + 5
    s.text(x, KEYBAR_Y + 2, 'GRN YES  ANY NO' if x + 90 <= W else 'GRN YES ANY NO', '1b' if T.get('large') else 1, T['hfg'])


def divider(s, T, y):
    s.fill(0, y, W, 1, T['deco'])


def centred(s, y, t, font, c):
    s.text((W - len(t) * fw(font)) // 2, y, t, font, c)


# --- PACE ---------------------------------------------------------------------------------------------
PACE = [('P', 'green', 'SR7TM AU Tomaszo', '438.6750 FM', True),
        ('A', 'blue', 'SR7VV AU+F Gora', '439.1125 DMR', False),
        ('C', 'amber', 'SIMPLEX 145.500', '145.5000 FM', False),
        ('E', 'red', None, None, False)]
PACE_NAMES = ['PRIMARY', 'ALTERNATE', 'CONTINGENCY', 'EMERGENCY']


def pace(T, sel=1, confirm_clear=False, sked=None):
    s = R.Screen(T['bg'])
    title(s, T, 'PACE PLAN', f'{sel + 1}/4')
    rh = (KEYBAR_Y - BODY_Y + 1) // 4
    for i, (l, ck, name, freq, on) in enumerate(PACE):
        y = BODY_Y + i * rh
        c = T[ck]
        isSel = i == sel
        tc = T['bg'] if isSel else T['fg']
        mc = T['bg'] if isSel else T['deco']
        s.fill(0, y, 20, rh - 1, c)
        s.text(6, y + (rh - 1 - 16) // 2, l, 3, T['bg'])
        if isSel:
            s.fill(21, y, W - 21, rh - 1, T['sel'])
        elif i < 3:
            s.fill(21, y + rh - 1, W - 21, 1, T['deco'])
        if name:
            txt(s, T, 24, y + 3, name, '1b', tc)
            txt(s, T, 24, y + 12, freq, 1, mc)
            if 24 + (len(freq) + 1 + len(PACE_NAMES[i])) * 6 <= W - 2:
                txt(s, T, W - 2, y + 12, PACE_NAMES[i], 1, mc, True)
            if on:
                chip(s, T, W - 2, y + 3, 'ON', T['bg'] if isSel else c, outline=isSel, right=True)
        else:
            txt(s, T, 24, y + 3, PACE_NAMES[i], '1b', T['bg'] if isSel else c)
            txt(s, T, 24, y + 12, '* stores the channel', 1, mc)
            chip(s, T, W - 2, y + 3, 'EMPTY', T['bg'] if isSel else T['deco'], outline=True, right=True)
    strip(s, T)
    if sked:  # the next sked count-down (menuPACE.c skedText()), the slot name only when it fits
        rw = txt(s, T, W - 3, STRIP_Y + 2, sked, 1, T['amber'], True)
        rw += txt(s, T, W - 3 - rw - 4, STRIP_Y + 2, 'SKED', 1, T['deco'], True) + 4
    else:
        rw = txt(s, T, W - 3, STRIP_Y + 2, '3/4 SET', 1, T['deco'], True)
    x = 2 + chip(s, T, 2, STRIP_Y + 2, 'ON P', T['green']) + 4
    if x + len('PRIMARY') * 6 + 10 <= W - 3 - rw:
        txt(s, T, x, STRIP_Y + 2, 'PRIMARY', 1, T['hfg'])
    if confirm_clear:
        confirm(s, T, f'CLEAR {PACE[sel][0]}?')
    else:
        keybar(s, T, '1-4 GO * STO 0 CLR # SKED')
    return s


# --- radio check ---------------------------------------------------------------------------------------
def rating(T, rd):
    return ('OK', T['green']) if rd >= 4 else (('WEAK', T['amber']) if rd == 3 else ('POOR', T['red']))


def radio_check(T, empty=False, confirm_clear=False):
    s = R.Screen(T['bg'])
    title(s, T, 'RADIO CHECK', '0/3' if empty else '3/3')
    strip(s, T)
    rx = not empty
    x = 2 + chip(s, T, 2, STRIP_Y + 2, 'RX' if rx else 'IDLE', T['green'] if rx else T['deco'], outline=not rx) + 4
    x += txt(s, T, x, STRIP_Y + 2, 'S7' if rx else 'S0', '1b', T['hfg']) + 6
    txt(s, T, x, STRIP_Y + 2, '-105dBm' if rx else '-131dBm', 1, T['hfg'])
    if rx:
        txt(s, T, W - 3, STRIP_Y + 2, '2.3s', 1, T['green'], True)
    else:
        txt(s, T, W - 3, STRIP_Y + 2, 'FM', 1, T['deco'], True)
    y = BODY_Y
    if empty:
        centred(s, y + 14, 'LISTENING', 3, T['fg'])
        txt(s, T, 4, y + 40, 'Each received over is', 1, T['deco'])
        txt(s, T, 4, y + 50, 'rated R1-5 / S1-9.', 1, T['deco'])
    else:
        word, c = rating(T, 5)
        txt(s, T, 2, y, '5 BY 9', 4, c)
        cw = len(word) * 8 + 6
        s.fill(W - 2 - cw, y + 7, cw, 18, c); s.text(W - 2 - cw + 3, y + 8, word, 3, T['bg'])
        txt(s, T, 2, y + 34, 'FIVE BY NINE', '1b', T['fg'])
        txt(s, T, 2, y + 44, 'FM 6.4s AVG -88 PK -84', 1, T['deco'])
        y += 54
        divider(s, T, y)
        y += 3
        for tm, rd, st, dbm in (('14:31', 3, 6, -108), ('14:28', 2, 4, -118)):
            txt(s, T, 2, y, tm, 1, T['deco'])
            txt(s, T, 36, y, f'{rd} BY {st}', '1b', T['fg'])
            txt(s, T, 80, y, f'{dbm}dBm', 1, T['deco'])
            word, c = rating(T, rd)
            chip(s, T, W - 2, y, word, c, right=True)
            y += 11
    if confirm_clear:
        confirm(s, T, 'CLEAR ALL?')
    else:
        keybar(s, T, 'RED EXIT' if empty else '0 CLEAR  RED EXIT')
    return s


# --- event log -----------------------------------------------------------------------------------------
EVENTS = [('14:32', 'TX', 'red', 'CH3 RAYNET-1 12s'), ('14:31', 'RX', 'green', 'CH3 RAYNET-1 8s'),
          ('14:30', 'MSG', 'green', 'SP7ABC-9 Punkt 2 got'), ('14:28', 'APR', 'blue', 'SR7NJ 12.3km'),
          ('14:20', 'DMR', 'green', 'SP5ABC TG260'), ('14:05', 'DTM', 'amber', '*123#'),
          ('13:58', 'SST', 'blue', 'PD120 120 lines'), ('13:40', 'SOS', 'red', 'SOS on'),
          ('13:30', 'SYS', 'deco', 'Power on')]


def type_chip(s, T, x, y, name, c, sel):
    if sel:
        s.rect(x, y - 1, 22, 9, T['bg']); s.text(x + 2, y, name, '1b', T['bg'])
    else:
        s.fill(x, y - 1, 22, 9, c); s.text(x + 2, y, name, '1b', T['bg'])


def event_log(T, sel=1, confirm_clear=False, empty=False):
    s = R.Screen(T['bg'])
    if empty:
        title(s, T, 'Event log', '0')
        strip(s, T); chip(s, T, 2, STRIP_Y + 2, 'EMPTY', T['deco'], outline=True)
        centred(s, BODY_Y + 16, 'Log empty', 3, T['fg'])
        centred(s, BODY_Y + 40, 'TX, RX, DMR, APRS, MSG,', 1, T['deco'])
        centred(s, BODY_Y + 50, 'DTMF, SOS, SSTV events', 1, T['deco'])
        keybar(s, T, 'RED EXIT')
        return s
    title(s, T, 'Event log', f'{sel + 1}/412')
    large = T.get('large')
    full = EVENTS[sel][3]
    big = large and len(full) * 8 <= W - 4
    dy = KEYBAR_Y - (16 if large else 8) - 2
    rows = (dy - 1 - BODY_Y) // 11
    strip(s, T)
    txt(s, T, 3, STRIP_Y + 2, '2026-11-22 ' + EVENTS[sel][0] + ':07Z', 1, T['hfg'])
    type_chip(s, T, W - 24, STRIP_Y + 2, EVENTS[sel][1], T[EVENTS[sel][2]], False)
    divider(s, T, dy - 2)
    txt(s, T, 2, dy + (4 if (large and not big) else 0), full, 3 if big else 1, T['fg'])
    for r in range(rows):
        tm, name, ck, text = EVENTS[r]
        y = BODY_Y + r * 11
        on = r == sel
        if on:
            s.fill(0, y - 2, W, 11, T['sel'])
        txt(s, T, 2, y, tm, 1, T['bg'] if on else T['deco'])
        type_chip(s, T, 34, y, name, T[ck], on)
        txt(s, T, 60, y, text[:16], 1, T['bg'] if on else T['fg'])
    if confirm_clear:
        confirm(s, T, 'CLEAR LOG?')
    else:
        keybar(s, T, 'UP/DN  0 CLEAR  RED EXIT')
    return s


# --- coverage ------------------------------------------------------------------------------------------
def level_c(T, v):
    return T['green'] if v >= -100 else (T['amber'] if v >= -115 else T['red'])


def coverage(T, confirm_clear=False):
    s = R.Screen(T['bg'])
    title(s, T, 'COVERAGE', '412/3840')
    strip(s, T)
    x = 2 + chip(s, T, 2, STRIP_Y + 2, 'REC', T['red']) + 4
    kv(s, T, x, STRIP_Y + 2, 'EVERY', '5s', T['hfg'])
    w = txt(s, T, W - 3, STRIP_Y + 2, '10%', 1, T['hfg'], True)
    txt(s, T, W - 3 - w - 4, STRIP_Y + 2, 'MEM', 1, T['deco'], True)
    y = BODY_Y
    dbm = -97
    txt(s, T, 2, y, str(dbm), 4, level_c(T, dbm))
    txt(s, T, 84, y + 2, 'S8', 3, T['fg'])
    chip(s, T, W - 2, y + 5, 'RF', T['green'], right=True)
    x = 84 + txt(s, T, 84, y + 21, 'dBm', 1, T['deco']) + 6
    txt(s, T, x, y + 21, 'GOOD', '1b', level_c(T, dbm))
    gt, gh = y + 34, 26
    s.rect(0, gt - 1, W, gh + 2, T['deco'])
    gy = lambda d: gt + int((-50 - max(-130, min(-50, d))) * (gh - 1) / 80)  # noqa: E731
    for ref in (-60, -80, -100, -120):
        for gx in range(2, W - 2, 4):
            s.fill(gx, gy(ref), 1, 1, T['deco'])
    for i in range(78):
        v = int(-100 + 12 * math.sin(i / 7.0) - (i % 9) - (22 if 40 < i < 52 else 0))
        gx = W - 2 - (78 - i) * 2
        top = gy(v); h = gt + gh - top
        c = level_c(T, v)
        if T['cues'] and v < -115:
            s.fill(gx, top, 2, min(h, 2), c)
        elif T['cues'] and v < -100:
            for r in range(0, h, 2):
                s.fill(gx, top + r, 2, 1, c)
        else:
            s.fill(gx, top, 2, h, c)
    y = gt + gh + 4
    x = 2 + kv(s, T, 2, y, 'SES', '96', T['fg']) + 8
    x += kv(s, T, x, y, 'MIN', '-121', level_c(T, -121)) + 8
    kv(s, T, x, y, 'MAX', '-84', level_c(T, -84))
    y += 10
    x = 2 + chip(s, T, 2, y, 'GPS', T['green']) + 4
    txt(s, T, x, y, 'KO01CJ', 1, T['fg'])
    if confirm_clear:
        confirm(s, T, 'CLEAR ALL?')
    else:
        keybar(s, T, 'GRN STOP  # INT  RED EXIT')
    return s


# --- position ------------------------------------------------------------------------------------------
def position(T):
    s = R.Screen(T['bg'])
    title(s, T, 'POSITION', '14:32Z')
    strip(s, T)
    x = 2 + chip(s, T, 2, STRIP_Y + 2, 'GPS', T['green']) + 4
    txt(s, T, x, STRIP_Y + 2, '3D SAT 9', 1, T['hfg'])
    w = txt(s, T, W - 3, STRIP_Y + 2, '0.9', 1, T['hfg'], True)
    txt(s, T, W - 3 - w - 4, STRIP_Y + 2, 'HDOP', 1, T['deco'], True)
    y = BODY_Y
    txt(s, T, 2, y + 4, 'MGRS', 1, T['deco'])
    txt(s, T, 32, y, '34U DB', 3, T['green'])
    if T.get('large'):
        txt(s, T, 2, y + 17, '44348', 4, T['fg'])
        txt(s, T, 2, y + 50, '94611', 4, T['fg'])
        w = txt(s, T, W - 2, y + 4, 'KO01CJ', 1, T['blue'], True)
        txt(s, T, W - 2 - w - 4, y + 4, 'LOC', 1, T['deco'], True)
        txt(s, T, 86, y + 39, 'E', 1, T['deco'])
        txt(s, T, 86, y + 72, 'N', 1, T['deco'])
        txt(s, T, W - 2, y + 22, '187m', 1, T['fg'], True)
        txt(s, T, 102, y + 22, 'ALT', 1, T['deco'])
        txt(s, T, 102, y + 55, 'WPT', 1, T['deco'])
        txt(s, T, W - 2, y + 65, '1250m 245', 1, T['blue'], True)
    else:
        txt(s, T, 32, y + 16, '44348 94611', 3, T['fg'])
        y += 34; kv(s, T, 2, y, 'UTM', '34U 444348E 5694611N', T['fg'])
        y += 10; kv(s, T, 2, y, 'DEG', '51.40000N 20.20000E', T['fg'])
        y += 10; kv(s, T, 2, y, 'LOC', 'KO01CJ', T['blue'])
        w = txt(s, T, W - 2, y, '187m', 1, T['fg'], True); txt(s, T, W - 2 - w - 4, y, 'ALT', 1, T['deco'], True)
        y += 10; divider(s, T, y); y += 3
        txt(s, T, 2, y, 'SUN  07:02 - 15:41 LT', 1, T['amber'])
        y += 10; txt(s, T, 2, y, 'WPT 1250m  245 WSW', 1, T['blue'])
    keybar(s, T, '* MARK WAYPOINT  RED EXIT')
    return s


# --- SSTV ----------------------------------------------------------------------------------------------
def sstv_idle(T, doppler=False):
    s = R.Screen(T['bg'])
    title(s, T, 'SSTV', '3/8')
    strip(s, T)
    x = 2 + chip(s, T, 2, STRIP_Y + 2, 'RX', T['green']) + 3
    if doppler:
        x += chip(s, T, x, STRIP_Y + 2, 'DOPPLER', T['blue'], outline=True) + 3
    txt(s, T, x, STRIP_Y + 2, '1900Hz', 1, T['hfg'])
    txt(s, T, W - 3, STRIP_Y + 2, '-112dBm', 1, T['deco'], True)
    y = BODY_Y
    centred(s, y + 6, 'Waiting for', 3, T['fg'])
    centred(s, y + 22, 'SSTV picture', 3, T['fg'])
    divider(s, T, y + 42)
    f = '437.7963 ISS DOP' if doppler else '145.800 MHz ISS'
    if len(f) * 8 <= W - 4:
        centred(s, y + 47, f, 3, T['blue'])
    else:
        centred(s, y + 51, f, '1b' if T.get('large') else 1, T['blue'])
    centred(s, y + 68, 'ROBOT MARTIN SCOTTIE PD', '1b' if T.get('large') else 1, T['deco'])
    keybar(s, T, 'DOPPLER  0 CLEAR  RED EXIT' if doppler else 'UP/DN QRG 0 CLEAR RED EXIT')
    return s


def picture(s, rows):
    # stand-in for a received picture (true colours, not themed): sky, sea, sun
    for r in range(rows):
        for x in range(0, W, 2):
            y = 8 + r
            if r < 70:
                c = (40 + r, 90 + r, 200)
            else:
                c = (10, 60 + (r - 70), 120 + (x % 7) * 4)
            if (x - 110) ** 2 + (r - 40) ** 2 < 200:
                c = (255, 210, 60)
            s.fill(x, y, 2, 1, (c[0] << 16) | (c[1] << 8) | c[2])


def sstv_bar(T, state='rx', doppler=False, mode='PD120', row=63):
    s = R.Screen(T['bg'])
    picture(s, 120 if state != 'rx' else row)
    sh = 8
    s.fill(0, 0, W, sh, T['hbg'])
    word, c = {'rx': ('RX', T['green']), 'done': ('DONE', T['blue'])}[state]
    w = len(word) * 6 + 4
    s.fill(0, 0, w, sh, c); s.text(2, 0, word, '1b', T['bg'])
    x = w + 4
    x += txt(s, T, x, 0, mode, 1, T['hfg']) + 6
    mid, mc = ('DOPPLER', T['blue']) if doppler else ('145.800', T['hfg'])
    if state == 'rx':
        right = f'{row}/120'
    else:
        right = 'SAVED 3/8'; mid, mc = '0 NEW', T['deco']
    if state == 'rx':
        rw = len(right) * 6
        bx = W - rw - 4
        fill = row * (rw + 4) // 120
        s.fill(bx, 0, fill, sh, T['green'])
        for i, ch in enumerate(right):
            cx = bx + 2 + i * 6
            txt(s, T, cx, 0, ch, 1, T['bg'] if cx + 3 < bx + fill else T['hfg'])
        rw += 2
    else:
        rw = txt(s, T, W - 2, 0, right, 1, T['deco'], True)
    if x + len(mid) * 6 + 6 <= W - 2 - rw:
        txt(s, T, x, 0, mid, 1, mc)
    return s


# --- net control ---------------------------------------------------------------------------------------
# (station, check-in, age, quiet)
NET = [('SP7ABC', '14:02', '1m', False), ('SP7XYZ-9', '14:03', '12m', True), ('2601234', '14:05', '40s', False),
       ('FM#1', '14:06', '3m', False), ('SP5ABC-15', '14:08', '16m', True), ('FM#2', '14:10', '9m', False),
       ('SQ7KLM', '14:11', '2m', False)]


def net_chip(s, T, y, quiet, on):
    # IN: solid block, bold text; QUIET: frame, regular text (3 px from the right edge)
    word = 'QUIET' if quiet else 'IN'
    w = len(word) * 6 + 4
    x = W - 3 - w
    if quiet:
        c = T['bg'] if on else T['amber']
        s.rect(x, y - 1, w, 9, c); s.text(x + 2, y, word, 1, c)
    elif on:
        s.fill(x, y - 1, w, 9, T['bg']); s.text(x + 2, y, word, '1b', T['sel'])
    else:
        s.fill(x, y - 1, w, 9, T['green']); s.text(x + 2, y, word, '1b', T['bg'])


def net(T, state='run', sel=1):
    s = R.Screen(T['bg'])
    running = state != 'off'
    rows = NET if state != 'empty' else []
    title(s, T, 'NET CONTROL', (f'{sel + 1}/{len(rows)}' if rows else '0') if running else None)
    strip(s, T)
    x = 2 + chip(s, T, 2, STRIP_Y + 2, 'RUN' if running else 'OFF', T['green'] if running else T['deco'], outline=not running) + 4
    if running:
        x += txt(s, T, x, STRIP_Y + 2, '0:42', '1b', T['hfg']) + 5
        x += txt(s, T, x, STRIP_Y + 2, 'Q10', 1, T['hfg']) + 5
    name = 'SR7TM AU Tomaszo'[:(W - 3 - x) // 6]
    txt(s, T, W - 3, STRIP_Y + 2, name, 1, T['deco'], True)
    y = BODY_Y
    if not running:
        centred(s, y + 10, 'NO NET', 3, T['fg'])
        txt(s, T, 4, y + 34, 'GREEN starts a net.', 1, T['deco'])
        txt(s, T, 4, y + 44, 'DMR / APRS stations', 1, T['deco'])
        txt(s, T, 4, y + 54, 'heard check in.', 1, T['deco'])
        txt(s, T, 4, y + 70, 'Last net: 7 stations', 1, T['fg'])
        keybar(s, T, 'GRN START  RED EXIT')
        return s
    if not rows:
        centred(s, y + 10, 'LISTENING', 3, T['fg'])
        txt(s, T, 4, y + 34, 'Stations heard on DMR', 1, T['deco'])
        txt(s, T, 4, y + 44, 'or APRS check in.', 1, T['deco'])
        txt(s, T, 4, y + 58, '* adds an FM station.', 1, T['deco'])
        keybar(s, T, '* FM  # QUIET  0 END')
        return s
    for i, (call, tin, age, quiet) in enumerate(rows[:(KEYBAR_Y - 1 - BODY_Y) // 11]):
        y = BODY_Y + i * 11
        on = i == sel
        if on:
            s.fill(0, y - 2, W, 11, T['sel'])
        txt(s, T, 2, y, call, '1b', T['bg'] if on else T['fg'])
        txt(s, T, 62, y, tin, 1, T['bg'] if on else T['deco'])
        txt(s, T, 118, y, age, 1, T['bg'] if on else (T['amber'] if quiet else T['fg']), True)
        net_chip(s, T, y, quiet, on)
    if state == 'end':
        confirm(s, T, 'END NET?')
    else:
        keybar(s, T, 'GRN HEARD * FM # Q 0 END' if rows[sel][0].startswith('FM#') else 'GRN QSO  * FM  # Q  0 END')
    return s


SCREENS = {
    'pace': lambda T: [pace(T), pace(T, 3, confirm_clear=True)],
    'radio_check': lambda T: [radio_check(T), radio_check(T, empty=True)],
    'eventlog': lambda T: [event_log(T), event_log(T, 2, confirm_clear=True)],
    'coverage': lambda T: [coverage(T)],
    'net': lambda T: [net(T, 'off'), net(T, 'empty'), net(T, 'run', 1), net(T, 'run', 3), net(T, 'end', 2)],
    'position': lambda T: [position(T)],
    'sstv': lambda T: [sstv_idle(T), sstv_idle(T, doppler=True), sstv_bar(T), sstv_bar(T, doppler=True, mode='Scottie 1', row=100),
                       sstv_bar(T, 'done')],
}


def main():
    for name, fn in SCREENS.items():
        panels = []
        per = None
        for _, T in THEMES:
            p = fn(T)
            per = len(p)
            panels += p
        # one row per style
        A.sheet(os.path.join(HERE, f'tools_{name}.png'), panels, cols=per)
    print(' '.join(f'tools_{n}.png' for n in SCREENS))


if __name__ == '__main__':
    main()
