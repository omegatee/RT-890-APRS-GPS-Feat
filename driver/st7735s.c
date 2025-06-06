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

#include "driver/delay.h"
#include "driver/pins.h"
#include "driver/st7735s.h"
#include "ui/gfx.h"

static void SendByte(uint8_t Data)
{
	uint8_t i;

	for (i = 0; i < 8; i++) {
		if (Data & 0x80U) {
			gpio_bits_set(GPIOA, BOARD_GPIOA_LCD_SDA);
		} else {
			gpio_bits_reset(GPIOA, BOARD_GPIOA_LCD_SDA);
		}
		gpio_bits_reset(GPIOA, BOARD_GPIOA_LCD_SCL);
		gpio_bits_set(GPIOA, BOARD_GPIOA_LCD_SCL);
		Data <<= 1;
	}
}

static void WritePixel(uint16_t Color)
{
	ST7735S_SendCommand(ST7735S_CMD_RAMWR);
	ST7735S_SendU16(Color);
}

void ST7735S_SendCommand(ST7735S_Command_t Command)
{
	gpio_bits_reset(GPIOF, BOARD_GPIOF_LCD_DCX);
	gpio_bits_reset(GPIOC, BOARD_GPIOC_LCD_CS);

	SendByte(Command);

	gpio_bits_set(GPIOC, BOARD_GPIOC_LCD_CS);
	gpio_bits_set(GPIOF, BOARD_GPIOF_LCD_DCX);
}

void ST7735S_SendData(uint8_t Data)
{
	gpio_bits_reset(GPIOC, BOARD_GPIOC_LCD_CS);

	SendByte(Data);

	gpio_bits_set(GPIOC, BOARD_GPIOC_LCD_CS);
}

void ST7735S_SetPosition(uint8_t X, uint8_t Y)
{
	ST7735S_SendCommand(ST7735S_CMD_CASET);
	ST7735S_SendU16(Y);
	ST7735S_SendCommand(ST7735S_CMD_RASET);
	ST7735S_SendU16(X);
	ST7735S_SendCommand(ST7735S_CMD_RAMWR);
}

void ST7735S_SendU16(uint16_t Data)
{
	gpio_bits_reset(GPIOC, BOARD_GPIOC_LCD_CS);

	SendByte((Data >> 8) & 0xFF);
	SendByte((Data >> 0) & 0xFF);

	gpio_bits_set(GPIOC, BOARD_GPIOC_LCD_CS);
}

void ST7735S_SetPixel(uint8_t X, uint8_t Y, uint16_t Color)
{
	ST7735S_SetPosition(X, Y);
	WritePixel(Color);
}

