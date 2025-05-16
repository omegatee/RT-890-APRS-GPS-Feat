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

#include "driver/st7735s.h"
#include "misc.h"
#include "ui/gfx.h"

uint16_t COLOR_RED;
uint16_t COLOR_GREEN;
uint16_t COLOR_BLUE;
uint16_t COLOR_GREY;
uint16_t COLOR_BLACK;
uint16_t COLOR_WHITE;

uint16_t COLOR_BACKGROUND;
uint16_t COLOR_FOREGROUND;

uint16_t gColorForeground;
uint16_t gColorBackground;

void DISPLAY_FillColor(uint16_t Color)
{
	uint16_t i;

	ST7735S_SetPosition(0, 0);
	for (i = 0; i < (160 * 128); i++) {
		ST7735S_SendU16(Color);
	}
}

void DISPLAY_Fill(uint8_t X0, uint8_t X1, uint8_t Y0, uint8_t Y1, uint16_t Color)
{
	uint8_t y;

	for (; X0 <= X1; X0++) {
		ST7735S_SetPosition(X0, Y0);
		for (y = Y0; y <= Y1; y++) {
			ST7735S_SendU16(Color);
		}
	}
}

void DISPLAY_DrawRectangle(uint8_t X, uint8_t Y, uint8_t W, uint8_t H, uint16_t Color)
{
	DISPLAY_Fill(X, X + W - 1, Y, Y + H - 1, Color);
}

void UI_SetColors(uint8_t DarkMode)
{
	COLOR_BLACK = COLOR_RGB( 0,  0,  0);
	COLOR_WHITE = COLOR_RGB(31, 63, 31);
	COLOR_GREY  = COLOR_RGB(16, 32, 16);
	
	if (DarkMode) {
		COLOR_RED   = COLOR_RGB(31,  0,  0);
		COLOR_GREEN = COLOR_RGB( 0, 63,  0);
		COLOR_BLUE  = COLOR_RGB(16, 24, 31);

		COLOR_BACKGROUND = COLOR_BLACK;
		COLOR_FOREGROUND = COLOR_WHITE;
	} else {
		COLOR_RED   = COLOR_RGB(31,  0,  0);
		COLOR_GREEN = COLOR_RGB( 0, 63,  0);
		COLOR_BLUE  = COLOR_RGB( 0,  0, 31);
		
		COLOR_BACKGROUND = COLOR_WHITE;
		COLOR_FOREGROUND = COLOR_BLACK;
	}

	gColorBackground = COLOR_BACKGROUND;
	gColorForeground = COLOR_FOREGROUND;

	DISPLAY_FillColor(COLOR_BACKGROUND);
}