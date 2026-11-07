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

#ifndef UI_HELPER_H
#define UI_HELPER_H

#include <stdbool.h>
#include <stdint.h>

enum UI_Icon_t {
	ICON_LOCK       = 0x0008U,
	ICON_BELL       = 0x080AU,
	ICON_SCAN       = 0x120DU,
	ICON_DUAL_WATCH = 0x1F11U,
	ICON_BATTERY    = 0x3013U,
	ICON_VOX        = 0x4318U,
	ICON_TR         = 0x660FU,
	ICON_RR         = 0x750FU,
};

typedef enum UI_Icon_t UI_Icon_t;

void UI_DrawString(uint8_t X, uint8_t Y, const char *String, uint8_t Size);
void UI_DrawSmallCharacter(uint8_t X, uint8_t Y, char Digit);
void UI_DrawDialTypedDigits(const char *pDigits, uint8_t Dial);
void UI_DrawSmallString(uint8_t X, uint8_t Y, const char *String, uint8_t Size);
void UI_DrawStatusIcon(uint8_t X, UI_Icon_t Icon, bool bDraw, uint16_t Color);
void UI_DrawRoger(void);
void UI_DrawDialRegisters(uint8_t Dial);
void UI_DrawDialBarFrame(uint8_t Y);
void UI_DrawDialMemName(uint8_t Dial, const char *pName);
void UI_DrawDialModulation(uint8_t Mode, uint8_t gModulationType, uint8_t Dial);
void UI_DrawDialFrequency(uint32_t Frequency, uint8_t Dial, uint16_t Color);
void UI_DrawBigDigit(uint8_t X, uint8_t Y, uint8_t Digit);
void UI_DrawDialCss(uint8_t CodeType, uint16_t Code, uint8_t Encrypt, bool bMute, uint8_t Dial);
void FormatRssiDbm(int16_t RssidB);
void ConvertRssiToDbm(uint16_t Rssi);
void ConvertRssiToSmeter(uint16_t Rssi);
void UI_DrawDialRxDbm(uint8_t Vfo, bool Clear);
void UI_DrawDialRxCtcss(uint8_t Vfo, uint16_t Code);
void UI_DrawRxSmeter (uint8_t Vfo, bool Clear);
void UI_DrawDialTxPower(uint8_t pLevel, uint8_t Dial);
void UI_DrawDialModeLabel(uint16_t StoredNumber, uint8_t Dial);
void UI_DrawDialAntennaIcon(uint8_t Dial);
void UI_DrawDTMF(void);
void UI_DrawFMFrequency(uint16_t Frequency);
void UI_DrawFM(void);
void UI_DrawBitmap(uint8_t X,uint8_t Y, uint8_t H, uint8_t W, const uint8_t *pBitmap);
void UI_DrawFrame(uint8_t X0, uint8_t X1, uint8_t Y0, uint8_t Y1, uint8_t Thickness, uint16_t Color);
void UI_DrawDialog(void);
void UI_DrawDialSignalBar(uint8_t Level, uint8_t Dial);
void UI_DrawSomething(void);
void UI_DrawDialMainMarker(bool bOverride, uint8_t Dial);
void UI_DrawSky(void);
void UI_DrawDialFrequencyText(const char *String, uint8_t Dial, bool bFlag);
void UI_DrawBootVoltage(uint8_t X, uint8_t Y);
void UI_DrawDecimal(const char *pInput);
void UI_DrawMenuPosition(const char *pString);
void UI_DrawStringSwitchType(void);
void UI_DrawRadar(void);
void UI_DrawGolay(void);
void UI_DrawMenuMemNumber(const char *pString);
void UI_DrawBand(void);
void UI_DrawScanFrequency(uint32_t Frequency);
void UI_DrawCtdcScan(void);
void UI_DrawCtcssCode(uint16_t Code);
void UI_DrawDcsCodeN(uint16_t Code);
void UI_DrawDTMFString(void);
void UI_DrawMuteInfo(bool bIs24Bit, uint32_t Golay);
void UI_DrawNone(void);
void UI_DrawScan(void);
#ifdef ENABLE_SCANLIST_DISPLAY
void UI_DrawDialScanLists(uint8_t Dial);
#endif

#endif

