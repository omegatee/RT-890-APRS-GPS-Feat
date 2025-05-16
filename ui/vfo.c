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
#include "radio/settings.h"
#include "driver/uart.h"//
#include "ui/gfx.h"
#include "ui/helper.h"
#include "ui/vfo.h"

static void DrawBandwidth(uint8_t width, uint8_t dial)
{
	const uint8_t Y = 43 - (dial * 41);

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

void UI_DrawDial(uint8_t dial)
{
	UI_DrawName(dial, gVfoState[dial].Name);
	gColorForeground = COLOR_FOREGROUND;
#ifdef ENABLE_SCANLIST_DISPLAY
	if (gSettings.WorkModeA && gRadioMode == RADIO_MODE_QUIET
#ifdef ENABLE_KEEP_MONITOR_MODE_UP_DN
		&& !gMonitorMode
#endif
	) {
		UI_DrawScanLists(dial);
	} else {
#endif
#if defined ENABLE_RX_BAR || defined ENABLE_TX_BAR
		UI_DrawVfoFrame(dial);
#endif
#ifdef ENABLE_SCANLIST_DISPLAY
	}
#endif

	if (dial == gCurrentDial) {
		if (gRadioMode == RADIO_MODE_RX) {
			UI_DrawModType(2, gVfoState[dial].gModulationType, dial);
			gColorForeground = COLOR_BLUE;
		} else if (gRadioMode == RADIO_MODE_TX) {
			UI_DrawModType(1, gVfoState[dial].gModulationType, dial);
			gColorForeground = COLOR_RED;
		} else {
			UI_DrawModType(0, gVfoState[dial].gModulationType, dial);
			gColorForeground = COLOR_FOREGROUND;
		}
	} else {
		UI_DrawModType(0, gVfoState[dial].gModulationType, dial);
		gColorForeground = COLOR_FOREGROUND;
	}

	if (gSettings.CurrentDial == dial && gFrequencyReverse) {
		gColorForeground = COLOR_RED;
		UI_DrawFrequency(gVfoState[dial].TX.Frequency, dial, COLOR_RED);
		UI_DrawCss(gVfoState[dial].TX.CodeType, gVfoState[dial].TX.Code, gVfoState[dial].Encrypt, gVfoState[dial].bMuteEnabled, dial);
	} else {
		UI_DrawFrequency(gVfoInfo[dial].Frequency, dial, gColorForeground);
		UI_DrawCss(gVfoInfo[dial].CodeType, gVfoInfo[dial].Code, gVfoState[dial].Encrypt, gVfoState[dial].bMuteEnabled, dial);
	}

	UI_DrawTxPower(gVfoState[dial].gTXPower, dial);
	gColorForeground = COLOR_FOREGROUND;
	if (gSettings.WorkModeA) {
		UI_DrawChannel(gSettings.VfoChNo[dial], dial);
	} else {
		UI_DrawChannel(dial ? 1000 : 999, dial);
	}

	DrawBandwidth(gVfoState[dial].gBandWidth, dial);
}

