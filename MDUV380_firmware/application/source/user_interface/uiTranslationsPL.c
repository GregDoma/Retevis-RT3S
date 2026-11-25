/*
 * GENERATED FILE, DO NOT EDIT: tools/translations/gen_translations.py from tools/translations/pl.tsv
 *
 * Fork (M0SQQ): Polish texts of the fork screens, see TR() in uiLocalisation.h.
 * Sorted by strcmp() of the English text (binary search). Kept after the codec (.upper_text).
 *
 * Use of this source code or binary releases for commercial purposes is
 * strictly forbidden (see license.txt).
 */
#include <stdint.h>
#include <stddef.h>
#include "user_interface/uiLocalisation.h"

#if defined(PLATFORM_MDUV380) || defined(PLATFORM_MD380) || defined(PLATFORM_RT84_DM1701) || defined(PLATFORM_MD2017)
#define TR_SECTION __attribute__((section(".upper_text")))
#else
#define TR_SECTION
#endif

static const char en0[] TR_SECTION = "\nGREEN = TUNE";
static const char pl0[] TR_SECTION = "\nZIEL = USTAW";
static const char en1[] TR_SECTION = "  ENTER RX";
static const char pl1[] TR_SECTION = "  WPISZ RX";
static const char en2[] TR_SECTION = "  ENTER TX";
static const char pl2[] TR_SECTION = "  WPISZ TX";
static const char en3[] TR_SECTION = "  no own pos";
static const char pl3[] TR_SECTION = "  brak w\210. poz";
static const char en4[] TR_SECTION = "  no position";
static const char pl4[] TR_SECTION = "  brak pozycji";
static const char en5[] TR_SECTION = " (no APRS cfg)";
static const char pl5[] TR_SECTION = " (brak konf. APRS)";
static const char en6[] TR_SECTION = "# VIEW  1 MSG  RED EXIT";
static const char pl6[] TR_SECTION = "# WIDOK 1 WIAD CZER WYJ";
static const char en7[] TR_SECTION = "#: enter lat/lon";
static const char pl7[] TR_SECTION = "#: wpisz szer/d\210";
static const char en8[] TR_SECTION = "%d BY %d";
static const char pl8[] TR_SECTION = "%d NA %d";
static const char en9[] TR_SECTION = "%d/%d SAVED";
static const char pl9[] TR_SECTION = "%d/%d ZAPIS.";
static const char en10[] TR_SECTION = "%d/%d SET";
static const char pl10[] TR_SECTION = "%d/%d UST";
static const char en11[] TR_SECTION = "%s  %d BY %d  %d dBm";
static const char pl11[] TR_SECTION = "%s  %d NA %d  %d dBm";
static const char en12[] TR_SECTION = "%s %u.%us AVG %d PK %d";
static const char pl12[] TR_SECTION = "%s %u.%us \214R %d SZC %d";
static const char en13[] TR_SECTION = "%s BY %s";
static const char pl13[] TR_SECTION = "%s NA %s";
static const char en14[] TR_SECTION = "%u H";
static const char pl14[] TR_SECTION = "%u GODZ";
static const char en15[] TR_SECTION = "%u stn";
static const char pl15[] TR_SECTION = "%u st.";
static const char en16[] TR_SECTION = "(bad AX.25 header)";
static const char pl16[] TR_SECTION = "(z\210y nag\210\363wek AX.25)";
static const char en17[] TR_SECTION = "(bad header)";
static const char pl17[] TR_SECTION = "(z\210y nag\210\363wek)";
static const char en18[] TR_SECTION = "(or Options > General).";
static const char pl18[] TR_SECTION = "(lub Opcje > Og\363lne).";
static const char en19[] TR_SECTION = "* FM  # QUIET  0 END";
static const char pl19[] TR_SECTION = "* FM  # CISZA  0 KONIEC";
static const char en20[] TR_SECTION = "* MARK WAYPOINT  RED EXIT";
static const char pl20[] TR_SECTION = "* ZAZNACZ PKT  CZER WYJ";
static const char en21[] TR_SECTION = "* RAW";
static const char pl21[] TR_SECTION = "* SUR";
static const char en22[] TR_SECTION = "* RAW  # FOX  0 RST  9 APRS";
static const char pl22[] TR_SECTION = "* SUR  # LIS  0 RST  9 APRS";
static const char en23[] TR_SECTION = "* RAW  # FOX  9 APRS";
static const char pl23[] TR_SECTION = "* SUR  # LIS  9 APRS";
static const char en24[] TR_SECTION = "* RAW # VIEW 0 CLR RED EXIT";
static const char pl24[] TR_SECTION = "* HEX # WID 0 KAS CZER WYJ";
static const char en25[] TR_SECTION = "* READ  GRN REPLY  1 NEW";
static const char pl25[] TR_SECTION = "* CZYTAJ ZIEL ODP 1 NOWA";
static const char en26[] TR_SECTION = "* SAVE HERE  # LAT/LON";
static const char pl26[] TR_SECTION = "* ZAPISZ TU # SZER/D\207";
static const char en27[] TR_SECTION = "* STORE";
static const char pl27[] TR_SECTION = "* ZAPISZ";
static const char en28[] TR_SECTION = "* adds an FM station.";
static const char pl28[] TR_SECTION = "* dodaje stacj\206 FM.";
static const char en29[] TR_SECTION = "* stores the channel";
static const char pl29[] TR_SECTION = "* zapisuje kana\210";
static const char en30[] TR_SECTION = "*: saved";
static const char pl30[] TR_SECTION = "*: zapisane";
static const char en31[] TR_SECTION = "*: saved list";
static const char pl31[] TR_SECTION = "*: zapisane";
static const char en32[] TR_SECTION = "-- empty";
static const char pl32[] TR_SECTION = "-- pusty";
static const char en33[] TR_SECTION = "-- hex, %u bytes%s --";
static const char pl33[] TR_SECTION = "-- hex, %u bajt\363w%s --";
static const char en34[] TR_SECTION = "0 CLEAR";
static const char pl34[] TR_SECTION = "0 KASUJ";
static const char en35[] TR_SECTION = "0 CLEAR  RED EXIT";
static const char pl35[] TR_SECTION = "0 KASUJ  CZER WYJ";
static const char en36[] TR_SECTION = "0 NEW";
static const char pl36[] TR_SECTION = "0 NOWY";
static const char en37[] TR_SECTION = "0-9 TYPE  SK2+GRN GPS ON";
static const char pl37[] TR_SECTION = "0-9 WPISZ  SK2+ZIEL GPS W\207";
static const char en38[] TR_SECTION = "0-9 TYPE DD.DDDD DDD.DDDD";
static const char pl38[] TR_SECTION = "0-9 WPISZ DD.DDDD DDD.DDDD";
static const char en39[] TR_SECTION = "1-3 QRG  # NEXT  RED LIST";
static const char pl39[] TR_SECTION = "1-3 QRG # NAST CZER LISTA";
static const char en40[] TR_SECTION = "1-3 QRG  9 SSTV  RED LIST";
static const char pl40[] TR_SECTION = "1-3 QRG 9 SSTV CZER LISTA";
static const char en41[] TR_SECTION = "1-4 GO";
static const char pl41[] TR_SECTION = "1-4 ID\217";
static const char en42[] TR_SECTION = "1-4 GO  # SKED  (VFO)";
static const char pl42[] TR_SECTION = "1-4 ID\217  # SKED  (VFO)";
static const char en43[] TR_SECTION = "1-4 GO * STO 0 CLR # SKED";
static const char pl43[] TR_SECTION = "1-4 ID\217 *ZAP 0 KAS #SKED";
static const char en44[] TR_SECTION = "</> PASS  SK2+GRN ALARM";
static const char pl44[] TR_SECTION = "</> PRZELOT SK2+ZIEL ALARM";
static const char en45[] TR_SECTION = "> Type a message...";
static const char pl45[] TR_SECTION = "> Wpisz wiadomo\215\202...";
static const char en46[] TR_SECTION = "? (* to type)";
static const char pl46[] TR_SECTION = "? (* aby wpisa\202)";
static const char en47[] TR_SECTION = "AGE";
static const char pl47[] TR_SECTION = "WIEK";
static const char en48[] TR_SECTION = "AGO";
static const char pl48[] TR_SECTION = "TEMU";
static const char en49[] TR_SECTION = "ALERT";
static const char pl49[] TR_SECTION = "ALARM";
static const char en50[] TR_SECTION = "ALT";
static const char pl50[] TR_SECTION = "WYS";
static const char en51[] TR_SECTION = "ALT %dm";
static const char pl51[] TR_SECTION = "WYS %dm";
static const char en52[] TR_SECTION = "ALTERNATE";
static const char pl52[] TR_SECTION = "ZAPASOWY";
static const char en53[] TR_SECTION = "ANY KEY: STOP";
static const char pl53[] TR_SECTION = "KLAWISZ: STOP";
static const char en54[] TR_SECTION = "AOS in";
static const char pl54[] TR_SECTION = "AOS za";
static const char en55[] TR_SECTION = "AOS in %s";
static const char pl55[] TR_SECTION = "AOS za %s";
static const char en56[] TR_SECTION = "APR";
static const char pl56[] TR_SECTION = "KWI";
static const char en57[] TR_SECTION = "APRS RX on/off";
static const char pl57[] TR_SECTION = "APRS RX w\210/wy\210";
static const char en58[] TR_SECTION = "APRS RX: off";
static const char pl58[] TR_SECTION = "APRS RX: wy\210";
static const char en59[] TR_SECTION = "APRS RX: on";
static const char pl59[] TR_SECTION = "APRS RX: w\210";
static const char en60[] TR_SECTION = "APRS beacon";
static const char pl60[] TR_SECTION = "Ramka APRS";
static const char en61[] TR_SECTION = "APRS frames";
static const char pl61[] TR_SECTION = "Ramki APRS";
static const char en62[] TR_SECTION = "APRS message";
static const char pl62[] TR_SECTION = "Wiadomo\215\202 APRS";
static const char en63[] TR_SECTION = "APRS messages";
static const char pl63[] TR_SECTION = "Wiad. APRS";
static const char en64[] TR_SECTION = "APRS msg: no ACK";
static const char pl64[] TR_SECTION = "Wiad. APRS: brak ACK";
static const char en65[] TR_SECTION = "APRS on/off";
static const char pl65[] TR_SECTION = "APRS w\210/wy\210";
static const char en66[] TR_SECTION = "APRS radar";
static const char pl66[] TR_SECTION = "Radar APRS";
static const char en67[] TR_SECTION = "APRS raw";
static const char pl67[] TR_SECTION = "APRS hex";
static const char en68[] TR_SECTION = "APRS stations";
static const char pl68[] TR_SECTION = "Stacje APRS";
static const char en69[] TR_SECTION = "APRS: TX not allowed";
static const char pl69[] TR_SECTION = "APRS: TX zabroniony";
static const char en70[] TR_SECTION = "APRS: no ch. config";
static const char pl70[] TR_SECTION = "APRS: brak konf. kana\210u";
static const char en71[] TR_SECTION = "AT";
static const char pl71[] TR_SECTION = "GODZ";
static const char en72[] TR_SECTION = "AUG";
static const char pl72[] TR_SECTION = "SIE";
static const char en73[] TR_SECTION = "Again: SOS OFF";
static const char pl73[] TR_SECTION = "Jeszcze raz: SOS WY\207";
static const char en74[] TR_SECTION = "Again: SOS ON";
static const char pl74[] TR_SECTION = "Jeszcze raz: SOS W\207";
static const char en75[] TR_SECTION = "Ahead=up";
static const char pl75[] TR_SECTION = "Prz\363d=g\363ra";
static const char en76[] TR_SECTION = "All 8 used\n0 deletes one";
static const char pl76[] TR_SECTION = "Wszystkie 8 zaj\206te\n0 usuwa jeden";
static const char en77[] TR_SECTION = "Alt  +%u -%um";
static const char pl77[] TR_SECTION = "Wys  +%u -%um";
static const char en78[] TR_SECTION = "Avg  %u.%01ukm/h";
static const char pl78[] TR_SECTION = "\214r.  %u.%01ukm/h";
static const char en79[] TR_SECTION = "BLACKOUT OFF";
static const char pl79[] TR_SECTION = "ZACIEMNIENIE WY\207";
static const char en80[] TR_SECTION = "BLACKOUT ON";
static const char pl80[] TR_SECTION = "ZACIEMNIENIE W\207";
static const char en81[] TR_SECTION = "BRG";
static const char pl81[] TR_SECTION = "AZ";
static const char en82[] TR_SECTION = "Band scope";
static const char pl82[] TR_SECTION = "Panorama";
static const char en83[] TR_SECTION = "Blackout";
static const char pl83[] TR_SECTION = "Zaciemnienie";
static const char en84[] TR_SECTION = "CARRIER";
static const char pl84[] TR_SECTION = "NO\214NA";
static const char en85[] TR_SECTION = "CLEAR %c?";
static const char pl85[] TR_SECTION = "USUN\203\201 %c?";
static const char en86[] TR_SECTION = "CLEAR ALL?";
static const char pl86[] TR_SECTION = "USU\211 WSZYST.?";
static const char en87[] TR_SECTION = "CLEAR ALL? GREEN = yes";
static const char pl87[] TR_SECTION = "USU\211 WSZYST.? ZIEL = tak";
static const char en88[] TR_SECTION = "CLEAR LAST HEARD?";
static const char pl88[] TR_SECTION = "USUN\203\201 OST. S\207YSZANE?";
static const char en89[] TR_SECTION = "CLEAR LOG?";
static const char pl89[] TR_SECTION = "USUN\203\201 DZIENNIK?";
static const char en90[] TR_SECTION = "CLEAR THE WHOLE LOG?";
static const char pl90[] TR_SECTION = "USUN\203\201 CA\207Y DZIENNIK?";
static const char en91[] TR_SECTION = "CONFIRM";
static const char pl91[] TR_SECTION = "POTWIERD\217";
static const char en92[] TR_SECTION = "CONTINGENCY";
static const char pl92[] TR_SECTION = "AWARYJNY";
static const char en93[] TR_SECTION = "COVERAGE";
static const char pl93[] TR_SECTION = "ZASI\205G";
static const char en94[] TR_SECTION = "CRS";
static const char pl94[] TR_SECTION = "KURS";
static const char en95[] TR_SECTION = "CW decoder";
static const char pl95[] TR_SECTION = "Dekoder CW";
static const char en96[] TR_SECTION = "Checked at power on.";
static const char pl96[] TR_SECTION = "Sprawdzane przy w\210\204cz.";
static const char en97[] TR_SECTION = "Classic";
static const char pl97[] TR_SECTION = "Klasyczny";
static const char en98[] TR_SECTION = "DAILY %02u:%02u";
static const char pl98[] TR_SECTION = "CODZ. %02u:%02u";
static const char en99[] TR_SECTION = "DATE";
static const char pl99[] TR_SECTION = "DATA";
static const char en100[] TR_SECTION = "DAY";
static const char pl100[] TR_SECTION = "DZIE\211";
static const char en101[] TR_SECTION = "DEC";
static const char pl101[] TR_SECTION = "GRU";
static const char en102[] TR_SECTION = "DEG";
static const char pl102[] TR_SECTION = "STOP";
static const char en103[] TR_SECTION = "DELETE %d?";
static const char pl103[] TR_SECTION = "USUN\203\201 %d?";
static const char en104[] TR_SECTION = "DELETE S%d?";
static const char pl104[] TR_SECTION = "USUN\203\201 S%d?";
static const char en105[] TR_SECTION = "DELETED";
static const char pl105[] TR_SECTION = "USUNI\205TY";
static const char en106[] TR_SECTION = "DIST";
static const char pl106[] TR_SECTION = "ODL";
static const char en107[] TR_SECTION = "DMR / APRS stations";
static const char pl107[] TR_SECTION = "Stacje DMR / APRS";
static const char en108[] TR_SECTION = "DONE";
static const char pl108[] TR_SECTION = "GOTOWE";
static const char en109[] TR_SECTION = "DOPPLER  0 CLEAR  RED EXIT";
static const char pl109[] TR_SECTION = "DOPPLER  0 KAS  CZER WYJ";
static const char en110[] TR_SECTION = "DST";
static const char pl110[] TR_SECTION = "ODL";
static const char en111[] TR_SECTION = "DTMF decoder";
static const char pl111[] TR_SECTION = "Dekoder DTMF";
static const char en112[] TR_SECTION = "DTMF, SOS, SSTV events";
static const char pl112[] TR_SECTION = "DTMF, SOS, SSTV";
static const char en113[] TR_SECTION = "DUR";
static const char pl113[] TR_SECTION = "CZAS";
static const char en114[] TR_SECTION = "Day/Night";
static const char pl114[] TR_SECTION = "Dzie\213/Noc";
static const char en115[] TR_SECTION = "Dist %u.%02ukm";
static const char pl115[] TR_SECTION = "Dyst %u.%02ukm";
static const char en116[] TR_SECTION = "Dist %um";
static const char pl116[] TR_SECTION = "Dyst %um";
static const char en117[] TR_SECTION = "EDIT SKED";
static const char pl117[] TR_SECTION = "EDYCJA SKEDU";
static const char en118[] TR_SECTION = "EIGHT";
static const char pl118[] TR_SECTION = "OSIEM";
static const char en119[] TR_SECTION = "EMCOM colours";
static const char pl119[] TR_SECTION = "Kolory EMCOM";
static const char en120[] TR_SECTION = "EMCOM contr";
static const char pl120[] TR_SECTION = "EMCOM kontr";
static const char en121[] TR_SECTION = "EMERGENCY";
static const char pl121[] TR_SECTION = "RATUNKOWY";
static const char en122[] TR_SECTION = "EMERGENCY SOS";
static const char pl122[] TR_SECTION = "SOS RATUNKOWE";
static const char en123[] TR_SECTION = "EMPTY";
static const char pl123[] TR_SECTION = "PUSTY";
static const char en124[] TR_SECTION = "END NET?";
static const char pl124[] TR_SECTION = "ZAKO\211CZY\201?";
static const char en125[] TR_SECTION = "ERROR";
static const char pl125[] TR_SECTION = "B\207\203D";
static const char en126[] TR_SECTION = "EVERY";
static const char pl126[] TR_SECTION = "CO";
static const char en127[] TR_SECTION = "EVERY %uH %02u:%02u";
static const char pl127[] TR_SECTION = "CO %uH %02u:%02u";
static const char en128[] TR_SECTION = "EVERY %uM :%02u";
static const char pl128[] TR_SECTION = "CO %uM :%02u";
static const char en129[] TR_SECTION = "Each received over is";
static const char pl129[] TR_SECTION = "Ka\222de odebrane nadanie";
static const char en130[] TR_SECTION = "FAIL";
static const char pl130[] TR_SECTION = "B\207\203D";
static const char en131[] TR_SECTION = "FAIR";
static const char pl131[] TR_SECTION = "\214REDNI";
static const char en132[] TR_SECTION = "FEB";
static const char pl132[] TR_SECTION = "LUT";
static const char en133[] TR_SECTION = "FIVE";
static const char pl133[] TR_SECTION = "PI\205\201";
static const char en134[] TR_SECTION = "FIX none (settings)";
static const char pl134[] TR_SECTION = "FIX brak (ustawienia)";
static const char en135[] TR_SECTION = "FM ONLY";
static const char pl135[] TR_SECTION = "TYLKO FM";
static const char en136[] TR_SECTION = "FM only";
static const char pl136[] TR_SECTION = "Tylko FM";
static const char en137[] TR_SECTION = "FM, e.g. 144.800 MHz";
static const char pl137[] TR_SECTION = "FM, np. 144.800 MHz";
static const char en138[] TR_SECTION = "FOUR";
static const char pl138[] TR_SECTION = "CZTERY";
static const char en139[] TR_SECTION = "FOX";
static const char pl139[] TR_SECTION = "LIS";
static const char en140[] TR_SECTION = "FULL";
static const char pl140[] TR_SECTION = "PE\207NA";
static const char en141[] TR_SECTION = "Fox hunt: %s";
static const char pl141[] TR_SECTION = "Lis: %s";
static const char en142[] TR_SECTION = "GAP";
static const char pl142[] TR_SECTION = "BRAK";
static const char en143[] TR_SECTION = "GO TO CH";
static const char pl143[] TR_SECTION = "KANA\207 NR";
static const char en144[] TR_SECTION = "GOOD";
static const char pl144[] TR_SECTION = "DOBRY";
static const char en145[] TR_SECTION = "GPS OFF";
static const char pl145[] TR_SECTION = "GPS WY\207";
static const char en146[] TR_SECTION = "GREEN = yes";
static const char pl146[] TR_SECTION = "ZIEL = tak";
static const char en147[] TR_SECTION = "GREEN OPEN * POS RED EXIT";
static const char pl147[] TR_SECTION = "ZIEL OTW\323RZ * POZ CZER WYJ";
static const char en148[] TR_SECTION = "GREEN SAVE RED CLEAR < DEL";
static const char pl148[] TR_SECTION = "ZIEL ZAPISZ CZER KAS <USU\211";
static const char en149[] TR_SECTION = "GREEN set";
static const char pl149[] TR_SECTION = "ZIEL ustaw";
static const char en150[] TR_SECTION = "GREEN sets one";
static const char pl150[] TR_SECTION = "ZIEL ustawia";
static const char en151[] TR_SECTION = "GREEN starts a net.";
static const char pl151[] TR_SECTION = "ZIEL rozpoczyna sie\202.";
static const char en152[] TR_SECTION = "GREEN starts a net:";
static const char pl152[] TR_SECTION = "ZIEL rozpoczyna sie\202:";
static const char en153[] TR_SECTION = "GREEN yes, other key no";
static const char pl153[] TR_SECTION = "ZIEL tak, inny klawisz nie";
static const char en154[] TR_SECTION = "GREEN: mark here";
static const char pl154[] TR_SECTION = "ZIEL: zaznacz tu";
static const char en155[] TR_SECTION = "GRN CALL  * QSO  RED EXIT";
static const char pl155[] TR_SECTION = "ZIEL WYW  * QSO  CZER WYJ";
static const char en156[] TR_SECTION = "GRN CALL  RED EXIT";
static const char pl156[] TR_SECTION = "ZIEL WYW  CZER WYJ";
static const char en157[] TR_SECTION = "GRN EDIT  0 DEL  RED BACK";
static const char pl157[] TR_SECTION = "ZIEL EDYT 0 USU\211 CZER WR\323\201";
static const char en158[] TR_SECTION = "GRN EDIT 0 DFLT RED BACK";
static const char pl158[] TR_SECTION = "ZIEL EDYT 0 DOM CZER WR\323\201";
static const char en159[] TR_SECTION = "GRN GO  RGT NAME  0 DEL";
static const char pl159[] TR_SECTION = "ZIEL ID\217 PRAW NAZWA 0 USU\211";
static const char en160[] TR_SECTION = "GRN HEARD * FM # Q 0 END";
static const char pl160[] TR_SECTION = "ZIEL S\207YSZ * FM # C 0 KON";
static const char en161[] TR_SECTION = "GRN MSG";
static const char pl161[] TR_SECTION = "ZIEL WIAD";
static const char en162[] TR_SECTION = "GRN MSG * WPT # VIEW 0 CLR";
static const char pl162[] TR_SECTION = "ZIEL WIAD *PKT #WID 0 KAS";
static const char en163[] TR_SECTION = "GRN OK";
static const char pl163[] TR_SECTION = "ZIEL OK";
static const char en164[] TR_SECTION = "GRN OK  # DEL  RED BACK";
static const char pl164[] TR_SECTION = "ZIEL OK  # USU\211  CZER WR\323\201";
static const char en165[] TR_SECTION = "GRN OK  RED CANCEL";
static const char pl165[] TR_SECTION = "ZIEL OK CZER ANUL";
static const char en166[] TR_SECTION = "GRN QSO  * FM  # Q  0 END";
static const char pl166[] TR_SECTION = "ZIEL QSO  * FM  # C  0 KON";
static const char en167[] TR_SECTION = "GRN REC  # INT  0 CLR";
static const char pl167[] TR_SECTION = "ZIEL ZAP  # INT  0 KAS";
static const char en168[] TR_SECTION = "GRN REPLY 1 NEW # VIEW";
static const char pl168[] TR_SECTION = "ZIEL ODP 1 NOWA # WIDOK";
static const char en169[] TR_SECTION = "GRN REPLY UP/DN  RED BACK";
static const char pl169[] TR_SECTION = "ZIEL ODP UP/DN  CZER WR\323\201";
static const char en170[] TR_SECTION = "GRN SEND  * TO  RED BACK";
static const char pl170[] TR_SECTION = "ZIEL WY\214LIJ * DO CZER WR\323\201";
static const char en171[] TR_SECTION = "GRN START  RED EXIT";
static const char pl171[] TR_SECTION = "ZIEL START  CZER WYJ";
static const char en172[] TR_SECTION = "GRN STOP  # INT  RED EXIT";
static const char pl172[] TR_SECTION = "ZIEL STOP # INT CZER WYJ";
static const char en173[] TR_SECTION = "GRN YES     RED NO";
static const char pl173[] TR_SECTION = "ZIEL TAK    CZER NIE";
static const char en174[] TR_SECTION = "GRN YES  ANY NO";
static const char pl174[] TR_SECTION = "ZIEL TAK  INNY NIE";
static const char en175[] TR_SECTION = "GRN YES ANY NO";
static const char pl175[] TR_SECTION = "ZIEL TAK INNY NIE";
static const char en176[] TR_SECTION = "GRN starts a session";
static const char pl176[] TR_SECTION = "ZIEL zaczyna sesj\206";
static const char en177[] TR_SECTION = "HERE";
static const char pl177[] TR_SECTION = "TU";
static const char en178[] TR_SECTION = "HOURLY :%02u";
static const char pl178[] TR_SECTION = "CO GODZ :%02u";
static const char en179[] TR_SECTION = "IDLE";
static const char pl179[] TR_SECTION = "CZEKA";
static const char en180[] TR_SECTION = "IN";
static const char pl180[] TR_SECTION = "ODB";
static const char en181[] TR_SECTION = "INFO";
static const char pl181[] TR_SECTION = "INFO";
static const char en182[] TR_SECTION = "Init failed";
static const char pl182[] TR_SECTION = "B\210\204d inicjalizacji";
static const char en183[] TR_SECTION = "JAN";
static const char pl183[] TR_SECTION = "STY";
static const char en184[] TR_SECTION = "JUL";
static const char pl184[] TR_SECTION = "LIP";
static const char en185[] TR_SECTION = "JUN";
static const char pl185[] TR_SECTION = "CZE";
static const char en186[] TR_SECTION = "KEY";
static const char pl186[] TR_SECTION = "KLUCZ";
static const char en187[] TR_SECTION = "KEYL";
static const char pl187[] TR_SECTION = "KLAW";
static const char en188[] TR_SECTION = "LAT";
static const char pl188[] TR_SECTION = "SZER";
static const char en189[] TR_SECTION = "LENGTH";
static const char pl189[] TR_SECTION = "CZAS";
static const char en190[] TR_SECTION = "LISTENING";
static const char pl190[] TR_SECTION = "NAS\207UCH";
static const char en191[] TR_SECTION = "LOC";
static const char pl191[] TR_SECTION = "LOK";
static const char en192[] TR_SECTION = "LOC %s";
static const char pl192[] TR_SECTION = "LOK %s";
static const char en193[] TR_SECTION = "LOCK";
static const char pl193[] TR_SECTION = "BLOK";
static const char en194[] TR_SECTION = "LON";
static const char pl194[] TR_SECTION = "D\207";
static const char en195[] TR_SECTION = "LOS in %s";
static const char pl195[] TR_SECTION = "LOS za %s";
static const char en196[] TR_SECTION = "Last net: %u stations";
static const char pl196[] TR_SECTION = "Ost. sie\202: %u stacji";
static const char en197[] TR_SECTION = "Lat ";
static const char pl197[] TR_SECTION = "Sz. ";
static const char en198[] TR_SECTION = "Listening...";
static const char pl198[] TR_SECTION = "Nas\210uch...";
static const char en199[] TR_SECTION = "Log empty";
static const char pl199[] TR_SECTION = "Dziennik pusty";
static const char en200[] TR_SECTION = "Lon ";
static const char pl200[] TR_SECTION = "D\210. ";
static const char en201[] TR_SECTION = "Long GREEN: reset";
static const char pl201[] TR_SECTION = "D\210ugo ZIEL: zeruj";
static const char en202[] TR_SECTION = "MANUAL";
static const char pl202[] TR_SECTION = "R\205CZNA";
static const char en203[] TR_SECTION = "MAR";
static const char pl203[] TR_SECTION = "MAR";
static const char en204[] TR_SECTION = "MAY";
static const char pl204[] TR_SECTION = "MAJ";
static const char en205[] TR_SECTION = "MEM";
static const char pl205[] TR_SECTION = "PAM";
static const char en206[] TR_SECTION = "MSG";
static const char pl206[] TR_SECTION = "WIAD";
static const char en207[] TR_SECTION = "MSG %u";
static const char pl207[] TR_SECTION = "WIAD %u";
static const char en208[] TR_SECTION = "Max @ %dkm %03d deg";
static const char pl208[] TR_SECTION = "Max @ %dkm %03d st.";
static const char en209[] TR_SECTION = "Max @ %dm %03d deg";
static const char pl209[] TR_SECTION = "Max @ %dm %03d st.";
static const char en210[] TR_SECTION = "Max @ (no GPS fix)";
static const char pl210[] TR_SECTION = "Max @ (brak fix GPS)";
static const char en211[] TR_SECTION = "Max @ no position";
static const char pl211[] TR_SECTION = "Max @ brak pozycji";
static const char en212[] TR_SECTION = "Move %u:%02u:%02u";
static const char pl212[] TR_SECTION = "Ruch %u:%02u:%02u";
static const char en213[] TR_SECTION = "N UP";
static const char pl213[] TR_SECTION = "N G\323RA";
static const char en214[] TR_SECTION = "NET CONTROL";
static const char pl214[] TR_SECTION = "KONTROLA SIECI";
static const char en215[] TR_SECTION = "NEW WAYPOINT";
static const char pl215[] TR_SECTION = "NOWY PUNKT";
static const char en216[] TR_SECTION = "NEXT S%d";
static const char pl216[] TR_SECTION = "NAST. S%d";
static const char en217[] TR_SECTION = "NINE";
static const char pl217[] TR_SECTION = "DZIEWI\205\201";
static const char en218[] TR_SECTION = "NO ACK";
static const char pl218[] TR_SECTION = "BEZ ACK";
static const char en219[] TR_SECTION = "NO CLK";
static const char pl219[] TR_SECTION = "BEZ ZEG";
static const char en220[] TR_SECTION = "NO CLOCK";
static const char pl220[] TR_SECTION = "BRAK ZEGARA";
static const char en221[] TR_SECTION = "NO FIX";
static const char pl221[] TR_SECTION = "BRAK FIX";
static const char en222[] TR_SECTION = "NO GPS";
static const char pl222[] TR_SECTION = "BRAK GPS";
static const char en223[] TR_SECTION = "NO NET";
static const char pl223[] TR_SECTION = "BRAK SIECI";
static const char en224[] TR_SECTION = "NO POS";
static const char pl224[] TR_SECTION = "BRAK POZ";
static const char en225[] TR_SECTION = "NO SKED";
static const char pl225[] TR_SECTION = "BRAK SKEDU";
static const char en226[] TR_SECTION = "NONE";
static const char pl226[] TR_SECTION = "BRAK";
static const char en227[] TR_SECTION = "NOT SENT";
static const char pl227[] TR_SECTION = "NIEWYS\207.";
static const char en228[] TR_SECTION = "NOV";
static const char pl228[] TR_SECTION = "LIS";
static const char en229[] TR_SECTION = "NOW";
static const char pl229[] TR_SECTION = "TERAZ";
static const char en230[] TR_SECTION = "Net control";
static const char pl230[] TR_SECTION = "Kontrola sieci";
static const char en231[] TR_SECTION = "New waypoint";
static const char pl231[] TR_SECTION = "Nowy punkt";
static const char en232[] TR_SECTION = "No GPS fix";
static const char pl232[] TR_SECTION = "Brak fix GPS";
static const char en233[] TR_SECTION = "No GPS fix - not logging";
static const char pl233[] TR_SECTION = "Brak fix GPS - bez zapisu";
static const char en234[] TR_SECTION = "No GPS receiver found.";
static const char pl234[] TR_SECTION = "Nie znaleziono GPS.";
static const char en235[] TR_SECTION = "No fix";
static const char pl235[] TR_SECTION = "Brak fix";
static const char en236[] TR_SECTION = "No messages";
static const char pl236[] TR_SECTION = "Brak wiadomo\215ci";
static const char en237[] TR_SECTION = "No net running.";
static const char pl237[] TR_SECTION = "Brak aktywnej sieci.";
static const char en238[] TR_SECTION = "No own pos";
static const char pl238[] TR_SECTION = "Brak w\210. pozycji";
static const char en239[] TR_SECTION = "No position";
static const char pl239[] TR_SECTION = "Brak pozycji";
static const char en240[] TR_SECTION = "No satellites yet";
static const char pl240[] TR_SECTION = "Brak satelit\363w";
static const char en241[] TR_SECTION = "No stations";
static const char pl241[] TR_SECTION = "Brak stacji";
static const char en242[] TR_SECTION = "No waypoint";
static const char pl242[] TR_SECTION = "Brak punktu";
static const char en243[] TR_SECTION = "North=up";
static const char pl243[] TR_SECTION = "P\210n=g\363ra";
static const char en244[] TR_SECTION = "OCT";
static const char pl244[] TR_SECTION = "PA\217";
static const char en245[] TR_SECTION = "OFF";
static const char pl245[] TR_SECTION = "WY\207";
static const char en246[] TR_SECTION = "OFF PLAN";
static const char pl246[] TR_SECTION = "POZA PLAN.";
static const char en247[] TR_SECTION = "ON";
static const char pl247[] TR_SECTION = "W\207";
static const char en248[] TR_SECTION = "ON %c";
static const char pl248[] TR_SECTION = "W\207 %c";
static const char en249[] TR_SECTION = "ONE";
static const char pl249[] TR_SECTION = "JEDEN";
static const char en250[] TR_SECTION = "OUT";
static const char pl250[] TR_SECTION = "NAD";
static const char en251[] TR_SECTION = "Open sky helps (1-5 min)";
static const char pl251[] TR_SECTION = "Otwarte niebo (1-5 min)";
static const char en252[] TR_SECTION = "PACE %c (empty)";
static const char pl252[] TR_SECTION = "PACE %c (pusty)";
static const char en253[] TR_SECTION = "PACE PLAN";
static const char pl253[] TR_SECTION = "PLAN PACE";
static const char en254[] TR_SECTION = "PAUSE";
static const char pl254[] TR_SECTION = "PAUZA";
static const char en255[] TR_SECTION = "POOR";
static const char pl255[] TR_SECTION = "Z\207Y";
static const char en256[] TR_SECTION = "POS";
static const char pl256[] TR_SECTION = "POZ";
static const char en257[] TR_SECTION = "POSITION";
static const char pl257[] TR_SECTION = "POZYCJA";
static const char en258[] TR_SECTION = "PRIMARY";
static const char pl258[] TR_SECTION = "G\207\323WNY";
static const char en259[] TR_SECTION = "PRIVATE";
static const char pl259[] TR_SECTION = "PRYWAT.";
static const char en260[] TR_SECTION = "PTS %u/%u  EVERY %us";
static const char pl260[] TR_SECTION = "PKT %u/%u  CO %us";
static const char en261[] TR_SECTION = "PWR";
static const char pl261[] TR_SECTION = "MOC";
static const char en262[] TR_SECTION = "QSO logged";
static const char pl262[] TR_SECTION = "QSO zapisane";
static const char en263[] TR_SECTION = "QUIET";
static const char pl263[] TR_SECTION = "CISZA";
static const char en264[] TR_SECTION = "QUIET %s";
static const char pl264[] TR_SECTION = "CISZA %s";
static const char en265[] TR_SECTION = "RADIO CHECK";
static const char pl265[] TR_SECTION = "TEST \207\203CZNO\214CI";
static const char en266[] TR_SECTION = "RAW SIG %u  NOISE %u";
static const char pl266[] TR_SECTION = "SUR SYG %u  SZUM %u";
static const char en267[] TR_SECTION = "REC";
static const char pl267[] TR_SECTION = "ZAP";
static const char en268[] TR_SECTION = "RED CLOSE";
static const char pl268[] TR_SECTION = "CZER ZAMKNIJ";
static const char en269[] TR_SECTION = "RED EXIT";
static const char pl269[] TR_SECTION = "CZER WYJ";
static const char en270[] TR_SECTION = "RUN";
static const char pl270[] TR_SECTION = "AKT";
static const char en271[] TR_SECTION = "RX only";
static const char pl271[] TR_SECTION = "Tylko RX";
static const char en272[] TR_SECTION = "Reset failed";
static const char pl272[] TR_SECTION = "B\210\204d resetu";
static const char en273[] TR_SECTION = "Roster full";
static const char pl273[] TR_SECTION = "Lista pe\210na";
static const char en274[] TR_SECTION = "S%d NOW";
static const char pl274[] TR_SECTION = "S%d TERAZ";
static const char en275[] TR_SECTION = "SATS";
static const char pl275[] TR_SECTION = "SAT";
static const char en276[] TR_SECTION = "SAVED %u/%u";
static const char pl276[] TR_SECTION = "ZAPIS. %u/%u";
static const char en277[] TR_SECTION = "SAVED 0 NEW";
static const char pl277[] TR_SECTION = "ZAPIS. 0 NOWY";
static const char en278[] TR_SECTION = "SAVING";
static const char pl278[] TR_SECTION = "ZAPIS";
static const char en279[] TR_SECTION = "SCAN";
static const char pl279[] TR_SECTION = "SKAN";
static const char en280[] TR_SECTION = "SEARCH";
static const char pl280[] TR_SECTION = "SZUKA";
static const char en281[] TR_SECTION = "SEP";
static const char pl281[] TR_SECTION = "WRZ";
static const char en282[] TR_SECTION = "SEVEN";
static const char pl282[] TR_SECTION = "SIEDEM";
static const char en283[] TR_SECTION = "SIX";
static const char pl283[] TR_SECTION = "SZE\214\201";
static const char en284[] TR_SECTION = "SK2 + GREEN to OK";
static const char pl284[] TR_SECTION = "SK2 + ZIEL = OK";
static const char en285[] TR_SECTION = "SK2+GREEN turns it on";
static const char pl285[] TR_SECTION = "SK2+ZIEL w\210\204cza go";
static const char en286[] TR_SECTION = "SK2+GRN GPS ON  RED EXIT";
static const char pl286[] TR_SECTION = "SK2+ZIEL GPS W\207 CZER WYJ";
static const char en287[] TR_SECTION = "SKED %d NOW\n%s%s";
static const char pl287[] TR_SECTION = "SKED %d TERAZ\n%s%s";
static const char en288[] TR_SECTION = "SKED in 1 min\nS%d %s";
static const char pl288[] TR_SECTION = "SKED za 1 min\nS%d %s";
static const char en289[] TR_SECTION = "SLOT %d";
static const char pl289[] TR_SECTION = "MIEJSCE %d";
static const char en290[] TR_SECTION = "SOS (press x2)";
static const char pl290[] TR_SECTION = "SOS (2x)";
static const char en291[] TR_SECTION = "SOS PAUSED";
static const char pl291[] TR_SECTION = "SOS WSTRZYMANY";
static const char en292[] TR_SECTION = "SOS: OFF";
static const char pl292[] TR_SECTION = "SOS: WY\207";
static const char en293[] TR_SECTION = "SOS: ON";
static const char pl293[] TR_SECTION = "SOS: W\207";
static const char en294[] TR_SECTION = "SOS: TX not allowed";
static const char pl294[] TR_SECTION = "SOS: TX zabroniony";
static const char en295[] TR_SECTION = "SOS: no APRS config";
static const char pl295[] TR_SECTION = "SOS: brak konfig. APRS";
static const char en296[] TR_SECTION = "SPD";
static const char pl296[] TR_SECTION = "PR\205D";
static const char en297[] TR_SECTION = "SSTV picture";
static const char pl297[] TR_SECTION = "obraz SSTV";
static const char en298[] TR_SECTION = "SUN  %02d:%02d - %02d:%02d %s";
static const char pl298[] TR_SECTION = "S\207O\211CE %02d:%02d-%02d:%02d %s";
static const char en299[] TR_SECTION = "SUN  %s";
static const char pl299[] TR_SECTION = "S\207O\211CE %s";
static const char en300[] TR_SECTION = "SUN  no clock";
static const char pl300[] TR_SECTION = "S\207O\211CE brak zegara";
static const char en301[] TR_SECTION = "Save failed";
static const char pl301[] TR_SECTION = "B\210\204d zapisu";
static const char en302[] TR_SECTION = "Saved %u/%u";
static const char pl302[] TR_SECTION = "Zapisane %u/%u";
static const char en303[] TR_SECTION = "Saved WP%d";
static const char pl303[] TR_SECTION = "Zapisano PK%d";
static const char en304[] TR_SECTION = "Saved WP%d\n%.8s";
static const char pl304[] TR_SECTION = "Zapisano PK%d\n%.8s";
static const char en305[] TR_SECTION = "Scope: to peak";
static const char pl305[] TR_SECTION = "Panorama: szczyt";
static const char en306[] TR_SECTION = "Screen";
static const char pl306[] TR_SECTION = "Styl";
static const char en307[] TR_SECTION = "Searching: %s";
static const char pl307[] TR_SECTION = "Szukanie: %s";
static const char en308[] TR_SECTION = "Session: no points yet";
static const char pl308[] TR_SECTION = "Sesja: brak punkt\363w";
static const char en309[] TR_SECTION = "Stations heard on DMR";
static const char pl309[] TR_SECTION = "Stacje s\210yszane na DMR";
static const char en310[] TR_SECTION = "Storage init.";
static const char pl310[] TR_SECTION = "Pami\206\202 zainicjow.";
static const char en311[] TR_SECTION = "Storage reset";
static const char pl311[] TR_SECTION = "Pami\206\202 wyczyszczona";
static const char en312[] TR_SECTION = "Switch to an FM channel";
static const char pl312[] TR_SECTION = "Prze\210\204cz na kana\210 FM";
static const char en313[] TR_SECTION = "THANKS FOR LISTENING";
static const char pl313[] TR_SECTION = "DZI\205KI ZA NAS\207UCH";
static const char en314[] TR_SECTION = "THREE";
static const char pl314[] TR_SECTION = "TRZY";
static const char en315[] TR_SECTION = "TO";
static const char pl315[] TR_SECTION = "DO";
static const char en316[] TR_SECTION = "TO %s";
static const char pl316[] TR_SECTION = "DO %s";
static const char en317[] TR_SECTION = "TO: callsign (-SSID)";
static const char pl317[] TR_SECTION = "DO: znak (-SSID)";
static const char en318[] TR_SECTION = "TUNE";
static const char pl318[] TR_SECTION = "KANA\207";
static const char en319[] TR_SECTION = "TWO";
static const char pl319[] TR_SECTION = "DWA";
static const char en320[] TR_SECTION = "TX OFF";
static const char pl320[] TR_SECTION = "TX WY\207";
static const char en321[] TR_SECTION = "TX POWER";
static const char pl321[] TR_SECTION = "MOC TX";
static const char en322[] TR_SECTION = "TX blocked";
static const char pl322[] TR_SECTION = "TX zablokowany";
static const char en323[] TR_SECTION = "TX stopped";
static const char pl323[] TR_SECTION = "TX zatrzymany";
static const char en324[] TR_SECTION = "TX, RX, DMR, APRS, MSG,";
static const char pl324[] TR_SECTION = "TX, RX, DMR, APRS, WIAD,";
static const char en325[] TR_SECTION = "Target %s";
static const char pl325[] TR_SECTION = "Cel %s";
static const char en326[] TR_SECTION = "The receiver is off.";
static const char pl326[] TR_SECTION = "Odbiornik wy\210\204czony.";
static const char en327[] TR_SECTION = "Theme saved";
static const char pl327[] TR_SECTION = "Motyw zapisany";
static const char en328[] TR_SECTION = "Trip reset";
static const char pl328[] TR_SECTION = "Trasa wyzerowana";
static const char en329[] TR_SECTION = "Trip: no fix yet";
static const char pl329[] TR_SECTION = "Trasa: brak fix";
static const char en330[] TR_SECTION = "Turn the GPS on, or set";
static const char pl330[] TR_SECTION = "W\210\204cz GPS albo ustaw";
static const char en331[] TR_SECTION = "UP/DN  0 CLEAR  RED EXIT";
static const char pl331[] TR_SECTION = "UP/DN  0 KASUJ  CZER WYJ";
static const char en332[] TR_SECTION = "UP/DN  < > SET  GRN SAVE";
static const char pl332[] TR_SECTION = "UP/DN <> ZMIE\211 ZIEL ZAPISZ";
static const char en333[] TR_SECTION = "UP/DN NEXT  RED BACK";
static const char pl333[] TR_SECTION = "UP/DN DALEJ  CZER WR\323\201";
static const char en334[] TR_SECTION = "UP/DN PAGE  RED EXIT";
static const char pl334[] TR_SECTION = "UP/DN STRONA  CZER WYJ";
static const char en335[] TR_SECTION = "UP/DN PAGE  SK2+RED OFF";
static const char pl335[] TR_SECTION = "UP/DN STRONA SK2+CZER WY\207";
static const char en336[] TR_SECTION = "UP/DN QRG 0 CLEAR RED EXIT";
static const char pl336[] TR_SECTION = "UP/DN QRG 0 KAS CZER WYJ";
static const char en337[] TR_SECTION = "UP/DN SCROLL  * RED BACK";
static const char pl337[] TR_SECTION = "UP/DN PRZEWI\211 * CZER WR\323\201";
static const char en338[] TR_SECTION = "UP/DN VIEW # NEXT RED LIST";
static const char pl338[] TR_SECTION = "UP/DN WID #NAST CZER LISTA";
static const char en339[] TR_SECTION = "UP/DN field RED del";
static const char pl339[] TR_SECTION = "UP/DN pole CZER kas";
static const char en340[] TR_SECTION = "WAIT %u";
static const char pl340[] TR_SECTION = "CZEKA %u";
static const char en341[] TR_SECTION = "WARNING";
static const char pl341[] TR_SECTION = "UWAGA";
static const char en342[] TR_SECTION = "WAYPOINT NAME";
static const char pl342[] TR_SECTION = "NAZWA PUNKTU";
static const char en343[] TR_SECTION = "WAYPOINTS";
static const char pl343[] TR_SECTION = "PUNKTY";
static const char en344[] TR_SECTION = "WEAK";
static const char pl344[] TR_SECTION = "S\207ABY";
static const char en345[] TR_SECTION = "WPT";
static const char pl345[] TR_SECTION = "PKT";
static const char en346[] TR_SECTION = "WPT %ukm  %03d %s";
static const char pl346[] TR_SECTION = "PKT %ukm  %03d %s";
static const char en347[] TR_SECTION = "WPT %um  %03d %s";
static const char pl347[] TR_SECTION = "PKT %um  %03d %s";
static const char en348[] TR_SECTION = "WPT none";
static const char pl348[] TR_SECTION = "PKT brak";
static const char en349[] TR_SECTION = "WPT none (* marks)";
static const char pl349[] TR_SECTION = "PKT brak (* zaznacza)";
static const char en350[] TR_SECTION = "Waiting for";
static const char pl350[] TR_SECTION = "Czekam na";
static const char en351[] TR_SECTION = "Waiting for a call...";
static const char pl351[] TR_SECTION = "Czekam na wywo\210anie...";
static const char en352[] TR_SECTION = "Waypoint set";
static const char pl352[] TR_SECTION = "Punkt ustawiony";
static const char en353[] TR_SECTION = "Waypoint: %s";
static const char pl353[] TR_SECTION = "Punkt: %s";
static const char en354[] TR_SECTION = "Waypoint: %s\nlist full";
static const char pl354[] TR_SECTION = "Punkt: %s\nlista pe\210na";
static const char en355[] TR_SECTION = "Waypoints";
static const char pl355[] TR_SECTION = "Punkty";
static const char en356[] TR_SECTION = "Wipe themes,";
static const char pl356[] TR_SECTION = "Usu\213 motywy,";
static const char en357[] TR_SECTION = "^v MODE  GRN OK  RED BACK";
static const char pl357[] TR_SECTION = "^v TRYB ZIEL OK CZER WR\323\201";
static const char en358[] TR_SECTION = "always down";
static const char pl358[] TR_SECTION = "ca\210a noc";
static const char en359[] TR_SECTION = "always up";
static const char pl359[] TR_SECTION = "ca\210y dzie\213";
static const char en360[] TR_SECTION = "boot img, beep, TLE";
static const char pl360[] TR_SECTION = "obraz start., bip, TLE";
static const char en361[] TR_SECTION = "channel deleted";
static const char pl361[] TR_SECTION = "kana\210 usuni\206ty";
static const char en362[] TR_SECTION = "dBm  SIGNAL";
static const char pl362[] TR_SECTION = "dBm  SYGNA\207";
static const char en363[] TR_SECTION = "dBm  noise";
static const char pl363[] TR_SECTION = "dBm  szum";
static const char en364[] TR_SECTION = "empty - * stores";
static const char pl364[] TR_SECTION = "pusty - * zapisuje";
static const char en365[] TR_SECTION = "from the settings";
static const char pl365[] TR_SECTION = "z ustawie\213";
static const char en366[] TR_SECTION = "heard check in.";
static const char pl366[] TR_SECTION = "s\210yszane si\206 melduj\204.";
static const char en367[] TR_SECTION = "heard yet";
static const char pl367[] TR_SECTION = "z odbioru";
static const char en368[] TR_SECTION = "needs GPS fix/location";
static const char pl368[] TR_SECTION = "wymaga GPS/lokalizacji";
static const char en369[] TR_SECTION = "no GPS fix";
static const char pl369[] TR_SECTION = "brak fix GPS";
static const char en370[] TR_SECTION = "no callsign";
static const char pl370[] TR_SECTION = "brak znaku";
static const char en371[] TR_SECTION = "no clock";
static const char pl371[] TR_SECTION = "brak zegara";
static const char en372[] TR_SECTION = "no fix / location";
static const char pl372[] TR_SECTION = "brak pozycji";
static const char en373[] TR_SECTION = "no position";
static const char pl373[] TR_SECTION = "brak pozycji";
static const char en374[] TR_SECTION = "no satellites";
static const char pl374[] TR_SECTION = "brak satelit\363w";
static const char en375[] TR_SECTION = "no sig";
static const char pl375[] TR_SECTION = "brak";
static const char en376[] TR_SECTION = "noTX";
static const char pl376[] TR_SECTION = "bezTX";
static const char en377[] TR_SECTION = "not logging";
static const char pl377[] TR_SECTION = "nie zapisuje";
static const char en378[] TR_SECTION = "off";
static const char pl378[] TR_SECTION = "wy\210";
static const char en379[] TR_SECTION = "or APRS check in.";
static const char pl379[] TR_SECTION = "lub APRS si\206 melduj\204.";
static const char en380[] TR_SECTION = "or UP/DOWN: FM preset";
static const char pl380[] TR_SECTION = "lub UP/DOWN: preset FM";
static const char en381[] TR_SECTION = "polar (UPS)";
static const char pl381[] TR_SECTION = "polarny (UPS)";
static const char en382[] TR_SECTION = "pos";
static const char pl382[] TR_SECTION = "poz";
static const char en383[] TR_SECTION = "rated R1-5 / S1-9.";
static const char pl383[] TR_SECTION = "ocenia R1-5 / S1-9.";
static const char en384[] TR_SECTION = "rings";
static const char pl384[] TR_SECTION = "okr\206gi";
static const char en385[] TR_SECTION = "signal";
static const char pl385[] TR_SECTION = "sygna\210";
static const char en386[] TR_SECTION = "the location in Options.";
static const char pl386[] TR_SECTION = "lokalizacj\206 w Opcjach.";
static const char en387[] TR_SECTION = "time left";
static const char pl387[] TR_SECTION = "pozosta\210o";
static const char en388[] TR_SECTION = "waiting for CW";
static const char pl388[] TR_SECTION = "czekam na CW";
static const char en389[] TR_SECTION = "waiting for DTMF";
static const char pl389[] TR_SECTION = "czekam na DTMF";

