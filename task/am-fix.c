
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

// code to 'try' and reduce the AM demodulator saturation problem
//
// that is until someone works out how to properly configure the BK chip !

#include "task/am-fix.h"
#include "app/radio.h"
#include "driver/bk4819.h"
#include "driver/uart.h"
#include "radio/settings.h"
#include "helper/helper.h"///
#include "ui/helper.h"///
#include "ui/gfx.h"///
#include "misc.h"

#ifdef ENABLE_AM_FIX


uint16_t gAmFixCountdown;


void Task_AM_fix()
{
int16_t rxLevel;
uint16_t micLevel;
uint16_t biasLevel;
	
	if(gAmFixCountdown != 0 || !gExtendedSettings.AmFixEnabled || gMainVfo->gModulationType != MOD_AM) {
		return;
	}

	switch (gRadioMode) {
		case RADIO_MODE_QUIET:
			BK4819_SetAGCLevel(0);
			gAmFixCountdown = 100;// 100
			return;
		case RADIO_MODE_TX:
UI_DrawSmallString(15,86,"TX",2);
			/// ====================================================== DO TX STUFF
			// trying to enable mic but without FM modulation
			//BK4819_WriteRegister(0x30, 0xC3FE);
			//BK4819_WriteRegister(0x30, 0xC1FA);
			// try with REG_40<12>
			
			// read mic level
			micLevel = BK4819_ReadRegister(0x64)>>8;//16-bit
			micLevel/=2;
//			micLevel = BK4819_ReadRegister(0x6F)&0x7F;
			
			// apply mic level to PA bias
			biasLevel = BK4819_ReadRegister(0x36);
			biasLevel&=0x00FF;
			biasLevel |= micLevel<<8;
			BK4819_WriteRegister(0x36, biasLevel);

			gAmFixCountdown = 40;
			break;

		case RADIO_MODE_RX:
			/// ====================================================== DO RX STUFF
			// sample the current RSSI level
			rxLevel = BK4819_GetRSSI_DBM();

			if(rxLevel>-50){			// very strong signal -- 
				BK4819_SetAGCLevel(5);
			}
			else if(rxLevel>-62){			// strong signal -- looks good
				BK4819_SetAGCLevel(6);
			}
			else if(rxLevel>-83){			// mid signal	-- verified 		
				BK4819_SetAGCLevel(7);
			}
			else{						// weak signal
				BK4819_SetAGCLevel(8);
			}

			gAmFixCountdown = 40;/// 100
			break;
			
		case RADIO_MODE_INCOMING:
			BK4819_SetAGCLevel(0);
			gAmFixCountdown = 100;/// 1000
			break;
		}

		//gAmFixCountdown = 400;/// 1000
		//BK4819_SetAGCLevel(0);
}


#endif
