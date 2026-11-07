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

#ifndef RADIO_CHANNELS_H
#define RADIO_CHANNELS_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Vocabulary used by the function names DIAL_*, MEM_* and *Vfo*:
 *
 *   DIAL  one of the two radio blocks on the screen: 0 = upper, 1 = lower.
 *   VFO   free frequency selection: the frequency is typed in or stepped.
 *   MEM   a stored channel (memory). Stored as 0..998, shown as CH001..CH999.
 *
 * Every dial is independently in VFO mode or in MEM mode (DIAL_IsMemMode).
 * The VFO of the upper dial is kept in storage slot 999, that of the lower dial in slot 1000;
 * DIAL_GetStoredNumber() returns the slot the dial currently points to.
 */

typedef struct __attribute__((packed)) {
	uint32_t Frequency;
	uint16_t Code:12;
	uint16_t CodeType:4;
} FrequencyInfo_t;

typedef struct __attribute__((packed)) {
	FrequencyInfo_t RX;
	FrequencyInfo_t TX;
	// 0x0C
	uint32_t Golay:24;
	uint32_t Unknown0:4;
	uint32_t bIs24Bit:1;
	uint32_t bMuteEnabled:1;
	uint32_t Encrypt:2;
	// 0x10
	uint8_t Available:1;
	uint8_t gModulationType:2;
	uint8_t BCL:2;
	uint8_t ScanAdd:1;
	uint8_t bIsLowPower:1;
	uint8_t bIsNarrow:1;
	// 0x11
	uint8_t gBandWidth:4;		/// added
	uint8_t gTXPower:4;			/// added
	uint8_t Scramble;
	uint8_t IsInscanList;		// 8 lists: 1 bit per list
	uint8_t _0x14;
	uint8_t _0x15;
	char Name[10];
} ChannelInfo_t;

extern uint16_t gFreeChannelsCount;	// number of stored channels (MEM) that are programmed

// ---- MEM: stored channels
bool MEM_LoadToDial(uint16_t MemNo, uint8_t Dial);	// true = empty / invalid
uint16_t MEM_LoadNextUp(uint16_t MemNo, uint8_t Dial);	// next programmed MEM, loaded into the dial
uint16_t MEM_LoadNextDown(uint16_t MemNo, uint8_t Dial);
void MEM_Save(uint16_t MemNo, const ChannelInfo_t *pState);
void MEM_CountStored(void);				// forces both dials to VFO mode when there is none

// ---- DIAL: loading, saving and stepping the upper / lower dial
uint16_t DIAL_GetStoredNumber(uint8_t Dial);		// MEM number, or the VFO slot 999 / 1000
void DIAL_Load(uint8_t Dial);				// load the dial in its own mode (VFO or MEM)
void DIAL_LoadVfo(uint8_t Dial);
void DIAL_LoadMem(uint8_t Dial);
void DIAL_Reload(uint8_t Dial);				// plain re-read of what the dial points to
void DIAL_SaveCurrent(void);

bool DIAL_StepMem(uint8_t Key, bool OnlyFromScanlist);
void DIAL_StepVfo(uint8_t Key);
void DIAL_GoToTypedMem(void);				// MEM number typed on the keypad
void DIAL_SetTypedVfoFrequency(void);			// frequency typed on the keypad
void DIAL_SetVfoFrequency(uint32_t Frequency);

// ---- FM broadcast and NOAA (not part of the dial concept)
void CHANNELS_NextFM(uint8_t Key);
#ifdef ENABLE_NOAA
void CHANNELS_NextNOAA(uint8_t Key);
void CHANNELS_SetNoaaChannel(uint8_t Channel);
#endif

#endif

