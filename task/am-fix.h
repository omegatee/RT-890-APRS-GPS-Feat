
/* Copyright 2023 OneOfEleven
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

#ifndef AM_FIXH

#include <stdint.h>
#include <stdbool.h>

#ifdef ENABLE_AM_FIX
	//extern int16_t rssi_gain_diff[2];
    extern uint16_t gAmFixCountdown;

	//void AM_fix_init(void);
	//void AM_fix_reset(const int vfo);
	void Task_AM_fix(void);
		
#endif

#endif