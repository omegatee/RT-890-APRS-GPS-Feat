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

#ifndef DRIVER_UART_H
#define DRIVER_UART_H

#include <stdint.h>
#include <stdbool.h>

extern uint8_t Buffer1[256];
extern uint8_t Buffer1Length;
extern uint8_t Buffer2[256];
extern uint8_t Buffer2Length;

void UART_Init(uint8_t Port, uint32_t BaudRate);

void HandlerUSART1(void);
void HandlerUSART2(void);

void UART_SendByte(uint8_t Port, uint8_t Data);
void UART_Send(uint8_t Port,const void *pBuffer, uint8_t Size);
uint8_t UART_ReceiveByte(uint8_t Port, uint8_t *dat);
void UART_printf(uint8_t Port,const char *str, ...);

#endif

