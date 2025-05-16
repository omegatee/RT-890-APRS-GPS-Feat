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

#ifndef APP_SPECTRUM_H
#define APP_SPECTRUM_H

typedef struct Registers {
  const char Name[10];
  uint8_t RegAddr;
  uint8_t Offset;
  uint16_t Mask;
  uint16_t DiplayDigits;
} Registers;

void APP_RegEdit(void);

#endif