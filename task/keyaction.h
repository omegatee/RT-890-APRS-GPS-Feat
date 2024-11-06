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

#ifndef TASK_KEYACTION_H
#define TASK_KEYACTION_H

// ACHTUNG: this list must match the one in ui/menu.c
enum {
	ACTION_NONE = 0,
	ACTION_MONITOR,
	ACTION_FREQUENCY_DETECT,
	ACTION_REPEATER_MODE,
	ACTION_PRESET_CHANNEL,
	ACTION_APRS_BEACON,/// it was ACTION_LOCAL_ALARM 
	ACTION_REMOTE_ALARM,
#ifdef ENABLE_NOAA
	ACTION_NOAA_CHANNEL,
#endif
	ACTION_SEND_TONE,
	ACTION_ROGER_BEEP,
	ACTION_FM_RADIO,
	ACTION_SCAN,
	ACTION_AM_FIX,
	ACTION_VOX,
	ACTION_TX_POWER,
	ACTION_SQ_LEVEL,
	ACTION_DUAL_STANDBY,
	ACTION_BACKLIGHT,
	ACTION_FREQ_STEP,
	ACTION_BEEP,
	ACTION_TOGGLE_SCANLIST,		// add/remove current channel from current scanlist
	ACTION_DTMF_DECODE,
	ACTION_DUAL_DISPLAY,
	ACTION_TX_FREQ,
	ACTION_LOCK,
#ifdef ENABLE_SPECTRUM
	ACTION_SPECTRUM,
#endif
	ACTION_DARK_MODE,
	ACTION_AGC_MODE,
#ifdef ENABLE_REGISTER_EDIT
	ACTION_REG_EDIT,
#endif
	ACTION_MIC_GAIN,
	ACTION_MODULATION,
	ACTION_BANDWIDTH,
	ACTIONS_COUNT,	// used to count the number of actions, keep this last
};

void SetDefaultKeyShortcuts(uint8_t IncludeSideKeys);
void KeypressAction(uint8_t Action);

#endif
