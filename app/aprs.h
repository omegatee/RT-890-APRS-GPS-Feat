#include "misc.h"
#include "app/css.h"

static const ChannelInfo_t gAPRSDefaultChannels[3] = {
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
		.bIsLowPower = 0, // for debug
		.bIsNarrow = 1,

		.gBandWidth = 0x0F,
		.gTXPower = 0x0F,
		.Scramble = 0x00,
		.IsInscanList = 0xFF,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
//		.Name = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
		.Name = { "APRS EUR V" }
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

		.gBandWidth = 0x0F,
		.gTXPower = 0x0F,
		.Scramble = 0x00,
		.IsInscanList = 0xFF,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = { "APRS EUR U" }
	},
	
	{
		.RX = { .Frequency = 14439000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 14439000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
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

		.gBandWidth = 0x0F,
		.gTXPower = 0x0F,
		.Scramble = 0x00,
		.IsInscanList = 0xFF,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = { "APRS USA V" }
	}

};

void APRS_send_Flag(uint16_t cnt);

void APRS_add_Address(char *call, char ssid, bool last);
void APRS_add_CTRL(void);
void APRS_add_Pos(void);
void APRS_send_FCS(void);

void APRS_send_Frame(void);


void APRS_send_Packet(uint8_t Type);
