#include <string.h>
#include <stdio.h>
#include "driver/uart.h"
#include "ui/gfx.h"
#include "ui/helper.h"
#include "helper/helper.h"
#include "misc.h"

char gTime[16];
char gLatY[16];
char gLatS[ 2];
char gLonX[16];
char gLonS[ 2];
char gAlti[8];
bool gGPS_Fix;


void GPSRead_xRMC(char *data){

uint8_t cnt;
	
	// ------------------------ Get GPS Time
	cnt=0;
	while(*data!=0x2C && data){
		gTime[cnt++]=*data;
		++data;
	}
	++data;
	// ------------------------ skip
	cnt=0;
	while(*data!=0x2C && data){
		;
		++data;
	}
	++data;
	// ------------------------ Get Latitude
	cnt=0;
	while(*data!=0x2C && data){
		gLatY[cnt++]=*data;
		++data;
	}
	++data;
	// ------------------------ Get Latitude sign
	cnt=0;
	while(*data!=0x2C && data){
		gLatS[cnt++]=*data;
		++data;
	}
	++data;
	// ------------------------ Get Longitude
	cnt=0;
	while(*data!=0x2C && data){
		gLonX[cnt++]=*data;
		++data;
	}
	++data;
	// ------------------------ Get Longitude sign
	cnt=0;
	while(*data!=0x2C && data){
		gLonS[cnt++]=*data;
		++data;
	}
	++data;
	// ------------------------ skip speed
	cnt=0;
	while(*data!=0x2C && data){

		++data;
	}
	++data;
	// ------------------------ skip track
	cnt=0;
	while(*data!=0x2C && data){

		++data;
	}
	++data;
	// ------------------------ skip UTC date
	cnt=0;
	while(*data!=0x2C && data){

		++data;
	}
	++data;
	// ------------------------ skip mag.decl
	cnt=0;
	while(*data!=0x2C && data){

		++data;
	}
	++data;
	// ------------------------ skip mag.decl sign
	cnt=0;
	while(*data!=0x2C && data){

		++data;
	}
	++data;
	// ------------------------ Get Mode
	cnt=0;
	if(*data != 'N')
		gGPS_Fix=1;
	else
		gGPS_Fix=0;

	

}
	
void GPSRead_xGGA(char *data){
uint8_t cnt;
	
	// ------------------------ Get GPS Time
	cnt=0;
	while(*data!=0x2C && data){
		gTime[cnt++]=*data;
		++data;
	}
	++data;
	// ------------------------ Get Latitude
	cnt=0;
	while(*data!=0x2C && data){
		gLatY[cnt++]=*data;
		++data;
	}
	++data;
	// ------------------------ Get Latitude sign
	cnt=0;
	while(*data!=0x2C && data){
		gLatS[cnt++]=*data;
		++data;
	}
	++data;
	// ------------------------ Get Longitude
	cnt=0;
	while(*data!=0x2C && data){
		gLonX[cnt++]=*data;
		++data;
	}
	++data;
	// ------------------------ Get Longitude sign
	cnt=0;
	while(*data!=0x2C && data){
		gLonS[cnt++]=*data;
		++data;
	}
	++data;
	// ------------------------ skip Position Quality
	cnt=0;
	if(*data > '0'){
		gGPS_Fix=1;
	}
	else{
		gGPS_Fix=0;
	}	
		++data;
	
	++data;
	// ------------------------ skip Number of Satellites
	cnt=0;
	while(*data!=0x2C && data){
		;
		++data;
	}
	++data;
	// ------------------------ skip HDOP
	cnt=0;
	while(*data!=0x2C && data){
		;
		++data;
	}
	++data;
	// ------------------------ Get Altitude
	cnt=0;
	while(*data!=0x2C && data){
		gAlti[cnt++]=*data;
		++data;
	}

}

void GPSProcess(char * buffer)
{
char *pBuff;

// EXAMPLE:
//	$GNRMC,164015.000,A,4027.25753,N,00328.45384,W,0.00,0.00,101024,,,A,V*19
//	$GNGGA,164017.000,4027.25749,N,00328.45394,W,1,13,1.2,604.7,M,52.0,M,,*5C
	
	pBuff=buffer;
	
	while(*pBuff!=0x2C && pBuff++);			// get command

	pBuff++;
	
	if(strncmp((char *)&buffer[3],"RMC",3)==0){
		GPSRead_xRMC(pBuff);
		return;
	}

	if(strncmp((char *)&buffer[3],"GGA",3)==0){
		GPSRead_xGGA(pBuff);
		return;
	}

}