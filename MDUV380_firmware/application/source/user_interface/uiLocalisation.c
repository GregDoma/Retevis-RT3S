/*
 * Copyright (C) 2019-2025 Roger Clark, VK3KYY / G4KYF
 *                         Daniel Caujolle-Bert, F1RMB
 *
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. Use of this source code or binary releases for commercial purposes is strictly forbidden. This includes, without limitation,
 *    incorporation in a commercial product or incorporation into a product or project which allows commercial use.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "main.h"
#include "user_interface/uiLocalisation.h"

#include "user_interface/languages/english.h"
#include "user_interface/languages/polish.h"
#include <string.h>


/*
 * Note.
 *
 * Do not re-order the list of languages, unless you also change the MagicNumber in the settings
 * Because otherwise the radio will load a different language than the one the user previously saved when the radio was turned off
 * Add new languages at the end of the list
 *
 */
const stringsTable_t languages[]=
{
		englishLanguage,        // englishLanguageName
		polishLanguage         // polishLanguageName (fork M0SQQ: built in, no .gla needed)
};
const stringsTable_t *currentLanguage;


uint8_t languagesGetCount(void)
{
	return (sizeof(languages) / sizeof(languages[0]));
}

char currentLanguageGetSymbol(LanguageSymbol_t s)
{
	return currentLanguage->symbols[s];
}

// Fork: upper case conversion aware of the remapped font glyphs (HX8353E_charset.h),
// so that titles converted to capitals keep the Polish (and other) diacritics,
// e.g. 'ł' (\210) -> 'Ł' (\207). Characters without an upper case glyph are returned unchanged.
char languageCharToUpper(char c)
{
	uint8_t u = (uint8_t)c;

	if ((u >= 'a') && (u <= 'z'))
	{
		return (char)(u - ('a' - 'A'));
	}

	switch (u)
	{
		case 0x80: return (char)0x7F; // č -> Č
		case 0x82: return (char)0x81; // ć -> Ć
		case 0x84: return (char)0x83; // ą -> Ą
		case 0x86: return (char)0x85; // ę -> Ę
		case 0x88: return (char)0x87; // ł -> Ł
		case 0x8B: return (char)0x89; // ń -> Ń
		case 0x8D: return (char)0x8C; // ś -> Ś
		case 0x90: return (char)0x8F; // ź -> Ź
		case 0x92: return (char)0x91; // ż -> Ż
		case 0x96: return (char)0x95; // ş -> Ş
		case 0x98: return (char)0x97; // ğ -> Ğ
		case 0x9A: return (char)0x8A; // š -> Š
		case 0x9B: return (char)0x99; // ř -> Ř
		case 0x9D: return (char)0x9C; // ě -> Ě
		case 0x9E: return (char)0x8E; // ž -> Ž
		case 0xA1: return (char)0xA0; // ů -> Ů
		case 0xA3: return (char)0xA2; // ő -> Ő
		case 0xA5: return (char)0xA4; // ű -> Ű
		case 0xA7: return (char)0xA6; // ň -> Ň
		case 0xAA: return (char)0xA9; // ď -> Ď
		default:
			break;
	}

	// Latin-1 small letters (à..þ) -> capitals (À..Þ), except the free font positions (æ/Æ, ÷, þ/Þ)
	if ((u >= 0xE0) && (u <= 0xFD) && (u != 0xE6) && (u != 0xF7))
	{
		return (char)(u - 0x20);
	}

	return c;
}

// Fork (M0SQQ): see TR() in uiLocalisation.h. Binary search in the table sorted by strcmp().
const char *uiTranslate(const char *english)
{
	if ((english == NULL) || (currentLanguage != &languages[polishLanguageName]))
	{
		return english;
	}

	int32_t lo = 0;
	int32_t hi = ((int32_t)uiTranslationsPLCount - 1);

	while (lo <= hi)
	{
		int32_t mid = ((lo + hi) >> 1);
		int c = strcmp(english, uiTranslationsPL[mid].en);

		if (c == 0)
		{
			return uiTranslationsPL[mid].pl;
		}

		if (c < 0)
		{
			hi = (mid - 1);
		}
		else
		{
			lo = (mid + 1);
		}
	}

	return english;
}
