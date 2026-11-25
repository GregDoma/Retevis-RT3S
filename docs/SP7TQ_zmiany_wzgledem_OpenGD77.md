# Zmiany SP7TQ względem OpenGD77 R20260131 — przegląd

Opracowanie dla: **Greg M0SQQ**, repozytorium [GregDoma/Retevis-RT3S](https://github.com/GregDoma/Retevis-RT3S).
Kredyty: OpenGD77 (VK3KYY, F1RMB, DG4KLU i inni), fork EA4IPW, fork SP7TQ (Marcin SP7TQ).

Stan na 2026-11-25, `main` = `a2471d3` (SP7TQ.5 + 57 commitów bez wydania).
Raport powstał wyłącznie z analizy kodu i historii git, kompilacji i testów hosta.
Na radiu nic nie było sprawdzane. Kodu firmware nie zmieniano.

## 1. Pochodzenie i zakres

| Etap | Commity | Autor | Daty |
|---|---|---|---|
| OpenGD77 R20260131 (import źródeł) | `0b81ffd` | EA4IPW | 2026-05-18 |
| Fork EA4IPW (build, codec, custom data, band scope) | 21 | Miguel Angel Ajo Pelayo | 2026-05-18 … 06-09 |
| SP7TQ, tagi EA4IPW.3 i SP7TQ.1 … SP7TQ.5 | 103 | Marcin Grochulski | 2026-11-18 … 11-22 |
| Po SP7TQ.5, bez tagu i bez wpisu w CHANGELOG | 57 | Ben Williams (51), SP7TQ (6, w tym merge) | 2026-11-23 … 11-25 |

Różnica `0b81ffd..a2471d3`: 181 commitów, 256 plików (177 dodanych, 76 zmienionych, 3 usunięte), +32 607 / −955 linii.

**Plik `firmware/OpenGD77_MDUV380_RT3S_SP7TQ.5_build-test.bin` w projekcie nie jest wydaniem SP7TQ.5.**
Zawiera ciąg wersji `OpenGD77 a2471d3`, czyli został zbudowany z obecnego `main` (SP7TQ.5 + 57 commitów,
w tym nowe AGC DMR opisane w pkt 4). Ma ten sam rozmiar (839 128 B) co mój build z `a2471d3`.
Bajty się różnią (22 817 pozycji, przesunięcia adresów o 2–4 B), co wskazuje na inną wersję kompilatora.
Nie da się tego potwierdzić bez informacji, jakim toolchainem zbudowano plik.

## 2. Czego fork NIE zmienia (ważne dla problemu z node'em FM)

Pliki toru radiowego są identyczne z bazą OpenGD77 (sprawdzone `git diff 0b81ffd a2471d3`):

* `functions/trx.c` (squelch, CTCSS/DCS, przełączanie RX/TX),
* `hardware/AT1846S.c`, `hardware/HR-C6000.c`, `hardware/radioHardwareInterface.c`,
* `functions/rxPowerSaving.c`, cały katalog `Core/`.

Wniosek: obsługa squelcha i tonów na kanale analogowym jest taka sama jak w oficjalnym OpenGD77.
Zgadza się to z obserwacją, że na oficjalnym OpenGD77 objaw jest identyczny. Fork zmienia w torze FM tylko to:

* przechwytywanie audio RX z HR-C6000 przez I2S dla dekodera APRS / DTMF / CW / SSTV (gdy I2S jest wolne),
* rysowanie ekranu (EMCOM jest domyślnym stylem),
* zachowanie klawiszy podczas skanowania (pkt 3.3).

## 3. Zmiany w istniejącym kodzie OpenGD77 (największe ryzyko)

To są zmiany w plikach bazowych, a nie nowe moduły. W pierwszej kolejności warto je sprawdzić na radiu.

### 3.1 Wywołania kodeka AMBE (EA4IPW, `969bd3e`)
`dmr_codec/codec_interface.c`: argumenty dla `AMBE_DECODE` / `AMBE_ENCODE` / `AMBE_ENCODE_ECC` są teraz
podawane przez zmienne rejestrowe z ograniczeniami wejść/wyjść asm. Wcześniej nowszy GCC mógł usunąć przypisania do
r0–r2. Zmiana jest poprawna, ale dotyczy bezpośrednio działania DMR RX i TX.
`codec_bin.S` ma sekcję oznaczoną jako `"a", %progbits`, a linker umieszcza ją pod stałym adresem `0x0807537C`.

### 3.2 Audio, I2S, AGC DMR
* `interfaces/i2s.c`: nowy tryb ciągłego DMA I2S dla odbioru APRS (`i2sAprsRxStart/Stop`).
  Każde `I2SStartDMA()` (dźwięk, prompt, audio DMR) najpierw go zatrzymuje. Start trybu APRS czeka na zegar WS z limitem
  (`I2SStartDMA()` nadal czeka bez limitu, jak w oryginale).
* `functions/sound.c` + nowy `functions/dmrRxAgc.c` (Ben Williams, **po SP7TQ.5**, bez wydania):
  całkowicie nowy algorytm AGC DMR RX (krzywa TA8BMO, mediana, limiter, „właściciel” = stacja z last heard,
  kwarantanna buforów przy zmianie stacji). Wartości ustawienia 1..8 oznaczają teraz −9…+12 dB, więc
  **zapisane ustawienie AGC nie brzmi tak samo jak wcześniej**. Wartości >8 są obcinane do 8. Domyślnie AGC = Off.
  Autor sam opisuje procedurę walidacji na radiu w `docs/dmr-rx-agc.md`, która nie została jeszcze wykonana.
* Dodatkowo synchronizacja melodii (`melodyGeneration`) i zatrzymanie ponawiania SPI dla nieaktualnej melodii.

### 3.3 Skanowanie kanałów (`uiChannelMode.c`, SP7TQ.5, *untested*)
* `▲` działa także w stanie `SCAN_STATE_SHORT_PAUSED` (wcześniej nic nie robił).
* `▼` na sygnale zatrzymuje skan zamiast odwracać kierunek.
* Jeśli poprzedni tick pominął kanał nuisance-deleted, skan wyszukuje kolejny (wcześniej mógł utknąć).

### 3.4 Codeplug i pamięć SPI flash
* **Automatyczne formatowanie obszaru custom data (64 KB) przy starcie** (EA4IPW `abb941c`): jeśli na
  `FLASH_ADDRESS_OFFSET` nie ma sygnatury `OpenGD77`, 16 sektorów jest kasowanych bez pytania.
  W tym obszarze są motywy, obraz startowy, własny beep i TLE satelitów. Jest też menu ręcznego resetu.
* `codeplugGetLastUsedChannelNumberInCurrentZone()`: ochrona przed zawieszeniem po skróceniu strefy w CPS
  (SP7TQ.2). Przy nieprawidłowym indeksie fork wraca do kanału 1 i zapisuje ten wybór.
* `codeplugAPRSSetDataForIndex()`: **radio zapisuje do codeplugu (EEPROM)** komentarz APRS edytowany na radiu (SP7TQ.4).
* „Use location” kanału nie jest już flagą. Wynika z tego, czy współrzędne są niezerowe (EA4IPW `959d1c7`).
  Skutek: kanały ze współrzędnymi są brane do sortowania po odległości nawet bez flagi.
* Nowy podział ostatnich 2 MB SPI flash (16 MB). Sprawdziłem, że obszary są ciągłe i na siebie nie nachodzą:

| Adres | Rozmiar | Zawartość |
|---|---|---|
| 14 MB | 64 KB | dziennik zdarzeń (2048 wpisów) |
| 14 MB + 64 KB | 1600 KB | log NMEA GPS (oryginalnie 2 MB) |
| 14 MB + 1664 KB | 320 KB | obrazy SSTV (8 × 40 KB) |
| 16 MB − 64 KB | 64 KB | dane terenowe: PACE, odpowiedzi APRS, waypointy, sked |

  Log NMEA zapisany przez wcześniejszy firmware w tym obszarze zostanie częściowo nadpisany.

### 3.5 Ustawienia (`settings.h/.c`)
* Bity wcześniej nieużywane dostały nowe znaczenie: bit 2 → EMCOM mono, bit 10 → EMCOM contrast,
  bit 20 → styl klasyczny, bit 29 → APRS RX w tle. Struktura i magic nie zmieniły się, więc nie ma resetu ustawień.
  Jeśli poprzedni firmware zostawił w tych bitach jedynki, po aktualizacji zmieni się styl ekranu.
* Domyślnie włączony waterfall w VFO sweep (bit 15 `vfoSweepSettings`).
* Zapis ustawień w trybie blackout idzie z kopii z wartościami podświetlenia użytkownika.

### 3.6 Pętla główna (`applicationMain.c`)
Nowe ticki co obieg pętli: `aprsRxTick`, `emcomSosTick`, `eventLogTick`, `netTick`, `skedTick`.
Doszła obsługa akcji quickkey (`0x7Fxx`) oraz przechwytywanie GREEN dla karty SKED.

### 3.7 Inne zmiany w plikach bazowych
`voicePrompts.c`, `Leds.c`, `sound.c` (blokada w trybie blackout), `batteryAndPowerManagement.c` / `adc.c`
(dokładniejsza średnia napięcia do szacowania czasu pracy), `gps.c` (TTFF, trip computer),
`menuChannelDetails.c` (moc w VFO), `uiLockScreen.c`, `uiVFOMode.c`, `HX8353E_display.c` (styl płaski),
`hotspot.c`, `usb_com.c` (zrzuty ekranu), 22 pliki językowe (nowe teksty, `.gla` w wersji 8,
starsze paczki językowe się nie wczytają).

## 4. Zmiany po SP7TQ.5 (są w Twoim .bin, nie ma ich w CHANGELOG)

* `68dfc1d` **Lock screen: naprawa zawieszenia w stylu EMCOM.** W SP7TQ.5 zablokowanie klawiatury/PTT przy
  domyślnym stylu EMCOM zostawiało zamrożony ekran. Twój build ma poprawkę.
* `e219cc1` APRS RX: pasek klawiszy w każdym widoku.
* Ben Williams (PR #17, #23, #25): nowe AGC DMR RX (pkt 3.2) z testami hosta, przywrócone zabezpieczenia
  przekazania I2S między APRS a dźwiękiem, ekrany EMCOM z odległością do stacji DMR/przemiennika (opcjonalnie),
  czas lokalny/UTC w stopce, opcja visual volume, napięcie w pasku statusu, ekran DMR standby, CI z wyborem targetu.

## 5. Funkcje oznaczone przez autora jako nietestowane

Źródła: `CHANGELOG.md` (*untested*), `PLANS/rt3s_ideas_backlog.md` („untested on hardware / on air”), README.
Autor w CHANGELOG pisze, że oznaczone funkcje „kompilują się i przechodzą testy hosta, ale wymagają sprawdzenia w eterze”.

| Wersja | Funkcja | Dotyka kodu bazowego? |
|---|---|---|
| EA4IPW.3 | APRS RX: widok stacji, radar, odbiór w tle (dekodowanie ramek potwierdzone w eterze) | I2S |
| EA4IPW.3 | Fox-hunt na ekranie RSSI | nie |
| EA4IPW.3 | Strona waypointu GPS | nie |
| SP7TQ.1 | Odbiór SSTV („untested on air”) | I2S |
| SP7TQ.4 | Teksty APRS na radiu: komentarz (zapis do codeplugu), odpowiedzi, wiadomości pisane z klawiatury | **codeplug EEPROM** |
| SP7TQ.5 | Zapisane waypointy (8 w SPI flash) | flash |
| SP7TQ.5 | Przypomnienia sked | pętla główna, klawisz GREEN |
| SP7TQ.5 | Blackout / tryb cichy | **dźwięk, prompty, LED, zapis ustawień** |
| SP7TQ.5 | Szacowany czas pracy baterii | ADC |
| SP7TQ.5 | Net control + log QSO | nie |
| SP7TQ.5 | Odświeżony UI EMCOM, etap 1 i 2 (ekran TX, karty TX stopped/blocked, wiadomości APRS, okna komunikatów) | ekran TX |
| SP7TQ.5 | Style dostępności EMCOM mono / contr | bity ustawień |
| SP7TQ.5 | SSTV++ (presety ISS, Doppler, zapis obrazów) | flash, strojenie |
| SP7TQ.5 | **Skan na zajętym kanale** | **`uiChannelMode.c`** |
| SP7TQ.5 | Nowe strony GPS i ekran satelitów w stylu EMCOM | nie |
| SP7TQ.5 | **Beacony SOS na większej liczbie ekranów** | **TX APRS** |
| SP7TQ.5 | Potwierdzanie kasowania (PACE, Radio check, Last heard, Event log, Coverage) | nie |
| po .5 | **Nowe AGC DMR RX** (nie oznaczone, ale autor wymaga walidacji sprzętowej w `docs/dmr-rx-agc.md`) | **`sound.c`** |

## 6. Kompilacja i zasoby (sprawdzone 2026-11-25)

`arm-none-eabi-gcc 13.2.1`, `make all PLATFORM=MDUV380`: build bez błędów. Ostrzeżenia `#warning` występują tylko
w niezmienionych plikach bazowych (`AT1846S.c`, `radioHardwareInterface.c`).
Testy hosta `make -C MDUV380_firmware/tests/aprs_rx`: wszystkie przeszły (APRS, DTMF/CW, SSTV, tekst, net, sked,
bateria, AGC, odległość EMCOM). Testy hosta nie obejmują sprzętu.

| Zasób | Baza (`1555779`, R20260131 + build EA4IPW) | `a2471d3` |
|---|---|---|
| Kod + stałe (text + rodata) | 320 156 B | 439 744 B (+119,6 KB) |
| Wolne flash MCU przed kodekiem AMBE (`0x0807537C`) | ok. 108 KB | **18 176 B** |
| Wolne flash MCU łącznie (limit 976 KB) | — | ok. 156 KB |
| RAM 128 KB: wolne po .data + .bss + rezerwa heap/stack | 616 B | 792 B |
| CCMRAM 64 KB: wolne | 4 612 B | **32 B** |

Uwagi do tabeli:
* Miejsce przed kodekiem jest prawie wyczerpane. Dlatego SP7TQ przeniósł ekrany narzędzi terenowych do sekcji
  `.upper_text` za kodekiem. Nowe moduły trzeba tam dopisywać jawnie w linker scripcie.
* CCMRAM jest pełny. Każdy nowy bufor statyczny musi trafić do zwykłego RAM, a tam też zostało niewiele miejsca.

## 7. Co sprawdzić na radiu w pierwszej kolejności

Przed wgraniem: kopia codeplugu i notatka o obecnej wersji firmware.

1. **FM: kanał i VFO, squelch, CTCSS/DCS, RX-only.** Tor jest niezmieniony, ale to podstawowa funkcja.
   Warto sprawdzić przy APRS RX w tle wyłączonym i włączonym (`SK1+4`), czy audio FM i otwieranie squelcha
   zachowują się tak samo.
2. **DMR RX/TX.** Kodek po zmianie asm (3.1), z AGC = Off, a potem kilka ustawień AGC na głośnej i cichej stacji.
   Szczegółowa procedura jest w `docs/dmr-rx-agc.md`, punkty 1–5.
3. **Przejścia I2S:** odbiór APRS, potem beep/prompt, potem nadawanie i odbiór DMR. Szukamy trzasków, wyciszenia
   i złego toru audio.
4. **Blokada klawiatury/PTT w stylu EMCOM.** Poprawka z `68dfc1d` nie ma jeszcze potwierdzenia na sprzęcie.
5. **Skan kanałów** na zajętym kanale: `▲`, `▼`, pokrętło, RED/PTT. Do tego kanał nuisance-deleted.
6. **Blackout (`SK1+5`):** czy TX, 1750 Hz i DTMF działają, czy po wyłączeniu wracają podświetlenie i dźwięki,
   czy po restarcie zapisane jest podświetlenie użytkownika.
7. **Pierwszy start po wgraniu:** komunikat „Storage init.” oznacza, że obszar custom data został sformatowany.
   Sprawdź motyw i obraz startowy. Po aktualizacji z wersji starszej niż SP7TQ.3 radio raz startuje w VFO.
8. **Strefy skrócone w CPS** (poprawka SP7TQ.2) i ostatni kanał strefy.
9. **APRS → Comment:** edycja zapisuje codeplug, więc potem odczytaj codeplug w CPS i porównaj.
10. Pozostałe funkcje z tabeli w pkt 5 według tego, czego używasz.

## 8. Rozjazdy w dokumentacji

* Brak sekcji CHANGELOG dla 57 commitów po SP7TQ.5 (AGC, poprawka lock screen, opcje EMCOM).
* Nazwa pliku w projekcie sugeruje SP7TQ.5, a w środku jest `a2471d3` (pkt 1).
