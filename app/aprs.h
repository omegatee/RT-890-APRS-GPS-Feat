#include "misc.h"
#include "app/css.h"

static const ChannelInfo_t gAPRSDefaultChannels[2] = {
	{
		.RX = { .Frequency = 14480000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 14480000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = 0,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 1,
		.bIsNarrow = 1,

		._0x11 = 0x11,
		.Scramble = 0x00,
		.IsInscanList = 0xFF,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
//		.Name = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
		.Name = { 'A', 'P', 'R', 'S', 0x20,'E', 'U', 'R', 0x20, 'V' },
	},
	
	{
		.RX = { .Frequency = 43250000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 43250000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = 0,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		._0x11 = 0x11,
		.Scramble = 0x00,
		.IsInscanList = 0xFF,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
//		.Name = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
		.Name = { 'A', 'P', 'R', 'S', 0x20,'E', 'U', 'R', 0x20, 'U' },
	}
};


void APRS_send_crc(void);
void APRS_send_Flag(uint8_t cnt);
void APRS_send_Field(char *field, uint8_t tLen);
void APRS_send_Node(char *call, char ssid);
