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
#ifdef ENABLE_FM_RADIO
	#include "app/fm.h"
#endif
#include "app/radio.h"
#include "driver/beep.h"
#include "driver/bk4819.h"
#include "driver/delay.h"
#include "driver/key.h"
#include "driver/pins.h"
#include "driver/speaker.h"
#include "driver/uart.h"
#include "helper/dtmf.h"
#include "helper/helper.h"
#include "helper/inputbox.h"
#include "misc.h"
#include "radio/data.h"
#include "radio/scheduler.h"
#include "radio/settings.h"
#include "task/alarm.h"
#ifdef ENABLE_NOAA
	#include "task/noaa.h"
	#include "ui/noaa.h"
#endif
#include "task/ptt.h"
#include "task/am-fix.h"
#include "task/scanner.h"
#include "task/screen.h"
#include "ui/boot.h"
#include "ui/gfx.h"
#include "ui/helper.h"
#include "ui/main.h"
#include "ui/vfo.h"

uint8_t gCurrentDial;
ChannelInfo_t *gMainVfo;
ChannelInfo_t gVfoState[4];
FrequencyInfo_t gVfoInfo[2];

bool gNoaaMode;
uint16_t gCode;

static void EnableTxAmp(bool bEnable)
{
	if (!bEnable) {
		gpio_bits_reset(GPIOB, BOARD_GPIOB_TX_BIAS_LDO);
	} else {
		gpio_bits_set(GPIOB, BOARD_GPIOB_TX_BIAS_LDO);
		if (gUseUhfFilter) {
			gpio_bits_set(GPIOB, BOARD_GPIOB_TX_AMP_SEL);
		} else {
			gpio_bits_reset(GPIOB, BOARD_GPIOB_TX_AMP_SEL);
		}
	}
}

static void TuneCurrentDial(void)
{
	if (gSettings.RepeaterMode == 2) {
		// Frequency reversal
		gVfoInfo[gCurrentDial]  = gMainVfo->TX;
		gVfoInfo[!gCurrentDial] = gVfoState[!gCurrentDial].TX;
	}
	else if (gSettings.RepeaterMode == 1) {
		// Talk around
		gVfoInfo[gCurrentDial].Frequency  = gMainVfo->RX.Frequency;
		gVfoInfo[gCurrentDial].CodeType   = CODE_TYPE_OFF;
		gVfoInfo[gCurrentDial].Code       = 0;

		gVfoInfo[!gCurrentDial].Frequency = gVfoState[!gCurrentDial].RX.Frequency;
		gVfoInfo[!gCurrentDial].CodeType  = CODE_TYPE_OFF;
		gVfoInfo[!gCurrentDial].Code      = 0;
	}
	else {
		// OFF
		gVfoInfo[gCurrentDial]  = gMainVfo->RX;
		gVfoInfo[!gCurrentDial] = gVfoState[!gCurrentDial].RX;
	}

	if (!gScannerMode) {
		gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_GREEN);
	}
	gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_RED);

	gRadioMode = RADIO_MODE_QUIET;
	EnableTxAmp(false);
	BK4819_SetFrequency(gVfoInfo[gCurrentDial].Frequency);
	BK4819_SelectRFPath(gVfoInfo[gCurrentDial].Frequency);///
	gCode = gVfoInfo[gCurrentDial].Code;
	if (gMainVfo->bMuteEnabled) {
		CSS_SetCustomCode(gMainVfo->bIs24Bit, gMainVfo->Golay, !gMainVfo->bIsNarrow);///
	}
	else {
		CSS_SetStandardCode(gVfoInfo[gCurrentDial].CodeType, gCode, gMainVfo->Encrypt, !gMainVfo->bIsNarrow);///
	}
	BK4819_SetSquelchMode();
	///BK4819_SetSquelchGlitch(!gMainVfo->bIsNarrow);
	///BK4819_SetSquelchNoise(!gMainVfo->bIsNarrow);
	///BK4819_SetSquelchRSSI(!gMainVfo->bIsNarrow);
		BK4819_SetSquelchGlitch(gMainVfo->gBandWidth);///
		BK4819_SetSquelchNoise(gMainVfo->gBandWidth);///
		BK4819_SetSquelchRSSI(gMainVfo->gBandWidth);///
	BK4819_EnableRX();
	BK4819_SetFilterBandwidth(gMainVfo->gBandWidth,0);///
	///BK4819_EnableFilter(true);
}

