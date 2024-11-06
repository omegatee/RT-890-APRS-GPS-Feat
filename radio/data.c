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

#include <stdio.h>
#include <string.h>
#include "misc.h"
#include "app/radio.h"
#include "app/aprs.h"
#include "driver/bk4819.h"///
#include "driver/delay.h"///
#include "driver/uart.h"
#include "driver/gps.h"
#include "driver/afsk.h"///
#include "helper/dtmf.h"
#include "helper/helper.h"
#include "radio/data.h"
#include "radio/hardware.h"
#include "radio/settings.h"
#include "ui/gfx.h"
#include "ui/helper.h"

static char FSK[16];


static uint32_t GetU32(uint8_t Index)
{
	uint32_t Value;
	uint8_t i;

	Value = 0;
	for (i = 1; i < 5; i++) {
		Value = (Value << 8) + FSK[Index + i];
	}

	return Value;
}

static void DrawFSK(void)
{
	uint32_t Value0;
	uint32_t Value5;
	uint8_t i;

	Value0 = GetU32(0);
	Value5 = GetU32(5);
	UI_DrawDialog();
	gColorForeground = COLOR_RED;

	Int2Ascii(Value0, 8);
	for (i = 8; i > 3; i--) {
		gShortString[i] = gShortString[i - 1];
	}
	gShortString[3] = '.';
	gShortString[9] = FSK[5];
	UI_DrawString(10, 54, gShortString, 10);

	Int2Ascii(Value5, 7);
	for (i = 7; i > 2; i--) {
		gShortString[i] = gShortString[i - 1];
	}
	gShortString[2] = '.';
	gShortString[8] = FSK[10];
	UI_DrawString(10, 38, gShortString, 9);
}

static void DrawReceivedFSKID(void)
{
	if (gScreenMode == SCREEN_MAIN && !gReceptionMode) {
		UI_DrawDialog();
		gColorForeground = COLOR_RED;
		if (gCurrentDial == 1) {
			UI_DrawString(10, 54, "Area B ID:", 10);
		} else {
			UI_DrawString(10, 54, "Area A ID:", 10);
		}
		UI_DrawString(10, 38, FSK, 16);
	}
}

void DATA_ReceiverInit(void)
{
	BK4819_EnableFFSK1200(true);
	DTMF_ClearString();
	DTMF_FSK_InitReceive(0);
}

bool DATA_ReceiverCheck(void)
{
	uint16_t Result;
	uint8_t i;

	Result = BK4819_ReadRegister(0x0B);
	BK4819_WriteRegister(2, 0);

	if (gSettings.DtmfState == DTMF_STATE_KILLED && (Result & 0x1000U) == 0) {
		return false;
	}

	if (Result & 0x0010U && gSettings.DtmfState != DTMF_STATE_KILLED) {
		for (i = 0; i < 8; i++) {
			const uint16_t Data = BK4819_ReadRegister(0x5F);
			FSK[(i * 2) + 0] = (Data >> 8) & 0xFFU;
			FSK[(i * 2) + 1] = (Data >> 0) & 0xFFU;
		}
		// BK4819_SetAF(BK4819_AF_MUTE);
		if (FSK[0] == 0xEE) {
			DrawFSK();
		} else {
			DrawReceivedFSKID();
		}
		gFskDataReceived = true;
		BK4819_WriteRegister(0x3F, 0x0000);
		BK4819_WriteRegister(0x59, 0x0028);
	} else if (Result & 0x1000U) {
		if (gDTMF_WriteIndex > 13) {
			gDTMF_WriteIndex = 13;
			// The original overflows!
			for (i = 0; i < 13; i++) {
				gDTMF_String[i] = gDTMF_String[i + 1];
			}
			gDTMF_String[i] = 0;
		}
		gDTMF_String[gDTMF_WriteIndex++] = DTMF_GetCharacterFromKey((Result >> 8) & 0xFU);

		if (gDTMF_Wake.Length == 0 || !DTMF_strcmp(&gDTMF_Wake, gDTMF_String)) {
			if (gSettings.DtmfState == DTMF_STATE_KILLED) {
				return false;
			}
			if (gDTMF_Kill.Length != 0 && DTMF_strcmp(&gDTMF_Kill, gDTMF_String)) {
				gSettings.DtmfState = DTMF_STATE_KILLED;
				gSettings.CurrentDial = gCurrentDial;
				SETTINGS_SaveGlobals();
				HARDWARE_Reboot();
			}
			if (gDTMF_Stun.Length != 0 && DTMF_strcmp(&gDTMF_Stun, gDTMF_String)) {
				gSettings.DtmfState = DTMF_STATE_STUNNED;
				SETTINGS_SaveGlobals();
				UI_DrawStatusIcon(4, ICON_LOCK, true, COLOR_RED);
			}
		} else if (gSettings.DtmfState == DTMF_STATE_STUNNED) {
			gSettings.DtmfState = DTMF_STATE_NORMAL;
			SETTINGS_SaveGlobals();
			UI_DrawStatusIcon(4, ICON_LOCK, gSettings.Lock, COLOR_FOREGROUND);
		} else if (gSettings.DtmfState == DTMF_STATE_KILLED) {
			gSettings.DtmfState = DTMF_STATE_NORMAL;
			SETTINGS_SaveGlobals();
			HARDWARE_Reboot();
		}
		if (gScreenMode == SCREEN_MAIN && !gFrequencyDetectMode) {
			UI_DrawDTMFString();
			if (gDTMF_Settings.Display) {
				gDataDisplay = true;
			}
		}

		return true;
	}

	return false;
}

