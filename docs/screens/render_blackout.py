#!/usr/bin/env python3
# Stage 4a mock-ups: blackout / silent mode and the battery time left, in the EMCOM styles
# (day, night, mono, contrast, mono / contrast at night). Per style: the main screen with the
# time left in the footer, the "BLACKOUT ON" card, the footer with BLK next to the busiest
# right item (TX OFF) and the Radio infos battery page. Layout: uiEmcom.c drawFooter(),
# blackout.c, menuRadioInfos.c (battery page). Example data.
#
#   python3 docs/screens/render_blackout.py     (writes field_blackout.png, field_battery.png)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import render_screens as R  # noqa: E402
import render_access as A  # noqa: E402
import render_refresh as F  # noqa: E402
import render_tools as T  # noqa: E402

W, H = R.W, R.H
STYLES = (('day', R.DARK), ('night', R.NIGHT), ('mono', R.MONO), ('contrast', R.CONTRAST),
          ('mono night', R.MONO_NIGHT), ('contrast night', R.CONTRAST_NIGHT))
THEMES = dict(T.THEMES)


def main_screen(P, est='~6h40', blk=False, right=None, rc=None):
    s = R.Screen(P['bg']); R.status_bar(s, P); R.bar(s, P, 0); R.zone_row(s, P); R.hero(s, P); R.divs(s, P)
    R.params_fm(s, P); R.emcom_rows(s, P); R.footer(s, P, est=est, blk=blk, right=right, rc=rc)
    return s


def card(P):
    return F.alert(P, 'info', 'BLACKOUT ON', under=main_screen(P, blk=True))


def battery_page(TH, volts='7.6V', pct=67, est='~6h40'):
    # menuRadioInfos.c, RADIO_INFOS_BATTERY_LEVEL (MD-UV380 branch): x = 88, level in FG_DECORATION
    s = R.Screen(TH['bg'])
    T.title(s, TH, 'Battery', '1/7')
    x, lh = 88, H - 28
    deco, fg = TH['deco'], TH['fg']
    s.rect(x + 1, 20, 26, H - 22, deco); s.rect(x, 19, 28, H - 20, deco); s.fill(x + 9, 15, 10, 6, deco)
    h = lh * pct // 100
    s.fill(x + 4, 23 + lh - h, 20, h, deco)
    s.text((x - 32) // 2, 20, volts, 3, fg)
    t = f'{pct}%'
    s.text((x - len(t) * 8) // 2, H - 20, t, 3, fg)
    e = est.lstrip('~')  # the label says it is an estimate
    s.text((x - len(e) * 8) // 2, 54, e, 3, fg)
    s.text((x - 9 * 6) // 2, 54 + 16 + 2, 'time left', 1, deco)
    return s


def main():
    panels = []
    for name, P in STYLES:
        panels += [main_screen(P, blk=True), card(P), main_screen(P, est=None, blk=True, right='TX OFF', rc=P['tx'])]
    A.sheet(os.path.join(HERE, 'field_blackout.png'), panels, cols=3)

    panels = []
    for name, P in STYLES:
        TH = THEMES[name]
        panels += [main_screen(P), battery_page(TH), battery_page(TH, '8.1V', 94, '--')]
    A.sheet(os.path.join(HERE, 'field_battery.png'), panels, cols=3)
    print('field_blackout.png field_battery.png')


if __name__ == '__main__':
    main()