static bool TuneTX(bool bUseMic)
{
	if (gSettings.RepeaterMode == 2) {
		gVfoInfo[gCurrentDial] = gMainVfo->RX;
	}
	else if (gSettings.RepeaterMode == 1) {
		gVfoInfo[gCurrentDial].Frequency = gMainVfo->RX.Frequency;
		gVfoInfo[gCurrentDial].CodeType  = gMainVfo->TX.CodeType;
		gVfoInfo[gCurrentDial].Code      = gMainVfo->TX.Code;
	}
	else {
		gVfoInfo[gCurrentDial] = gMainVfo->TX;
	}

	gCode = gVfoInfo[gCurrentDial].Code;
	BK4819_SetFrequency(gVfoInfo[gCurrentDial].Frequency);
	if (gSettings.BandInfo[gCurrentFrequencyBand] == BAND_136MHz && gVfoInfo[gCurrentDial].Frequency >= 13600000) {	///WT:
		///BK4819_EnableFilter(false);
			BK4819_SelectRFPath(gVfoInfo[gCurrentDial].Frequency);
		if (gMainVfo->bMuteEnabled) {
			CSS_SetCustomCode(gMainVfo->bIs24Bit, gMainVfo->Golay, !gMainVfo->bIsNarrow);///
			gTxCodeType = CODE_TYPE_DCS_N;
		}
		else {
			CSS_SetStandardCode(gVfoInfo[gCurrentDial].CodeType, gVfoInfo[gCurrentDial].Code, gMainVfo->Encrypt, gMainVfo->bIsNarrow);
			gTxCodeType = gMainVfo->TX.CodeType;
		}
		gRadioMode = RADIO_MODE_TX;
		BK4819_SetRFTXDeviation(gMainVfo->gBandWidth);
		BK4819_EnableTX(bUseMic);
		BK4819_EnableScramble(gMainVfo->Scramble);
		if (gMainVfo->Scramble == 0) {
			BK4819_SetAFResponseCoefficients(true, true, gCalibration.TX_3000Hz_Coefficient);
		} else {
			BK4819_SetAFResponseCoefficients(true, true, 5);
		}
		EnableTxAmp(true);
		///BK4819_SetupPowerAmplifier(gMainVfo->bIsLowPower ? gTxPowerLevelLow : gTxPowerLevelHigh, gVfoInfo[gCurrentDial].Frequency);
		BK4819_SetupPowerAmplifier(gMainVfo->gTXPower, gVfoInfo[gCurrentDial].Frequency);

		return true;
	} else {																														///WT
		TuneCurrentDial();

		return false;
	}
}

static void SpecialRxTxLoop(void)
{
	static bool bFlag;

	while (1) {
		while (1) {
			while (gSpecialTimer) {
			}
			gSpecialTimer = 30000;
			if (!bFlag) {
				break;
			}
			bFlag = false;
			RADIO_EndTX();
			RADIO_StartRX();
		}
		bFlag = true;
		RADIO_EndRX();
		RADIO_StartTX(true);
	}
}

