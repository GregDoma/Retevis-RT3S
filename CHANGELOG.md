# Zmiany: wersja M0SQQ

**Autor modyfikacji: Greg M0SQQ** ([GregDoma/Retevis-RT3S](https://github.com/GregDoma/Retevis-RT3S)).

Baza: fork SP7TQ ([sptq/opengd77-rt3s-experiments](https://github.com/sptq/opengd77-rt3s-experiments)), wydanie
R20260131-SP7TQ.8 (commit `ad10e44`, 2026-11-27), oparty na forku EA4IPW i OpenGD77 R20260131. Historia zmian SP7TQ jest w historii gita
tego repo oraz w jego repozytorium. Tagi wersji: `R<RRRRMMDD>-SP7TQ.<N>-M0SQQ.<M>`.

## Niewydane

### Baza podniesiona do SP7TQ.8 (z `a2471d3`, 40 commitów SP7TQ)

Pełny opis w [CHANGELOG SP7TQ](https://github.com/sptq/opengd77-rt3s-experiments/blob/main/CHANGELOG.md). Najważniejsze:

- SP7TQ.6: **DMR SMS** (odbiór i wysyłanie SMS-ów jawnym tekstem, skrzynka w SPI flash, usługi BrandMeister 262993/262994;
  ścieżka danych z dondch/OpenGD77-AES256 bez szyfrowania), ekran **Dekodery** (DTMF, CW, nowy dekoder selektywny STQC,
  tylko odbiór), **zegar** Zulu/lokalny/oba z ustawianiem z klawiatury, na ekranie RSSI `UP`/`DOWN` zmieniają squelch FM,
  poprawki: blokada klawiatury w stylu EMCOM (#14), krok VFO po przejściu kanał → VFO (#20). Sterta FreeRTOS 20 → 16 KB,
  ekrany SMS i Dekodery za kodekiem.
- SP7TQ.7: zawieszanie radia przy komunikatach głosowych na nowych kafelkach menu (#21).
- SP7TQ.8: pauza zapisu dziennika zdarzeń (`#`, #32), APRS: wiadomości przez IGate z usług internetowych (#34), stały podgląd
  ramki (#35), wyłączenie nasłuchu po długim RED (#36), DMR SMS: `1` zaczyna nową wiadomość (#40), pasek EMCOM zawsze
  pokazuje baterię, powiadomienie squelch w krokach 5 % (PR #30 desertblade).

### Autor modyfikacji na radiu (PR #6)

- `Greg M0SQQ` w podziękowaniach (*Firmware info*, `DOWN`), `Mod: M0SQQ` na stronie szczegółów buildu,
  `OpenGD77 <commit> M0SQQ` na pasku wersji ekranu startowego EMCOM (`FORK_MOD_AUTHOR` w `uiGlobals.h`).
  Kredyty OpenGD77 i SP7TQ bez zmian. CI przyjmuje tagi `R<RRRRMMDD>-SP7TQ.<N>-M0SQQ.<M>`.

### Porządki w repo

- Nowe `README.md` (po polsku): pochodzenie kodu i autorzy, budowanie trzech wariantów (RT3S, RT3S 10 W, DM-1701),
  wgrywanie z kodekiem z firmware-dawcy, aktualizacja z SP7TQ przez `git merge`, licencja.
- Opis funkcji SP7TQ przeniesiony z jego README do `docs/FEATURES.md` (tekst SP7TQ bez zrzutów ekranu).
- Usunięte pliki SP7TQ niepotrzebne do budowania i używania firmware: zrzuty ekranów i skrypty do ich renderowania
  (`docs/screens/`, 1,2 MB), notatki robocze `PLANS/`, jego `CHANGELOG.md`, szablon zgłoszeń `.github/ISSUE_TEMPLATE/`.
- Usunięte niedziałające skrypty: `prepare` i `prepare.bat` (wołały nieistniejące `codec_cleaner.Linux`/`.exe`;
  Makefile używa `codec_cleaner.py`), `tools/package_bin_with_languages/package.bat` (katalogi, których nie ma w repo),
  `MDUV380_firmware/tools/FirmwareLoader` (wrapper STM32CubeIDE; Makefile woła loader bezpośrednio).
- Kod firmware bez zmian. Wszystkie platformy zostają, w tym RT84 / DM-1701.
