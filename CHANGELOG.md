# Zmiany: wersja M0SQQ

**Autor modyfikacji: Greg M0SQQ** ([GregDoma/Retevis-RT3S](https://github.com/GregDoma/Retevis-RT3S)).

Baza: fork SP7TQ ([sptq/opengd77-rt3s-experiments](https://github.com/sptq/opengd77-rt3s-experiments)), commit `a2471d3`
(R20260131-SP7TQ.5 + późniejsze zmiany), oparty na forku EA4IPW i OpenGD77 R20260131. Historia zmian SP7TQ jest w historii gita
tego repo oraz w jego repozytorium. Tagi wersji: `R<RRRRMMDD>-SP7TQ.<N>-M0SQQ.<M>`.

## Niewydane

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