#ifdef ENABLE_NOAA
static void TuneNOAA(void)
{
	gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_GREEN);
	gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_RED);

	gRadioMode = RADIO_MODE_QUIET;
	EnableTxAmp(false);
	BK4819_SetFrequency(gMainVfo->RX.Frequency);
	BK4819_SelectRFPath(gMainVfo->RX.Frequency);///
	if (!gNoaaMode) {
		BK4819_WriteRegister(0x51, 0x0000);
	} else {
		BK4819_WriteRegister(0x51, 0x9400 | gFrequencyBandInfo.CtcssTxGainWide);
		BK4819_WriteRegister(0x07, 0x152C);
	}
	BK4819_SetSquelchMode();
	BK4819_SetSquelchGlitch(false);
	BK4819_SetSquelchNoise(false);
	BK4819_SetSquelchRSSI(false);
	BK4819_EnableScramble(0);
	BK4819_EnableCompander(false);
	BK4819_EnableRX();
	BK4819_SetFilterBandwidth(1,0);// 1 - narrow
	///BK4819_EnableFilter(true);
}
#endif

static void TuneAPRS(void)
{
	gRadioMode = RADIO_MODE_QUIET;
	EnableTxAmp(false);
	BK4819_SetFrequency(gMainVfo->RX.Frequency);
}


// Public

void RADIO_Init(void)
{

	if (!gpio_input_data_bit_read(GPIOF, BOARD_GPIOF_KEY_SIDE1)) { ///WT: What's this for ? Calibration ?
		if (KEY_GetButton() == KEY_1) {
			UART_Init(1,19200);
			do {
				while (!gpio_input_data_bit_read(GPIOF, BOARD_GPIOF_KEY_SIDE1)) {
				}
			} while (KEY_GetButton() == KEY_1);
			KEY_Side1Counter = 0;
			KEY_KeyCounter = 0;
		}
	}


	SETTINGS_LoadCalibration();
	SETTINGS_LoadSettings();

	BK4819_Init();
#ifdef ENABLE_AM_FIX
	//AM_fix_init();
#endif

	if (gSettings.DtmfState != DTMF_STATE_KILLED) {
		UI_DrawBoot();
	}

	CHANNELS_CheckFreeChannels();

	if (gSettings.WorkModeA) {
		CHANNELS_LoadWorkMode();
	} else {
		CHANNELS_LoadVfoMode();
	}

	gCurrentDial = gSettings.CurrentDial;

	RADIO_Tune(gCurrentDial);

	if (gSettings.DtmfState != DTMF_STATE_KILLED) {
		UI_DrawMain(false);
		BK4819_EnableVox(gSettings.VoxLevel);
		if (!gpio_input_data_bit_read(GPIOB, BOARD_GPIOB_KEY_PTT)) {
			if (!gpio_input_data_bit_read(GPIOF, BOARD_GPIOF_KEY_SIDE1)) {
				if (!gpio_input_data_bit_read(GPIOA, BOARD_GPIOA_KEY_SIDE2)) {
					SpecialRxTxLoop();
				}
			}
		}
	}
}

void RADIO_Tune(uint8_t Vfo)
{
	gMainVfo = &gVfoState[Vfo];
	switch(Vfo){
		case 0:
		case 1:
			gNoaaMode = false;
			gCurrentDial = Vfo;
			TuneCurrentDial();
			break;
#ifdef ENABLE_NOAA
		case 2:
			TuneNOAA();
			break;
#endif
		case 3:
			TuneAPRS();
			break;
	}
}

void RADIO_StartRX(void)
{
	if (gScannerMode) {
		gScanLastRxFreqOrChannel = gSettings.WorkModeA ? gSettings.VfoChNo[gSettings.CurrentDial]
													  : gVfoInfo[gSettings.CurrentDial].Frequency;
	}
#ifdef ENABLE_FM_RADIO
	FM_Disable(FM_MODE_STANDBY);
#endif
	BK4819_StartAudio();
	if (!gFrequencyDetectMode) {
		DTMF_ClearString();
		DTMF_FSK_InitReceive(0);
		VOX_Timer = 0;
		Task_UpdateScreen();
		SCREEN_TurnOn();
		if (gScannerMode && gExtendedSettings.ScanResume == 2) {	// Time Operated
			SCANNER_Countdown = 5000;
		}
		if (gScreenMode == SCREEN_MAIN && !gDTMF_InputMode ) {
			if (gSettings.DualDisplay == 0 && gSettings.CurrentDial != gCurrentDial) {
				const uint8_t Y = gCurrentDial * 41;

				DISPLAY_Fill(1, 158, 1 + Y, 40 + Y, COLOR_BACKGROUND);
				DISPLAY_Fill(1, 158, 1 + ((!gCurrentDial) * 41), 40 + ((!gCurrentDial) * 41), COLOR_BACKGROUND);

				UI_DrawRegisters(!gCurrentDial);
			}
			UI_DrawDial(gCurrentDial);
			UI_DrawMainBitmap(true, gSettings.CurrentDial);///
///			UI_DrawRX(gCurrentDial); draws walky rx icon
		}
		if (gMainVfo->BCL == BUSY_LOCK_CSS && !gMonitorMode) {
			PTT_SetLock(PTT_LOCK_BUSY);
		}
		if (gSettings.TxPriority && gSettings.CurrentDial != gCurrentDial) {
			gSettings.CurrentDial ^= 1;
			SETTINGS_SaveGlobals();
		}
	}
}

