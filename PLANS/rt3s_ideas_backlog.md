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
| 8b | Waypoint: persist in settings, pick a contact/channel location as target | *idea* | |
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

