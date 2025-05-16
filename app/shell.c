#include <string.h>
#include "misc.h"
#include "app/shell.h"
#include "app/radio.h"
#include "driver/uart.h"
#include "driver/gps.h"
#include "driver/bk4819.h"
#include "radio/settings.h"
#include "radio/hardware.h"
#include "radio/data.h"
#include "task/keyaction.h"


bool gShellMode;

void SCPI_ProcessGeneric(char * cmd){
	
	if(strncmp(cmd,"IDN?",4)==0){
		UART_printf(1,"RADTEL,RT-890,%s,%s\r\n",WelcomeString,FW_VERSION);/// now, "Serial Number"
		return;
	}
	
	if(strncmp(cmd,"RST",3)==0){
		HARDWARE_Reboot();
		return;
	}
}

void SCPI_ProcessRADIO(char * cmd){
	
	if(strncmp(cmd,"FREQ?",5)==0){
		UART_printf(1,"%d,%d\r\n",gVfoState[gSettings.CurrentDial].RX.Frequency*10,gVfoState[gSettings.CurrentDial].TX.Frequency*10);
		return;
	}
	if(strncmp(cmd,"TX",2)==0){
		if(cmd[3]=='1' || cmd[4]=='N') 
			RADIO_StartTX(0);
		else
			RADIO_EndTX();
		return;
	}
	if(strncmp(cmd,"AGC",3)==0){
		BK4819_SetAGCLevel(cmd[4]-0x30);
IFDBG UART_printf(1,"%d-->AGC\r\n",cmd[4]-0x30);
	}
}

void SCPI_ProcessAPRS(char * cmd){
	if(strncmp(cmd,"APRS:SEND POS",5)==0){
		KeypressAction(ACTION_APRS_SEND_PACK);
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
	if(strncmp(cmd,"ALT?",4)==0){
		UART_printf(1,"%s\r\n",gAlti);
		return;
	}
}

void SCPI_ProcessREG(char * cmd){

uint8_t 	regN=0;
uint16_t 	regVAL=0;

	if(cmd[0]>0x2F && cmd[0]<0x3A) // ASCII for 0..9
		regN+=(cmd[0]-0x30)*16;
	if(cmd[0]>0x40 && cmd[0]<0x47) // ASCII for A..F
		regN+=(cmd[0]-0x37)*16;

	if(cmd[1]>0x2F && cmd[1]<0x3A) // ASCII for 0..9
		regN+=(cmd[1]-0x30);
	if(cmd[1]>0x40 && cmd[1]<0x47) // ASCII for A..F
		regN+=(cmd[1]-0x37);
		
	if(cmd[2]==0x3F){				// ?
		regVAL=BK4819_ReadRegister(regN);
IFDBG UART_printf(1,"REG_%02X=%04X\r\n",regN,regVAL);
		return;
	}
		
	if(cmd[2]!=0x20)
		return;
	
	if(cmd[3]>0x2F && cmd[3]<0x3A) // ASCII for 0..9
		regVAL+=(cmd[3]-0x30)*4096;
	if(cmd[3]>0x40 && cmd[3]<0x47) // ASCII for A..F
		regVAL+=(cmd[3]-0x37)*4096;

	if(cmd[4]>0x2F && cmd[4]<0x3A) // ASCII for 0..9
		regVAL+=(cmd[4]-0x30)*256;
	if(cmd[4]>0x40 && cmd[4]<0x47) // ASCII for A..F
		regVAL+=(cmd[4]-0x37)*256;

	if(cmd[5]>0x2F && cmd[5]<0x3A) // ASCII for 0..9
		regVAL+=(cmd[5]-0x30)*16;
	if(cmd[5]>0x40 && cmd[5]<0x47) // ASCII for A..F
		regVAL+=(cmd[5]-0x37)*16;

	if(cmd[6]>0x2F && cmd[6]<0x3A) // ASCII for 0..9
		regVAL+=(cmd[6]-0x30);
	if(cmd[6]>0x40 && cmd[6]<0x47) // ASCII for A..F
		regVAL+=(cmd[6]-0x37);
	
IFDBG UART_printf(1,"%04X-->REG_%02X\r\n",regVAL,regN);
	BK4819_WriteRegister(regN,regVAL);
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
	while((buffer[cnt]!=0x3A && buffer[cnt]!=0x20) && buffer[cnt]){
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
	
	if(strncmp(group,"APRS",4)==0){
		SCPI_ProcessAPRS(&buffer[cnt]);
		return;
	}
	
	if(strncmp(group,"RADIO",5)==0){
		SCPI_ProcessRADIO(&buffer[cnt]);
		return;
	}
	
	if(strncmp(group,"REG",3)==0){
		SCPI_ProcessREG(&buffer[cnt]);
		return;
	}

}