void RADIO_EndRX(void)
{
	gTailToneCounter = 0;
	BK4819_SetAF(BK4819_AF_MUTE);
	SPEAKER_TurnOff(SPEAKER_OWNER_RX);
	BK4819_EnableFFSK1200(false);
	BK4819_ResetFSK();
	DTMF_Disable();
	if (gScannerMode) {
		switch (gExtendedSettings.ScanResume) {
			case 1:		// Carrier Operated
				SCANNER_Countdown = 3000;
			case 2:		// Time Operated
				gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_GREEN);
				break;
			case 3:		// No resume
				SETTINGS_SaveState();
				break;
		}
	}
	TuneCurrentDial();
	if (!gFrequencyDetectMode) {
		if (gScreenMode == SCREEN_MAIN && !gDTMF_InputMode) {
			if (!gFskDataReceived && !gDataDisplay) {
				UI_DrawSomething();
			} else {
				VOX_Timer = 5000;
				gRedrawScreen = true;
			}
		} else {
			if (gDTMF_InputMode) {
				if (gFskDataReceived || gDataDisplay) {
					DELAY_WaitMS(500);
				}
				UI_DrawDTMF();
			}
			DATA_WasDataReceived();
			gDataDisplay = false;
		}
		gRxLinkCounter = 0;
		gNoToneCounter = 0;
		gIdleTimer = 10000;
		PTT_ClearLock(PTT_LOCK_INCOMING);
		PTT_ClearLock(PTT_LOCK_BUSY);
#ifdef ENABLE_FM_RADIO
		if (!gRedrawScreen) {
			FM_Resume();
		}
#endif
		gIncomingTimer = 1;
	} else {
		gSignalFound = true;
		gDetectorTimer = 500;
	}
}

void RADIO_StartAudio(void)
{
	gReceivingAudio = true;
	SCREEN_TurnOn();
	BK4819_StartAudio();
}

void RADIO_EndAudio(void)
{
	gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_GREEN);
	gReceivingAudio = false;
	///RADIO_Tune(2);
		RADIO_Tune(gSettings.CurrentDial);
	SPEAKER_TurnOff(SPEAKER_OWNER_RX);
	gRxLinkCounter = 0;
	gNoToneCounter = 0;
	gIncomingTimer = 1;
#ifdef ENABLE_NOAA
	NOAA_NextChannelCountdown = 3000;
#endif
}

void RADIO_Sleep(void)
{
	EnableTxAmp(false);
	///BK4819_EnableFilter(false);
	BK4819_WriteRegister(0x30, 0x0000);
	BK4819_WriteRegister(0x37, 0x1D00);
	gSaveMode = true;
}

