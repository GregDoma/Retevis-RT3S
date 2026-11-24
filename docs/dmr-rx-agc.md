# DMR receive AGC

The DMR receive AGC uses the tuning profile adapted from TA8BMO's OpenGD77
work: its target curve, thresholds, median window, rise/release response and
peak limiter are attributed in
`MDUV380_firmware/application/include/functions/dmrRxAgc.h` and
`MDUV380_firmware/application/source/functions/dmrRxAgc.c`. Station ownership, PCM bypass,
interrupt synchronization and host tests are maintained in this fork.

## Settings and upgrades

The stored setting is unchanged:

| Stored value | Menu value |
| ---: | ---: |
| 0 | Off |
| 1 | -9 dB |
| 2 | -6 dB |
| 3 | -3 dB |
| 4 | 0 dB |
| 5 | +3 dB |
| 6 | +6 dB |
| 7 | +9 dB |
| 8 | +12 dB |

AGC Off remains unchanged. Existing enabled settings retain their numeric
value, but the new TA8BMO-derived target curve means their gain behavior is not
the old algorithm's behavior. Values above 8 left by a legacy firmware are
clamped to 8 when settings load. No persistent structure or storage magic
number migration is performed. Recalibrate the preferred AGC setting after an
upgrade using a normal, loud and quiet station.

The factory default remains AGC Off (`dmrRxAGC = 0`), which is intentional:
receivers retain their previous unprocessed-audio behavior until the user
enables AGC.

## Ownership and audio buffers

The last-heard list assigns a monotonically increasing `agcStationToken` to a
new station or recycled entry. The active AGC owner is published with the
explicit list entry pointer, DMR ID and token; pointer equality alone is not an
identity guarantee. List reset invalidates the owner before rebuilding the
list. Station changes reset only the transient median history and output ramp,
while a recalled `rxAGCGain` supplies the learned peak for a known station.

`soundRefillData()` snapshots the owner before processing. If ownership is
unknown or changes during processing, it uses a local initialized state and a
bounded peak limiter; it does not commit median history, gain or learned peak
data. It does not flush queued decoder audio. The existing ring-buffer
lifecycle continues to drain queued PCM in order, so a transition can produce
one bounded buffer from the fallback path without introducing an indiscriminate
audio gap. An ordinary refill underrun preserves the learned station level and
clears only transient median history.

The disabled-AGC and voice-prompt branches copy PCM bytes directly. They do not
apply gain, interpolation or saturation, including for `INT16_MIN`.

## Interrupt and real-time constraints

On the RT3S target, `Core/Src/main.c` assigns DMA1 Stream 5 (I2S3_EXT_TX) NVIC
priority 6. `Core/Inc/FreeRTOSConfig.h` sets
`configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` to 5, so a task critical
section masks this DMA interrupt on this target. Ownership publication and
station reset therefore use a short bounded critical section only; no SPI,
delay, allocation or other blocking operation is added to the I2S refill path.
The sequence field also makes an inconsistent owner publication fail closed if
the helper is used outside that expected masked transition.

The top-level firmware Makefile discovers every
`application/source/**/*.c`, so `dmrRxAgc.c` is included in all platform
variants. The host test Makefile compiles the extracted module directly.

## Quiet-station limitation

An unknown station is qualified only when a peak is greater than 500. Sustained
peaks from 201 through 500 remain at unity gain and do not initialize the
learned level. This is preserved tuning, not a hardware measurement: no
HRC6000 audio capture was available to justify changing the threshold. The
host regression test records the boundary, and the physical procedure below
must specifically exercise quiet speech in this range.

## RT3S physical validation

Record the firmware version and back up the codeplug before flashing. With the
radio connected to a dummy load or suitable safe RF setup:

1. Set volume to its minimum audible level. Test AGC Off, then each of the
   eight settings with a normal station, a loud station and a quiet station.
2. Check unknown-station startup, a station switch in both loud-to-quiet and
   quiet-to-loud directions, rapid last-heard changes, and a recycled
   last-heard entry. Listen for clipping, sudden gain jumps or loss of speech.
3. Interrupt audio mid-transmission and allow an underrun/recovery. Repeat
   with voice prompts, rapid key beeps, melody replacement/cancellation and
   canceling the sound menu.
4. Receive APRS, then transmit and receive DMR. Confirm no click, mute or
   incorrect audio path appears at the APRS/I2S handoff.
5. Upgrade over an existing saved settings image. Confirm AGC Off remains Off,
   legacy values above 8 load as +12 dB, and enabled values can be
   recalibrated without changing the volume setting.

Automated host tests cover the AGC state machine, limiter, PCM helper,
ownership identities and deterministic boundaries. They do not replace this
radio procedure or prove hardware audio quality.
