/*
 * Fork: SSTV picture receive screen (main menu > SSTV), by Marcin SP7TQ.
 *
 * Open it on an FM channel / VFO, or step through the presets with UP / DOWN
 * (the channel / VFO, ISS 145.800, 437.800; RX only, the channel itself is not
 * changed). Opened from a satellite (satellite screen, '9') it keeps the
 * Doppler retune of that satellite running.
 * The RX audio goes to the SSTV decoder (functions/sstvRx.c), the mode comes
 * from the VIS header and the picture (160 x 120) is drawn line by line under
 * a one line status bar. Every complete picture is saved to the flash
 * (functions/sstvStore.h, 8 newest kept, read by the web tools) at once,
 * before anything can draw over it (about half a second). '0' clears
 * and waits for the next VIS, GREEN / RED leave. APRS / DTMF / CW decoding is
 * paused meanwhile (the SSTV decoder borrows their memory), and so are the
 * APRS beacons and message replies.
 *
 * Use of this source code or binary releases for commercial purposes is
 * strictly forbidden (see license.txt).
 */
#include "user_interface/uiGlobals.h"
#include "functions/trx.h"
#include "user_interface/menuSystem.h"
#include "user_interface/uiUtilities.h"
#include "user_interface/uiLocalisation.h"
#include "functions/sound.h"
#include "interfaces/i2s.h"
#include "functions/aprsRx.h"
#include "functions/sstvRx.h"
#include "functions/eventLog.h"
#include "functions/ticks.h"
#include "functions/sstvStore.h"
#include "functions/settings.h"
#include "user_interface/uiEmcom.h"

#if defined(HAS_SSTV_MENU) && defined(I2S_APRS_RX)

#define SSTV_Y0          (DISPLAY_SIZE_Y - SSTV_RX_ROWS) // picture under a one line status bar
// Screens of only 120 lines (DM-1701): the status line covers the top of the picture
#define SSTV_STATUS_H    ((SSTV_Y0 >= FONT_SIZE_1_HEIGHT) ? SSTV_Y0 : FONT_SIZE_1_HEIGHT)
#define SSTV_REDRAW_MS   250U
#define SSTV_CHARS       26

static uint32_t lastDraw = 0;
static sstvRxState_t lastState = SSTV_RX_IDLE;
static bool hasPicture = false;
static bool doppler = false;        // opened from the satellite screen
static uint32_t nextDopplerTime = 0;
static uint8_t preset = 0;          // 0: the channel / VFO as it was
static bool savedPicture = false;   // the picture on screen is in the flash
static CodeplugChannel_t *presetChannel = NULL; // RX only copy used by the presets (txFreq 0: PTT does nothing)

// RX frequencies in 10 Hz units (codeplug units)
static const struct
{
	uint32_t    freq;
	const char *name;
} SSTV_PRESETS[] =
{
	{ 0U,        NULL  },  // channel / VFO
	{ 14580000U, "ISS" },  // ISS SSTV events, 2 m
	{ 43780000U, NULL  },  // ISS 70 cm downlink (cross band repeater, some SSTV events)
};
#define SSTV_NUM_PRESETS  (sizeof(SSTV_PRESETS) / sizeof(SSTV_PRESETS[0]))

