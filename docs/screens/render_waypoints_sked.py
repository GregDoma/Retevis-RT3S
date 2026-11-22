#!/usr/bin/env python3
# Stage 4b mock-ups: saved waypoints (menuWaypoints.c) and the comms schedule (menuSked.c,
# functions/sked.c) in the EMCOM styles (day, night, mono, contrast, mono / contrast at night)
# and the classic style. Per style: waypoint list with a fix, without a fix on an empty slot,
# delete confirmation; sked list, editor, delete confirmation; the "SKED in 1 min" and
# "SKED NOW" cards over the main screen and the PACE strip with the next sked. Example data.
#
#   python3 docs/screens/render_waypoints_sked.py
#   (writes tools_waypoints.png, tools_sked.png, field_sked_cards.png, field_waypoints.png, field_sked.png)
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import render_screens as R  # noqa: E402
import render_access as A  # noqa: E402
import render_refresh as F  # noqa: E402
import render_tools as T  # noqa: E402
import render_blackout as B  # noqa: E402
import render_menus as M  # noqa: E402

W, H = R.W, R.H
txt, chip, title, strip, keybar, confirm = T.txt, T.chip, T.title, T.strip, T.keybar, T.confirm
BODY_Y, KEYBAR_Y, STRIP_Y = T.BODY_Y, T.KEYBAR_Y, T.STRIP_Y


def tri(s, pts, c):
    (x0, y0), (x1, y1), (x2, y2) = pts
    for y in range(min(y0, y1, y2), max(y0, y1, y2) + 1):
        for x in range(min(x0, x1, x2), max(x0, x1, x2) + 1):
            d0 = (x1 - x0) * (y - y0) - (y1 - y0) * (x - x0)
            d1 = (x2 - x1) * (y - y1) - (y2 - y1) * (x - x1)
            d2 = (x0 - x2) * (y - y2) - (y0 - y2) * (x - x2)
            if (d0 >= 0 and d1 >= 0 and d2 >= 0) or (d0 <= 0 and d1 <= 0 and d2 <= 0):
                s.fill(x, y, 1, 1, c)


def arrow(s, cx, cy, brg, c):
    # menuWaypoints.c drawArrow(): tip and wings 5 px, wings at +-150 deg, north up
    p = lambda a, r: (cx + round(math.sin(math.radians(a)) * r), cy - round(math.cos(math.radians(a)) * r))
    tip = p(brg, 5)
    tri(s, [tip, p(brg + 150, 5), (cx, cy)], c)
    tri(s, [tip, p(brg - 150, 5), (cx, cy)], c)


COMPASS = ['N', 'NNE', 'NE', 'ENE', 'E', 'ESE', 'SE', 'SSE', 'S', 'SSW', 'SW', 'WSW', 'W', 'WNW', 'NW', 'NNW']


