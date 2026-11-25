/*
 * Fork: DMR SMS inbox (main menu > DMR SMS), by Marcin SP7TQ. The messages received by
 * functions/dmrSms.c (RX path ported from dondch/OpenGD77-AES256, cleartext only).
 *
 * List, newest first: age, sender, the first words (unread ones bold, marked '*'). UP / DOWN
 * select, GREEN opens the message (wrapped text, marked read), '0' deletes the selected one, a
 * long '0' (then GREEN) deletes all, RED goes back / leaves. In the message UP / DOWN go to the
 * next / previous one. Receive only: sending is not implemented.
 *
 * Use of this source code or binary releases for commercial purposes is
 * strictly forbidden (see license.txt).
 */
#include "user_interface/uiGlobals.h"
#include "user_interface/menuSystem.h"
#include "user_interface/uiUtilities.h"
#include "user_interface/uiLocalisation.h"
#include "functions/sound.h"
#include "functions/dmrSms.h"
#include "user_interface/uiEmcom.h"

#if defined(HAS_DMR_SMS)

#define SMS_TITLE      "DMR SMS"
#define ROW_H          (FONT_SIZE_1_HEIGHT + 2)
#define LIST_Y         (MENU_HEADER_HEIGHT + 2)
#define FOOTER_H       20
#define ROWS           ((DISPLAY_SIZE_Y - LIST_Y - FOOTER_H) / ROW_H)
#define EM_ROW_H       11
#define NAME_X         28
#define TEXT_X         82
#define TEXT_CHARS     13      // (160 - 82) / 6
#define WRAP_CHARS_1   26      // 6 px font
#define WRAP_CHARS_3   20      // 8 px font

static uint32_t selected = 0;
static bool reading = false;
static bool confirmAll = false;

static void ageString(uint32_t t, char *buf, size_t size)
{
	uint32_t now = (uint32_t)uiDataGlobal.dateTimeSecs;
	uint32_t s;

	if ((t == 0U) || (now < t))
	{
		snprintf(buf, size, "--");
		return;
	}

	s = (now - t);
	if (s < 60U)
	{
		snprintf(buf, size, "%us", (unsigned)s);
	}
	else if (s < 3600U)
	{
		snprintf(buf, size, "%um", (unsigned)(s / 60U));
	}
	else if (s < 86400U)
	{
		snprintf(buf, size, "%uh", (unsigned)(s / 3600U));
	}
	else
	{
		snprintf(buf, size, "%ud", (unsigned)(s / 86400U));
	}
}

static void timeString(uint32_t t, char *buf, size_t size)
{
	struct tm tmv;
	time_t_custom secs = t;

	if (t == 0U)
	{
		snprintf(buf, size, "no clock");
		return;
	}
	gmtime_r_Custom(&secs, &tmv);
	snprintf(buf, size, "%02d-%02d %02d:%02dZ", (tmv.tm_mon + 1), tmv.tm_mday, tmv.tm_hour, tmv.tm_min);
}

// "TG 260" for a group message, else "" (private to us)
static void destString(const dmrSmsMessage_t *m, char *buf, size_t size)
{
	if (m->flags & DMR_SMS_FLAG_GROUP)
	{
		snprintf(buf, size, "TG %u", (unsigned)(m->dst & 0x00FFFFFFU));
	}
	else
	{
		snprintf(buf, size, "PRIV");
	}
}

// Word wrap at x 2 from y; returns the number of lines (drawn when 'draw')
static uint32_t wrapText(const char *str, uint32_t perLine, uint32_t maxLines, int16_t y, int16_t lineH, ucFont_t font, bool draw)
{
	char line[WRAP_CHARS_1 + 1];
	uint32_t lines = 0U;

	while (*str)
	{
		uint32_t len = strlen(str);
		uint32_t cut = len;

		if (len > perLine)
		{
			cut = perLine;
			while ((cut > 0U) && (str[cut] != ' '))
			{
				cut--;
			}
			if (cut == 0U)
			{
				cut = perLine;
			}
		}

		if (draw && (lines < maxLines))
		{
			memcpy(line, str, cut);
			line[cut] = 0;
			displayPrintCore(2, (int16_t)(y + (lines * lineH)), line, font, TEXT_ALIGN_LEFT, false);
		}
		lines++;
		str += cut;
		while (*str == ' ')
		{
			str++;
		}
	}
	return lines;
}

static bool emcom(void)
{
#if defined(HAS_EMCOM_UI)
	return emcomStyleEnabled();
#else
	return false;
#endif
}

