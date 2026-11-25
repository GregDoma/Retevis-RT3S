# Firmware OpenGD77 dla Retevis RT3S i DM-1701: wersja M0SQQ

**Autor modyfikacji: Greg M0SQQ** ([GregDoma/Retevis-RT3S](https://github.com/GregDoma/Retevis-RT3S)).

Firmware dla radiotelefonów DMR/FM z mikrokontrolerem STM32F405, układem RF AT1846S i układem DMR HR-C6000:

| Radio | `PLATFORM` | `VARIANT` | Plik wynikowy |
|---|---|---|---|
| Retevis RT3S / TYT MD-UV380 | `MDUV380` | (brak) | `OpenGD77_MDUV380.bin` |
| RT3S / MD-UV380 z modyfikacją 10 W | `MDUV380` | `UV380_PLUS_10W` | `OpenGD77_MDUV380_UV380_PLUS_10W.bin` |
| Retevis RT84 / Baofeng DM-1701 | `RT84_DM1701` | `DM1701` | `OpenGD77_RT84_DM1701_DM1701.bin` |

Nieoficjalna wersja, niezwiązana z projektem OpenGD77. Oficjalny firmware: <https://www.opengd77.com/>.

## Pochodzenie kodu

Repo zachowuje pełną historię projektów, na których się opiera. Dzięki temu zmiany SP7TQ można dociągać zwykłym `git merge`.

1. **OpenGD77 R20260131**, wydanie źródłowe MDUV380/DM1701. Autorzy: Kai DG4KLU (pomysł), Roger VK3KYY (prowadzący),
   Daniel F1RMB, Alex DL4LEX, Colin G4EML i inni. Szczegóły w [`UPSTREAM_README.md`](UPSTREAM_README.md).
2. **Fork EA4IPW** (Miguel Ángel Ajo Pelayo, [mangelajo/opengd77-rt3s-experiments](https://github.com/mangelajo/opengd77-rt3s-experiments)):
   Makefile do budowania poza STM32CubeIDE, poprawki linkera, CI, waterfall VFO.
3. **Fork SP7TQ** (Marcin Grochulski SP7TQ, z udziałem Bena Williamsa,
   [sptq/opengd77-rt3s-experiments](https://github.com/sptq/opengd77-rt3s-experiments)):
   ekran EMCOM, APRS RX, strony GPS, narzędzia polowe, satelity, SSTV, polowanie na lisa i radiosondy, quickkeys.
   Baza tego repo: commit `a2471d3` (R20260131-SP7TQ.5 + późniejsze zmiany).
   Opis funkcji (tekst SP7TQ): [`docs/FEATURES.md`](docs/FEATURES.md).
4. **Modyfikacje Greg M0SQQ** w tym repo: lista w [`CHANGELOG.md`](CHANGELOG.md).

Nagłówki z prawami autorskimi w plikach źródłowych pozostają bez zmian.

## Budowanie

Wymagany kompilator `arm-none-eabi-gcc` z newlib i Python 3. Na Debianie/Ubuntu:

```sh
sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi python3
cd MDUV380_firmware
make all PLATFORM=MDUV380                              # RT3S
make all PLATFORM=MDUV380 VARIANT=UV380_PLUS_10W       # RT3S 10 W
make all PLATFORM=RT84_DM1701 VARIANT=DM1701           # DM-1701 / RT84
```

Plik `.bin` powstaje w `MDUV380_firmware/build/`. Między wariantami wykonaj `make clean`.
`make build` i `make build-all` robią to samo w kontenerze podman/docker (`CONTAINER_ENGINE=docker`).

Testy dekodera APRS na PC: `make -C MDUV380_firmware/tests/aprs_rx`.

CI na GitHubie (`.github/workflows/build.yml`) buduje wszystkie trzy warianty i uruchamia testy przy każdym PR.

## Wgrywanie

Repo **nie zawiera kodeka AMBE**. Program wgrywający dokłada go przy flashowaniu z oryginalnego firmware producenta
(firmware-dawca). Radio w trybie DFU: wyłącz, przytrzymaj SK1, włącz.

```sh
cd MDUV380_firmware/tools
# za pierwszym razem wskaż oficjalny firmware jako źródło kodeka (ścieżka jest zapamiętywana);
# program przyjmuje tylko plik o sumie SHA-256 zapisanej w FW2645_SHA256_Checksum:
python3 opengd77_stm32_firmware_loader.py -m MD-UV380 -s /ścieżka/do/oficjalnego_firmware.bin -f ../build/OpenGD77_MDUV380.bin
python3 opengd77_stm32_firmware_loader.py -m MD-UV380 -f ../build/OpenGD77_MDUV380.bin   # RT3S
python3 opengd77_stm32_firmware_loader.py -m DM-1701  -f ../build/OpenGD77_RT84_DM1701_DM1701.bin
```

## Narzędzia PC

W `MDUV380_firmware/tools/`: zrzut ekranu przez USB (`opengd77_screenshot.py`), odczyt dziennika zdarzeń
(`opengd77_event_log.py`), log GPS do GPX (`opengd77_gps_log_to_gpx.py`), dane pokrycia (`opengd77_coverage.py`),
odbiór SSTV (`opengd77_sstv.py`), obsługa obszaru danych własnych OpenGD77 (`opengd77_custom_data.py`). Opis w [`docs/FEATURES.md`](docs/FEATURES.md).

## Aktualizacja z SP7TQ

```sh
git remote add upstream https://github.com/sptq/opengd77-rt3s-experiments   # raz
git fetch upstream
git switch -c sync-sp7tq main
git merge upstream/main
```

Pliki SP7TQ usunięte w tym repo (`CHANGELOG.md`, `PLANS/`, `docs/screens/`, `.github/ISSUE_TEMPLATE/`, stare skrypty)
dadzą konflikt „modify/delete”, gdy SP7TQ je zmieni. Rozwiązanie: `git rm` tych plików. W `README.md` zostaje wersja M0SQQ
(`git checkout --ours README.md`), a nowe opisy funkcji SP7TQ przenosi się ręcznie do `docs/FEATURES.md`.

## Wersje

Tagi wydań: `R<RRRRMMDD>-SP7TQ.<N>-M0SQQ.<M>`, np. `R20260131-SP7TQ.5-M0SQQ.1`: data bazy OpenGD77,
numer wydania SP7TQ, na którym opiera się wersja, i kolejny numer wersji M0SQQ.

## Licencja: tylko użytek niekomercyjny

Kod OpenGD77 jest na zmodyfikowanej licencji BSD-3-Clause z zakazem użytku komercyjnego
([`license.txt`](license.txt), [`MDUV380_firmware/tools/license.txt`](MDUV380_firmware/tools/license.txt)):

> 4. Use of this source code or binary releases for commercial purposes is strictly forbidden. This includes, without
> limitation, incorporation in a commercial product or incorporation into a product or project which allows commercial use.

Ograniczenie obowiązuje także to repo. Dołączone biblioteki mają własne licencje (pliki `LICENSE` w katalogach):
CMSIS (Apache-2.0), STM32F4xx HAL (BSD-3-Clause), ST USB Device Library (ST SLA0044), FreeRTOS (MIT), SEGGER RTT (BSD-like).

73, Greg M0SQQ
