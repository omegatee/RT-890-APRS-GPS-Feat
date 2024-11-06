#include <string.h>
#include "driver/uart.h"
//#include "driver/afsk.h"
#include "driver/delay.h"
#include "driver/gps.h"
#include "driver/bk4819.h"
#include "driver/speaker.h"
#include "radio/settings.h"
#include "misc.h"

#define SYM_TIME 833	// nominal is 833 (1/1200)


uint8_t		FrameBUFF[512];
uint16_t 	pFrameBUFF=0;

char myssid = 7;// "EA4BGH-7"

char digi[8] = "WIDE2";
char digissid = 2;
	
char dest[8] = "APRS";
char destssid = 0;

uint16_t crc = 0xFFFF;
char stuff_cnt = 0;

bool tone = 0;

void send_tone(bool ton)
{
	// Nominal for Bell 202 AFSK: MARK(1) - 1200 SPACE(0) - 2200
	if(ton){
		BK4819_SetToneFrequency(false, 1200);// trimmed 1200
//UART_SendByte(1,0x30);
	}
	else {
		BK4819_SetToneFrequency(false, 2200);// trimmed 2200 
//UART_SendByte(1,0x31);
	}

	DELAY_WaitUS(SYM_TIME);
	//DELAY_WaitMS(200); // for debug purposes
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
UART_SendByte(1,byte);
	for(i = 0; i < 8; i++) {
		bit = byte & 0x01;
		calc_crc(bit);
	  	if(bit){
			send_tone(tone);
		  	stuff_cnt++;
		  	if(stuff && (stuff_cnt==5)){
				tone ^= 1;
			  	send_tone(tone);
			  	stuff_cnt=0;
		  	}
	  	}
	  	else {
			tone ^= 1;
		  	send_tone(tone);
		  	stuff_cnt=0;
	  	}
		byte >>= 1;
  	}
	
	 /*
	/// https://matthewtran.dev/2021/01/stm32-aprs/#modulation
	*/
}


void APRS_send_Flag(uint8_t cnt)
{
    while(cnt--)					
		AFSK_SendByte(0x7E,0);// 0=no bitstuff
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
	
	ssid_byte=0x30;
	ssid_byte |= (ssid + 0x30) << 1;

	if (last)
		ssid_byte |= 1;

	FrameBUFF[pFrameBUFF++]=ssid_byte;
}

void APRS_add_CTRL(void)
{
	FrameBUFF[pFrameBUFF++]=(0x03); // CTRL -- a UI message, with no P/F request-for-response
	FrameBUFF[pFrameBUFF++]=(0xF0); // PID -- No Layer 3 protocol
}

void APRS_add_Pos(void)
{
uint8_t cnt;
	
//for debug:
//char LatY[]={"4027.25"};
//char LatS[]={"N"};
//char LonX[]={"00328.45"};
//char LonS[]={"W"};
	
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
	AFSK_SendByte(crc_lo,1);		// LO byte first
	AFSK_SendByte(crc_hi,1);	// HI byte
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
void APRS_send_Packet(uint8_t Type)
{
#if 0
int cnt;
int len=0;
// ristra Carlos char mess[]={"10000001100000011000000110000001100000001000000110000001100000011000000110000001000000000000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010000000100000001000000010010001101010011011000110000110011010100101011001001000010000100101011101110101100101011010001001010100111001110001010101010000001010110010011101001000000010001000100000111100100101110100000010111101101011011000001101010111011010001000100010110111010000110100001101001000101010001001101001101011100000110110101001100000001010001010100010101000101100110110100010011000000000110101011001001000101001110101100010001001100000110101100110110111011110001011100110110111010110001010011101001001101011011000111111001101101000000010101010"};

char mess[]={"100010001010011010101000010000000100000001000000011001001010011010100100100001100100000001000000010000000110011010101110100100101000100010001010011000100100000001100011000000111111000001001010011101010111001101110100001000000101001101101111011011010110010100100000010001000110000101110100011000011010010101101110"};	

//char mess[]={"101011001100111000"};	


	BK4819_SetAfGain(0xB325);
	BK4819_EnableTone1(true);

	//APRS_send_Flag(100);
	cnt=strlen(mess);
	while(cnt--)
		if(mess[len++]=='0')
			send_tone(1);
		else
			send_tone(0);
	//APRS_send_Flag(10);
#else
	
	/* ==============================================================  */

	BK4819_SetAfGain(0xB325);
	BK4819_EnableTone1(true);
	//SPEAKER_TurnOn(SPEAKER_OWNER_SYSTEM);
	
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
		APRS_add_Pos();
	
	// send flag -----------------
	APRS_send_Flag(100);

	crc=0xFFFF;
	// send frame -----------------
	APRS_send_Frame();
	
	// send CRC -----------------
	APRS_send_FCS();
	
	// send flag -----------------
	APRS_send_Flag(10);
#endif	
	BK4819_EnableTone1(false);
}