void ST7735S_Init(void)
{
	// Not used?
	//DAT_20001118 = 0xFFFF;

	gpio_bits_set(GPIOF, GPIO_PINS_0);
	DELAY_WaitMS(1);

	gpio_bits_reset(GPIOF, GPIO_PINS_0);
	DELAY_WaitMS(1);

	gpio_bits_set(GPIOF, GPIO_PINS_0);
	DELAY_WaitMS(120);

	ST7735S_SendCommand(ST7735S_CMD_SLPOUT);
	DELAY_WaitMS(120);
	ST7735S_SendCommand(ST7735S_CMD_FRMCTR1);
	ST7735S_SendData(0x05);
	ST7735S_SendData(0x3C);
	ST7735S_SendData(0x3C);
	ST7735S_SendCommand(ST7735S_CMD_FRMCTR2);
	ST7735S_SendData(0x05);
	ST7735S_SendData(0x3C);
	ST7735S_SendData(0x3C);
	ST7735S_SendCommand(ST7735S_CMD_FRMCTR3);
	ST7735S_SendData(0x05);
	ST7735S_SendData(0x3C);
	ST7735S_SendData(0x3C);
	ST7735S_SendData(0x05);
	ST7735S_SendData(0x3C);
	ST7735S_SendData(0x3C);
	ST7735S_SendCommand(ST7735S_CMD_INVCTR);
	ST7735S_SendData(0x03);
	ST7735S_SendCommand(ST7735S_CMD_PWCTR1);
	ST7735S_SendData(0x28);
	ST7735S_SendData(0x08);
	ST7735S_SendData(0x04);
	ST7735S_SendCommand(ST7735S_CMD_PWCTR2);
	ST7735S_SendData(0xC0);
	ST7735S_SendCommand(ST7735S_CMD_PWCTR3);
	ST7735S_SendData(0x0D);
	ST7735S_SendData(0x00);
	ST7735S_SendCommand(ST7735S_CMD_PWCTR4);
	ST7735S_SendData(0x8D);
	ST7735S_SendData(0x2A);
	ST7735S_SendCommand(ST7735S_CMD_PWCTR5);
	ST7735S_SendData(0x8D);
	ST7735S_SendData(0xEE);
	ST7735S_SendCommand(ST7735S_CMD_VMCTR1);
	ST7735S_SendData(0x1A);
	ST7735S_SendCommand(ST7735S_CMD_MADCTL);
	ST7735S_SendData(0xC8);
	ST7735S_SendCommand(ST7735S_CMD_GMCTRP1);
	ST7735S_SendData(0x04);
	ST7735S_SendData(0x22);
	ST7735S_SendData(0x07);
	ST7735S_SendData(0x0A);
	ST7735S_SendData(0x2E);
	ST7735S_SendData(0x30);
	ST7735S_SendData(0x25);
	ST7735S_SendData(0x2A);
	ST7735S_SendData(0x28);
	ST7735S_SendData(0x26);
	ST7735S_SendData(0x2E);
	ST7735S_SendData(0x3A);
	ST7735S_SendData(0x00);
	ST7735S_SendData(0x01);
	ST7735S_SendData(0x03);
	ST7735S_SendData(0x13);
	ST7735S_SendCommand(ST7735S_CMD_GMCTRN1);
	ST7735S_SendData(0x04);
	ST7735S_SendData(0x16);
	ST7735S_SendData(0x06);
	ST7735S_SendData(0x0D);
	ST7735S_SendData(0x2D);
	ST7735S_SendData(0x26);
	ST7735S_SendData(0x23);
	ST7735S_SendData(0x27);
	ST7735S_SendData(0x27);
	ST7735S_SendData(0x25);
	ST7735S_SendData(0x2D);
	ST7735S_SendData(0x3B);
	ST7735S_SendData(0x00);
	ST7735S_SendData(0x01);
	ST7735S_SendData(0x04);
	ST7735S_SendData(0x13);
	ST7735S_SendCommand(ST7735S_CMD_COLMOD);
	ST7735S_SendData(0x05);
	DISPLAY_FillColor(COLOR_BACKGROUND);
	ST7735S_SendCommand(ST7735S_CMD_DISPON);
}

void ST7735S_SetAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
	ST7735S_SendCommand(ST7735_RASET);
	ST7735S_SendData(0x00);
	ST7735S_SendData(x0);
	ST7735S_SendData(0x00);
	ST7735S_SendData(x1);

	ST7735S_SendCommand(ST7735_CASET);
	ST7735S_SendData(0x00);
	ST7735S_SendData(y0);
	ST7735S_SendData(0x00);
	ST7735S_SendData(y1);

	ST7735S_SendCommand(ST7735_RAMWR);
}


void ST7735S_defineScrollArea(uint16_t x, uint16_t x2)
{

	/* tfa: top fixed area: nr of line from top of the frame mem and display) */
	uint16_t tfa = 160 - x2 + 1;
	/* vsa: height of the vertical scrolling area in nr of line of the frame mem
	   (not the display) from the vertical scrolling address. the first line appears
	   immediately after the bottom most line of the top fixed area. */
	uint16_t vsa = x2 - x + 1;
	/* bfa: bottom fixed are in nr of lines from bottom of the frame memory and display */
	uint16_t bfa = x + 1;

	if (tfa + vsa + bfa < 162)
		return;

	/* reset mv */

	ST7735S_SendCommand(ST7735_MADCTL);
	ST7735S_SendData(0xC8 & ~(1 << 5));

	ST7735S_SendCommand(ST7735_SCRLAR);

	ST7735S_SendU16(tfa);
	ST7735S_SendU16(vsa);
	ST7735S_SendU16(bfa);
}

void ST7735S_scroll(uint8_t line)
{
	ST7735S_SendCommand(ST7735_VSCSAD);
	ST7735S_SendU16(line);
}