static void PlayRogerBeep(uint8_t Mode)
{
	BEEP_Enable();

	if (Mode == 1) {
		BK4819_SetToneFrequency(false, 1000);
		SPEAKER_TurnOn(SPEAKER_OWNER_SYSTEM);
		DELAY_WaitMS(25);
		BK4819_SetToneFrequency(false, 0);
		DELAY_WaitMS(25);
		BK4819_SetToneFrequency(false, 1000);
		DELAY_WaitMS(25);
		BK4819_SetToneFrequency(false, 0);
		DELAY_WaitMS(25);
		BK4819_SetToneFrequency(false, 1000);
		DELAY_WaitMS(25);
	} else {
		BK4819_SetToneFrequency(false, 590);
		SPEAKER_TurnOn(SPEAKER_OWNER_SYSTEM);
		DELAY_WaitMS(60);
		BK4819_SetToneFrequency(false, 660);
		DELAY_WaitMS(60);
		BK4819_SetToneFrequency(false, 730);
		DELAY_WaitMS(60);
	}

	BEEP_Disable();
}

void RADIO_Retune(void)
{
	if (gSettings.WorkModeA) {
		CHANNELS_LoadChannel(gSettings.VfoChNo[gSettings.CurrentDial], gSettings.CurrentDial);
	} else {
		CHANNELS_LoadChannel(gSettings.CurrentDial ? 1000 : 999, gSettings.CurrentDial);
	}
	RADIO_Tune(gSettings.CurrentDial);
}

#ifdef ENABLE_NOAA
void RADIO_NoaaRetune(void)
{
	gReceptionMode = false;
	gReceivingAudio = false;
	if (gRadioMode == RADIO_MODE_RX) {
		RADIO_EndAudio();
	}
	RADIO_Retune();
	RADIO_Tune(gSettings.CurrentDial);
	gScreenMode = SCREEN_MAIN;
	UI_DrawMain(true);
}

void RADIO_NoaaTune(void)
{
	gReceptionMode = true;
	UI_DrawSky();
	UI_DrawNOAA(gNOAA_ChannelNow);
	CHANNELS_SetNoaaChannel(gNOAA_ChannelNow);
	RADIO_Tune(2);
	NOAA_NextChannelCountdown = 3000;
	gScreenMode = SCREEN_NOAA;
	gNoaaMode = false;
}
#endif

void VFO_ClearCss(void)
{
	gVfoState[gSettings.CurrentDial].RX.CodeType = CODE_TYPE_OFF;
	gVfoState[gSettings.CurrentDial].TX.CodeType = CODE_TYPE_OFF;
	gVfoState[gSettings.CurrentDial].RX.Code = 0;
	gVfoState[gSettings.CurrentDial].TX.Code = 0;
	gVfoState[gSettings.CurrentDial].Encrypt = 0;
}

void VFO_ClearMute(void)
{
	gVfoState[gSettings.CurrentDial].bIs24Bit = 0;
	gVfoState[gSettings.CurrentDial].bMuteEnabled = 0;
	gVfoState[gSettings.CurrentDial].Golay = 0;
}

void RADIO_SaveCurrentDial(void)
{
	if (gSettings.WorkModeA) {
		CHANNELS_SaveChannel(gSettings.VfoChNo[gSettings.CurrentDial], &gVfoState[gSettings.CurrentDial]);
	} else {
		CHANNELS_SaveChannel(gSettings.CurrentDial ? 1000 : 999, &gVfoState[gSettings.CurrentDial]);
	}
	RADIO_Tune(gSettings.CurrentDial);
}

