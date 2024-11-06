#include <string.h>
#include "driver/uart.h"
#include "driver/afsk.h"
#include "misc.h"

void APRS_send_crc(void)
{
  unsigned char crc_lo = crc ^ 0xFF;
  unsigned char crc_hi = (crc >> 8) ^ 0xFF;

  AFSK_SendByte(crc_lo, true); 
  AFSK_SendByte(crc_hi, true);
  	//AFSK_SendByte(crc_hi, true); //???
  	//AFSK_SendByte(crc_lo, true);
}

void APRS_send_Flag(uint8_t cnt)
{
    while(cnt--)					
		AFSK_SendByte(0x7E, 0);
	crc = 0xffff;
}

	
void APRS_send_Field(char *field, uint8_t tLen)
{
uint8_t fLen;
uint8_t c;

    fLen=strlen(field);
    if(fLen>tLen){
        UART_printf(1,"ERR: field [%s] is bigger than %d\r\n",field,tLen);
        return;
    }

	c=0;
	while(tLen--){
		if(c<fLen)
			AFSK_SendByte(field[c] << 1, true);
		else
			AFSK_SendByte(' ' << 1, true);
		c++;
	}
}

void APRS_send_Node(char *call, char ssid)
{
IFDBG UART_printf(1,"\r\nFIELD: [%s]%d\r\n",call,ssid);
/* ??
 * ALL DEST, SOURCE, & DIGI are left shifted 1 bit, ASCII format.
 * DIGI ssid is left shifted 1 bit + 1
*/  
	APRS_send_Field(call,6);
	AFSK_SendByte((ssid + 0x30) << 1, true); // if DIGI, +1 (??)
	
}
