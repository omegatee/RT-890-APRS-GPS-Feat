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

#ifdef ENABLE_FM_RADIO
	#include "app/fm.h"
#endif
#include "app/menu.h"
#include "app/radio.h"
#include "app/shell.h"
#include "driver/audio.h"
#include "driver/beep.h"
#include "driver/key.h"
#include "helper/dtmf.h"
#include "helper/helper.h"
#include "helper/inputbox.h"
#include "misc.h"
#include "radio/scheduler.h"
#include "radio/settings.h"
#include "task/alarm.h"
#include "task/keyaction.h"
#include "task/lock.h"
#ifdef ENABLE_NOAA
	#include "task/noaa.h"
#endif
#include "task/screen.h"
#include "ui/dialog.h"
#include "ui/gfx.h"
#include "ui/helper.h"
#include "ui/vfo.h"

#include "driver/uart.h" // fot test only; remove

bool bBeep740;

#ifdef ENABLE_FM_RADIO
static void FM_AppendDigit(char Digit)
{
	if (gInputBoxWriteIndex == 0) {
		INPUTBOX_Pad(0, 10);
		if (Digit > 1) {
			gInputBox[gInputBoxWriteIndex] = 0;
			gInputBoxWriteIndex = 1;
		}
	}
	gInputBox[gInputBoxWriteIndex++] = Digit;
	gInputBoxWriteIndex %= 4U;
	UI_DrawDecimal(gInputBox);
	if (gInputBoxWriteIndex == 0) {
		FM_UpdateFrequency();
	}
}
#endif

static void CHANNEL_AppendDigit(char Digit)
{
	if (gInputBoxWriteIndex == 0) {
		INPUTBOX_Pad(0, '-');
	}
	gInputBox[gInputBoxWriteIndex++] = Digit;
	gInputBoxWriteIndex %= 3U;
	UI_DrawDigits(gInputBox, gSettings.CurrentDial);
	if (gInputBoxWriteIndex == 0) {
		CHANNELS_UpdateChannel();
	}
}

static void VFO_AppendDigit(char Digit)
{
	if (gInputBoxWriteIndex == 0) {
		INPUTBOX_Pad(0, 10);
	}
	gInputBox[gInputBoxWriteIndex++] = Digit;
	gInputBoxWriteIndex %= 8U;
	UI_DrawFrequencyEx(gInputBox, gSettings.CurrentDial, gFrequencyReverse);
	if (gInputBoxWriteIndex == 0) {
		CHANNELS_UpdateVFO();
	}
}

