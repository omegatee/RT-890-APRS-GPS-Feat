#include <string.h>
#include "external/printf/printf.h"
#include "driver/uart.h"
#include "driver/speaker.h"///
#include "driver/delay.h"
#include "driver/gps.h"
#include "driver/bk4819.h"
#include "driver/battery.h"
#include "radio/settings.h"
#include "app/radio.h"
#include "app/aprs.h"
#include "ui/vfo.h"
#include "misc.h"

#define SYM_TIME 770	// nominal must be 833 (1/1200)
// 760 - fail
// 765 - some fail
// 770 - 0K <--
// 775 - 0K
// 780 - fail

uint8_t		FrameBUFF[512];
uint16_t 	pFrameBUFF=0;

// DESTINATION
char dest[8] = "APRS";
char destssid = 0;

// MYCALL
// extern myCALL
char myssid = 7;
/*
-0 Your primary station usually fixed and message capable
-1 generic additional station, digi, mobile, wx, etc
-2 generic additional station, digi, mobile, wx, etc
-3 generic additional station, digi, mobile, wx, etc
-4 generic additional station, digi, mobile, wx, etc
-5 Other network sources (Dstar, Iphones, Blackberry's etc)
-6 Special activity, Satellite ops, camping or 6 meters, etc
-7 walkie talkies, HT's or other human portable
-8 boats, sailboats, RV's or second main mobile
-9 Primary Mobile (usually message capable)
-10 internet, Igates, echolink, winlink, AVRS, APRN, etc
-11 balloons, aircraft, spacecraft, etc
-12 APRStt, DTMF, RFID, devices, one-way trackers， etc
-13 Weather stations
-14 Truckers or generally full time drivers
-15 generic additional station, digi, mobile, wx, etc
*/
// DIGI
char digi[8] = "WIDE2";
char digissid = 2;
	

uint16_t crc = 0xFFFF;
char stuff_cnt = 0;

bool tone = 0;

void send_tone(bool ton)
{
	// Nominal for Bell 202 AFSK: MARK(1) - 1200 SPACE(0) - 2200
	if(ton==1){
		BK4819_SetToneFrequency(false, 1200);// trimmed 1200
	}
	else {
		BK4819_SetToneFrequency(false, 2200);// trimmed 2200 
	}

	DELAY_WaitUS(SYM_TIME);
}


void calc_crc(bool in_bit)
{
  unsigned short xor_in;
  
  xor_in = crc ^ in_bit;
  crc >>= 1;

  if(xor_in & 0x01)
    crc ^= 0x8408;
}

void AFSK_SendByte(char byte, bool stuff){
	
uint8_t i;
bool bit;

	for(i = 0; i < 8; i++) {
		bit = byte & 0x01;
		calc_crc(bit);
		
		// -------------------------  bit is one
	  	if(bit){
			send_tone(tone);
		  	stuff_cnt++;
		  	if(stuff && (stuff_cnt==5)){
				tone ^= 1;
			  	send_tone(tone);
			  	stuff_cnt=0;
		  	}
	  	}
		// ------------------------- bit is zero
	  	else {
			tone ^= 1;
		  	send_tone(tone);
		  	stuff_cnt=0;
	  	}
		
		byte >>= 1;
  	}
	
	 /*                                              thanks to
	/// https://matthewtran.dev/2021/01/stm32-aprs/#modulation
	*/
}


void APRS_send_Flag(uint16_t cnt)
{
    while(cnt--)					
		AFSK_SendByte(0x7E,0);// 0=no bitstuff in flag
}

void APRS_add_Address(char *addr, char ssid, bool last)
{

uint8_t fLen;
uint8_t c;
uint8_t ssid_byte;
	
    fLen=strlen(addr);

	for(c=0;c<6;c++){
		if(c<fLen)
			FrameBUFF[pFrameBUFF++]=(addr[c] << 1);
		else
			FrameBUFF[pFrameBUFF++]=(' ' << 1);
	}
	
	ssid_byte = ((ssid + 0x30) << 1);
	if (last)
		ssid_byte |= 1;

	FrameBUFF[pFrameBUFF++]=ssid_byte;
}

