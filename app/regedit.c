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
#include "app/regedit.h"
#include "driver/bk4819.h"
#include "driver/key.h"
#include "helper/helper.h"
#include "ui/gfx.h"
#include "ui/helper.h"
#include "ui/main.h"
#include "driver/delay.h"
#include "misc.h"

//#include "driver/uart.h"
//#include "external/printf/printf.h"
#endif

static const Registers RegisterTable[] = {
		{"LNAS      ", 0x10, 8,  0b11,           1},
		{"LNA       ", 0x10, 5,  0b111,          1},
		{"PGA       ", 0x10, 0,  0b111,          1},
		{"MIX       ", 0x10, 3,  0b11,           1},
		{"BW        ", 0x43, 12, 0b111,          1},
		{"WEAK      ", 0x43, 9,  0b111,          1},
		{"TX DEV ON ", 0x40, 12, 0b1,            1},
		{"TX DEV LVL", 0x40, 0,  0b111111111111, 4}
};

static const uint8_t RegCount = sizeof(RegisterTable) / sizeof(RegisterTable[0]);

static uint8_t bExit;
static uint8_t RegIndex = 1;
static uint16_t RegValue;
static uint16_t SettingValue;
static uint8_t CurrentReg;

void ChangeRegValue(uint8_t bUp, uint8_t HowMany) {

	uint16_t FullMask;

	if (bUp) {
		SettingValue = (SettingValue + HowMany) % (RegisterTable[RegIndex].Mask + 1);
	} else {
		SettingValue = (SettingValue + (RegisterTable[RegIndex].Mask + 1) - HowMany) % (RegisterTable[RegIndex].Mask + 1);
	}

	FullMask = RegisterTable[RegIndex].Mask << RegisterTable[RegIndex].Offset;

	SettingValue <<= RegisterTable[RegIndex].Offset;
	RegValue = (RegValue & ~FullMask) | SettingValue;

	BK4819_WriteRegister(CurrentReg, RegValue);

}

void RegEditCheckKeys(void) {
	KEY_t Key;
	static KEY_t LastKey;

	Key = KEY_GetButton();

	if (Key != LastKey) {
		switch (Key) {
			case KEY_NONE:
				break;
			case KEY_EXIT:
				bExit = true;
				break;
			case KEY_UP:
				RegIndex = (RegIndex + 1) % RegCount;
				break;
			case KEY_DOWN:
				RegIndex = (RegIndex + RegCount - 1) % RegCount;
				break;
			case KEY_1:
				BK4819_ToggleAGCMode();
				break;
			case KEY_2:
				ChangeRegValue(false, 1);
				break;
			case KEY_3:
				ChangeRegValue(true, 1);
				break;
			case KEY_5:
				ChangeRegValue(false, 10);
				break;
			case KEY_6:
				ChangeRegValue(true, 10);
				break;
			case KEY_8:
				ChangeRegValue(false, 100);
				break;
			case KEY_9:
				ChangeRegValue(true, 100);
				break;
			default:
				break;
		}
		LastKey = Key;
	}
}

void APP_RegEdit(void) {

	uint16_t ActiveGainReg;

	RADIO_EndAudio();  // Just in case audio is open

	DISPLAY_Fill(0, 159, 1, 96, COLOR_BACKGROUND);

	bExit = false;
	gScreenMode = SCREEN_REGEDIT;

	while (1) {
		RegEditCheckKeys();

		if (bExit) {
			gScreenMode = SCREEN_MAIN;
			UI_DrawMain(false);
			return;
		}

		UI_DrawVoltage(0);

		ActiveGainReg = BK4819_ReadRegister(0x7e);
		ActiveGainReg >>= 12;
		ActiveGainReg &= 0b111;

		CurrentReg = (RegIndex < 4) ? RegisterTable[RegIndex].RegAddr + ActiveGainReg : RegisterTable[RegIndex].RegAddr;

		RegValue = BK4819_ReadRegister(CurrentReg);

		SettingValue = RegValue >> RegisterTable[RegIndex].Offset;
		SettingValue &= RegisterTable[RegIndex].Mask;

		gColorForeground = COLOR_FOREGROUND;
		UI_DrawString(20, 30, RegisterTable[RegIndex].Name, 10);
		gShortString[1] = ' ';
		gShortString[2] = ' ';
		gShortString[3] = ' ';
		Int2Ascii(SettingValue, RegisterTable[RegIndex].DiplayDigits);
		UI_DrawString(105, 30, gShortString, 4);

		DELAY_WaitMS(100);
	}
}