static void MAIN_KeyHandler(KEY_t Key)
{
	uint8_t Vfo = 1;

	VOX_Timer = 0;
	Task_UpdateScreen();
	if (gScannerMode && Key != KEY_UP && Key != KEY_DOWN && Key != KEY_MENU && Key != KEY_EXIT) {
		SETTINGS_SaveState();
		return;
	}

	switch (Key) {
	case KEY_0: case KEY_1: case KEY_2: case KEY_3:
	case KEY_4: case KEY_5: case KEY_6: case KEY_7:
	case KEY_8: case KEY_9:
#ifdef ENABLE_FM_RADIO
		if (gFM_Mode < FM_MODE_SCROLL_UP) {
			if (gFM_Mode == FM_MODE_PLAY) {
				FM_AppendDigit(Key);
			} else
#endif
			if (gSettings.WorkModeA && !gFrequencyReverse) {
				CHANNEL_AppendDigit(Key);
			} else {
				VFO_AppendDigit(Key);
			}
#ifdef ENABLE_VOICE
			AUDIO_PlayDigit(Key);
#endif
			BEEP_Play(740, 2, 100,0);
#ifdef ENABLE_FM_RADIO
		}
#endif
		break;

	case KEY_MENU:
IFDBG UART_printf(1,"k:MENU\r\n");
			if (gScannerMode) {
			// stop scanner and restore last RX channel or frequency
			if (gSettings.WorkModeA) {
				CHANNELS_LoadChannel(gScanLastRxFreqOrChannel, gSettings.CurrentDial);
				gSettings.VfoChNo[gSettings.CurrentDial] = gScanLastRxFreqOrChannel;
			} else {
				gVfoInfo[gSettings.CurrentDial].Frequency = gScanLastRxFreqOrChannel;
				gVfoState[gSettings.CurrentDial].RX.Frequency = gScanLastRxFreqOrChannel;
			}
			RADIO_Tune(gSettings.CurrentDial);
			UI_DrawDial(gSettings.CurrentDial);
			SETTINGS_SaveState();
			return;
		}
		if (gInputBoxWriteIndex == 0) {
#ifdef ENABLE_FM_RADIO
			if (gFM_Mode > FM_MODE_STANDBY) {
				break;
			}
#endif
			gFrequencyReverse = false;
			MENU_Redraw(true);
		}
		else {
			INPUTBOX_Pad(gInputBoxWriteIndex, 0);
#ifdef ENABLE_FM_RADIO
			if (gFM_Mode == FM_MODE_PLAY) {
				FM_UpdateFrequency();
				UI_DrawFMFrequency(gSettings.FmFrequency);
			} else
#endif
			if (gSettings.WorkModeA && !gFrequencyReverse) {
				CHANNELS_UpdateChannel();
			} else {
				CHANNELS_UpdateVFO();
			}
		}
		BEEP_Play(740, 3, 80,0);
		break;

	case KEY_UP:
	case KEY_DOWN:
		if (gInputBoxWriteIndex == 0) {
#ifdef ENABLE_FM_RADIO
			if (gFM_Mode < FM_MODE_PLAY) {
#endif
				if (!gReceptionMode) {
					if (!gScannerMode) {
#ifdef ENABLE_KEEP_MONITOR_MODE_UP_DN
						;/// TODO: write something about gMonitorMode here
						if(!gMonitorMode)
							RADIO_CancelMode();
#else
						RADIO_CancelMode();
#endif
						if (gSettings.WorkModeA) {
							CHANNELS_NextChannelMr(Key, false);
							SETTINGS_SaveGlobals();
#ifdef ENABLE_AUDIO
							AUDIO_PlayChannelNumber();
#endif
						} else {
							CHANNELS_NextChannelVfo(Key);
							CHANNELS_SaveChannel(gSettings.CurrentDial ? 1000 : 999, &gVfoState[gSettings.CurrentDial]);
							CHANNELS_LoadChannel(gSettings.CurrentDial ? 1000 : 999, gSettings.CurrentDial);
							RADIO_Tune(gSettings.CurrentDial);
						}
					} else {
						// TODO: Logic is correct but doesn't make sense
						// gSettings.StandbyArea = (Key + 1) & 1;
						// SETTINGS_SaveGlobals();
						gManualScanDirection = (Key == KEY_DOWN);
						if (gRadioMode== RADIO_MODE_RX) {
							gForceScan = true;
						}
					}
#ifdef ENABLE_NOAA
				} else {
					CHANNELS_NextNOAA(Key);
					NOAA_NextChannelCountdown = 3000;
#endif
				}
#ifdef ENABLE_FM_RADIO
			} else if (gFM_Mode == FM_MODE_PLAY) {
				CHANNELS_NextFM(Key);
			} else {
				FM_Play();
			}
#endif
		}
		BEEP_Play(740, 2, 100,0);
		break;

	case KEY_EXIT:

IFDBG UART_printf(1,"k:EXIT\r\n");

		if (gScannerMode) {
			// stop scanner and restore initial channel or frequency
			if (gSettings.WorkModeA) {
				CHANNELS_LoadChannel(gScanStartFreqOrChannel, gSettings.CurrentDial);
				gSettings.VfoChNo[gSettings.CurrentDial] = gScanStartFreqOrChannel;
			} else {
				gVfoInfo[gSettings.CurrentDial].Frequency = gScanStartFreqOrChannel;
				gVfoState[gSettings.CurrentDial].RX.Frequency = gScanStartFreqOrChannel;
			}
			RADIO_Tune(gSettings.CurrentDial);
			UI_DrawDial(gSettings.CurrentDial);
			SETTINGS_SaveState();
			return;
		}
		if (gInputBoxWriteIndex) {
#ifdef ENABLE_FM_RADIO
			if (gFM_Mode == FM_MODE_PLAY) {
				RADIO_DrawFmMode();
			} else
#endif
			if (gSettings.WorkModeA) {
				RADIO_DrawChannelMode();
			} else {
				RADIO_DrawFrequencyMode();
			}
			BEEP_Play(440, 4, 80,0);
			break;
		}
#ifdef ENABLE_FM_RADIO
		if (gFM_Mode < FM_MODE_PLAY) {
#endif
			if (gFrequencyReverse) {
				gFrequencyReverse = false;
				UI_DrawDial(gSettings.CurrentDial);
				BEEP_Play(440, 4, 80,0);
				break;
			}
			RADIO_CancelMode();
			if (gSettings.DualDisplay == 0) {
				DISPLAY_Fill(1, 158, 1 + (gCurrentDial * 41), 40 + (gCurrentDial * 41), COLOR_BACKGROUND);
				DISPLAY_Fill(1, 158, 1 + ((!gCurrentDial) * 41), 40 + ((!gCurrentDial) * 41), COLOR_BACKGROUND);
				UI_DrawRegisters(gSettings.CurrentDial);
			}
			UI_DrawMainBitmap(false, gSettings.CurrentDial);
			gSettings.CurrentDial ^= 1;
#ifdef ENABLE_VOICE
			gAudioOffsetIndex = gAudioOffsetLast;
#endif
			RADIO_Tune(gSettings.CurrentDial);
			SETTINGS_SaveGlobals();
			gFrequencyReverse = false;
			UI_DrawDial(gCurrentDial);
			UI_DrawMainBitmap(true, gSettings.CurrentDial);
			if (Vfo != 0) {
				BEEP_Play(740, 3, 80,0);
			} else {
				BEEP_Play(440, 4, 80,0);
			}
#ifdef ENABLE_FM_RADIO
		}
#endif
		break;

	case KEY_STAR:
		break;

	case KEY_HASH:
#ifdef ENABLE_FM_RADIO
		if (gFM_Mode < FM_MODE_PLAY) {
#endif
			gFrequencyReverse = false;
			RADIO_CancelMode();
			if (gFreeChannelsCount == 0) {
				UI_DrawDialogText(DIALOG_NO_CH_AVAILABLE, true);
				break;
			}
			gInputBoxWriteIndex = 0;
#ifdef ENABLE_VOICE
			gAudioOffsetIndex = gAudioOffsetLast;
#endif
			gSettings.WorkModeA ^= 1;							///WT:
			SETTINGS_SaveGlobals();
			if (gSettings.WorkModeA) {
				CHANNELS_LoadWorkMode();
			} else {
				CHANNELS_LoadVfoMode();
			}
			RADIO_Tune(gSettings.CurrentDial);
			if (gSettings.DualDisplay == 0) {
				UI_DrawDial(gSettings.CurrentDial);
			} else {
				UI_DrawDial(0);
				UI_DrawDial(1);
			}
			if (Vfo != 0) {
				BEEP_Play(740, 3, 80,0);
			} else {
				BEEP_Play(440, 4, 80,0);
			}
#ifdef ENABLE_FM_RADIO
		}
#endif
		break;

	default:
		if (Vfo != 0) {
			BEEP_Play(740, 3, 80,0);
		} else {
			BEEP_Play(440, 4, 80,0);
		}
		break;
	}
}