static void drawEmpty(int16_t y)
{
	displayThemeApply(THEME_ITEM_FG_DEFAULT, THEME_ITEM_BG);
	displayPrintCentered((y + 16), "No messages", FONT_SIZE_3);
	displayThemeApply(THEME_ITEM_FG_DECORATION, THEME_ITEM_BG);
	displayPrintCentered((y + 40), "Private DMR SMS to your", FONT_SIZE_1);
	displayPrintCentered((y + 50), "ID and group SMS show here", FONT_SIZE_1);
	displayThemeResetToDefault();
}

// Classic style footer: two lines on the header colour
static void classicFooter(const char *line1, const char *line2, themeItem_t line1Colour)
{
	int16_t fy = (DISPLAY_SIZE_Y - FOOTER_H);

	displayThemeApply(THEME_ITEM_BG_HEADER_TEXT, THEME_ITEM_BG);
	displayFillRect(0, fy, DISPLAY_SIZE_X, FOOTER_H, false);
	displayThemeApply(line1Colour, THEME_ITEM_BG_HEADER_TEXT);
	displayPrintCore(3, (fy + 1), line1, FONT_SIZE_1_BOLD, TEXT_ALIGN_LEFT, false);
	displayThemeApply(THEME_ITEM_FG_HEADER_TEXT, THEME_ITEM_BG_HEADER_TEXT);
	displayPrintCore(3, (fy + 11), line2, FONT_SIZE_1, TEXT_ALIGN_LEFT, false);
	displayThemeResetToDefault();
}

static void drawList(void)
{
	char buf[32];
	char name[SCREEN_LINE_BUFFER_SIZE];
	dmrSmsMessage_t m;
	uint32_t count = dmrSmsCount();
	bool em = emcom();
	int16_t listY = (em ? EMCOM_UI_BODY_Y : LIST_Y);
	int16_t rowH = (em ? EM_ROW_H : ROW_H);
	uint32_t rows = (em ? (uint32_t)((EMCOM_UI_KEYBAR_Y - 1 - EMCOM_UI_BODY_Y) / EM_ROW_H) : ROWS);

	displayClearBuf();
	if (selected >= count)
	{
		selected = ((count > 0U) ? (count - 1U) : 0U);
	}
	snprintf(buf, sizeof(buf), "%u/%u", (unsigned)((count > 0U) ? (selected + 1U) : 0U), (unsigned)count);

#if defined(HAS_EMCOM_UI)
	if (em)
	{
		emcomUiTitle(SMS_TITLE, buf);
		emcomUiStrip();
		if ((count > 0U) && dmrSmsGet(selected, &m))
		{
			char dest[16];

			timeString(m.time, buf, sizeof(buf));
			emcomUiText(3, (EMCOM_UI_STRIP_Y + 2), buf, FONT_SIZE_1, THEME_ITEM_FG_HEADER_TEXT, false);
			destString(&m, dest, sizeof(dest));
			emcomUiChip((DISPLAY_SIZE_X - 2), (EMCOM_UI_STRIP_Y + 2), dest,
					((m.flags & DMR_SMS_FLAG_GROUP) ? THEME_ITEM_FG_RX_FREQ : THEME_ITEM_FG_RSSI_BAR), EMCOM_CHIP_SOLID, true);
		}
		else
		{
			emcomUiChip(2, (EMCOM_UI_STRIP_Y + 2), "EMPTY", THEME_ITEM_FG_DECORATION, EMCOM_CHIP_OUTLINE, false);
		}
	}
	else
#endif
	{
		menuDisplayTitle(SMS_TITLE);
		displayThemeApply(THEME_ITEM_FG_DECORATION, THEME_ITEM_BG_MENU_NAME);
		displayPrintCore((DISPLAY_SIZE_X - 3 - (int16_t)(strlen(buf) * 6)), 4, buf, FONT_SIZE_1, TEXT_ALIGN_LEFT, false);
		displayThemeResetToDefault();
	}

	if (count == 0U)
	{
		drawEmpty(listY);
#if defined(HAS_EMCOM_UI)
		if (em)
		{
			emcomUiKeyBar("RED EXIT");
		}
#endif
		displayRender();
		return;
	}

	uint32_t first = ((selected >= rows) ? (selected - rows + 1U) : 0U);

	for (uint32_t r = 0U; r < rows; r++)
	{
		uint32_t idx = (first + r);
		int16_t y = (int16_t)(listY + (r * rowH));
		bool sel = (idx == selected);

		if ((idx >= count) || (dmrSmsGet(idx, &m) == false))
		{
			break;
		}

		bool unread = ((m.flags & DMR_SMS_FLAG_UNREAD) != 0U);
		ucFont_t font = (unread ? FONT_SIZE_1_BOLD : FONT_SIZE_1);

		if (sel)
		{
			displayThemeApply(THEME_ITEM_BG_MENU_ITEM_SELECTED, THEME_ITEM_BG);
			displayFillRect(0, (y - (em ? 2 : 1)), DISPLAY_SIZE_X, rowH, false);
		}

		// em style selected row: text in the background colour on the selection bar; classic: inverted print
		themeItem_t fg = ((sel && em) ? THEME_ITEM_BG : THEME_ITEM_FG_DEFAULT);
		themeItem_t dim = ((sel && em) ? THEME_ITEM_BG : THEME_ITEM_FG_DECORATION);
		bool invert = (sel && (em == false));

		ageString(m.time, buf, sizeof(buf));
		displayThemeApply((unread ? ((sel && em) ? THEME_ITEM_BG : THEME_ITEM_FG_RSSI_BAR) : dim), THEME_ITEM_BG);
		displayPrintCore(2, y, (unread ? "*" : " "), FONT_SIZE_1_BOLD, TEXT_ALIGN_LEFT, invert);
		displayThemeApply(dim, THEME_ITEM_BG);
		displayPrintCore(8, y, buf, FONT_SIZE_1, TEXT_ALIGN_LEFT, invert);

		dmrSmsPeerName(m.src, name, sizeof(name));
		snprintf(buf, sizeof(buf), "%.8s", name);
		displayThemeApply(fg, THEME_ITEM_BG);
		displayPrintCore(NAME_X, y, buf, font, TEXT_ALIGN_LEFT, invert);

		snprintf(buf, sizeof(buf), "%.*s", TEXT_CHARS, m.text);
		displayPrintCore(TEXT_X, y, buf, font, TEXT_ALIGN_LEFT, invert);
		displayThemeResetToDefault();
	}

#if defined(HAS_EMCOM_UI)
	if (em)
	{
		if (confirmAll)
		{
			emcomUiConfirmBar("DELETE ALL?");
		}
		else
		{
			emcomUiKeyBar("GRN READ  0 DEL  RED EXIT");
		}
		displayRender();
		return;
	}
#endif

	if (confirmAll)
	{
		classicFooter("DELETE ALL MESSAGES?", "GREEN yes, other key no", THEME_ITEM_FG_TX_COUNTER);
	}
	else
	{
		classicFooter("GREEN read  0 delete", "long 0 delete all", THEME_ITEM_FG_HEADER_TEXT);
	}
	displayRender();
}