def compass(b):
    return COMPASS[int((b + 11.25) // 22.5) % 16]


# slot: (name, metres, bearing, target)
WPTS = [('CAMP', 1240, 48, True), ('WP2', 350, 205, False), ('SR7TM', 12400, 292, False), ('WATER', 2100, 95, False),
        None, ('SQ7KLM-9', 7800, 160, False), None, None]


def dist(m):
    return f'{m}m' if m < 1000 else (f'{m // 1000}.{(m % 1000) // 100}km' if m < 100000 else f'{m // 1000}km')


def wp_rows(s, TH, y0, sel, fix):
    used = 0
    for i, w in enumerate(WPTS):
        y = y0 + i * 11
        on = i == sel
        tc = TH['bg'] if on else TH['fg']
        mc = TH['bg'] if on else TH['deco']
        if on:
            s.fill(0, y, W, 10, TH['sel'])
        if w is None:
            txt(s, TH, 2, y + 1, str(i + 1), 1, mc)
            txt(s, TH, 10, y + 1, '-- empty', 1, mc)
            continue
        used += 1
        name, m, b, tgt = w
        txt(s, TH, 2, y + 1, '>' if tgt else str(i + 1), 1, (TH['bg'] if on else TH['blue']) if tgt else mc)
        txt(s, TH, 10, y + 1, name, '1b', TH['blue'] if (tgt and not on) else tc)
        if not fix:
            txt(s, TH, 118, y + 1, '--', 1, mc, True)
            continue
        txt(s, TH, 118, y + 1, dist(m), 1, tc, True)
        txt(s, TH, 142, y + 1, compass(b), 1, mc, True)
        arrow(s, 151, y + 5, b, TH['bg'] if on else TH['green'])
    return used


def waypoints(TH, sel=0, fix=True, delete=False):
    s = R.Screen(TH['bg'])
    title(s, TH, 'WAYPOINTS', f'{sel + 1}/8')
    used = wp_rows(s, TH, BODY_Y, sel, fix)
    strip(s, TH)
    chip(s, TH, 2, STRIP_Y + 2, 'FIX' if fix else 'NO FIX', TH['green'] if fix else TH['amber'], outline=not fix)
    txt(s, TH, W - 3, STRIP_Y + 2, f'{used}/8 SAVED', 1, TH['deco'], True)
    if delete:
        confirm(s, TH, f'DELETE {sel + 1}?')
    else:
        keybar(s, TH, 'GRN GO  RGT NAME  0 DEL' if WPTS[sel] else '* SAVE HERE  # LAT/LON')
    return s


SKEDS = [  # when, length/tune, chip, countdown, state colour
    ('HOURLY :00', '10M PACE P', 'ON', '41:20', 'blue'),
    ('DAILY 18:00', '15M CH SR7TM AU To', 'ON', '3H12', 'blue'),
    ('EVERY 30M :15', '5M NONE', 'NOW', '02:10', 'green'),
    None,
]


def sked_list(TH, sel=0, delete=False):
    s = R.Screen(TH['bg'])
    title(s, TH, 'SKED', f'{sel + 1}/4')
    rh = (KEYBAR_Y - BODY_Y + 1) // 4
    for i, k in enumerate(SKEDS):
        y = BODY_Y + i * rh
        on = i == sel
        c = TH[k[4]] if k else TH['deco']
        tc = TH['bg'] if on else TH['fg']
        mc = TH['bg'] if on else TH['deco']
        s.fill(0, y, 20, rh - 1, c)
        s.text(4, y + (rh - 1 - 8) // 2, f'S{i + 1}', '1b', TH['bg'])
        if on:
            s.fill(21, y, W - 21, rh - 1, TH['sel'])
        elif i < 3:
            s.fill(21, y + rh - 1, W - 21, 1, TH['deco'])
        if k is None:
            txt(s, TH, 24, y + 3, 'NO SKED', '1b', mc)
            txt(s, TH, 24, y + 12, 'GREEN sets one', 1, mc)
            chip(s, TH, W - 2, y + 3, 'EMPTY', TH['bg'] if on else TH['deco'], outline=True, right=True)
            continue
        when, line2, st, cd, ck = k
        txt(s, TH, 24, y + 3, when, '1b', tc)
        if st == 'NOW':
            chip(s, TH, W - 2, y + 3, 'NOW', TH['bg'] if on else TH['green'], outline=on, right=True)
        else:
            chip(s, TH, W - 2, y + 3, st, TH['bg'] if on else c, outline=True, right=True)
        w = txt(s, TH, W - 2, y + 12, cd, 1, TH['bg'] if on else (TH['green'] if st == 'NOW' else TH['amber']), True)
        maxc = (W - 2 - w - 6 - 24) // 6
        txt(s, TH, 24, y + 12, line2[:maxc], 1, mc)
    strip(s, TH)
    x = 2 + chip(s, TH, 2, STRIP_Y + 2, 'S3 NOW', TH['green']) + 4
    txt(s, TH, x, STRIP_Y + 2, '02:10', 1, TH['hfg'])
    txt(s, TH, W - 3, STRIP_Y + 2, 'UTC 14:47', 1, TH['deco'], True)
    if delete:
        confirm(s, TH, f'DELETE S{sel + 1}?')
    else:
        keybar(s, TH, 'GRN EDIT  0 DEL  RED BACK')
    return s


EDIT = [('EVERY', 'DAY'), ('AT', '18:00 UTC'), ('LENGTH', '15 MIN'), ('TUNE', 'PACE A'), ('ALERT', 'ON')]


def sked_edit(TH, field=1, emcom=True):
    s = R.Screen(TH['bg'])
    if emcom:
        title(s, TH, 'EDIT SKED', 'S2')
        strip(s, TH)
        x = 2 + chip(s, TH, 2, STRIP_Y + 2, 'NEXT S1', TH['amber'], outline=True) + 4
        txt(s, TH, x, STRIP_Y + 2, '41:20', 1, TH['hfg'])
        txt(s, TH, W - 3, STRIP_Y + 2, 'UTC 14:47', 1, TH['deco'], True)
    else:
        M.title_bar(s, R.DARK, 'Sked 2')
    y0 = BODY_Y if emcom else 16
    for f, (k, v) in enumerate(EDIT):
        y = y0 + f * 15
        on = f == field
        if on:
            s.fill(0, y, W, 13, TH['sel'])
        txt(s, TH, 4, y + 3, k, 1, TH['bg'] if on else TH['deco'])
        txt(s, TH, 58, y + 3, v, '1b', TH['bg'] if on else TH['fg'])
        if on:
            txt(s, TH, 50, y + 3, '<', '1b', TH['bg'])
            txt(s, TH, W - 3, y + 3, '>', '1b', TH['bg'], True)
    txt(s, TH, 4, y0 + 75 + (1 if emcom else 0), 'DAILY 18:00 15M', 1, TH['amber'])
    keybar(s, TH, 'UP/DN  < > SET  GRN SAVE')
    return s


def cards(P):
    warn = F.alert(P, 'warning', 'SKED in 1 min\nS1 SR7TM AU Tomaszo', under=B.main_screen(P))
    now = F.alert(P, 'info', 'SKED 1 NOW\nSR7TM AU Tomaszo\nGREEN = TUNE', under=B.main_screen(P))
    return [warn, now]


# --- classic (menuWaypoints.c / menuSked.c without the EMCOM style) --------------------------------
def classic_theme():
    P = R.DARK
    return dict(bg=P['bg'], fg=P['fg'], deco=P['mut'], sel=P['line'], green=P['rx'], amber=P['scan'], red=P['tx'],
                blue=P['info'], hbg=P['bar'], hfg=P['barFg'])


def classic_waypoints():
    TH = classic_theme()
    s = R.Screen(TH['bg'])
    M.title_bar(s, R.DARK, 'Waypoints')
    wp_rows_classic(s, TH, 16, 0)
    s.fill(0, H - 11, W, 11, TH['hbg']); s.text(3, H - 9, 'GRN GO  RGT NAME  0 DEL', 1, TH['hfg'])
    return s


def wp_rows_classic(s, TH, y0, sel):
    for i, w in enumerate(WPTS):
        y = y0 + i * 11
        on = i == sel
        if on:
            s.fill(0, y, W, 10, 0x3BD16F)
        tc = TH['bg'] if on else TH['fg']
        mc = TH['bg'] if on else TH['deco']
        if w is None:
            s.text(2, y + 1, str(i + 1), 1, mc); s.text(10, y + 1, '-- empty', 1, mc)
            continue
        name, m, b, tgt = w
        s.text(2, y + 1, '>' if tgt else str(i + 1), 1, tc if on else (TH['blue'] if tgt else mc))
        s.text(10, y + 1, name, '1b', tc if (on or not tgt) else TH['blue'])
        s.text(118, y + 1, dist(m), 1, tc, True)
        s.text(142, y + 1, compass(b), 1, mc, True)
        arrow(s, 151, y + 5, b, TH['bg'] if on else TH['green'])


def classic_sked():
    TH = dict(classic_theme(), sel=0x3BD16F)
    # classic: plain title, no strip, rows from y 16 (25 px rows)
    c = R.Screen(TH['bg'])
    M.title_bar(c, R.DARK, 'Sked')
    rh = (H - 16 - 11 - 1) // 4
    for i, k in enumerate(SKEDS):
        y = 16 + i * rh
        on = i == 0
        col = TH[k[4]] if k else TH['deco']
        c.fill(0, y, 20, rh - 1, col); c.text(4, y + (rh - 1 - 8) // 2, f'S{i + 1}', '1b', TH['bg'])
        if on:
            c.fill(21, y, W - 21, rh - 1, TH['sel'])
        tc, mc = (TH['bg'], TH['bg']) if on else (TH['fg'], TH['deco'])
        if k is None:
            c.text(24, y + 3, 'NO SKED', '1b', mc); c.text(24, y + 12, 'GREEN sets one', 1, mc)
            T.chip(c, TH, W - 2, y + 3, 'EMPTY', TH['deco'], outline=True, right=True)
            continue
        when, line2, st, cd, ck = k
        c.text(24, y + 3, when, '1b', tc)
        T.chip(c, TH, W - 2, y + 3, st, TH['bg'] if on else (TH['green'] if st == 'NOW' else col), outline=(st != 'NOW') or on, right=True)
        w = len(cd) * 6
        c.text(W - 2, y + 12, cd, 1, TH['bg'] if on else TH['amber'], True)
        c.text(24, y + 12, line2[:(W - 2 - w - 6 - 24) // 6], 1, mc)
    c.fill(0, H - 11, W, 11, TH['hbg']); c.text(3, H - 9, 'GRN EDIT  0 DEL  RED BACK', 1, TH['hfg'])
    return c


def main():
    panels = []
    for _, TH in T.THEMES:
        panels += [waypoints(TH, 0), waypoints(TH, 4, fix=False), waypoints(TH, 2, delete=True)]
    A.sheet(os.path.join(HERE, 'tools_waypoints.png'), panels, cols=3)

    panels = []
    for _, TH in T.THEMES:
        panels += [sked_list(TH, 0), sked_edit(TH), sked_list(TH, 1, delete=True), T.pace(TH, 0, sked='S1 41:20')]
    A.sheet(os.path.join(HERE, 'tools_sked.png'), panels, cols=4)

    panels = []
    for _, P in B.STYLES:
        panels += cards(P)
    A.sheet(os.path.join(HERE, 'field_sked_cards.png'), panels, cols=2)

    classic_waypoints().png(os.path.join(HERE, 'field_waypoints.png'))
    A.sheet(os.path.join(HERE, 'field_sked.png'), [classic_sked(), sked_edit(dict(classic_theme(), sel=0x3BD16F), emcom=False)], cols=2)
    print('tools_waypoints.png tools_sked.png field_sked_cards.png field_waypoints.png field_sked.png')


if __name__ == '__main__':
    main()
