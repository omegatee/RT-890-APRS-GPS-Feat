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
#include "driver/battery.h"
#include "driver/uart.h"
#include "driver/gps.h"
#include "helper/dtmf.h"
#include "helper/inputbox.h"
#include "helper/helper.h"
#include "misc.h"
#include "radio/settings.h"
#include "ui/gfx.h"
#include "ui/helper.h"
#include "ui/main.h"
#include "ui/vfo.h"
#include "task/aprs.h"

void DrawStatusBar(void)
{
	DISPLAY_Fill(0, 159, 0, 96, COLOR_BACKGROUND);
	// DISPLAY_DrawRectangle0(0, 41, 160, 1, gSettings.BorderColor);

	#ifdef ENABLE_STATUS_BAR_LINE
		DISPLAY_DrawRectangle0(0, 83, 160, 1, gSettings.BorderColor);
	#endif

	if (gSettings.DtmfState == DTMF_STATE_STUNNED) {
		UI_DrawStatusIcon(4, ICON_LOCK, true, COLOR_RED);
	} else {
		UI_DrawStatusIcon(4, ICON_LOCK, gSettings.Lock, COLOR_FOREGROUND);
	}

	UI_DrawStatusIcon(56, ICON_DUAL_WATCH, gSettings.DualStandby, COLOR_FOREGROUND);
	UI_DrawStatusIcon(80, ICON_VOX, gSettings.Vox, COLOR_FOREGROUND);
	UI_DrawRoger();
	UI_DrawRepeaterMode();
	UI_DrawStatusIcon(139, ICON_BATTERY, true, COLOR_FOREGROUND);
	UI_DrawBattery(!gSettings.RepeaterMode);
}

void UI_DrawMain(bool bSkipStatus)
{
	if (bSkipStatus) {
		DISPLAY_Fill(0, 159, 0, 81, COLOR_BACKGROUND);
		// DISPLAY_DrawRectangle0(0, 41, 160, 1, gSettings.BorderColor);
	} else {
		DrawStatusBar();
	}

	if (gSettings.DualDisplay == 0 && (gRadioMode != RADIO_MODE_RX || gSettings.CurrentDial == gCurrentDial)) {
		UI_DrawVfo(gSettings.CurrentDial);
		UI_DrawVoltage(!gSettings.CurrentDial);
	} else {
		UI_DrawVfo(!gCurrentDial);
		UI_DrawVfo(gCurrentDial);
	}

	if (gInputBoxWriteIndex != 0) {
		if (gSettings.WorkModeA) {
			UI_DrawDigits(gInputBox, gSettings.CurrentDial);
		} else {
			UI_DrawFrequencyEx(gInputBox, gSettings.CurrentDial, gFrequencyReverse);
		}
	}
	if (gRadioMode != RADIO_MODE_RX && gRadioMode != RADIO_MODE_TX) {
		UI_DrawMainBitmap(true, gSettings.CurrentDial);
	}
	if (gRadioMode == RADIO_MODE_RX && gDTMF_WriteIndex != 0) {
		gDataDisplay = false;
		UI_DrawDTMFString();
		if (gDTMF_Settings.Display) {
			gDataDisplay = true;
		}
	}
}

void UI_DrawRepeaterMode(void)
{
	if (gSettings.RepeaterMode) {
		DISPLAY_DrawRectangle0(119, 86, 30, 8, gColorBackground);
		UI_DrawStatusIcon(119, gSettings.RepeaterMode == 1 ? ICON_TR : ICON_RR, true, COLOR_FOREGROUND);
	}
}

void UI_DrawBattery(bool bDisplayVoltage)
{
	uint8_t i;
	uint16_t Color;

	for (i = 15; i > 0; i--) {
		if (gBatteryVoltage > gCalibration.BatteryCalibration[i - 1]) {
			break;
		}
	}
	// Battery voltage icon
	if (i < 6) {
		Color = COLOR_RED;
	} else if (i < 7) {
		Color = COLOR_RGB(31, 41, 0);
	} else {
		Color = COLOR_GREEN;
	}
	DISPLAY_DrawRectangle0(142, 86, 15 - i, 8, gColorBackground);
	DISPLAY_DrawRectangle0(157 - i, 86, i, 8, Color);

	// Battery voltage text
	if (bDisplayVoltage){
		UI_DrawStatusIcon(119, ICON_RR, false, 0);	// Clear Repeater icon
		gColorForeground = COLOR_FOREGROUND;
		Int2Ascii(gBatteryVoltage, 2);
		gShortString[2] = gShortString[1];
		gShortString[1] = '.';
		//gShortString[3] = 'V';
		UI_DrawSmallString(119, 86, gShortString, 3);///
	}
	
	
/// rented space to display GPS clock
if(gAPRSInterval){
	gColorForeground = COLOR_RED;
}
else{
	if(gGPS_Fix)
		gColorForeground = COLOR_GREEN;
	else
	 	gColorForeground = COLOR_GREY;
}
UI_DrawSmallString(78,86,gTime,6);
gColorForeground = COLOR_FOREGROUND;

}