static void HandlerShort(KEY_t Key)
{

	if (gSettings.Lock || gRadioMode == RADIO_MODE_TX || (gReceptionMode && Key != KEY_UP && Key != KEY_DOWN)) {
		return;
	}

	if (gEnableLocalAlarm) {
		ALARM_Stop();
		return;
	}
	switch (gScreenMode) {
	case SCREEN_MAIN:
		MAIN_KeyHandler(Key);
		break;
	case SCREEN_MENU:
		MENU_KeyHandler(Key);
		break;
	case SCREEN_SETTING:
		MENU_SettingKeyHandler(Key);
		break;
	default:
		break;
	}
}

static void HandlerLong(KEY_t Key)
{
	if (gSettings.bEnableDisplay && gEnableBlink) {
		SCREEN_TurnOn();
		BEEP_Play(740, 2, 100,0);
		return;
	}

	bBeep740 = true;
	if (!gReceptionMode
#ifdef ENABLE_FM_RADIO
			&& ( gFM_Mode == FM_MODE_OFF || Key == KEY_0 || Key == KEY_HASH || Key == KEY_UP || Key == KEY_DOWN)
#endif
	) {
		SCREEN_TurnOn();
		if (gScreenMode == SCREEN_MAIN) {
			switch (Key) {
			case KEY_UP:
			case KEY_DOWN:
				if (gInputBoxWriteIndex == 0) {
#ifdef ENABLE_FM_RADIO
					if (gFM_Mode == FM_MODE_OFF) {
#endif
						RADIO_CancelMode();
						if (gSettings.WorkModeA) {
							do {
								CHANNELS_NextChannelMr(Key, false);
							} while (KEY_GetButton() != KEY_NONE);
							SETTINGS_SaveGlobals();
#ifdef ENABLE_AUDIO
							AUDIO_PlayChannelNumber();
#endif
						} else {
							do {
								RADIO_Tune(gSettings.CurrentDial);
								CHANNELS_NextChannelVfo(Key);
							} while (KEY_GetButton() != KEY_NONE);
							CHANNELS_SaveChannel(gSettings.CurrentDial ? 1000 : 999, &gVfoState[gSettings.CurrentDial]);
							CHANNELS_LoadChannel(gSettings.CurrentDial ? 1000 : 999, gSettings.CurrentDial);
						}
						RADIO_Tune(gSettings.CurrentDial);
#ifdef ENABLE_FM_RADIO
					} else {
						if (Key == KEY_UP) {
							gFM_Mode = FM_MODE_SCROLL_UP;
							gSettings.FmFrequency += 2;
							if (gSettings.FmFrequency > 1080) {
								gSettings.FmFrequency = 640;
							}
						} else {
							gFM_Mode = FM_MODE_SCROLL_DOWN;
							gSettings.FmFrequency -= 2;
							if (gSettings.FmFrequency < 640) {
								gSettings.FmFrequency = 1080;
							}
						}
						UI_DrawFMFrequency(gSettings.FmFrequency);
						FM_SetVolume(0);
					}
#endif
					bBeep740 = Key - KEY_UP;
				}
				break;

			case KEY_0:
			case KEY_1:
			case KEY_2:
			case KEY_3:
			case KEY_4:
			case KEY_5:
			case KEY_6:
			case KEY_7:
			case KEY_8:
			case KEY_9:
				KeypressAction(gExtendedSettings.KeyShortcut[Key]);
				break;

			case KEY_STAR:
				KeypressAction(gExtendedSettings.KeyShortcut[10]);
				break;

			case KEY_HASH:
				KeypressAction(gExtendedSettings.KeyShortcut[11]);
				break;

			case KEY_MENU:
				KeypressAction(gExtendedSettings.KeyShortcut[12]);
				break;

			case KEY_EXIT:
				KeypressAction(gExtendedSettings.KeyShortcut[13]);
				break;

			default:
				break;
			}
		} else {
			if (Key != KEY_UP && Key != KEY_DOWN) {
				return;
			}
			do {
				if (gScreenMode == SCREEN_MENU) {
					MENU_Next(Key);
				} else if (gScreenMode == SCREEN_SETTING) {
					MENU_ScrollSetting(Key);
				}
			} while (KEY_GetButton() != KEY_NONE);
			if (gScreenMode == SCREEN_MENU) {
///				MENU_PlayAudio((gMenuIndex + gSettingIndex) % gSettingsCount);
			}
			bBeep740 = Key - KEY_UP;
		}
		if (bBeep740) {
			BEEP_Play(740, 3, 80,0);
		} else {
			BEEP_Play(440, 4, 80,0);
		}
	}
}