static void drawMessage(void)
{
	char buf[32];
	char name[SCREEN_LINE_BUFFER_SIZE];
	char dest[16];
	dmrSmsMessage_t m;
	bool em = emcom();

	if (dmrSmsGet(selected, &m) == false)
	{
		reading = false;
		drawList();
		return;
	}

	displayClearBuf();
	dmrSmsPeerName(m.src, name, sizeof(name));
	destString(&m, dest, sizeof(dest));
	timeString(m.time, buf, sizeof(buf));

	int16_t bodyY;
	int16_t bodyBottom;

#if defined(HAS_EMCOM_UI)
	if (em)
	{
		emcomUiTitle(name, dest);
		emcomUiStrip();
		emcomUiText(3, (EMCOM_UI_STRIP_Y + 2), buf, FONT_SIZE_1, THEME_ITEM_FG_HEADER_TEXT, false);
		snprintf(buf, sizeof(buf), "%u", (unsigned)(m.src & 0x00FFFFFFU));
		emcomUiText((DISPLAY_SIZE_X - 3), (EMCOM_UI_STRIP_Y + 2), buf, FONT_SIZE_1, THEME_ITEM_FG_HEADER_TEXT, true);
		bodyY = EMCOM_UI_BODY_Y;
		bodyBottom = (EMCOM_UI_KEYBAR_Y - 1);
	}
	else
#endif
	{
		menuDisplayTitle(name);
		displayThemeApply(THEME_ITEM_FG_DECORATION, THEME_ITEM_BG);
		displayPrintCore(2, LIST_Y, buf, FONT_SIZE_1, TEXT_ALIGN_LEFT, false);
		displayPrintCore((DISPLAY_SIZE_X - 2 - (int16_t)(strlen(dest) * 6)), LIST_Y, dest, FONT_SIZE_1, TEXT_ALIGN_LEFT, false);
		bodyY = (LIST_Y + FONT_SIZE_1_HEIGHT + 4);
		bodyBottom = (DISPLAY_SIZE_Y - FOOTER_H);
	}

	// 8x16 when the whole text fits, else the small font
	uint32_t room3 = (uint32_t)((bodyBottom - bodyY) / FONT_SIZE_3_HEIGHT);
	uint32_t room1 = (uint32_t)((bodyBottom - bodyY) / (FONT_SIZE_1_HEIGHT + 1));
	bool big = (wrapText(m.text, WRAP_CHARS_3, 0U, 0, 0, FONT_SIZE_3, false) <= room3);

	displayThemeApply(THEME_ITEM_FG_DEFAULT, THEME_ITEM_BG);
	if (big)
	{
		wrapText(m.text, WRAP_CHARS_3, room3, bodyY, FONT_SIZE_3_HEIGHT, FONT_SIZE_3, true);
	}
	else
	{
		wrapText(m.text, WRAP_CHARS_1, room1, bodyY, (FONT_SIZE_1_HEIGHT + 1), FONT_SIZE_1, true);
	}
	displayThemeResetToDefault();

#if defined(HAS_EMCOM_UI)
	if (em)
	{
		emcomUiKeyBar("UP/DN  0 DEL  RED BACK");
		displayRender();
		return;
	}
#endif
	snprintf(buf, sizeof(buf), "%u/%u", (unsigned)(selected + 1U), (unsigned)dmrSmsCount());
	classicFooter(buf, "UP/DN next  0 delete  RED back", THEME_ITEM_FG_HEADER_TEXT);
	displayRender();
}

