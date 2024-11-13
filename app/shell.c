#include <string.h>
#include "app/shell.h"
#include "app/radio.h"
#include "driver/uart.h"
#include "driver/gps.h"
#include "radio/settings.h"
#include "radio/hardware.h"
#include "radio/data.h"
#include "task/keyaction.h"


bool gShellMode;

void SCPI_ProcessGeneric(char * cmd){
	
	if(strncmp(cmd,"IDN?",4)==0){
		UART_printf(1,"RADTEL,RT-890,%s,0.6\r\n",WelcomeString);/// now, "Serial Number"
		return;
	}
	
	if(strncmp(cmd,"RST",3)==0){
		HARDWARE_Reboot();
		return;
	}
}

void SCPI_ProcessRADIO(char * cmd){
	
	if(strncmp(cmd,"FREQ?",5)==0){
		UART_printf(1,"%d,%d\r\n",gVfoState[gCurrentDial].RX.Frequency*10,gVfoState[gCurrentDial].TX.Frequency*10);
		return;
	}
	if(strncmp(cmd,"TX",2)==0){
		if(cmd[3]=='1' || cmd[4]=='N') 
			RADIO_StartTX(0);
		else
			RADIO_EndTX();
		return;
	}
	if(strncmp(cmd,"APRS:SEND POS",5)==0){
		KeypressAction(ACTION_APRS_SEND_POS);
		return;
	}
}

void SCPI_ProcessGPS(char * cmd){

	if(strncmp(cmd,"TIME?",5)==0){
		UART_printf(1,"%s\r\n",gTime);
		return;
	}
	if(strncmp(cmd,"LAT?",4)==0){
		UART_printf(1,"%s\r\n",gLatY);
		return;
	}
	if(strncmp(cmd,"LON?",4)==0){
		UART_printf(1,"%s\r\n",gLonX);
		return;
	}
}

void SHELL_Process(char *buffer){
	
uint8_t cnt;
char group[16];
	
	if(strncmp(buffer,"EXIT",4)==0){
		UART_printf(1,"\r\n<\r\n");
		gShellMode=false;
		return;
	}

	
	// Get the group name
	cnt=0;
	while(buffer[cnt]!=0x3A && buffer[cnt]){
		group[cnt]=buffer[cnt];
		cnt++;
	}
	cnt++;
	
	// Call the group handler
	if(buffer[0]=='*'){
		SCPI_ProcessGeneric(&buffer[1]);
		return;
	}
	
	if(strncmp(group,"GPS",3)==0){
		SCPI_ProcessGPS(&buffer[cnt]);
		return;
	}
	
	if(strncmp(group,"RADIO",5)==0){
		SCPI_ProcessRADIO(&buffer[cnt]);
		return;
	}

}