void APRS_add_CTRL(void)
{
	FrameBUFF[pFrameBUFF++]=(0x03); // CTRL -- a UI message, with no P/F request-for-response
	FrameBUFF[pFrameBUFF++]=(0xF0); // PID -- No Layer 3 protocol
}

void APRS_add_Status(char * text)
{
uint8_t cnt,slen;
	
	slen=strlen(text);
	
	FrameBUFF[pFrameBUFF++]='>';
	for(cnt=0;cnt<slen;cnt++)
		FrameBUFF[pFrameBUFF++]=text[cnt];
}

void APRS_add_Pos(void)
{
uint8_t cnt;
	
	FrameBUFF[pFrameBUFF++]='!';
	
	gLatY[7]=0;
	for(cnt=0;cnt<7;cnt++)
		FrameBUFF[pFrameBUFF++]=gLatY[cnt];
	FrameBUFF[pFrameBUFF++]=gLatS[0];
	
	FrameBUFF[pFrameBUFF++]='T';
	
	gLonX[8]=0;
	for(cnt=0;cnt<8;cnt++)
		FrameBUFF[pFrameBUFF++]=gLonX[cnt];
	FrameBUFF[pFrameBUFF++]=gLonS[0];
	
	FrameBUFF[pFrameBUFF++]='a';
}


void APRS_send_Frame(void)
{
uint16_t cnt;
	
	for(cnt=0;cnt<pFrameBUFF;cnt++)
		AFSK_SendByte(FrameBUFF[cnt],1);// 1=bitstuffed
}

void APRS_send_FCS(void)
{
  unsigned char crc_lo = crc ^ 0xff;
  unsigned char crc_hi = (crc >> 8) ^ 0xff;
	AFSK_SendByte(crc_lo,1);	// LO byte first
	AFSK_SendByte(crc_hi,1);	// HI byte
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
void APRS_send_Packet(uint8_t Type)
{
char msg[64];
				uint8_t vv;
				uint8_t d;
				uint8_t u;

	/// set APRS frequency first
	gVfoState[gSettings.CurrentDial]=gAPRSDefaultChannels[0];

	BK4819_SetAfGain(0xB32A);
	BK4819_EnableTone1(true);
//SPEAKER_TurnOn(SPEAKER_OWNER_SYSTEM);

	RADIO_StartTX(0);

	/* ==============================================================  */
	
	// Compose Frame
		FrameBUFF[0]=0;
		pFrameBUFF=0;
		// compose header -----------------
		// DEST
		APRS_add_Address(dest,destssid,0);
		// SOURCE
		APRS_add_Address(myCALL,myssid,0);
		// DIGI
		APRS_add_Address(digi,digissid,1); // 1=last Address
		// CTRL
		APRS_add_CTRL();

		// compose payload -----------------
		switch(Type){
			case 0:
				vv = BATTERY_GetVoltage();
				d = vv/10;// trick to avoid atof
				u = vv-(d*10);
				snprintf(msg,sizeof(msg),"HELLO.RT-890 APRS Test.Batt=%d.%dV",d,u);
				APRS_add_Status(msg);
				break;
			case 1:
/*for no GPS
sprintf(gTime,"235959");
sprintf(gLatY,"4026.90");
sprintf(gLatS,"N");
sprintf(gLonX,"00328.35");
sprintf(gLonS,"W");*/
				APRS_add_Pos();
				break;
		}
	
	// send flag -----------------
	APRS_send_Flag(48);

	crc=0xFFFF;
	// send frame -----------------
	APRS_send_Frame();
	
	// send CRC -----------------
	APRS_send_FCS();
	
	// send flag -----------------
	APRS_send_Flag(4);
	//	AFSK_SendByte(0x7E,0);

	BK4819_EnableTone1(false);
	
	/// restore initial dial mode
	if (gSettings.WorkModeA) {
		CHANNELS_LoadChannel(gSettings.VfoChNo[gSettings.CurrentDial], gSettings.CurrentDial);
	} else {
		CHANNELS_LoadChannel(gSettings.CurrentDial ? 1000 : 999, gSettings.CurrentDial);
	}
	RADIO_Tune(gSettings.CurrentDial);
	UI_DrawVfo(gSettings.CurrentDial);

}
