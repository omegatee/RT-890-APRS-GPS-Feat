/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include "app/radio.h"
#include "misc.h"
#include "radio/channels.h"
#include "radio/settings.h"
#include "driver/uart.h"//
#include "ui/gfx.h"
#include "ui/helper.h"
#include "ui/vfo.h"

static void DrawDialBandwidth(uint8_t width, uint8_t Dial)
{
	const uint8_t Y = 43 - (Dial * 41);

	gColorForeground = COLOR_FOREGROUND;
	switch(width){
		case 0:
			UI_DrawSmallString(148, Y, "NN", 2);
			break;
		case 1:
			UI_DrawSmallString(148, Y, "N ", 2);
			break;
		case 2:
			UI_DrawSmallString(148, Y, "W ", 2);
			break;
		case 3:
			UI_DrawSmallString(148, Y, "WW", 2);
			break;
	}
}

void UI_DrawDial(uint8_t Dial)
{
	UI_DrawDialMemName(Dial, gVfoState[Dial].Name);
	gColorForeground = COLOR_FOREGROUND;
#ifdef ENABLE_SCANLIST_DISPLAY
	if (DIAL_IsMemMode(Dial) && gRadioMode == RADIO_MODE_QUIET
#ifdef ENABLE_KEEP_MONITOR_MODE_UP_DN
		&& !gMonitorMode
#endif
	) {
		UI_DrawDialScanLists(Dial);
	} else {
#endif
#if defined ENABLE_RX_BAR || defined ENABLE_TX_BAR
		UI_DrawDialBarFrame(Dial);
#endif
#ifdef ENABLE_SCANLIST_DISPLAY
	}
#endif

	if (Dial == gCurrentDial) {
		if (gRadioMode == RADIO_MODE_RX) {
			UI_DrawDialModulation(2, gVfoState[Dial].gModulationType, Dial);
			gColorForeground = COLOR_BLUE;
		} else if (gRadioMode == RADIO_MODE_TX) {
			UI_DrawDialModulation(1, gVfoState[Dial].gModulationType, Dial);
			gColorForeground = COLOR_RED;
		} else {
			UI_DrawDialModulation(0, gVfoState[Dial].gModulationType, Dial);
			gColorForeground = COLOR_FOREGROUND;
		}
	} else {
		UI_DrawDialModulation(0, gVfoState[Dial].gModulationType, Dial);
		gColorForeground = COLOR_FOREGROUND;
	}

	if (gSettings.CurrentDial == Dial && gFrequencyReverse) {
		gColorForeground = COLOR_RED;
		UI_DrawDialFrequency(gVfoState[Dial].TX.Frequency, Dial, COLOR_RED);
		UI_DrawDialCss(gVfoState[Dial].TX.CodeType, gVfoState[Dial].TX.Code, gVfoState[Dial].Encrypt, gVfoState[Dial].bMuteEnabled, Dial);
	} else {
		UI_DrawDialFrequency(gVfoInfo[Dial].Frequency, Dial, gColorForeground);
		UI_DrawDialCss(gVfoInfo[Dial].CodeType, gVfoInfo[Dial].Code, gVfoState[Dial].Encrypt, gVfoState[Dial].bMuteEnabled, Dial);
	}

	UI_DrawDialTxPower(gVfoState[Dial].gTXPower, Dial);
	gColorForeground = COLOR_FOREGROUND;
	UI_DrawDialModeLabel(DIAL_GetStoredNumber(Dial), Dial);

	DrawDialBandwidth(gVfoState[Dial].gBandWidth, Dial);
}