void DATA_SendDeviceName(void)
{
	uint8_t i;

	BK4819_WriteRegister(0x3F, 0x8000);
	BK4819_WriteRegister(0x59, 0x8028);
	BK4819_WriteRegister(0x59, 0x0028);

	for (i = 0; i < 16; i += 2) { /// register 0x5F: FSK Word Input/Output
		BK4819_WriteRegister(0x5F, (myCALL[i] << 8) | myCALL[i + 1]);/// transmit ?
	}

	BK4819_WriteRegister(0x59, 0x2828); 

	for (i = 0; i < 200; i++) {
		const uint16_t Value = BK4819_ReadRegister(0x0C);/// Interrupt Indicator

		DELAY_WaitMS(5);

		if (Value & 1U) { /// Interrupt Request
			break;
		}
	}

	BK4819_WriteRegister(0x02, 0x0000);/// clear FSK flags
}


void DATA_SendAPRSPos(void)
{
#if 0
int cnt;
int len=0;
char mess[]={"10000001100000011000000110000001100000001000000110000001100000011000000110000001000000000000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010010001101010011011000110000110011010100101011001001000010000100101011101110101100101011010001001010100111001110001010101010000001010110010011101001000000010001000100000111100100101110100000010111101101011011000001101010111011010001000100010110111010000110100001101001000101010001001101001101011100000110110101001100000001010001010100010101000101100110110100010011000000000110101011001001000101001110101100010001001100000110101100110110111011110001011100110110111010110001010011101001001101011011000111111001101101000000010101010"};

	BK4819_SetAfGain(0xB325);
	BK4819_EnableTone1(true);

	cnt=strlen(mess);
	while(cnt--)
		if(mess[len++]=='1')
			send_tone(1);
		else
			send_tone(0);
#else
		
char myssid = 7;// "EA4BGH-7"

char digi[8] = "WIDE2";
char digissid = 2;
	
char dest[8] = "APK5C1";
char destssid = 0;

	BK4819_SetAfGain(0xB325);
	BK4819_EnableTone1(true);
	
	
	// send flag -----------------
	APRS_send_Flag(100);
	
	// send header -----------------
	// DEST
	APRS_send_Node(dest,destssid);
	// SOURCE
	APRS_send_Node(myCALL,myssid);
	// DIGI
	APRS_send_Node(digi,digissid);
	// CTRL
	AFSK_SendByte(0x03, true); // FLD
	AFSK_SendByte(0xF0, true); // PID

	// send payload -----------------
    AFSK_SendByte('!', true);
	gLatY[7]=0;
	APRS_send_Field(gLatY,7);AFSK_SendByte(gLatS[0], true);
    AFSK_SendByte('T', true);
	gLonX[8]=0;
	APRS_send_Field(gLonX,8);AFSK_SendByte(gLonS[0], true);
	AFSK_SendByte('a', true);
	
	// send CRC -----------------
	APRS_send_crc();
	
	// send flag -----------------
	APRS_send_Flag(10);
#endif	
	
	BK4819_EnableTone1(false);
}

bool DATA_WasDataReceived(void)
{
	if (gFskDataReceived) {
		gRedrawScreen = false;
		gFskDataReceived = false;
		return true;
	}

	return false;
}