static void updateScreen(void)
{
	if (reading)
	{
		dmrSmsMarkRead(selected);
		drawMessage();
	}
	else
	{
		drawList();
	}
}

menuStatus_t menuDmrSms(uiEvent_t *ev, bool isFirstRun)
{
	static uint32_t lastSerial = 0U;

	if (isFirstRun)
	{
		menuDataGlobal.numItems = 0;
		selected = 0U;
		reading = false;
		confirmAll = false;
		lastSerial = dmrSmsSerial();
		updateScreen();
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
			if (confirmAll)
			{
				// decided on a key release (not the release of the long '0' that asked)
				if ((ev->keys.event & KEY_MOD_UP) && ((ev->keys.key != KEY_0) || ((ev->keys.event & KEY_MOD_LONG) == 0)))
				{
					if (KEYCHECK_SHORTUP(ev->keys, KEY_GREEN))
					{
						dmrSmsDeleteAll();
						selected = 0U;
						soundSetMelody(MELODY_ACK_BEEP);
					}
					confirmAll = false;
					lastSerial = dmrSmsSerial();
					updateScreen();
				}
			}
			else if (KEYCHECK_SHORTUP(ev->keys, KEY_RED))
			{
				if (reading)
				{
					reading = false;
					updateScreen();
				}
				else
				{
					menuSystemPopPreviousMenu();
					return MENU_STATUS_SUCCESS;
				}
			}
			else if (KEYCHECK_SHORTUP_NUMBER(ev->keys) && BUTTONCHECK_DOWN(ev, BUTTON_SK2))
			{
				saveQuickkeyMenuIndex(ev->keys.key, menuSystemGetCurrentMenuNumber(), 0, 0);
			}
			else if (KEYCHECK_SHORTUP(ev->keys, KEY_GREEN))
			{
				if (reading)
				{
					reading = false;
				}
				else if (dmrSmsCount() > 0U)
				{
					reading = true;
				}
				updateScreen();
			}
			else if (KEYCHECK_LONGDOWN(ev->keys, KEY_0) && (reading == false) && (dmrSmsCount() > 0U))
			{
				confirmAll = true;
				updateScreen();
			}
			else if (KEYCHECK_SHORTUP(ev->keys, KEY_0) && (dmrSmsCount() > 0U))
			{
				dmrSmsDelete(selected);
				lastSerial = dmrSmsSerial();
				soundSetMelody(MELODY_ACK_BEEP);
				if (dmrSmsCount() == 0U)
				{
					reading = false;
				}
				updateScreen();
			}
			else if (KEYCHECK_PRESS(ev->keys, KEY_DOWN))
			{
				if ((selected + 1U) < dmrSmsCount())
				{
					selected++;
				}
				updateScreen();
			}
			else if (KEYCHECK_PRESS(ev->keys, KEY_UP))
			{
				if (selected > 0U)
				{
					selected--;
				}
				updateScreen();
			}
		}
	}

	// A message arrived while the screen is shown: keep the same one selected
	uint32_t s = dmrSmsSerial();
	if (s != lastSerial)
	{
		if ((selected != 0U) || reading)
		{
			selected++;
		}
		lastSerial = s;
		updateScreen();
	}

	return MENU_STATUS_SUCCESS;
}

#else
menuStatus_t menuDmrSms(uiEvent_t *ev, bool isFirstRun)
{
	UNUSED_PARAMETER(ev);
	UNUSED_PARAMETER(isFirstRun);
	menuSystemPopPreviousMenu();
	return MENU_STATUS_SUCCESS;
}
#endif
