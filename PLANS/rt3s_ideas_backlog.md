# RT3S idea backlog

Feature ideas for running this firmware on the Retevis RT3S (`PLATFORM=MDUV380`).
They are based on the R20260131 code and on the RT3S hardware:

* STM32F405 + HR-C6000 (DMR) + AT1846S
* 160×128 colour LCD
* GPS
* 16 MB SPI flash (W25Q128, part id `0x4018`)
* PTT, SK1, SK2, channel knob, keypad. **No top/orange button**: `BUTTON_ORANGE`
  is emulated with SK1 + GREEN.

Status legend: **done**, **in progress**, *idea*.

## Already in the firmware (upstream + fork)

* DMR Tier II (CC/TS, TG lists, private and all calls, Talker Alias, filters), FM
  CTCSS/DCS, tone scan, 1750 Hz, DTMF send, VOX, dual watch, scan.
* GPS screen, Maidenhead locator, APRS beaconing (TX only, SmartBeaconing),
  roaming (zone sorted by repeater distance), satellite predictions with Doppler.
* Colour themes (day/night), voice prompts, 20+ languages (Polish included),
  hotspot (MMDVMHost/BlueDV), CPS over USB.
* Fork: VFO band scope + waterfall, custom-data area auto-recovery/reset.

## Backlog

