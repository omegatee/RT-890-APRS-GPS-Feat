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

#include <string.h>
#include "app/css.h"
#ifdef ENABLE_FM_RADIO
	#include "app/fm.h"
#endif
#include "app/radio.h"
#include "driver/audio.h"
#include "driver/key.h"
#include "driver/serial-flash.h"
#include "helper/inputbox.h"
#include "misc.h"
#include "radio/channels.h"
#include "radio/settings.h"
#include "ui/helper.h"
#ifdef ENABLE_NOAA
	#include "ui/noaa.h"
#endif
#include "ui/vfo.h"
#ifdef ENABLE_SCANLIST_DISPLAY
#if defined(ENABLE_RX_BAR) || defined(ENABLE_TX_BAR)
#include "ui/gfx.h"
#endif
#endif

static const ChannelInfo_t VfoTemplate[2] = {
	{
		.RX = { .Frequency = 43002500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 43002500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = MOD_FM,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		.gBandWidth = 0x0F,
		.gTXPower = 0x0F,
		.Scramble = 0x00,
		.IsInscanList = 0x00,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = "          ",
	},
	{

		.RX = { .Frequency = 43502500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 43502500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x00,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = MOD_FM,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		.gBandWidth = 0x0F,
		.gTXPower = 0x0F,
		.Scramble = 0x00,
		.IsInscanList = 0x00,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = "          ",
	},
};

#ifdef ENABLE_NOAA
static const ChannelInfo_t gNoaaDefaultChannels[11] = {
	{
		.RX = { .Frequency = 16255000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16255000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = MOD_FM,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		.gBandWidth = 0x0F,
		.gTXPower = 0x0F,
		.Scramble = 0x00,
		.IsInscanList = 0x00,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = "          ",
	},
	{
		.RX = { .Frequency = 16240000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16240000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = MOD_FM,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		.gBandWidth = 0x0F,
		.gTXPower = 0x0F,
		.Scramble = 0x00,
		.IsInscanList = 0x00,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = "          ",
	},
	{
		.RX = { .Frequency = 16247500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16247500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = MOD_FM,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		.gBandWidth = 0x0F,
		.gTXPower = 0x0F,
		.Scramble = 0x00,
		.IsInscanList = 0x00,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = "          ",
	},
	{
		.RX = { .Frequency = 16242500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16242500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = MOD_FM,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		.gBandWidth = 0x0F,
		.gTXPower = 0x0F,
		.Scramble = 0x00,
		.IsInscanList = 0x00,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = "          ",
	},
	{
		.RX = { .Frequency = 16245000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16245000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = MOD_FM,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		.gBandWidth = 0x0F,
		.gTXPower = 0x0F,
		.Scramble = 0x00,
		.IsInscanList = 0x00,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = "          ",
	},
	{
		.RX = { .Frequency = 16250000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16250000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = MOD_FM,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		.gBandWidth = 0x0F,
		.gTXPower = 0x0F,
		.Scramble = 0x00,
		.IsInscanList = 0x00,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = "          ",
	},
	{
		.RX = { .Frequency = 16252500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16252500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = MOD_FM,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		.gBandWidth = 0x0F,
		.gTXPower = 0x0F,
		.Scramble = 0x00,
		.IsInscanList = 0x00,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = "          ",
	},
	{
		.RX = { .Frequency = 16165000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16165000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = MOD_FM,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		.gBandWidth = 0x0F,
		.gTXPower = 0x0F,
		.Scramble = 0x00,
		.IsInscanList = 0x00,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = "          ",
	},
	{
		.RX = { .Frequency = 16177500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16177500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = MOD_FM,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		.gBandWidth = 0x0F,
		.gTXPower = 0x0F,
		.Scramble = 0x00,
		.IsInscanList = 0x00,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = "          ",
	},
	{
		.RX = { .Frequency = 16175000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16175000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = MOD_FM,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		.gBandWidth = 0x0F,
		.gTXPower = 0x0F,
		.Scramble = 0x00,
		.IsInscanList = 0x00,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = "          ",
	},
	{
		.RX = { .Frequency = 16200000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16200000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = MOD_FM,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		.gBandWidth = 0x0F,
		.gTXPower = 0x0F,
		.Scramble = 0x00,
		.IsInscanList = 0x00,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = "          ",
	},
};
#endif

uint16_t gFreeChannelsCount;

bool DIAL_StepMem(uint8_t Key, bool OnlyFromScanlist) {
	uint16_t startChannel = gSettings.VfoChNo[gSettings.CurrentDial];
	do {
		if (Key == KEY_UP) {
			gSettings.VfoChNo[gSettings.CurrentDial] = MEM_LoadNextUp(gSettings.VfoChNo[gSettings.CurrentDial], gSettings.CurrentDial);
		} else {
			gSettings.VfoChNo[gSettings.CurrentDial] = MEM_LoadNextDown(gSettings.VfoChNo[gSettings.CurrentDial], gSettings.CurrentDial);
		}
		if (gSettings.VfoChNo[gSettings.CurrentDial] == startChannel)
			return false;	// empty list
	} while (OnlyFromScanlist && !((gVfoState[gSettings.CurrentDial].IsInscanList >> gExtendedSettings.CurrentScanList) & 1));
	RADIO_Tune(gSettings.CurrentDial);
#ifdef ENABLE_FM_RADIO
	if (gFM_Mode < FM_MODE_PLAY) {
#endif
	UI_DrawDial(gSettings.CurrentDial);
#ifdef ENABLE_FM_RADIO
	}
#endif
	return true;
}

void DIAL_StepVfo(uint8_t Key)
{
	ChannelInfo_t *pInfo = &gVfoState[gSettings.CurrentDial];

	if (!gFrequencyReverse) {
		if (Key == KEY_UP) {
			pInfo->RX.Frequency += gFrequencyStep;
			if (gSettings.bFLock) {
				if (pInfo->RX.Frequency > 13600000 && pInfo->RX.Frequency < 14400000) {
					pInfo->RX.Frequency = 14400000;
				}
				if (pInfo->RX.Frequency > 14600000 && pInfo->RX.Frequency < 43000000) {
					pInfo->RX.Frequency = 43000000;
				}
				if (pInfo->RX.Frequency > 44000000) {
					pInfo->RX.Frequency = 10800000;
				}
			} else if (pInfo->RX.Frequency > 130000000) {
				pInfo->RX.Frequency = 130000000;
			}
		} else {
			pInfo->RX.Frequency -= gFrequencyStep;
			if (gSettings.bFLock) {
				if (pInfo->RX.Frequency < 10800000) {
					pInfo->RX.Frequency = 44000000;
				}
				if (pInfo->RX.Frequency > 13600000 && pInfo->RX.Frequency < 14400000) {
					pInfo->RX.Frequency = 13600000;
				}
				if (pInfo->RX.Frequency > 14600000 && pInfo->RX.Frequency < 43000000) {
					pInfo->RX.Frequency = 14600000;
				}
			} else if (pInfo->RX.Frequency < 1000000) {
				pInfo->RX.Frequency = 1000000;
			}
		}
		pInfo->TX.Frequency = pInfo->RX.Frequency;
		gVfoInfo[gSettings.CurrentDial].Frequency = pInfo->RX.Frequency;
	} else {
		if (Key == KEY_UP) {
			pInfo->TX.Frequency += gFrequencyStep;
			if (gSettings.bFLock) {
				if (pInfo->TX.Frequency > 13600000 && pInfo->TX.Frequency < 14400000) {
					pInfo->TX.Frequency = 14400000;
				}
				if (pInfo->TX.Frequency > 14600000 && pInfo->TX.Frequency < 43000000) {
					pInfo->TX.Frequency = 43000000;
				}
				if (pInfo->TX.Frequency > 44000000) {
					pInfo->TX.Frequency = 10800000;
				}
			} else if (pInfo->TX.Frequency > 130000000) {
				pInfo->TX.Frequency = 130000000;
			}
		} else {
			pInfo->TX.Frequency -= gFrequencyStep;
			if (gSettings.bFLock) {
				if (pInfo->TX.Frequency < 10800000) {
					pInfo->TX.Frequency = 44000000;
				}
				if (pInfo->TX.Frequency > 13600000 && pInfo->TX.Frequency < 14400000) {
					pInfo->TX.Frequency = 13600000;
				}
				if (pInfo->TX.Frequency > 14600000 && pInfo->TX.Frequency < 43000000) {
					pInfo->TX.Frequency = 14600000;
				}
			} else if (pInfo->TX.Frequency < 1000000) {
				pInfo->TX.Frequency = 1000000;
			}
		}
		gVfoInfo[gSettings.CurrentDial].Frequency = pInfo->TX.Frequency;
	}
#ifdef ENABLE_FM_RADIO
	if (gFM_Mode < FM_MODE_PLAY) {
#endif
		UI_DrawDial(gSettings.CurrentDial);
#ifdef ENABLE_FM_RADIO
	}
#endif
}

#ifdef ENABLE_NOAA
void CHANNELS_NextNOAA(uint8_t Key)
{
	if (gRadioMode == RADIO_MODE_RX) {
		RADIO_EndAudio();
	}
	if (Key == KEY_UP) {
		gNOAA_ChannelNow = (gNOAA_ChannelNow + 1) % 11;
	} else {
		gNOAA_ChannelNow = (gNOAA_ChannelNow + 10) % 11;
	}
	UI_DrawNOAA(gNOAA_ChannelNow);
	CHANNELS_SetNoaaChannel(gNOAA_ChannelNow);
	RADIO_Tune(2);
}
#endif

void CHANNELS_NextFM(uint8_t Key)
{
	if (Key == KEY_UP) {
		gSettings.FmFrequency++;
		if (gSettings.FmFrequency > 1080) {
			gSettings.FmFrequency = 640;
		}
	} else {
		gSettings.FmFrequency--;
		if (gSettings.FmFrequency < 640) {
			gSettings.FmFrequency = 1080;
		}
	}
	SETTINGS_SaveGlobals();
	UI_DrawFMFrequency(gSettings.FmFrequency);
#ifdef ENABLE_FM_RADIO
	FM_Play();
#endif
}

void DIAL_GoToTypedMem(void)
{
	uint16_t Channel = 0;
	uint8_t i;

	for (i = 0; i < 3; i++) {
		Channel = (Channel * 10) + gInputBox[i];
	}
	gInputBoxWriteIndex = 0;
	INPUTBOX_Pad(0, 10);
	if (Channel != 0) {
		Channel--;
		if (!MEM_LoadToDial(Channel, gSettings.CurrentDial)) {
			if (gSettings.VfoChNo[gSettings.CurrentDial] != Channel) {
				gSettings.VfoChNo[gSettings.CurrentDial] = Channel;
				SETTINGS_SaveGlobals();
				RADIO_Tune(gSettings.CurrentDial);
				UI_DrawDial(gSettings.CurrentDial);
			}
			return;
		}
	}
	MEM_LoadToDial(gSettings.VfoChNo[gSettings.CurrentDial], gSettings.CurrentDial);
	RADIO_Tune(gSettings.CurrentDial);
	UI_DrawDial(gSettings.CurrentDial);
}

void DIAL_SetTypedVfoFrequency(void)
{
	uint32_t Frequency = 0;
	uint8_t i;

	RADIO_CancelMode();
	for (i = 0; i < 8; i++) {
		Frequency = (Frequency * 10) + gInputBox[i];
	}

	gInputBoxWriteIndex = 0;
	INPUTBOX_Pad(0, 10);

	DIAL_SetVfoFrequency(Frequency);
}

void DIAL_SetVfoFrequency(uint32_t Frequency)
{
	if (
			(!gSettings.bFLock && (Frequency >= 1000000  && Frequency <= 130000000)) ||
			(gSettings.bFLock && (
					(Frequency >= 14400000 && Frequency <= 14600000) ||
					(Frequency >= 43000000 && Frequency <= 44000000) ||
					(Frequency >= 10800000 && Frequency <= 13600000)))
			) {
		if (!gFrequencyReverse) {
			gVfoState[gSettings.CurrentDial].RX.Frequency = Frequency;
		}
		gVfoState[gSettings.CurrentDial].TX.Frequency = Frequency;
		MEM_Save(gSettings.CurrentDial ? 1000 : 999, &gVfoState[gSettings.CurrentDial]);
		MEM_LoadToDial(gSettings.CurrentDial ? 1000 : 999, gSettings.CurrentDial);
		RADIO_Tune(gSettings.CurrentDial);
	}
	UI_DrawDial(gSettings.CurrentDial);
}

bool MEM_LoadToDial(uint16_t MemNo, uint8_t Dial)
{
	uint32_t Frequency;

	SFLASH_Read(&gVfoState[Dial], 0x3C2000 + (MemNo * sizeof(ChannelInfo_t)), sizeof(ChannelInfo_t));
	if (gSettings.bFLock) {
		Frequency = gVfoState[Dial].RX.Frequency;
		if (Frequency > 44000000) {
			return true;
		}
		if (Frequency > 14600000 && Frequency < 43000000) {
			return true;
		}
		if (Frequency > 13600000 && Frequency < 14400000) {
			return true;
		}
		if (Frequency < 10800000) {
			return true;
		}
		Frequency = gVfoState[Dial].TX.Frequency;
		if (Frequency > 44000000) {
			return true;
		}
		if (Frequency > 14600000 && Frequency < 43000000) {
			return true;
		}
		if (Frequency > 13600000 && Frequency < 14400000) {
			return true;
		}
		if (Frequency < 10800000) {
			return true;
		}
	}

	return gVfoState[Dial].Available;
}

void MEM_CountStored(void)
{
	uint16_t i;

	gFreeChannelsCount = 0;
	for (i = 0; i < 999; i++) {
		if (!MEM_LoadToDial(i, 0)) {
			gFreeChannelsCount++;
		}
	}
	if (gFreeChannelsCount == 0) {
		// no channel to select: both dials have to be in VFO mode
		DIAL_SetMemMode(0, false);
		DIAL_SetMemMode(1, false);
		SETTINGS_SaveGlobals();
		DIAL_LoadVfo(0);
		DIAL_LoadVfo(1);
	}
}

// Channel number a dial currently points to: its stored channel in channel mode,
// or its VFO slot (999 / 1000) in VFO mode.
uint16_t DIAL_GetStoredNumber(uint8_t Dial)
{
	if (DIAL_IsMemMode(Dial)) {
		return gSettings.VfoChNo[Dial];
	}
	return Dial ? 1000 : 999;
}

// Loads the dial according to its own work mode.
void DIAL_Reload(uint8_t Dial)
{
	MEM_LoadToDial(DIAL_GetStoredNumber(Dial), Dial);
}

// Loads the VFO slot of one dial, restoring the default VFO if the stored one is invalid.
void DIAL_LoadVfo(uint8_t Dial)
{
	ChannelInfo_t VfoState[2];

	memcpy(&VfoState[0], &VfoTemplate, sizeof(VfoState));

	while (MEM_LoadToDial(Dial ? 1000 : 999, Dial)) {
		SFLASH_Update(&VfoState[Dial], Dial ? 0x3C9D00 : 0x3C9CE0, sizeof(VfoState[Dial]));
	}
}

// Loads the stored channel of one dial, moving to the next valid one if it was removed.
void DIAL_LoadMem(uint8_t Dial)
{
#ifdef ENABLE_SCANLIST_DISPLAY
#if defined(ENABLE_RX_BAR) || defined(ENABLE_TX_BAR)
	DISPLAY_Fill(20, 127, 43 - (Dial * 41), 49 - (Dial * 41), COLOR_BACKGROUND);
#endif
#endif
	if (MEM_LoadToDial(gSettings.VfoChNo[Dial], Dial)) {
		gSettings.VfoChNo[Dial] = MEM_LoadNextUp(gSettings.VfoChNo[Dial], Dial);
		SETTINGS_SaveGlobals();
	}
}

// Loads one dial according to its own work mode (channel or VFO).
void DIAL_Load(uint8_t Dial)
{
	if (DIAL_IsMemMode(Dial)) {
		DIAL_LoadMem(Dial);
	} else {
		DIAL_LoadVfo(Dial);
	}
#ifdef ENABLE_VOICE
	if (Dial == gSettings.CurrentDial && gScreenMode == SCREEN_MAIN && gSettings.DtmfState != DTMF_STATE_KILLED) {
		if (DIAL_IsMemMode(Dial)) {
			AUDIO_PlayMemNumber();
		} else {
			AUDIO_PlaySampleOptional(0);
		}
	}
#endif
}

uint16_t MEM_LoadNextUp(uint16_t MemNo, uint8_t Dial)
{
	MemNo++;
	while (1) {
		MemNo %= 999;
		if (!MEM_LoadToDial(MemNo, Dial)) {
			break;
		}
		MemNo++;
	}

	return MemNo;
}

uint16_t MEM_LoadNextDown(uint16_t MemNo, uint8_t Dial)
{
	MemNo += 998;
	while (1) {
		MemNo %= 999;
		if (!MEM_LoadToDial(MemNo, Dial)) {
			break;
		}
		MemNo += 998;
	}

	return MemNo;
}

void MEM_Save(uint16_t MemNo, const ChannelInfo_t *pState)
{
	SFLASH_Update(pState, 0x3C2000 + (MemNo * sizeof(*pState)), sizeof(*pState));
}

#ifdef ENABLE_NOAA
void CHANNELS_SetNoaaChannel(uint8_t Channel)
{
	gVfoState[2] = gNoaaDefaultChannels[Channel];
}
#endif

void DIAL_SaveCurrent(void)
{
	MEM_Save(DIAL_GetStoredNumber(gSettings.CurrentDial), &gVfoState[gSettings.CurrentDial]);

	RADIO_Tune(gSettings.CurrentDial);
}