// IRQ context: the decoder writes the pixels straight into the screen buffer
static void sstvPixel(uint16_t x, uint16_t row, uint8_t r, uint8_t g, uint8_t b)
{
	uint16_t *fb = displayGetPrimaryScreenBuffer();

	fb[((SSTV_Y0 + row) * DISPLAY_SIZE_X) + x] = displayConvertRGB888ToNative(((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

// Coloured field with the text in the background colour; returns its width
static int16_t chip(int16_t x, int16_t y, int16_t h, const char *text, themeItem_t colour, themeItem_t bg)
{
	int16_t w = (int16_t)((strlen(text) * 6) + 4);

	displayThemeApply(colour, bg);
	displayFillRect(x, y, w, h, false);
	displayThemeApply(colour, THEME_ITEM_BG);
	displayPrintCore((x + 2), (y + ((h - FONT_SIZE_1_HEIGHT) / 2)), text, FONT_SIZE_1_BOLD, TEXT_ALIGN_LEFT, true);
	return w;
}

static const char *rxStateText(const aprsRxStatus_t *rx, themeItem_t *colour)
{
	if (rx->state == APRS_RX_STATE_LISTENING)
	{
		*colour = THEME_ITEM_FG_RSSI_BAR;
		return "RX";
	}
	if (trxGetMode() != RADIO_MODE_ANALOG)
	{
		*colour = THEME_ITEM_FG_RSSI_BAR_S9P;
		return "FM ONLY";
	}
	if (rx->state == APRS_RX_STATE_NO_CLOCK)
	{
		*colour = THEME_ITEM_FG_TX_COUNTER;
		return "NO CLK";
	}
	*colour = THEME_ITEM_FG_RSSI_BAR_S9P;
	return "PAUSE";
}

// "145.800 ISS", "437.7963 ISS DOP": the frequency the screen listens on
static void freqText(char *buf, size_t size)
{
	uint32_t f = currentChannelData->rxFreq; // 10 Hz units
	uint32_t mhz = (f / 100000U);
	uint32_t frac = (f % 100000U);

	if (doppler)
	{
		snprintf(buf, size, "%u.%04u %.8s DOP", (unsigned)mhz, (unsigned)(frac / 10U), menuSatelliteActiveName());
	}
	else
	{
		const char *name = SSTV_PRESETS[preset].name;

		snprintf(buf, size, "%u.%03u MHz%s%s", (unsigned)mhz, (unsigned)(frac / 100U), ((name != NULL) ? " " : ""), ((name != NULL) ? name : ""));
	}
}

// Back to the channel / VFO the screen was opened on (as the satellite screen does)
static void restoreChannel(void)
{
	if (menuSystemGetRootMenuNumber() == UI_CHANNEL_MODE)
	{
		currentChannelData = &channelScreenChannelData;
		uiChannelModeLoadChannelData(true, false);
	}
	else
	{
		currentChannelData = &settingsVFOChannel[nonVolatileSettings.currentVFONumber];
		uiVFOModeLoadChannelData(false);
	}
}

static void applyPreset(uint8_t newPreset)
{
	if (newPreset == 0U)
	{
		if (preset != 0U)
		{
			restoreChannel();
		}
		presetChannel = NULL;
	}
	else
	{
		if (preset == 0U)
		{
			presetChannel = menuSatelliteBorrowChannelData();
			memcpy(presetChannel, currentChannelData, sizeof(CodeplugChannel_t));
		}
		presetChannel->rxFreq = SSTV_PRESETS[newPreset].freq;
		presetChannel->txFreq = 0U; // RX only: a downlink is never transmitted on
		presetChannel->chMode = RADIO_MODE_ANALOG;
		presetChannel->rxTone = CODEPLUG_CSS_TONE_NONE;
		codeplugChannelSetFlag(presetChannel, CHANNEL_FLAG_BW_25K, 1);

		currentChannelData = presetChannel;
		trxSetModeAndBandwidth(RADIO_MODE_ANALOG, true);
		trxSetRxCSS(RADIO_DEVICE_PRIMARY, CODEPLUG_CSS_TONE_NONE);
		trxSetFrequency(presetChannel->rxFreq, presetChannel->rxFreq, DMR_MODE_AUTO);
		trxPostponeReadRSSIAndNoise(100);
	}
	preset = newPreset;
}

#if defined(HAS_EMCOM_UI)
static void drawIdleEmcom(const sstvRxStatus_t *st, const aprsRxStatus_t *rx);
static void drawPictureBarEmcom(const sstvRxStatus_t *st, const aprsRxStatus_t *rx);
#endif

// No picture yet: the full layout of the other screens (title, status strip, hint, key bar)
static void drawIdle(const sstvRxStatus_t *st, const aprsRxStatus_t *rx)
{
	char line[SSTV_CHARS + 1];
	themeItem_t colour;
	const char *state = rxStateText(rx, &colour);
	int16_t sy = MENU_HEADER_HEIGHT;

#if defined(HAS_EMCOM_UI)
	if (emcomStyleEnabled())
	{
		drawIdleEmcom(st, rx);
		return;
	}
#endif

	displayClearBuf();
	menuDisplayTitle("SSTV");

	displayThemeApply(THEME_ITEM_BG_HEADER_TEXT, THEME_ITEM_BG);
	displayFillRect(0, sy, DISPLAY_SIZE_X, 11, false);
	int16_t x = (int16_t)(2 + chip(2, (sy + 1), 9, state, colour, THEME_ITEM_BG_HEADER_TEXT) + 4);

	if (st->lastVis)
	{
		snprintf(line, sizeof(line), "VIS %u ?", (unsigned)st->lastVis); // not a known mode
	}
	else if (st->freqHz)
	{
		snprintf(line, sizeof(line), "%u Hz", (unsigned)st->freqHz);
	}
	else
	{
		line[0] = 0;
	}
	displayThemeApply(THEME_ITEM_FG_HEADER_TEXT, THEME_ITEM_BG_HEADER_TEXT);
	displayPrintCore(x, (sy + 2), line, FONT_SIZE_1, TEXT_ALIGN_LEFT, false);
	snprintf(line, sizeof(line), "%ddBm", trxGetRSSIdBm(RADIO_DEVICE_PRIMARY));
	displayThemeApply(THEME_ITEM_FG_DECORATION, THEME_ITEM_BG_HEADER_TEXT);
	displayPrintCore((DISPLAY_SIZE_X - 3 - (int16_t)(strlen(line) * 6)), (sy + 2), line, FONT_SIZE_1, TEXT_ALIGN_LEFT, false);

	if (trxGetMode() != RADIO_MODE_ANALOG)
	{
		displayThemeApply(THEME_ITEM_FG_RSSI_BAR_S9P, THEME_ITEM_BG);
		displayPrintCentered(44, "FM only", FONT_SIZE_3);
		displayThemeApply(THEME_ITEM_FG_DEFAULT, THEME_ITEM_BG);
		displayPrintCentered(66, "Switch to an FM channel", FONT_SIZE_1);
		displayPrintCentered(76, "or UP/DOWN: FM preset", FONT_SIZE_1);
	}
	else
	{
		displayThemeApply(THEME_ITEM_FG_DEFAULT, THEME_ITEM_BG);
		displayPrintCentered(36, "Waiting for", FONT_SIZE_3);
		displayPrintCentered(54, "SSTV picture", FONT_SIZE_3);
		displayThemeApply(THEME_ITEM_FG_RX_FREQ, THEME_ITEM_BG);
		freqText(line, sizeof(line));
		displayPrintCentered(78, line, FONT_SIZE_1);
		displayThemeApply(THEME_ITEM_FG_DECORATION, THEME_ITEM_BG);
		displayPrintCentered(90, "ROBOT MARTIN SCOTTIE PD", FONT_SIZE_1);
#if defined(HAS_SSTV_STORE)
		snprintf(line, sizeof(line), "Saved %u/%u", (unsigned)sstvStoreCount(), (unsigned)SSTV_STORE_SLOTS);
		displayPrintCentered(102, line, FONT_SIZE_1);
#endif
	}

	displayThemeApply(THEME_ITEM_BG_HEADER_TEXT, THEME_ITEM_BG);
	displayFillRect(0, (DISPLAY_SIZE_Y - 11), DISPLAY_SIZE_X, 11, false);
	displayThemeApply(THEME_ITEM_FG_HEADER_TEXT, THEME_ITEM_BG_HEADER_TEXT);
	displayPrintCore(3, (DISPLAY_SIZE_Y - 9), (doppler ? "DOPPLER  0 CLEAR  RED EXIT" : "UP/DN QRG 0 CLEAR RED EXIT"), FONT_SIZE_1, TEXT_ALIGN_LEFT, false);
	displayThemeResetToDefault();
}

// Picture shown: compact bar above it (state field, mode, line / done)
static void drawPictureBar(const sstvRxStatus_t *st, const aprsRxStatus_t *rx)
{
	char line[SSTV_CHARS + 1];
	themeItem_t colour;
	const char *state = rxStateText(rx, &colour);

#if defined(HAS_EMCOM_UI)
	if (emcomStyleEnabled())
	{
		drawPictureBarEmcom(st, rx);
		return;
	}
#endif

	if ((st->state == SSTV_RX_DONE) && (rx->state == APRS_RX_STATE_LISTENING))
	{
		state = "DONE";
		colour = THEME_ITEM_FG_RX_FREQ;
	}

	displayThemeApply(THEME_ITEM_BG_HEADER_TEXT, THEME_ITEM_BG);
	displayFillRect(0, 0, DISPLAY_SIZE_X, SSTV_STATUS_H, false);
	int16_t x = (int16_t)(chip(0, 0, SSTV_STATUS_H, state, colour, THEME_ITEM_BG_HEADER_TEXT) + 4);

	displayThemeApply(THEME_ITEM_FG_HEADER_TEXT, THEME_ITEM_BG_HEADER_TEXT);
	displayPrintCore(x, ((SSTV_STATUS_H - FONT_SIZE_1_HEIGHT) / 2), st->modeName, FONT_SIZE_1, TEXT_ALIGN_LEFT, false);

	if (st->state == SSTV_RX_RECEIVING)
	{
		snprintf(line, sizeof(line), "%u/%u", (unsigned)(st->row + 1), (unsigned)SSTV_RX_ROWS);
	}
#if defined(HAS_SSTV_STORE)
	else if (sstvStoreBusy())
	{
		snprintf(line, sizeof(line), "SAVING");
	}
	else if (savedPicture)
	{
		snprintf(line, sizeof(line), "SAVED 0 NEW");
	}
#endif
	else
	{
		snprintf(line, sizeof(line), "0 NEW");
	}
	displayThemeApply(THEME_ITEM_FG_DECORATION, THEME_ITEM_BG_HEADER_TEXT);
	displayPrintCore((DISPLAY_SIZE_X - 2 - (int16_t)(strlen(line) * 6)), ((SSTV_STATUS_H - FONT_SIZE_1_HEIGHT) / 2), line, FONT_SIZE_1, TEXT_ALIGN_LEFT, false);
	displayThemeResetToDefault();
}

#if defined(HAS_EMCOM_UI)
// EMCOM style, no picture yet: title with the pictures saved, status strip (state, DOPPLER,
// tone / VIS, level), the frequency big, the key bar
static void drawIdleEmcom(const sstvRxStatus_t *st, const aprsRxStatus_t *rx)
{
	char line[SSTV_CHARS + 1];
	themeItem_t colour;
	const char *state = rxStateText(rx, &colour);
	int16_t y = EMCOM_UI_BODY_Y;

	displayClearBuf();
#if defined(HAS_SSTV_STORE)
	snprintf(line, sizeof(line), "%u/%u", (unsigned)sstvStoreCount(), (unsigned)SSTV_STORE_SLOTS);
	emcomUiTitle("SSTV", line);
#else
	emcomUiTitle("SSTV", NULL);
#endif

	emcomUiStrip();
	int16_t x = (int16_t)(2 + emcomUiChip(2, (EMCOM_UI_STRIP_Y + 2), state, colour, EMCOM_CHIP_SOLID, false) + 3);
	if (doppler)
	{
		x = (int16_t)(x + emcomUiChip(x, (EMCOM_UI_STRIP_Y + 2), "DOPPLER", THEME_ITEM_FG_RX_FREQ, EMCOM_CHIP_OUTLINE, false) + 3);
	}
	if (st->lastVis)
	{
		snprintf(line, sizeof(line), "VIS %u ?", (unsigned)st->lastVis); // not a known mode
	}
	else if (st->freqHz)
	{
		snprintf(line, sizeof(line), "%uHz", (unsigned)st->freqHz);
	}
	else
	{
		line[0] = 0;
	}
	emcomUiText(x, (EMCOM_UI_STRIP_Y + 2), line, FONT_SIZE_1, THEME_ITEM_FG_HEADER_TEXT, false);
	snprintf(line, sizeof(line), "%ddBm", trxGetRSSIdBm(RADIO_DEVICE_PRIMARY));
	emcomUiText((DISPLAY_SIZE_X - 3), (EMCOM_UI_STRIP_Y + 2), line, FONT_SIZE_1, THEME_ITEM_FG_DECORATION, true);

	if (trxGetMode() != RADIO_MODE_ANALOG)
	{
		displayThemeApply(THEME_ITEM_FG_RSSI_BAR_S9P, THEME_ITEM_BG);
		displayPrintCentered((y + 14), "FM only", FONT_SIZE_3);
		displayThemeResetToDefault();
		emcomUiText(4, (y + 38), "Switch to an FM channel", FONT_SIZE_1, THEME_ITEM_FG_DEFAULT, false);
		emcomUiText(4, (y + 48), "or UP/DOWN: FM preset", FONT_SIZE_1, THEME_ITEM_FG_DEFAULT, false);
	}
	else
	{
		displayThemeApply(THEME_ITEM_FG_DEFAULT, THEME_ITEM_BG);
		displayPrintCentered((y + 6), "Waiting for", FONT_SIZE_3);
		displayPrintCentered((y + 22), "SSTV picture", FONT_SIZE_3);
		emcomUiDivider(y + 42);
		freqText(line, sizeof(line));
		displayThemeApply(THEME_ITEM_FG_RX_FREQ, THEME_ITEM_BG);
		if ((strlen(line) * 8) <= (DISPLAY_SIZE_X - 4))
		{
			displayPrintCentered((y + 47), line, FONT_SIZE_3);
		}
		else
		{
			displayPrintCentered((y + 51), line, (emcomUiLarge() ? FONT_SIZE_1_BOLD : FONT_SIZE_1));
		}
		displayThemeApply(THEME_ITEM_FG_DECORATION, THEME_ITEM_BG);
		displayPrintCentered((y + 68), "ROBOT MARTIN SCOTTIE PD", (emcomUiLarge() ? FONT_SIZE_1_BOLD : FONT_SIZE_1));
		displayThemeResetToDefault();
	}

	emcomUiKeyBar(doppler ? "DOPPLER  0 CLEAR  RED EXIT" : "UP/DN QRG 0 CLEAR RED EXIT");
}

// EMCOM style, picture shown: the one line bar above it only (state, mode, frequency or DOPPLER,
// progress / saved), the progress also as a line along the bottom of the bar
static void drawPictureBarEmcom(const sstvRxStatus_t *st, const aprsRxStatus_t *rx)
{
	char right[SSTV_CHARS + 1];
	char middle[SSTV_CHARS + 1];
	themeItem_t colour;
	themeItem_t middleColour = THEME_ITEM_FG_HEADER_TEXT;
	const char *state = rxStateText(rx, &colour);
	int16_t ty = ((SSTV_STATUS_H - FONT_SIZE_1_HEIGHT) / 2);

	if ((st->state == SSTV_RX_DONE) && (rx->state == APRS_RX_STATE_LISTENING))
	{
		state = "DONE";
		colour = THEME_ITEM_FG_RX_FREQ;
	}

	if (doppler)
	{
		snprintf(middle, sizeof(middle), "DOPPLER");
		middleColour = THEME_ITEM_FG_RX_FREQ;
	}
	else
	{
		uint32_t f = currentChannelData->rxFreq; // 10 Hz units

		snprintf(middle, sizeof(middle), "%u.%03u", (unsigned)(f / 100000U), (unsigned)((f % 100000U) / 100U));
	}

	if (st->state == SSTV_RX_RECEIVING)
	{
		snprintf(right, sizeof(right), "%u/%u", (unsigned)(st->row + 1), (unsigned)SSTV_RX_ROWS);
	}
#if defined(HAS_SSTV_STORE)
	else if (sstvStoreBusy())
	{
		snprintf(right, sizeof(right), "SAVING");
	}
	else if (savedPicture)
	{
		snprintf(right, sizeof(right), "SAVED %u/%u", (unsigned)sstvStoreCount(), (unsigned)SSTV_STORE_SLOTS);
		snprintf(middle, sizeof(middle), "0 NEW");
		middleColour = THEME_ITEM_FG_DECORATION;
	}
#endif
	else
	{
		snprintf(right, sizeof(right), "0 NEW");
	}

	displayThemeApply(THEME_ITEM_BG_HEADER_TEXT, THEME_ITEM_BG);
	displayFillRect(0, 0, DISPLAY_SIZE_X, SSTV_STATUS_H, false);
	int16_t x = (int16_t)(chip(0, 0, SSTV_STATUS_H, state, colour, THEME_ITEM_BG_HEADER_TEXT) + 4);
	x = (int16_t)(x + emcomUiText(x, ty, st->modeName, FONT_SIZE_1, THEME_ITEM_FG_HEADER_TEXT, false) + 6);

	int16_t rw = emcomUiText((DISPLAY_SIZE_X - 2), ty, right, FONT_SIZE_1, ((st->state == SSTV_RX_RECEIVING) ? THEME_ITEM_FG_HEADER_TEXT : THEME_ITEM_FG_DECORATION), true);
	if ((x + (int16_t)(strlen(middle) * 6) + 6) <= (DISPLAY_SIZE_X - 2 - rw))
	{
		emcomUiText(x, ty, middle, FONT_SIZE_1, middleColour, false);
	}

	if (st->state == SSTV_RX_RECEIVING)
	{
		displayThemeApply(THEME_ITEM_FG_RSSI_BAR, THEME_ITEM_BG);
		displayFillRect(0, (SSTV_STATUS_H - 1), (int16_t)(((st->row + 1U) * DISPLAY_SIZE_X) / SSTV_RX_ROWS), 1, false);
	}
	displayThemeResetToDefault();
}
#endif

static void updateScreen(void)
{
	sstvRxStatus_t st;
	aprsRxStatus_t rx;

	sstvRxGetStatus(&st);
	aprsRxGetStatus(&rx);

	// a new picture starts: the whole screen goes to the picture
	if ((st.state == SSTV_RX_RECEIVING) && (lastState != SSTV_RX_RECEIVING))
	{
		displayThemeApply(THEME_ITEM_FG_DEFAULT, THEME_ITEM_BG);
		displayFillRect(0, 0, DISPLAY_SIZE_X, DISPLAY_SIZE_Y, true);
		displayThemeResetToDefault();
		hasPicture = true;
	}
	if ((st.state == SSTV_RX_DONE) && (lastState == SSTV_RX_RECEIVING))
	{
#if defined(HAS_EVENT_LOG)
		eventLogAdd(EVENT_LOG_SSTV, "%s %u lines", st.modeName, (unsigned)(st.row + 1));
#endif
#if defined(HAS_SSTV_STORE)
		// Saved right away, from the screen buffer: a key, a new picture or a screen pushed on
		// top would draw over it if the save were spread over the next ticks
		if (((uint32_t)(st.row + 1U) >= SSTV_STORE_MIN_ROWS) &&
				sstvStoreBegin(&displayGetPrimaryScreenBuffer()[SSTV_Y0 * DISPLAY_SIZE_X], (uint16_t)(st.row + 1U), st.modeName,
						currentChannelData->rxFreq, uiDataGlobal.dateTimeSecs))
		{
			lastState = st.state;
			drawPictureBar(&st, &rx); // "SAVING"
			displayRender();
			sstvStoreFinish();
			savedPicture = true;
		}
#endif
	}
	if (st.state == SSTV_RX_RECEIVING)
	{
		savedPicture = false;
	}
	lastState = st.state;

	if (hasPicture)
	{
		drawPictureBar(&st, &rx);
	}
	else
	{
		drawIdle(&st, &rx);
	}
	displayRender();
}

static void exitCallback(void *data)
{
	UNUSED_PARAMETER(data);
#if defined(HAS_SSTV_STORE)
	sstvStoreFinish(); // the picture is still in the screen buffer
#endif
	aprsRxSetSstv(false, NULL);
	aprsRxSetMonitor(false);
	if ((doppler == false) && (preset != 0U))
	{
		restoreChannel();
	}
	presetChannel = NULL;
	preset = 0U;
	doppler = false;
}

static void sstvScreenStart(uint32_t now)
{
	hasPicture = false;
	savedPicture = false;
	lastState = SSTV_RX_IDLE;
	lastDraw = 0;
	if ((presetChannel == NULL) || (currentChannelData != presetChannel))
	{
		preset = 0U; // else back from a screen pushed on top (PTT message ...): still on the preset
	}
	doppler = ((menuSystemGetPreviousMenuNumber() == MENU_SATELLITE) && menuSatelliteDopplerAvailable());
	nextDopplerTime = now;
	if (doppler)
	{
		currentChannelData->txFreq = 0U;
	}
	menuSystemRegisterExitCallback(exitCallback, NULL);
	aprsRxSetMonitor(true);
	aprsRxSetSstv(true, sstvPixel);
	updateScreen();
}

menuStatus_t menuSSTV(uiEvent_t *ev, bool isFirstRun)
{
	if (isFirstRun)
	{
		menuDataGlobal.numItems = 0;
		sstvScreenStart(ev->time);
		return MENU_STATUS_SUCCESS;
	}

	if (ev->hasEvent)
	{
		if ((ev->events & FUNCTION_EVENT) && (ev->function == FUNC_REDRAW))
		{
			updateScreen();
		}
		else if (ev->events & KEY_EVENT)
		{
			if (KEYCHECK_SHORTUP(ev->keys, KEY_RED) || KEYCHECK_SHORTUP(ev->keys, KEY_GREEN))
			{
				menuSystemPopPreviousMenu(); // exitCallback() stops the decoder and restores the channel
				return MENU_STATUS_SUCCESS;
			}
			else if ((doppler == false) && (KEYCHECK_PRESS(ev->keys, KEY_UP) || KEYCHECK_PRESS(ev->keys, KEY_DOWN)))
			{
				uint8_t p = (KEYCHECK_PRESS(ev->keys, KEY_UP) ? ((preset + 1U) % SSTV_NUM_PRESETS) :
						((preset + SSTV_NUM_PRESETS - 1U) % SSTV_NUM_PRESETS));

				applyPreset(p);
				sstvRxRestart(); // a picture half received on the old frequency is of no use
				hasPicture = false;
				savedPicture = false;
				lastState = SSTV_RX_IDLE;
				updateScreen();
			}
			else if (KEYCHECK_SHORTUP_NUMBER(ev->keys) && BUTTONCHECK_DOWN(ev, BUTTON_SK2))
			{
				saveQuickkeyMenuIndex(ev->keys.key, menuSystemGetCurrentMenuNumber(), 0, 0);
			}
			else if (KEYCHECK_SHORTUP(ev->keys, KEY_0))
			{
				sstvRxRestart();
				hasPicture = false;
				lastState = SSTV_RX_IDLE;
				soundSetMelody(MELODY_ACK_BEEP);
				updateScreen();
			}
		}
	}

	if (doppler && ((int32_t)(ev->time - nextDopplerTime) >= 0))
	{
		nextDopplerTime = (ev->time + 1000U);
		menuSatelliteDopplerTick();
		currentChannelData->txFreq = 0U; // RX only here too (the satellite screen sets the uplink again on return)
	}

	if ((ev->time - lastDraw) >= SSTV_REDRAW_MS)
	{
		lastDraw = ev->time;
		updateScreen();
	}

	return MENU_STATUS_SUCCESS;
}

#else
menuStatus_t menuSSTV(uiEvent_t *ev, bool isFirstRun)
{
	UNUSED_PARAMETER(ev);
	UNUSED_PARAMETER(isFirstRun);
	menuSystemPopPreviousMenu();
	return MENU_STATUS_SUCCESS;
}
#endif