void Task_CheckKeyPad(void)
{
	if (SCHEDULER_CheckTask(TASK_CHECK_KEY_PAD) && gSettings.DtmfState == DTMF_STATE_NORMAL) {
		KEY_t Key;

		SCHEDULER_ClearTask(TASK_CHECK_KEY_PAD);

		Key = KEY_GetButton();
		if (Key == KEY_NONE) {
			if (gDTMF_Playing) {
				gDTMF_Playing = false;
				DTMF_Disable();
				BEEP_Disable();
			}
			KEY_LongPressed = false;
			if (KEY_KeyCounter > 10) {
				KEY_KeyCounter = 0;
				if (gSettings.bEnableDisplay && gEnableBlink) {
					SCREEN_TurnOn();
					BEEP_Play(700, 2, 100,0);
				} else if (!gDTMF_InputMode) {
					HandlerShort(KEY_CurrentKey);
				} else {
					if (gDTMF_Input.Length < 14) {
						gDTMF_Input.String[gDTMF_Input.Length++] = DTMF_GetCharacterFromKey(KEY_CurrentKey);
						UI_DrawDTMF();
					}
					BEEP_Play(700, 2, 100,0);
				}
				SCREEN_TurnOn();
				gLockTimer = 0;
			}
			KEY_KeyCounter = 0;
			KEY_CurrentKey = KEY_NONE;
		} else {
			KEY_CurrentKey = Key;
			if (KEY_KeyCounter > 10 && gRadioMode == RADIO_MODE_TX && !gEnableLocalAlarm && !gDTMF_Playing) {
				gDTMF_Playing = true;
				DTMF_PlayTone(Key);
			} else if (KEY_KeyCounter > 1000) {
				KEY_LongPressed = true;
				gLockTimer = 0;
				HandlerLong(Key);
				KEY_KeyCounter = 0;
				gLockTimer = 0;
			}
		}
	}
}