void RADIO_StartTX(bool bUseMic)
{
	if (gRadioMode == RADIO_MODE_RX) {
		RADIO_EndRX();
	}
	RADIO_Tune(gSettings.CurrentDial);/// this is just Dial A or Dial B
	RADIO_DisableSaveMode();
	if (!TuneTX(bUseMic)) {
		if (gEnableLocalAlarm) {
			ALARM_Stop();
			return;
		}
	}
	else {
		gpio_bits_set(GPIOA, BOARD_GPIOA_LED_RED);
		VOX_Timer = 0;
		if (gDTMF_InputMode) {
			UI_DrawMain(true);
		}
		Task_UpdateScreen();
		UI_DrawMainBitmap(true, gSettings.CurrentDial);///
		UI_DrawDial(gSettings.CurrentDial);
/// Moved to end of transmission
///		if (gSettings.RogerBeep == 3) {
///			// Play a tone for 300ms to make sure squelch opens on remote end before FSK ID
///			BEEP_Enable();
///			BK4819_SetToneFrequency(false, 610);
///			SPEAKER_TurnOn(SPEAKER_OWNER_SYSTEM); // The local user should also hear it, we want to make sure we don't cut their voice
///			DELAY_WaitMS(150);
///			BEEP_Disable();
///			BK4819_EnableFFSK1200(true);
///			DATA_SendDeviceName();
///			BK4819_EnableFFSK1200(false);
///			BK4819_ResetFSK();
///		}
		if (gDTMF_Settings.Mode == DTMF_MODE_TX_START || gDTMF_Settings.Mode == DTMF_MODE_TX_START_END || gDTMF_InputMode) {
			DELAY_WaitMS(gDTMF_Settings.Delay * 100);
			if (!gDTMF_InputMode) {
				DTMF_PlayContact(&gDTMF_Contacts[gDTMF_Settings.Select]);
			} else {
				DTMF_PlayContact(&gDTMF_Input);
				DTMF_ResetString();
				gDTMF_InputMode = false;
			}
		}
	}
}

void RADIO_EndTX(void)
{
	if (gDTMF_Settings.Mode == DTMF_MODE_TX_END || gDTMF_Settings.Mode == DTMF_MODE_TX_START_END) {
		DTMF_PlayContact(&gDTMF_Contacts[gDTMF_Settings.Select]);
	}

	if (gSettings.RogerBeep == 3) {
		BK4819_EnableFFSK1200(true);
		DATA_SendDeviceName();
		BK4819_EnableFFSK1200(false);
		BK4819_ResetFSK();
	} 
	else {
		if (gSettings.RogerBeep && gSettings.RogerBeep != 3) {
			PlayRogerBeep(gSettings.RogerBeep);
		}
	}
	BK4819_GenTail(!gMainVfo->bIsNarrow);///
	gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_RED);
	BK4819_SetupPowerAmplifier(0,0);
	TuneCurrentDial();
	UI_DrawSomething();
	gBatteryTimer = 3000;
	gIdleTimer = 10000;
}

void RADIO_CancelMode(void)
{
	if (gRadioMode == RADIO_MODE_INCOMING) {
		gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_GREEN);
		gRadioMode = RADIO_MODE_QUIET;
	}
	if (gRadioMode == RADIO_MODE_QUIET) {
		return;
	}
	if (gRadioMode == RADIO_MODE_TX) {
		RADIO_EndTX();
	} else if (gRadioMode == RADIO_MODE_RX) {
		gMonitorMode = false;
		RADIO_EndRX();
	}
	VOX_Timer = 0;
	Task_UpdateScreen();
}

void RADIO_DisableSaveMode(void)
{
	if (gSaveMode) {
		BK4819_EnableRX();
		///BK4819_EnableFilter(true);
		gSaveMode = false;
		DELAY_WaitMS(10);
	}
}

void RADIO_DrawFmMode(void)
{
	gInputBoxWriteIndex = 0;
	INPUTBOX_Pad(0, 10);
	UI_DrawFMFrequency(gSettings.FmFrequency);
}

void RADIO_DrawChannelMode(void)
{
	gInputBoxWriteIndex = 0;
	INPUTBOX_Pad(0, '-');
	UI_DrawDial(gSettings.CurrentDial);
}

void RADIO_DrawFrequencyMode(void)
{
	RADIO_CancelMode();
	INPUTBOX_Pad(0, 10);
	gInputBoxWriteIndex = 0;
	if (gFrequencyReverse) {
		UI_DrawFrequency(gVfoState[gSettings.CurrentDial].TX.Frequency, gSettings.CurrentDial, COLOR_RED);
	} else {
		UI_DrawFrequency(gVfoState[gSettings.CurrentDial].RX.Frequency, gSettings.CurrentDial, COLOR_FOREGROUND);
	}
}