| # | Idea | Status | Notes |
|---|---|---|---|
| 1 | Waterfall: peak hold, strongest-signal marker, tune to peak | **done** | see `rt3s_quickkeys_and_waterfall.md` |
| 1b | Waterfall: band presets (`SK1+3`) | **done** | free cursor still an idea; knob is used for retune |
| 1c | Fix: `SK1+0` in sweep was swallowed by the "any key stops the scan" check | **done** | handled before that check now |
| 8c | GPS trip computer page (distance, moving time, avg/max speed, altitude gain/loss) | **done** | `functions/gpsTrip.c`, RAM only, long GREEN resets |
| 3 | Quickkeys menu + quickkey actions | **done** | see `rt3s_quickkeys_and_waterfall.md` |
| 2 | Fox-hunt mode on the RSSI screen (pitch ∝ RSSI, max-hold with GPS position and bearing) | **done** | `menuRSSIScreen.c`, `#` toggles; untested on hardware |
| 4 | GPS track log to SPI flash (16 MB), GPX export over USB/CPS, "back to start" screen | **done** | upstream already logs NMEA (GPS mode On+Log, last 2 MB); `tools/opengd77_gps_log_to_gpx.py` reads it over the CPS link; "back to start" = waypoint page |
| 5 | Replay the last DMR transmission locally (store AMBE frames in flash) | *idea* | ~27 B / 60 ms, so minutes fit easily |
| 6 | FM DTMF **decoding** (show received digits, simple selective call) | **done** | RSSI monitor DTMF view + background notification; selective call still an idea |
| 6b | CW decoder (repeater IDs on FM) | **done** | `toneRxDecoders.c`, host tests |
| 7 | Roaming++: auto-switch to the nearest repeater on low RSSI | *idea* | upstream already has auto re-sort on distance and a bearing notification, so little is left |
| 8 | GPS navigation: waypoint page (GREEN marks, arrow/distance/bearing back) | **done** | `menuGPS.c`; speed/course/alt already on the Direction page; waypoint is RAM-only; untested on hardware |
| 8b | Saved waypoints: 8 named positions in the field data flash, list with distance / bearing, navigate / save here / keypad entry / rename / delete, APRS station saved | **done** (2026-11-22) | see *Saved waypoints and sked* below; a contact / channel location as target is still an idea; untested on hardware |
| 9 | Satellite: automatic Doppler retune during a pass, cross-band hints | *idea* | `menuSatelliteScreen.c` |
| 10 | APRS RX decode (AFSK 1200): station list with distance/bearing, radar view | **done** (2026-11-19) | AFSK/AX.25 decoder on I2S RX audio; RSSI `9` monitor: frames / raw (TNC2+hex) / stations (dist, bearing) / radar views (`#`); positions: uncompressed, compressed, Mic-E, objects, items; background RX on `SK1+4` / quickkey with notifications. Background setting persisted (bit 29); APRS messages with auto ACK and canned replies; station -> GPS waypoint |
| 16 | Radiosonde hunting: 401/403/405 MHz scope presets + keypad waypoint entry + fox-hunt | **done** | no RS41/M20 decode: only 8 kHz voice-band audio reaches the MCU, the sondes send 4800/9600 bd GFSK |
| 17 | SSTV receive (Robot, Martin, Scottie, PD) on the RSSI monitor | **done** | `sstvRx.c`, encoder-based host test; untested on air |
| 11 | DMR SMS send/receive | *idea* | needs data-burst handling in `HR-C6000.c` |
| 12 | DMR GPS position (LRRP / Brandmeister) | *idea* | |
| 13 | Unify MD2017 / MD9600 trees | *idea* | `unify_sourcecodes.md` |
| 14 | Host-side unit tests (AX.25 encoder, Maidenhead, satellite maths, roaming sort, quickkey encoding) in CI | *idea* | |
| 15 | Ship a binary named for the RT3S in releases + flashing notes in the README | *idea* | |
| 18 | Colour-blind safe EMCOM style: `Opcje ekranu → Styl` = Classic / EMCOM / **EMCOM mono** | **done** (2026-11-22) | see *EMCOM mono style* below; untested on hardware |
| 18b | Low-vision EMCOM style **EMCOM contr**: white / yellow on black (night amber / red), bold text, 8x16 channel frequency, thick S-meter | **done** (2026-11-22) | same option row and shape cues as mono; untested on hardware |
| 19 | SSTV++ (issue #13): frequency on the SSTV screen, Doppler from the satellite mode, pictures saved to flash + web download | *in progress* | see *SSTV++* below |
| 20 | UI refresh, stage 1 (EMCOM styles): TX view, APRS messages, message boxes / notifications | **done** (2026-11-22) | see *UI refresh* below; untested on hardware |
| 20b | UI refresh, stage 2 (EMCOM styles): PACE, Radio check, Event log, Coverage, Position, SSTV | **done** (2026-11-22) | see *UI refresh* below; untested on hardware |
| 21 | Net control + QSO log: roster of stations heard since the net start (DMR last heard / APRS), FM manual check-ins, silence alarm, QSO from the roster / Last heard into the event log | **done** (2026-11-22) | see *Net control* below; untested on hardware |
| 22 | Blackout / silent mode: minimum backlight on key press only, no beeps / voice prompts / LEDs; `SK1+5` / quickkey | **done** (2026-11-22) | see *Blackout and battery time left* below; untested on hardware |
| 23 | Battery runtime estimate ("time left") on Radio infos and the EMCOM footer | **done** (2026-11-22) | see *Blackout and battery time left* below; untested on hardware |
| 24 | Comms schedule (sked) reminders: 4 skeds (every 15 min .. daily, UTC), 1 min warning + NOW card, GREEN tunes the PACE slot / channel, SYS log, countdown in the PACE strip | **done** (2026-11-22) | see *Saved waypoints and sked* below; untested on hardware |

## EMCOM mono style (done, #18)

**Done (2026-11-22)**, with a second variant *EMCOM contr* for low vision.
Notes on what differs from the plan below: option bits 2 and 10 of bank 0
(`BIT_UI_EMCOM_MONO`, `BIT_UI_EMCOM_CONTRAST`; bank 1 is not allocated, so
adding it would change the settings layout); the style names are hardcoded
like "Classic" / "EMCOM" (no language strings); low battery is `!` + an inverse
chip, no blink; the idle state chip is outlined, RX / SCAN solid, TX inverts the
whole status bar; the RSSI screen meter and the band scope waterfall also
follow; SSTV pictures keep their colours; the APRS radar already uses shapes
(dots, ring, square). Contrast / CVD numbers: `docs/screens/palette_check.py`,
mock-ups: `docs/screens/render_access.py`.


**Why:** the EMCOM screen says what the radio is doing by colour alone. RX is
green, TX red, scan amber and info blue, and the header uses the same colours
for the battery, the `a` APRS mark and the status chip. Red against green is the
pair that people with deuteranopia or protanopia cannot tell apart. These
colours are hard-coded, so a custom theme cannot fix them.

**What:** the existing `Styl` row in `Opcje ekranu` (today Classic / EMCOM,
`BIT_UI_CLASSIC_STYLE`) gets a third value, **EMCOM mono**:

* One hue in several shades: white, light grey, mid grey and dark grey on
  black. The night variant uses the same idea in shades of red, as the current
  night palette does.
* The state must be readable without any colour:
  * TX: a status chip in inverted video (white fill, black text) and the word
    `TX`. RX keeps the plain chip with `RX`, and scan gets `SCAN`. The bold
    "chip" text already exists, so only the fill changes.
  * Low battery: the battery outline blinks, or `!` appears before the
    percentage, instead of turning red.
  * APRS beacon on or off: a bold or thin `a`, which already works without
    colour. Keep it that way.
  * S-meter over S9: hatched or striped bar segments instead of amber.
* Menus get a grey preset from `emcomThemePreset()`, a third table next to day
  and night, with the selected item in inverted video.

**Where in the code:**

| File | Change |
|---|---|
| `uiEmcom.c` | `PALETTE_DARK` and `PALETTE_NIGHT` (around line 55): add `PALETTE_MONO` and pick it in the palette selector |
| `uiEmcom.c` | Status chip (around line 326), battery (around line 363), TX bar (around line 305): shape and inverted-video cues in mono mode |
| `uiEmcom.c` | `emcomThemePreset()` (around line 1128): mono table |
| `menuDisplayOptions.c` | Lines 306, 684 and 896: the style row cycles through three values |
| `settings.h` | One more option bit, for example `BIT_UI_EMCOM_MONO`, from the free bits in bank 0 or 1. No settings layout change, so no settings version bump |
| `languages/*.h` | One new string, "EMCOM mono" (MaxLen 16). English as the fallback, Polish "EMCOM mono" |

**Checks:**

* Run screenshots of the main screen in RX, TX, SCAN, low battery and SOS
  through a deuteranopia/protanopia simulator. Every state must be
  distinguishable.
* Use `tools/opengd77_screenshot.py`, or the web tool once it exists, for
  before and after shots for the guide.
* Day and night both work. A custom saved theme still wins in menus, as it
  does now.

**Also:** the band scope and waterfall colour map, and the SSTV and APRS radar
colours. Check whether they also need a mono map (a grey ramp). Decide this
during the plan.

## SSTV++ (in progress, #19, GitHub issue #13)

Asked by SP9TKN: pick the SSTV frequency (ISS uses 2 m and 70 cm), keep the pictures and download them, Doppler correction.

1. **Frequency on the SSTV screen.** UP / DOWN step through presets (145.800, 437.800, 435.000-band sats, plus the channel / VFO the screen was opened from). The tuned frequency is shown in the status line. Leaving restores the original frequency.
2. **Doppler.** Opened from the satellite screen, SSTV keeps the chosen satellite and retunes RX once a second with the satellite maths (`satellite.c`), as the satellite screen does. On 2 m FM the gain is small (±3.5 kHz); on 70 cm (±10 kHz) it keeps the carrier inside the filter. In FM Doppler does not shift the SSTV tones, it only costs SNR.
3. **Pictures in flash.** A finished picture (160 x 120, RGB565, 38 400 B) goes into a ring of slots carved from the end of the GPS NMEA log, with a small header (UTC, mode, frequency). The web tools get an *SSTV* tab with a gallery and PNG export, read over the CPS link like the coverage map.

## UI refresh (#20)

The EMCOM styles (EMCOM / mono / contr, day and night) cover more screens, one stage at a time.
Classic stays as it is. Mock-ups: `docs/screens/render_refresh.py`.

**Stage 1 (done, 2026-11-22):**

1. **TX view** (`uiEmcom.c`): elapsed time in 16x32, power level in 8x16, a TOT bar with the
   remaining time (the last 10 s: the bar counts these seconds only, hatched, countdown in an
   inverse chip in mono / contr), TX tone / bandwidth or TG / CC / TS + contact, the keys usable
   while transmitting (`SK2` 1750, `0-9` DTMF, `SK1` last heard). TX stop screens (RX only, out of
   band, time-out) and the `APRS Tx` banner of a PTT / timed beacon follow the style.
   Still classic while transmitting: VFO sweep / dual watch / scan limits (their own classic
   screen), the hotspot screen. The satellite TX screen and the TX last heard list were already
   styled.
2. **APRS messages** (`menuAPRSRx.c`): two-line rows (`IN` / `OUT` / `BLN` chip, callsign,
   distance, UTC time; ACK state as a symbol + word: check `ACK`, hourglass `WAIT n`, cross
   `NO ACK` / `REJ` / `NOT SENT`); `*` opens a read view (sender in 8x16, time, distance /
   bearing, the whole text word-wrapped, `UP` / `DOWN` to the next message, `GREEN` reply).
3. **Message boxes and notifications** (`uiMessageBox.c`, `uiNotification.c`): one card with the
   kind as a word and a glyph shape (info disc, warning triangle, error square, question
   diamond), long lines wrapped; squelch / volume / power as a value card. Kind: `UI_ALERT_*`,
   from the buttons / notification ID or given by the caller (`uiNotificationShowKind()`).

**Stage 2 (done, 2026-11-22):** the field tool screens. Shared helpers in `uiEmcom.c`
(`emcomUiTitle` with a counter, `emcomUiStrip`, `emcomUiChip` solid / outline, `emcomUiText`
bold in contr, `emcomUiKeyValue`, `emcomUiKeyBar`, `emcomUiConfirmBar`), menu theme items only, no
static data. Mock-ups: `docs/screens/render_tools.py` (`tools_*.png`, one row per style). Keys
unchanged; the clear confirmations (`0`, then `GREEN` on its release) show `CLEAR ...?` +
`GRN YES  ANY NO` in the key bar.

1. **PACE**: strip with the slot the radio is on (`ON P` + role, `OFF PLAN`, `VFO`) and `n/4 SET`;
   two-line rows: letter block, channel name, frequency / mode, role, state chip `ON` (solid),
   `EMPTY` / `DELETED` (outlined).
2. **Radio check**: strip `RX` / `IDLE`, S units, dBm, length of the over; last report in 16x32
   with a rating chip `OK` / `WEAK` / `POOR`, words, details; older reports as rows with the
   same chips.
3. **Event log**: strip with the full date and type of the selected entry; rows of time / type
   chip / text; the whole selected text above the key bar (8x16 in contr when it fits).
4. **Coverage**: strip state chip (`REC` / `NO FIX` / `STOP` / `FULL`), interval, memory used;
   points in the title; the level in 16x32 with `GOOD` / `WEAK` / `GAP`; with the shape cues weak
   samples are striped and gaps only a tick on the graph; session min / max, GPS locator.
5. **Position**: strip source chip (`GPS` / `MANUAL` / `NO POS`), fix, satellites, HDOP; UTC in
   the title; MGRS in 8x16, then UTM, degrees, locator / altitude, sun, waypoint. Contr: MGRS
   easting and northing in 16x32 (one per line, `E` / `N`), locator, altitude, waypoint beside.
6. **SSTV**: idle screen with the saved pictures in the title, strip (state, `DOPPLER`, tone /
   VIS, level), the frequency in 8x16. Over a picture only the 8 px bar above it (the picture is
   never covered): state, mode, frequency or `DOPPLER` (when it fits), `row/120`, then
   `SAVED n/8`; the progress as a block filling up behind `row/120`.

**Next stages (ideas):** PIN code entry, bearing notification, channel details / VFO special
modes, contact list and the quick menus in the EMCOM card look.

## Net control (done, #21)

**Done (2026-11-22).** `functions/netRoster.c` (pure: roster, quiet alarm, QSO text; host test
`tests/aprs_rx/test_net.c`), `functions/netControl.c` (state + `netTick()` in the main loop, once
per second), `user_interface/menuNetControl.c` (screen, EMCOM + classic), mock-ups
`tools_net.png` / `field_net.png`.

* **Event log format (shared with the web tools):** types `EVENT_LOG_QSO = 10` (`QSO`) and
  `EVENT_LOG_NET = 11` (`NET`). QSO text `<STATION> <REPORT>[ OK|RPT|NIL]` (station truncated to
  fit 19 chars): `SP7ABC-9 59`, `2601234 59`, `FM#3 59`. NET text `START[ <channel>]`,
  `IN <STATION>`, `QUIET <STATION>`, `END <n>`. Record time = event time (UTC, 0 without a clock).
* **Roster:** 18 x 18 B in RAM (key = DMR ID / APRS callsign hash / FM number, check-in and last
  heard in seconds since the start, 9 char name, alarm flag); 392 B of new RAM for the whole
  feature (incl. the menu tables). Stations beyond 18 are not added. Times are 16 bit seconds,
  so a net longer than 18 h stops updating.
* **Keys:** `GREEN` start / QSO 59 / FM row heard now, `*` FM check-in, `#` quiet limit
  10 / 15 / 30 / 5 min (RAM only: no spare settings bit in bank 0), `0` + `GREEN` end, `RED` leave.
  Last heard: `*` = QSO 59.
* The main menu tile has a fixed English name (`menuItemName()`, `MENU_STRING_NET_CONTROL`), no
  language string.
* **Quickkeys:** menu IDs >= 32 (Position, Coverage, Radio check, Net control) do not fit the 5 bit
  `QUICKKEY_MENUID`; they are `QUICKKEY_ACTION_MENU_*` actions now (`saveQuickkeyMenuIndex()` stores
  them so too), opened from the channel / VFO screen only. `QUICKKEY_MENUVALUE_CHECKED()` refuses an
  ID >= 32 in the choice table at compile time. Old slots that wrapped to menus 0..2 still open
  those (valid) menus.
* APRS objects / items (`aprsRxStation_t.isObject`) are not checked in.

**Ideas:** other reports than 59 (`OK` / `RPT` / `NIL` codes picked on the radio), the roster
saved in flash across a power cycle, a net summary export, FM check-in by DTMF ANI / CTCSS.


## Blackout and battery time left (done, #22, #23)

**Done (2026-11-22).** Mock-ups `field_blackout.png`, `field_battery.png` (`render_blackout.py`).

* **Blackout** (`functions/blackout.c`): `SK1 + 5` on the channel / VFO screen (`SK1` + 1/2/4/9 were
  taken, 0/3 are sweep keys; 5..8 were free) or `QUICKKEY_ACTION_BLACKOUT` (appended, the stored
  action numbers of the others are unchanged). The backlight mode / timeout / day, night and "off"
  levels are swapped for `BUTTONS` / user timeout (10 s for *never*) / 5 % / 0 % in RAM;
  `settingsSaveSettings()` writes a stack copy of the settings with the user values
  (`blackoutSettingsForSave()`; the live values stay dark, so the backlight timer never sees full
  brightness), so the radio always boots normal; a backlight value changed in *Options → Display*
  during the blackout (compared with the values when the menu was opened, so e.g. 5 % or *Buttons*
  count too) and a settings reset to the defaults are adopted as the user values. Beeps (`soundSetMelody()`, except the transmitted 1750 Hz / DTMF), voice prompts
  (`voicePromptsInit()` / `Append*()` / `Play()`: nothing is queued while active, the queue is
  cleared when the blackout is turned off) and the LEDs (`LedWrite()`, `LEDsState` still tracks RX for the EMCOM
  state) are gated by `blackoutActive()`. 7 B of RAM.
* **Battery time left** (`functions/batteryRuntime.c`, pure, host test `tests/aprs_rx/test_batt.c`):
  the existing average is truncated to 0.1 V (one step per ~35 min at a typical drain), so
  `adc.c` keeps a second, untruncated average. Every 150 s the highest voltage outside TX (and 5 s
  after it) is kept, 12 slots = 30 min; least squares slope, smoothed (1/4 per slot), mapped
  through the linear 6.4..8.2 V curve of the percentage; the minutes shown count down and are pulled
  1/4 of the way to each new estimate; clamp 99 h; `--` below 6 slots, rising voltage, a +0.08 V
  step (charger) or a -0.10 V step (charger removed). 65 B of RAM.

**Ideas:** a classic-style indicator (the header has no room), blackout in the event log, a
non-linear Li-ion curve for the percentage and the estimate, the estimate in the voice prompt.


## Saved waypoints and sked (done, #8b, #24)

**Done (2026-11-22).** Mock-ups `tools_waypoints.png`, `tools_sked.png`, `field_sked_cards.png`,
`field_waypoints.png`, `field_sked.png` (`render_waypoints_sked.py`), PACE strip in `tools_pace.png`.

* **Field data, sector 0** (`fieldData.h`): `0x800` 8 waypoints x 24 B (magic `WWP1`, lat / lon
  i32 deg * 1e6, name 8 chars, flags GPS / keypad / APRS), `0x900` 4 skeds x 16 B (magic `SKD1`,
  `skedTime_t`, stored channel). PACE (`0x000`) and the replies (`0x400`) do not move; each record
  carries its own magic, as sector 0 may still hold old GPS log bytes (only ever rewritten by the
  read-modify-write of `SPI_Flash_write()`). `_Static_assert`s for every boundary.
* **Saved waypoints** (`menuWaypoints.c`, `MENU_WAYPOINTS`, quickkey action *Waypoints*, GPS
  waypoint page `*`): rows name / distance / compass point / arrow (north up), `--` without a fix,
  `>` marks the current target. `GREEN` sets the GPS waypoint and opens the GPS waypoint page,
  `*` saves the fix (auto name `WP<n>`), `#` the keypad lat / lon entry of the GPS page (now shared:
  `gpsWaypointEdit*()`), `RIGHT` renames (`uiTextEdit`, A-Z 0-9 -), `0` + `GREEN` deletes. APRS RX:
  long `*` on a station sets the waypoint and saves it under the callsign. State in the CCM scratch;
  the selection is the menu's `currentItemIndex`.
* **Sked** (`functions/skedMath.c` pure + host test `tests/aprs_rx/test_sked.c`; `functions/sked.c`;
  `menuSked.c`, `MENU_SKED`, PACE `#`, quickkey action *Sked*): period 15 / 20 / 30 min, 1 / 2 / 3 /
  4 / 6 / 8 / 12 h or a day (divides 1440, so no midnight special case), offset from 00:00 UTC,
  length 1..60 min, tune none / PACE slot / stored channel, alert on / off. `skedTick()` reads the
  flash once per UTC minute (the clock must be set, 2024+): `SKED in 1 min` warning card + beep,
  `SKED n NOW` card + beep + `SYS SKED n`; `GREEN` on that card (`NOTIFICATION_ID_USER_SKED`, on the
  channel / VFO screen) calls `paceActivateChannel()` — never automatically. No card on the TX / CPS
  screens or while transmitting (still logged); no beep in a blackout. The PACE strip / classic key
  bar count down to the next sked (redrawn once per second).
* **ROM**: the code before the fixed codec address (0x0807537C) was full; the field tool screens
  (`menuSked`, `menuWaypoints`, `sked`, `skedMath`, net control, PACE, coverage, radio check,
  position, event log screen) go to a new `.upper_text` section after the codec
  (`STM32F405VGTX_FLASH.ld`), ~21 KB left before the codec.
* RAM: 48 B (two menu table entries, the sked tick state).

**Ideas:** a contact / channel location as waypoint target, waypoint names from the keypad entry,
sked per weekday, sked TX reminder with a callsign / net name, skeds in the event log export.