const uiTranslation_t uiTranslationsPL[] TR_SECTION =
{
	{ en0, pl0 },
	{ en1, pl1 },
	{ en2, pl2 },
	{ en3, pl3 },
	{ en4, pl4 },
	{ en5, pl5 },
	{ en6, pl6 },
	{ en7, pl7 },
	{ en8, pl8 },
	{ en9, pl9 },
	{ en10, pl10 },
	{ en11, pl11 },
	{ en12, pl12 },
	{ en13, pl13 },
	{ en14, pl14 },
	{ en15, pl15 },
	{ en16, pl16 },
	{ en17, pl17 },
	{ en18, pl18 },
	{ en19, pl19 },
	{ en20, pl20 },
	{ en21, pl21 },
	{ en22, pl22 },
	{ en23, pl23 },
	{ en24, pl24 },
	{ en25, pl25 },
	{ en26, pl26 },
	{ en27, pl27 },
	{ en28, pl28 },
	{ en29, pl29 },
	{ en30, pl30 },
	{ en31, pl31 },
	{ en32, pl32 },
	{ en33, pl33 },
	{ en34, pl34 },
	{ en35, pl35 },
	{ en36, pl36 },
	{ en37, pl37 },
	{ en38, pl38 },
	{ en39, pl39 },
	{ en40, pl40 },
	{ en41, pl41 },
	{ en42, pl42 },
	{ en43, pl43 },
	{ en44, pl44 },
	{ en45, pl45 },
	{ en46, pl46 },
	{ en47, pl47 },
	{ en48, pl48 },
	{ en49, pl49 },
	{ en50, pl50 },
	{ en51, pl51 },
	{ en52, pl52 },
	{ en53, pl53 },
	{ en54, pl54 },
	{ en55, pl55 },
	{ en56, pl56 },
	{ en57, pl57 },
	{ en58, pl58 },
	{ en59, pl59 },
	{ en60, pl60 },
	{ en61, pl61 },
	{ en62, pl62 },
	{ en63, pl63 },
	{ en64, pl64 },
	{ en65, pl65 },
	{ en66, pl66 },
	{ en67, pl67 },
	{ en68, pl68 },
	{ en69, pl69 },
	{ en70, pl70 },
	{ en71, pl71 },
	{ en72, pl72 },
	{ en73, pl73 },
	{ en74, pl74 },
	{ en75, pl75 },
	{ en76, pl76 },
	{ en77, pl77 },
	{ en78, pl78 },
	{ en79, pl79 },
	{ en80, pl80 },
	{ en81, pl81 },
	{ en82, pl82 },
	{ en83, pl83 },
	{ en84, pl84 },
	{ en85, pl85 },
	{ en86, pl86 },
	{ en87, pl87 },
	{ en88, pl88 },
	{ en89, pl89 },
	{ en90, pl90 },
	{ en91, pl91 },
	{ en92, pl92 },
	{ en93, pl93 },
	{ en94, pl94 },
	{ en95, pl95 },
	{ en96, pl96 },
	{ en97, pl97 },
	{ en98, pl98 },
	{ en99, pl99 },
	{ en100, pl100 },
	{ en101, pl101 },
	{ en102, pl102 },
	{ en103, pl103 },
	{ en104, pl104 },
	{ en105, pl105 },
	{ en106, pl106 },
	{ en107, pl107 },
	{ en108, pl108 },
	{ en109, pl109 },
	{ en110, pl110 },
	{ en111, pl111 },
	{ en112, pl112 },
	{ en113, pl113 },
	{ en114, pl114 },
	{ en115, pl115 },
	{ en116, pl116 },
	{ en117, pl117 },
	{ en118, pl118 },
	{ en119, pl119 },
	{ en120, pl120 },
	{ en121, pl121 },
	{ en122, pl122 },
	{ en123, pl123 },
	{ en124, pl124 },
	{ en125, pl125 },
	{ en126, pl126 },
	{ en127, pl127 },
	{ en128, pl128 },
	{ en129, pl129 },
	{ en130, pl130 },
	{ en131, pl131 },
	{ en132, pl132 },
	{ en133, pl133 },
	{ en134, pl134 },
	{ en135, pl135 },
	{ en136, pl136 },
	{ en137, pl137 },
	{ en138, pl138 },
	{ en139, pl139 },
	{ en140, pl140 },
	{ en141, pl141 },
	{ en142, pl142 },
	{ en143, pl143 },
	{ en144, pl144 },
	{ en145, pl145 },
	{ en146, pl146 },
	{ en147, pl147 },
	{ en148, pl148 },
	{ en149, pl149 },
	{ en150, pl150 },
	{ en151, pl151 },
	{ en152, pl152 },
	{ en153, pl153 },
	{ en154, pl154 },
	{ en155, pl155 },
	{ en156, pl156 },
	{ en157, pl157 },
	{ en158, pl158 },
	{ en159, pl159 },
	{ en160, pl160 },
	{ en161, pl161 },
	{ en162, pl162 },
	{ en163, pl163 },
	{ en164, pl164 },
	{ en165, pl165 },
	{ en166, pl166 },
	{ en167, pl167 },
	{ en168, pl168 },
	{ en169, pl169 },
	{ en170, pl170 },
	{ en171, pl171 },
	{ en172, pl172 },
	{ en173, pl173 },
	{ en174, pl174 },
	{ en175, pl175 },
	{ en176, pl176 },
	{ en177, pl177 },
	{ en178, pl178 },
	{ en179, pl179 },
	{ en180, pl180 },
	{ en181, pl181 },
	{ en182, pl182 },
	{ en183, pl183 },
	{ en184, pl184 },
	{ en185, pl185 },
	{ en186, pl186 },
	{ en187, pl187 },
	{ en188, pl188 },
	{ en189, pl189 },
	{ en190, pl190 },
	{ en191, pl191 },
	{ en192, pl192 },
	{ en193, pl193 },
	{ en194, pl194 },
	{ en195, pl195 },
	{ en196, pl196 },
	{ en197, pl197 },
	{ en198, pl198 },
	{ en199, pl199 },
	{ en200, pl200 },
	{ en201, pl201 },
	{ en202, pl202 },
	{ en203, pl203 },
	{ en204, pl204 },
	{ en205, pl205 },
	{ en206, pl206 },
	{ en207, pl207 },
	{ en208, pl208 },
	{ en209, pl209 },
	{ en210, pl210 },
	{ en211, pl211 },
	{ en212, pl212 },
	{ en213, pl213 },
	{ en214, pl214 },
	{ en215, pl215 },
	{ en216, pl216 },
	{ en217, pl217 },
	{ en218, pl218 },
	{ en219, pl219 },
	{ en220, pl220 },
	{ en221, pl221 },
	{ en222, pl222 },
	{ en223, pl223 },
	{ en224, pl224 },
	{ en225, pl225 },
	{ en226, pl226 },
	{ en227, pl227 },
	{ en228, pl228 },
	{ en229, pl229 },
	{ en230, pl230 },
	{ en231, pl231 },
	{ en232, pl232 },
	{ en233, pl233 },
	{ en234, pl234 },
	{ en235, pl235 },
	{ en236, pl236 },
	{ en237, pl237 },
	{ en238, pl238 },
	{ en239, pl239 },
	{ en240, pl240 },
	{ en241, pl241 },
	{ en242, pl242 },
	{ en243, pl243 },
	{ en244, pl244 },
	{ en245, pl245 },
	{ en246, pl246 },
	{ en247, pl247 },
	{ en248, pl248 },
	{ en249, pl249 },
	{ en250, pl250 },
	{ en251, pl251 },
	{ en252, pl252 },
	{ en253, pl253 },
	{ en254, pl254 },
	{ en255, pl255 },
	{ en256, pl256 },
	{ en257, pl257 },
	{ en258, pl258 },
	{ en259, pl259 },
	{ en260, pl260 },
	{ en261, pl261 },
	{ en262, pl262 },
	{ en263, pl263 },
	{ en264, pl264 },
	{ en265, pl265 },
	{ en266, pl266 },
	{ en267, pl267 },
	{ en268, pl268 },
	{ en269, pl269 },
	{ en270, pl270 },
	{ en271, pl271 },
	{ en272, pl272 },
	{ en273, pl273 },
	{ en274, pl274 },
	{ en275, pl275 },
	{ en276, pl276 },
	{ en277, pl277 },
	{ en278, pl278 },
	{ en279, pl279 },
	{ en280, pl280 },
	{ en281, pl281 },
	{ en282, pl282 },
	{ en283, pl283 },
	{ en284, pl284 },
	{ en285, pl285 },
	{ en286, pl286 },
	{ en287, pl287 },
	{ en288, pl288 },
	{ en289, pl289 },
	{ en290, pl290 },
	{ en291, pl291 },
	{ en292, pl292 },
	{ en293, pl293 },
	{ en294, pl294 },
	{ en295, pl295 },
	{ en296, pl296 },
	{ en297, pl297 },
	{ en298, pl298 },
	{ en299, pl299 },
	{ en300, pl300 },
	{ en301, pl301 },
	{ en302, pl302 },
	{ en303, pl303 },
	{ en304, pl304 },
	{ en305, pl305 },
	{ en306, pl306 },
	{ en307, pl307 },
	{ en308, pl308 },
	{ en309, pl309 },
	{ en310, pl310 },
	{ en311, pl311 },
	{ en312, pl312 },
	{ en313, pl313 },
	{ en314, pl314 },
	{ en315, pl315 },
	{ en316, pl316 },
	{ en317, pl317 },
	{ en318, pl318 },
	{ en319, pl319 },
	{ en320, pl320 },
	{ en321, pl321 },
	{ en322, pl322 },
	{ en323, pl323 },
	{ en324, pl324 },
	{ en325, pl325 },
	{ en326, pl326 },
	{ en327, pl327 },
	{ en328, pl328 },
	{ en329, pl329 },
	{ en330, pl330 },
	{ en331, pl331 },
	{ en332, pl332 },
	{ en333, pl333 },
	{ en334, pl334 },
	{ en335, pl335 },
	{ en336, pl336 },
	{ en337, pl337 },
	{ en338, pl338 },
	{ en339, pl339 },
	{ en340, pl340 },
	{ en341, pl341 },
	{ en342, pl342 },
	{ en343, pl343 },
	{ en344, pl344 },
	{ en345, pl345 },
	{ en346, pl346 },
	{ en347, pl347 },
	{ en348, pl348 },
	{ en349, pl349 },
	{ en350, pl350 },
	{ en351, pl351 },
	{ en352, pl352 },
	{ en353, pl353 },
	{ en354, pl354 },
	{ en355, pl355 },
	{ en356, pl356 },
	{ en357, pl357 },
	{ en358, pl358 },
	{ en359, pl359 },
	{ en360, pl360 },
	{ en361, pl361 },
	{ en362, pl362 },
	{ en363, pl363 },
	{ en364, pl364 },
	{ en365, pl365 },
	{ en366, pl366 },
	{ en367, pl367 },
	{ en368, pl368 },
	{ en369, pl369 },
	{ en370, pl370 },
	{ en371, pl371 },
	{ en372, pl372 },
	{ en373, pl373 },
	{ en374, pl374 },
	{ en375, pl375 },
	{ en376, pl376 },
	{ en377, pl377 },
	{ en378, pl378 },
	{ en379, pl379 },
	{ en380, pl380 },
	{ en381, pl381 },
	{ en382, pl382 },
	{ en383, pl383 },
	{ en384, pl384 },
	{ en385, pl385 },
	{ en386, pl386 },
	{ en387, pl387 },
	{ en388, pl388 },
	{ en389, pl389 },
};

const uint16_t uiTranslationsPLCount = 390;
