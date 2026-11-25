# Zasady pracy w repozytorium Retevis-RT3S

Firmware dla Retevis RT3S / TYT MD-UV380 oparty na forku OpenGD77 SP7TQ
(https://github.com/sptq/opengd77-rt3s-experiments), baza OpenGD77 R20260131.
Modyfikacje w tym repozytorium: **Greg M0SQQ** (GregDoma).

## Zasady
- Odpowiadaj po polsku, krótko i konkretnie. Odbiorcą jest doświadczony elektronik, więc niczego nie upraszczaj.
- Pracuj profesjonalnie: podawaj tylko fakty sprawdzone w kodzie, w wyniku buildu albo w testach. Nie zgaduj i nie wymyślaj. Wszystko, czego nie zweryfikowano (np. na radiu), oznacz wprost jako niesprawdzone.
- Każda zmiana idzie na osobnej gałęzi i trafia do `main` wyłącznie przez Pull Request.
- Nie usuwaj kodu ani zmian upstreamu bez wyraźnej potrzeby. Własne zmiany rób tak, żeby dało się je łatwo scalić z nowszym SP7TQ.
- Znak Grega to **M0SQQ**. Zawsze podawaj go jako autora modyfikacji: README, CHANGELOG, nagłówki dokumentów w `docs/`, opisy PR, narzędzia (`--version`, okno „O programie”, README) i firmware (kredyty, ekran informacji o wersji, pasek wersji na ekranie startowym, makro `FORK_MOD_AUTHOR`). Kredyty OpenGD77 i SP7TQ zostają bez zmian.
- Tagi wydań z tego repozytorium: `R<YYYYMMDD>-SP7TQ.<N>-M0SQQ.<M>`.
- Licencja OpenGD77 (`license.txt`) zabrania użytku komercyjnego.

## Remote'y
- `origin`: https://github.com/GregDoma/Retevis-RT3S
- `upstream`: https://github.com/sptq/opengd77-rt3s-experiments

Synchronizacja z SP7TQ odbywa się na gałęzi: `git fetch upstream && git merge upstream/main`, a potem przez PR.

## Budowanie
Wymagane pakiety: `gcc-arm-none-eabi`, `libnewlib-arm-none-eabi`, `make`, `python3`.

```sh
cd MDUV380_firmware
make all PLATFORM=MDUV380                            # RT3S / MD-UV380 -> build/OpenGD77_MDUV380.bin
make all PLATFORM=MDUV380 VARIANT=UV380_PLUS_10W     # wariant 10 W
make -C tests/aprs_rx                                # testy hosta (APRS/DTMF/CW/SSTV/AGC)
```

`make build` / `make build-all` budują w kontenerze (domyślnie podman, z dockerem: `CONTAINER_ENGINE=docker`).
Kodek AMBE nie jest częścią repozytorium. Doklejany jest przy flashowaniu z oryginalnego firmware (dawcy).
