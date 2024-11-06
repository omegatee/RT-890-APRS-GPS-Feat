#include "misc.h"
#include "driver/bk4819.h"
#include "driver/delay.h"
#include "driver/uart.h"

#define SYM_TIME 833	// nominal is 833 (1/1200)

char bit_stuff = 0;
bool tone = 0;
uint16_t crc=0xFFFF;

void send_tone(bool ton)
{
	// Nominal for Bell 202 AFSK: MARK(1) - 1200 SPACE(0) - 2200
	if(ton)
		BK4819_SetToneFrequency(false, 1200);// trimmed 1200
	else
		BK4819_SetToneFrequency(false, 2200);// trimmed 2200 

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


void AFSK_SendByte(char byte, bool BIT){
uint8_t i;
bool bit;

UART_SendByte(1,byte);
	for(i = 0; i < 8; i++){
    	bit = byte & 0x01;
		calc_crc(bit);
		if(bit){
			send_tone(tone);
			bit_stuff++;
			if((BIT) && (bit_stuff == 5)){
				tone ^= 1;
				send_tone(tone);
				bit_stuff = 0;
			}
		}
		else{
			tone ^= 1;
			send_tone(tone);
			bit_stuff = 0;
		}
		byte >>= 1;
	}
	tone = 0; /// ??? +++ mmm
}


