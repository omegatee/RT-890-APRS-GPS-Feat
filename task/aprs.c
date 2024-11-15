#include <stdbool.h>
#include <stdint.h>
#include "radio/channels.h"
#include "app/aprs.h"
#include "task/aprs.h"


uint32_t gAPRSInterval;
uint32_t gAPRSCounter;

void Task_APRSBeacon(void){
	
	if(gAPRSInterval>0 && gAPRSCounter==0){
		if ((gRadioMode != RADIO_MODE_QUIET) || (gScreenMode == SCREEN_MENU)){
			//gAPRSCounter=ONE_MIN/5; // re-try in 12 s
			return;
		}
		
		APRS_send_Packet(1);
		
		switch(gAPRSInterval){
			case 0:
				gAPRSCounter=0;
				break;
			case 1:
				gAPRSCounter=ONE_MIN;
				break;
			case 2:
				gAPRSCounter=ONE_MIN*5;
				break;
			case 3:
				gAPRSCounter=ONE_MIN*10;
				break;
			case 4:
				gAPRSCounter=ONE_MIN*20;
				break;
			case 5:
				gAPRSCounter=ONE_MIN*60;
				break;
		}
	}
}