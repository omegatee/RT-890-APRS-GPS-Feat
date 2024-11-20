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

#include <at32f421.h>
#include <stdbool.h>

#include "driver/uart.h"
#include "external/printf/printf.h"

#include "driver/gps.h"
#include "app/prog.h"
#include "app/shell.h"

uint8_t Buffer1[256];
uint8_t Buffer1Length;
uint8_t Buffer2[256];
uint8_t Buffer2Length;

static void usart_reset_ex(usart_type *uart, uint32_t baudrate)
{
	crm_clocks_freq_type info;
	uint32_t high, low;

	uart->ctrl2_bit.stopbn = USART_STOP_1_BIT;
	uart->ctrl1_bit.ren = TRUE;
	uart->ctrl1_bit.ten = TRUE;
	uart->ctrl1_bit.dbn = USART_DATA_8BITS;
	uart->ctrl1_bit.psel = FALSE;
	uart->ctrl1_bit.pen = FALSE;
	uart->ctrl3_bit.rtsen = FALSE;
	uart->ctrl3_bit.ctsen = FALSE;

	crm_clocks_freq_get(&info);

	///if (uart == USART1) { ???
		info.apb2_freq = info.apb1_freq;
	///}

	baudrate = (uint32_t)((((uint64_t)info.apb2_freq * 1000U) / 16U) / baudrate);
	high = baudrate / 1000U;
	low = (baudrate - (1000U * high)) * 16;
	if ((low % 1000U) < 500U) {
		low /= 1000U;
	} else {
		low = (low / 1000U) + 1;
		if (low >= 16) {
			low = 0;
			high++;
		}
	}
	uart->baudr_bit.div = (high << 4) | low;
}


void HandlerUSART1(void)
{	
	if (USART1->ctrl1_bit.rdbfien && USART1->sts & USART_RDBF_FLAG) {
		uint8_t Cmd;

		Buffer1[Buffer1Length++] = USART1->dt;
		Buffer1Length %= 256;
	
		if(gShellMode){
			// Read shell commands
			if(Buffer1[Buffer1Length-1]==0x0D || Buffer1[Buffer1Length-1]==0x0A){	
				SHELL_Process((char *)Buffer1);
				UART_printf(1,"\r\nA>");
				Buffer1Length = 0;
				Buffer1[0]=0;
			}
		}
		else {
			// Check for Shell request
			if(Buffer1Length == 2 && Buffer1[0]==0x3A && Buffer1[1]==0x3A){
				gShellMode=true;
				Buffer1Length = 0;
				Buffer1[0]=0;
				return;
			}

			// Check for CHIRP commands -------------------------------
			Cmd = Buffer1[0];
			if (Buffer1Length == 1 && Cmd != 0x35 && !(Cmd >= 0x40 && Cmd <= 0x4C) && Cmd != 0x52 && Cmd!=0x3A) { /// added 0x3A
				Prog_IsRunning = false;
				Prog_Timer = 0;
				UART_SendByte(1,0xFF);
				Buffer1Length = 0;
			}
			else{
				Program(Cmd);
			}
		}
	}	
}


void HandlerUSART2(void)
{

	if (USART2->ctrl1_bit.rdbfien && USART2->sts & USART_RDBF_FLAG) {

		Buffer2[Buffer2Length++] = USART2->dt;
		Buffer2Length %= 256;

		// Check for GPS commands ------------------------------------
		if(Buffer2Length==1 && Buffer2[0] != 0x24){ /// '$'
			Buffer2Length = 0;
		}
		else {
			if(Buffer2[Buffer2Length-1]==0x0D){	/// CR (eoc is 0x0D,0x0D,0x0A)
				GPSProcess((char *)Buffer2);
				Buffer2Length = 0;
			}
		}
	}
}

void UART_Init(uint8_t Port, uint32_t BaudRate)
{
	if(Port==1){
		usart_reset_ex(USART1, BaudRate);
		PERIPH_REG((uint32_t)USART1, USART_RDBF_INT) |= PERIPH_REG_BIT(USART_RDBF_INT);
		USART1->ctrl1_bit.uen = TRUE;
	}
	if(Port==2){
		usart_reset_ex(USART2, BaudRate);
		PERIPH_REG((uint32_t)USART2, USART_RDBF_INT) |= PERIPH_REG_BIT(USART_RDBF_INT);
		USART2->ctrl1_bit.uen = TRUE;
	}

}

void UART_SendByte(uint8_t Port, uint8_t Data)
{
	if(Port==1){
		USART1->dt = Data;
		while (!(USART1->sts & USART_TDBE_FLAG)) {
		}
	}
	if(Port==2){
		USART2->dt = Data;
		while (!(USART2->sts & USART_TDBE_FLAG)) {
		}
	}
}

uint8_t UART_ReceiveByte(uint8_t Port, uint8_t *dat)
{
	if(Port==1){
		if (Buffer1Length>0) {
			dat=(uint8_t *)Buffer1;
			Buffer1Length=0;
			return 1;
		}
	}
	if(Port==2){
		if (Buffer2Length>0) {
			dat=(uint8_t *)Buffer2;
			Buffer2Length=0;
			return 1;
		}
	}
	return 0;
}

void UART_Send(uint8_t Port,const void *pBuffer, uint8_t Size)
{
	const uint8_t *pBytes = (const uint8_t *)pBuffer;
	uint8_t i;

	for (i = 0; i < Size; i++) {
		UART_SendByte(Port,pBytes[i]);
	}
}

void UART_printf(uint8_t Port,const char *str, ...)
{
	char text[256];
	int  len;
	va_list va;
	va_start(va, str);
	len = vsnprintf(text, sizeof(text), str, va);
	va_end(va);
	UART_Send(Port,text, len);
}

