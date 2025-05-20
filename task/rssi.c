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

#include "app/css.h"
#include "app/radio.h"
#include "driver/bk4819.h"
#include "driver/uart.h"//
#include "helper/helper.h"
#include "misc.h"
#include "radio/data.h"
#include "radio/scheduler.h"
#include "task/rssi.h"
#include "task/scanner.h"
#include "task/vox.h"
#include "ui/helper.h"
#ifdef ENABLE_NOAA
	#include "ui/noaa.h"
#endif

enum {
	STATUS_NO_TONE = 0,
	STATUS_GOT_TONE,
	STATUS_TAIL_TONE,
};

/////////////////////////////////////////////////////////////////
static void CheckRxCTCSS(void)
{
REGx68 reg68;
uint16_t tone;

  reg68.val=BK4819_ReadRegister(0x68);
  if(reg68.Scan==0) {
    tone=(reg68.Tone*100000)/206488;
//    IFDBG UART_printf(1,"CTCSS:  %d %d\r\n",tone/10,tone%10);
    UI_DrawRxCTCSS(gCurrentDial, tone);
  }
  else {
    UI_DrawRxCTCSS(gCurrentDial, 0);
  }

}
///////////////////////////////////////////////////////////////////////
static uint8_t GetToneStatus(uint8_t CodeType, bool bMuteEnabled)
{
	uint16_t Value;
	
	Value = BK4819_ReadRegister(0x0C);

#ifdef ENABLE_NOAA
	if (gNoaaMode) {
		// Checks CTC1
		return (Value & 0x0400U) ? STATUS_GOT_TONE : STATUS_NO_TONE;
	}
#endif

	// Check Interrupt Request
	if (Value & 0x0001U && gRadioMode == RADIO_MODE_RX) {
		DATA_ReceiverCheck();
		Value = BK4819_ReadRegister(0x0C);
	}

	if (gMonitorMode) {
		return STATUS_GOT_TONE;
	}

	// Check CTC2(55hz)
	if (Value & 0x0800U) {
		return STATUS_TAIL_TONE;
	}

	// Check CTC1
	if (Value & 0x0400U) {
		if (CodeType == CODE_TYPE_CTCSS) {
			return STATUS_GOT_TONE;
		}
		if (CodeType == CODE_TYPE_DCS_N || CodeType == CODE_TYPE_DCS_I || bMuteEnabled) {
			return STATUS_TAIL_TONE;
		}
	}

	// Check DCS N
	if (Value & 0x4000U && (CodeType == CODE_TYPE_DCS_N || bMuteEnabled)) {
		return STATUS_GOT_TONE;
	}

	// Check DCS I
	if (Value & 0x8000U && CodeType == CODE_TYPE_DCS_I) {
		return STATUS_GOT_TONE;
	}

	if (CodeType == CODE_TYPE_OFF && !bMuteEnabled) {
		return STATUS_GOT_TONE;
	}

	return STATUS_NO_TONE;
}

static void CheckRSSI(void)
{
int16_t rssiDB;
uint16_t Power;
	
	if (gVoxRssiUpdateTimer == 0 && !gDataDisplay && !gDTMF_InputMode && !gFrequencyDetectMode && !gReceptionMode
			&& !gFskDataReceived && gScreenMode == SCREEN_MAIN ) {

#ifdef ENABLE_SLOWER_RSSI_TIMER
		gVoxRssiUpdateTimer = 500;
#else
		gVoxRssiUpdateTimer = 100;
#endif

		Power = BK4819_GetRSSI(); // rssi is in range [8..326]
		
		//Power = (uint16_t)(rssiDB+160);// TODO: check
		Power*=28;Power/=90;				// scale to full bar width
		UI_DrawBar(Power, gCurrentDial);

		rssiDB = BK4819_GetRSSI_DBM(); // rssiDB is in range [-166..8]
		FormatRssiDbm(rssiDB);
		UI_DrawRxDBM(gCurrentDial, false);

//		ConvertRssiToSmeter(RSSI);
//		UI_DrawRxSmeter(!gCurrentDial, false);
	}
}

void Task_CheckRSSI(void)
{
	
	if (gRadioMode != RADIO_MODE_TX){
		CheckRSSI(); // conficts with ENABLE_SCANLIST_DISPLAY
		
		if (gRadioMode != RADIO_MODE_QUIET && !gSaveMode && SCHEDULER_CheckTask(TASK_CHECK_RSSI)) {
			uint8_t Status;

			SCHEDULER_ClearTask(TASK_CHECK_RSSI);
			Status = GetToneStatus(gVfoInfo[gCurrentDial].CodeType, gMainVfo->bMuteEnabled);
			if (gRadioMode != RADIO_MODE_INCOMING) {
				if (Status == STATUS_TAIL_TONE) {
					gTailToneCounter++;
				} else {
					gTailToneCounter = 0;
				}
				if (Status == STATUS_NO_TONE) {
					gNoToneCounter++;
				}
				if (gTailToneCounter <= 10 && gNoToneCounter <= 1000) {
					gNoToneCounter = 0;
					///CheckRSSI();
					UI_DrawRegisters(!gCurrentDial);
				} else if (!gReceptionMode) {
					RADIO_EndRX();
				} else {
					RADIO_EndAudio();
				}
			}
			else
	#ifdef ENABLE_NOAA
			if (!gNoaaMode)
	#endif
			{
				if (gReceptionMode) {
					RADIO_StartAudio();
				}
				else if ((gVfoInfo[gCurrentDial].CodeType == CODE_TYPE_OFF && !gMainVfo->bMuteEnabled) || gMainVfo->gModulationType > MOD_FM || Status == STATUS_GOT_TONE) {
					RADIO_StartRX();
				}
	#ifdef ENABLE_NOAA
			}
			else if (Status == STATUS_GOT_TONE) {
				gReceptionMode = true;
				gNoaaMode = false;
				gNOAA_ChannelNow = gNOAA_ChannelNext;
				UI_DrawSky();
				UI_DrawNOAA(gNOAA_ChannelNow);
				gReceivingAudio = true;
				SCREEN_TurnOn();
				BK4819_StartAudio();
	#endif
			}
		}
	}
}

