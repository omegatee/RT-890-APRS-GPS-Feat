/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include "app/css.h"
#include "app/radio.h"
#include "bsp/gpio.h"
#include "driver/bk4819.h"
#include "driver/delay.h"
#include "driver/pins.h"
#include "driver/speaker.h"
#include "driver/uart.h"///
#include "helper/helper.h"
#include "misc.h"
#include "radio/settings.h"

#include "ui/helper.h"///

/*enum {
	GPIO_FILTER_UHF     = 1U << 5,
	GPIO_FILTER_VHF     = 1U << 6,
	GPIO_FILTER_UNKWOWN = 1U << 7,
};*/

static void I2C_Delay(volatile uint8_t Counter)
{
	while (Counter-- > 0) {
	}
}

static void SDA_SetOutput(void)
{
	gpio_init_type init;

	gpio_default_para_init_ex(&init);
	init.gpio_pins = BOARD_GPIOB_BK4819_SDA;
	init.gpio_mode = GPIO_MODE_OUTPUT;
	init.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	gpio_init(GPIOB, &init);
}

static void SDA_SetInput(void)
{
	gpio_init_type init;

	gpio_default_para_init_ex(&init);
	init.gpio_pins = BOARD_GPIOB_BK4819_SDA;
	init.gpio_mode = GPIO_MODE_INPUT;
	init.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	gpio_init(GPIOB, &init);
}


static void I2C_Send(uint8_t Data)
{
	uint8_t i;

	for (i = 0; i < 8; i++) {
		gpio_bits_reset(GPIOB, BOARD_GPIOB_BK4819_SCL);
		if (Data & 0x80U) {
			gpio_bits_set(GPIOB, BOARD_GPIOB_BK4819_SDA);
		} else {
			gpio_bits_reset(GPIOB, BOARD_GPIOB_BK4819_SDA);
		}
		I2C_Delay(10);
		gpio_bits_set(GPIOB, BOARD_GPIOB_BK4819_SCL);
		Data <<= 1;
		I2C_Delay(10);
	}
}

static uint16_t I2C_RecvU16(void)
{
	uint8_t i;
	uint16_t Data = 0;

	SDA_SetInput();

	gpio_bits_reset(GPIOB, BOARD_GPIOB_BK4819_SCL);
	for (i = 0; i < 16; i++) {
		Data <<= 1;
		gpio_bits_set(GPIOB, BOARD_GPIOB_BK4819_SCL);
		if (gpio_input_data_bit_read(GPIOB, BOARD_GPIOB_BK4819_SDA)) {
			Data |= 1;
		}
		I2C_Delay(10);
		gpio_bits_reset(GPIOB, BOARD_GPIOB_BK4819_SCL);
		I2C_Delay(10);
	}

	return Data;
}

void OpenAudio(uint8_t gModulationType)
{
	switch(gModulationType) {
		case MOD_FM:
			BK4819_SetAF(BK4819_AF_NORMAL);
			break;
		case MOD_AM:
			BK4819_SetAF(BK4819_AF_7U);
			break;
		case MOD_LSB:
			BK4819_SetAF(BK4819_AF_4U);
			break;
		case MOD_USB:
			BK4819_SetAF(BK4819_AF_5U);
			break;
	}
}



uint16_t BK4819_ReadRegister(uint8_t Reg)
{
	uint16_t Data;

	TMR1->ctrl1_bit.tmren = FALSE;

	SDA_SetOutput();

	gpio_bits_reset(GPIOB, BOARD_GPIOB_BK4819_SCL);
	gpio_bits_reset(GPIOB, BOARD_GPIOB_BK4819_CS);

	I2C_Send(0x80U | Reg);
	I2C_Delay(10);
	Data = I2C_RecvU16();

	gpio_bits_set(GPIOB, BOARD_GPIOB_BK4819_CS);

	TMR1->ctrl1_bit.tmren = TRUE;

	return Data;
}

void BK4819_WriteRegister(uint8_t Reg, uint16_t Data)
{
//IFDBG UART_printf(1,"WR %04X at %02X\r\n",Data,Reg);
	TMR1->ctrl1_bit.tmren = FALSE;

	SDA_SetOutput();

	gpio_bits_reset(GPIOB, BOARD_GPIOB_BK4819_SCL);
	gpio_bits_reset(GPIOB, BOARD_GPIOB_BK4819_CS);

	I2C_Send(Reg);
	I2C_Send((Data >> 8) & 0xFFU);
	I2C_Send((Data >> 0) & 0xFFU);

	gpio_bits_set(GPIOB, BOARD_GPIOB_BK4819_CS);

	TMR1->ctrl1_bit.tmren = TRUE;
}

uint16_t BK4819_GetRSSI(void)
{
REGx67 reg67;
	
	reg67.val = BK4819_ReadRegister(0x67);
	
	if(gVfoState[gCurrentDial].RX.Frequency >= 24000000){
		reg67.RSSI-=15; /// for UHF
	}

	return reg67.RSSI;
}

int16_t BK4819_GetRSSI_DBM(void)
{
uint16_t rVAL;
int16_t dbVAL;
	
	rVAL = BK4819_GetRSSI();
//IFDBG UART_printf(1,"RSSI =%d\t",rVAL);	
	
	dbVAL=rVAL/=2;
	dbVAL-=160;

	// result value could fall in range -160..95.5
//IFDBG UART_printf(1,"(%d dBm)\r\n",dbVAL);
	return dbVAL;
}

void BK4819_Init(void)
{
	BK4819_WriteRegister(0x00, 0x8000);// RESET
	BK4819_WriteRegister(0x00, 0x0000);// !RESET
	BK4819_WriteRegister(0x37, 0x1D0F);
	BK4819_WriteRegister(0x36, 0x0022); /// from UV-K5 (PA configuration)
	BK4819_WriteRegister(0x33, 0x1F00);

	BK4819_WriteRegister(0x35, 0x0000);
	BK4819_WriteRegister(0x1E, 0x4C58);
	BK4819_WriteRegister(0x1F, 0xA656);
	BK4819_WriteRegister(0x3E, gCalibration.BandSelectionThreshold); // this defaults to 8E6A (DS default)
	BK4819_WriteRegister(0x3F, 0x0000);
	BK4819_WriteRegister(0x2A, 0x4F18);
	BK4819_WriteRegister(0x53, 0xE678);
	BK4819_WriteRegister(0x2C, 0x5705);
	BK4819_WriteRegister(0x4B, 0x7102);
	BK4819_WriteRegister(0x77, 0x88EF); /// squelch mode
	BK4819_WriteRegister(0x26, 0x13A0);
	BK4819_SetAFResponseCoefficients(false, true,  gCalibration.RX_3000Hz_Coefficient);
	BK4819_SetAFResponseCoefficients(false, false, gCalibration.RX_300Hz_Coefficient);
	BK4819_SetAFResponseCoefficients(true,  true,  gCalibration.TX_3000Hz_Coefficient);
	BK4819_SetAFResponseCoefficients(true,  false, gCalibration.TX_300Hz_Coefficient);
	BK4819_EnableRX();
	BK4819_SetAF(BK4819_AF_MUTE);

	BK4819_SetOptGainTables();
	BK4819_SetAGCLevel(0);

	// REG 0x2B is not used ???????????????????????????????????????

	BK4819_WriteRegister(0x7B, 0xAE34); // RSSI Table as per DS default
	BK4819_WriteRegister(0x7C, 0x8000); // RSSI Table
	
			BK4819_WriteRegister(0x3D, 0x2AAB);				// DS: default 8.46 kHz IF
			//BK4819_WriteRegister(0x3D, 0);				// TEST: use Zero IF
	
 			//BK4819_WriteRegister(0x3E,0x9000);			// band switch !?!?!
			BK4819_WriteRegister(0x3E,0x8E6A);				// band switch default as per DS
}

void BK4819_SetAFResponseCoefficients(bool bTx, bool bLowPass, uint8_t Index)
{
	uint16_t HighPass;
	uint16_t LowPass;

	if (bLowPass) {
		LowPass = 0;
		switch (Index) {
		case 0: HighPass = 58908; break;
		case 1: HighPass = 57122; break;
		case 2: HighPass = 54317; break;
		case 3: HighPass = 52277; break;
		case 4: HighPass = 64002; break;
		default: HighPass = 62731; break;
		}
	} else {
		switch (Index) {
		case 0: HighPass = 35799; LowPass = 13575; break;
		case 1: HighPass = 35565; LowPass = 13793; break;
		case 2: HighPass = 35478; LowPass = 13830; break;
		case 3: HighPass = 35080; LowPass = 14201; break;
		case 4: HighPass = 36318; LowPass = 13146; break;
		case 5: HighPass = 36531; LowPass = 12986; break;
		case 6: HighPass = 36744; LowPass = 12801; break;
		default: HighPass = 35811; LowPass = 13613; break;
		}
	}
	if (bTx) {
		if (!bLowPass) {
			BK4819_WriteRegister(0x44, HighPass);
			BK4819_WriteRegister(0x45, LowPass);
		} else {
			BK4819_WriteRegister(0x74, HighPass);
		}
	} else {
		if (!bLowPass) {
			BK4819_WriteRegister(0x54, HighPass);
			BK4819_WriteRegister(0x55, LowPass);
		} else {
			BK4819_WriteRegister(0x75, HighPass);
		}
	}
}

void BK4819_EnableRX(void)
{
	BK4819_WriteRegister(0x37, 0x1F0F);
	DELAY_WaitMS(10);
	BK4819_WriteRegister(0x30, 0x0200);
	BK4819_WriteRegister(0x30, 0xBFF1);
}

void BK4819_SetAF(BK4819_AF_Type_t Type)
{
REGx47 reg47;
	
	reg47.AF_Byp=0;
	reg47.AF_Out=Type;
	reg47.AF_Inv=1;
	BK4819_WriteRegister(0x47,reg47.val);
}

void BK4819_SetFrequency(uint32_t Frequency)
{
	FREQUENCY_SelectBand(Frequency);
	Frequency = (Frequency - 32768U) + gFrequencyBandInfo.FrequencyOffset;
	BK4819_WriteRegister(0x38, (Frequency >>  0) & 0xFFFFU);
	BK4819_WriteRegister(0x39, (Frequency >> 16) & 0xFFFFU);
}

void BK4819_SetSquelchMode(void)
{
	switch (gExtendedSettings.SqMode) {
		case 0:
			BK4819_WriteRegister(0x77, 0xFFEF); // RSSI
			break;
		case 1:
			BK4819_WriteRegister(0x77, 0xCCEF); // RSSI + noise
			break;
		case 2:
			BK4819_WriteRegister(0x77, 0xAAEF); // RSSI + Glitch
			break;
		case 3:
			BK4819_WriteRegister(0x77, 0x88EF); // RSSI + noise + Glitch
			break;
	}
}

void BK4819_SetSquelchGlitch(uint8_t BWidth)
{

#ifdef ENABLE_ALT_SQUELCH
	uint16_t Value;

	if (gSettings.Squelch == 0){
		Value = 255;
	} else {
		Value = gExtendedSettings.SqGlitchBase - (gSettings.Squelch);
	}

	BK4819_WriteRegister(0x4E, (BK4819_ReadRegister(0x4E) & 0xFF00) | Value);

	if (gSettings.Squelch == 0 || gExtendedSettings.SqGlitchBase > 230){
		Value = 255;
	} else {
		Value = (Value * 10) / 9;
	}

	BK4819_WriteRegister(0x4D, 0xA0 << 8 | Value);
#else

	static const uint8_t gSquelchGlitchLevel[11] = {
		0x20,
		0x20,
		0x1E,
		0x1C,
		0x1A,
		0x18,
		0x16,
		0x14,
		0x12,
		0x10,
		0x0E,
	};
	
	swith (BWidth) {
		case 0:
		case 1:
			BK4819_WriteRegister(0x4D, gSquelchGlitchLevel[gSettings.Squelch] + 0x9FFF);
			BK4819_WriteRegister(0x4E, gSquelchGlitchLevel[gSettings.Squelch] + 0x4DFE);
			break;
		case 2:
		case 3:
			BK4819_WriteRegister(0x4D, gSquelchGlitchLevel[gSettings.Squelch] + 0xA000);
			BK4819_WriteRegister(0x4E, gSquelchGlitchLevel[gSettings.Squelch] + 0x4DFF);
			break;
	}
#endif

}

void BK4819_SetSquelchNoise(uint8_t BWidth)
{
#ifdef ENABLE_ALT_SQUELCH

	uint16_t Value;

	if (gSettings.Squelch == 0){
		Value = (127 << 8) | 127;
	} else {
		Value = gExtendedSettings.SqNoiseBase - (gSettings.Squelch * 6);
		if (gExtendedSettings.SqNoiseBase > 230) {
			Value = (255 << 8) | Value;
		} else if (BWidth<2) {///
			Value = ((((Value - 5) * 10) / 9) << 8) | Value;
		} else {
			Value = (((Value * 10) / 9) << 8) | Value;
		 }
	}

	BK4819_WriteRegister(0x4F, Value);

#else

	static const uint8_t gSquelchNoiseLevel[11] = {
		0x0A,
		0x0A,
		0x09,
		0x08,
		0x07,
		0x06,
		0x05,
		0x04,
		0x03,
		0x02,
		0x01,
	};

	uint8_t Level;
	uint16_t Value;

	Level = gSquelchNoiseLevel[gSettings.Squelch];
	if (BWidth<2) {///
		Value = ((gSquelchNoiseNarrow + 12 + Level) << 8) | (gSquelchNoiseNarrow + 6 + Level);
	} else {
		Value = ((gSquelchNoiseWide   + 12 + Level) << 8) | (gSquelchNoiseWide   - 6 + Level);
	}

	BK4819_WriteRegister(0x4F, Value);

#endif

}

void BK4819_SetSquelchRSSI(uint8_t BWidth)
{
#ifdef ENABLE_ALT_SQUELCH

	uint16_t Value;

	if (gSettings.Squelch == 0){
		Value = 0;
	} else {
		Value = gExtendedSettings.SqRSSIBase + (gSettings.Squelch * 8);
		Value = (Value << 8) | (Value * 9) / 10;
	}

	BK4819_WriteRegister(0x78, Value);
#else

	static const uint8_t gSquelchRssiLevel[11] = {
		0x00,
		0x00,
		0x02,
		0x04,
		0x06,
		0x0A,
		0x10,
		0x16,
		0x1C,
		0x22,
		0x26,
	};

	uint8_t Level;
	uint16_t Value;

	Level = gSquelchRssiLevel[gSettings.Squelch];
	if (BWidth<2) {///
		Value = ((gSquelchRSSINarrow - 8 + Level) << 8) | (gSquelchRSSINarrow - 14 + Level);
	} else {
		Value = ((gSquelchRSSIWide   - 8 + Level) << 8) | (gSquelchRSSIWide   - 14 + Level);
	}

	BK4819_WriteRegister(0x78, Value);

#endif

}


void BK4819_SetFilterBandwidth(uint8_t BWidth, bool weak)
{
REGx43 reg43;
	
	reg43._res3=1;
	switch (BWidth)
	{
		case 0:						// NARROW NARROW 6,25k
			reg43.BWd=0;
			reg43.BWdW=0;
			reg43.AFTX_LPF2=1;
			reg43.BW_Mode=1u;
			reg43.GainAfter=0u;	
			break;
		case 1:						// NARROW 12,5k
			reg43.BWd=4;
			reg43.BWdW=4;
			reg43.AFTX_LPF2=1;
			reg43.BW_Mode=0u;
			reg43.GainAfter=0u;	
			break;
		case 2:						// WIDE 25,0k
			reg43.BWd=3;
			reg43.BWdW=3;
			reg43.AFTX_LPF2=0;
			reg43.BW_Mode=2u;
			reg43.GainAfter=0u;	
			break;
		case 3:						// WIDE WIDE ?,0k
			reg43.BWd=7;
			reg43.BWdW=7;
			reg43.AFTX_LPF2=0;
			reg43.BW_Mode=3u;
			reg43.GainAfter=0u;	
			break;
	}
	BK4819_WriteRegister(0x43, reg43.val);
}


void BK4819_SelectRFPath(uint32_t Frequency)
{
// REG33 is 1FA0 below 240MHz and 1FC0 above
// bit 6 on = low band; bit 7 on = high band

REGx33 reg33;

	reg33.val = BK4819_ReadRegister(0x33);
	//reg33._res7 = 1; What's this for ???
	
	if( Frequency <= (gCalibration.BandSelectionThreshold*6144/10) ){// this equates to 252MHz with reg 3E @ 0x8E6A (DS default)
		/// LOW BAND
		reg33.GPIO5_Val = 1;
		reg33.GPIO6_Val = 0;
	}
	else{
		/// HIGH BAND
		reg33.GPIO5_Val = 0;
		reg33.GPIO6_Val = 1;
	}
IFDBG UART_printf(1,"RG33 at 0x%04X\r\n",reg33.val);
	BK4819_WriteRegister(0x33, reg33.val);
}

void BK4819_EnableScramble(uint8_t Scramble)
{
	uint16_t Value;

	BK4819_WriteRegister(0x71, 0x68DC | (Scramble * 1032));

	Value = BK4819_ReadRegister(0x31);
	if (Scramble) {
		Value |= 2;
	} else {
		Value &= ~2;
	}
	BK4819_WriteRegister(0x31, Value);
}

void BK4819_EnableCompander(bool bEnable)
{
	BK4819_WriteRegister(0x31, BK4819_ReadRegister(0x31) & ~8U);
	if (bEnable) {
		BK4819_WriteRegister(0x28, gCalibration.AF_RX_Expander);
		BK4819_WriteRegister(0x29, gCalibration.AF_TX_Compress);
	} else {
		BK4819_WriteRegister(0x28, 0x0000);
		BK4819_WriteRegister(0x29, 0x0000);
	}
}

void BK4819_EnableVox(bool bEnable)
{
	uint16_t Value;

	Value = BK4819_ReadRegister(0x31);
	if (bEnable) {
		Value |= 4U;
	} else {
		Value &= ~4U;
	}
	BK4819_WriteRegister(0x31, Value);
}

void BK4819_SetDefaultGainTables()
{
REGx49 reg49;
	
	// Default values as per DS
	BK4819_WriteRegister(0x10, 0x0038);  // 0,1,3,0
	BK4819_WriteRegister(0x11, 0x025a);  // 2,2,3,2
	BK4819_WriteRegister(0x12, 0x037b); 
	BK4819_WriteRegister(0x13, 0x03de);
	BK4819_WriteRegister(0x14, 0x0000);	 // 0,0,0,0
	
	reg49.AGC_LowThres=0x30;
	reg49.AGC_HighThres=0x50;
	reg49.Sel_Auto=0x0;
	BK4819_WriteRegister(0x49, reg49.val);

}

void BK4819_SetOptGainTables()
{
REGx10 reg10;
REGx49 reg49;
	
	// The 5 RX AGC Gain Tables:
	//  (Index for max. to min.: 3, 2, 1, 0, -1)
	
	// Gain Table[3]		// MAX GAIN
	reg10.LNA_GainS=3;
	reg10.LNA_Gain =7;
	reg10.MIX_Gain =3;
	reg10.PGA_Gain =7;
	BK4819_WriteRegister(0x13,reg10.val);
	
	// Gain Table[2]
	reg10.LNA_GainS=3;
	reg10.LNA_Gain =4;
	reg10.MIX_Gain =3;
	reg10.PGA_Gain =4;	
	BK4819_WriteRegister(0x12,reg10.val);
	
	// Gain Table[1]
	reg10.LNA_GainS=2;
	reg10.LNA_Gain =3;
	reg10.MIX_Gain =2;
	reg10.PGA_Gain =3;
	BK4819_WriteRegister(0x11,reg10.val);
	
	// Gain Table[0]
	reg10.LNA_GainS=2;
	reg10.LNA_Gain =1;
	reg10.MIX_Gain =2;
	reg10.PGA_Gain =1;
	BK4819_WriteRegister(0x10,reg10.val);

	
	// Gain Table[-1]		// MIN GAIN
	reg10.LNA_GainS=0;
	reg10.LNA_Gain =0;
	reg10.MIX_Gain =0;
	reg10.PGA_Gain =0;
	BK4819_WriteRegister(0x14,reg10.val);

	
		reg49.AGC_LowThres=20;
		reg49.AGC_HighThres=44;
		reg49.Sel_Auto = 0;
		BK4819_WriteRegister(0x49, reg49.val);
}

void BK4819_SetAGCLevel(uint8_t lvl)
{
REGx7E reg7E;

	reg7E.val = BK4819_ReadRegister(0x7E);

	if(lvl==0){
		//Turn AGC on
		reg7E.AGC_Mode = 0;
	}
	else{
		//Turn AGC off
		reg7E.AGC_Mode = 1;
		reg7E.AGC_Index = lvl>4? -1 : lvl-1;
	}
	BK4819_WriteRegister(0x7E,reg7E.val);
}



void BK4819_SetToneFrequency(bool Tone, uint16_t freq)
{
	BK4819_WriteRegister(0x71 + Tone, (freq * 103U) / 10U);
}

void BK4819_EnableFFSK1200(bool bEnable)
{
	if (bEnable) {
		BK4819_WriteRegister(0x70, 0x00E0);
		BK4819_WriteRegister(0x72, 0x3065);
		BK4819_WriteRegister(0x58, 0x37C3);
		BK4819_WriteRegister(0x5C, 0x5665);
		BK4819_WriteRegister(0x5D, 0x0F00);
	} else {
		BK4819_WriteRegister(0x70, 0x0000);
		BK4819_WriteRegister(0x58, 0x0000);
	}
}

void BK4819_ResetFSK(void)
{
	BK4819_WriteRegister(0x3F, 0x0000);
	BK4819_WriteRegister(0x59, 0x0028);
	BK4819_WriteRegister(0x30, 0x0000);
}

void BK4819_StartAudio(void)
{
uint16_t reg_73;
	
	gpio_bits_set(GPIOA, BOARD_GPIOA_LED_GREEN);
	gRadioMode = RADIO_MODE_RX;
	OpenAudio(gMainVfo->gModulationType);
	BK4819_SetFilterBandwidth(gMainVfo->gBandWidth,0);
	switch (gMainVfo->gModulationType){
		case MOD_FM:
			BK4819_SetDefaultGainTables();///BK4819_SetAGCLevel(4);
			BK4819_SetAfGain(0);
			
			BK4819_WriteRegister(0x4D, 0xA080); /// Glitch threshold for Squelch
			BK4819_WriteRegister(0x4E, 0x6F7C); /// Squelch delay

			BK4819_EnableScramble(gMainVfo->Scramble);
			BK4819_EnableCompander(true);

			// Clear bit 4 of register 73 (Enable AFC)
			reg_73 = BK4819_ReadRegister(0x73);
			BK4819_WriteRegister(0x73, reg_73 & ~0x10U); /// AFC settings

			if (gMainVfo->Scramble == 0) {
				BK4819_SetAFResponseCoefficients(false, true, gCalibration.RX_3000Hz_Coefficient);
			} 
			else {
				BK4819_SetAFResponseCoefficients(false, true, 4);
			}
			break;

		case MOD_AM:
		case MOD_USB:
		case MOD_LSB:
			BK4819_SetOptGainTables();///BK4819_SetAGCLevel(0);
			BK4819_SetAfGain(1);
			
			BK4819_EnableScramble(0);
			BK4819_EnableCompander(false);

			// Set bit 4 of register 73 (Disable AFC)
			reg_73 = BK4819_ReadRegister(0x73);
			BK4819_WriteRegister(0x73, reg_73 | 0x10U);

			if (gMainVfo->gModulationType > MOD_AM) { // if SSB
				BK4819_WriteRegister(0x37, 0b0001011000001111);
				BK4819_WriteRegister(0x3D, 0b0010101101000101);	// 0x2B45 --> undocumented IF selection
				//	BK4819_WriteRegister(0x3D, 0x2AAB);				// DS: default
				//	BK4819_WriteRegister(0x3D, 0);					// use Zero IF selection
				//	BK4819_WriteRegister(0x48, 0b0000001110101000);
			}
			break;
	}
	if (!gReceptionMode) {
		BK4819_EnableFFSK1200(true);
	}
	SPEAKER_TurnOn(SPEAKER_OWNER_RX);
}

void BK4819_SetAfGain(uint16_t Gain)
{
REGx48 reg48;

	reg48.AF_RX_Gain2 = 0x3F;
	reg48.AF_DAC_Gain = 0xF;
	switch (Gain){
		case 0:
			reg48.AF_RX_Gain1 = 0x01;	// -6dB gain
			break;
		case 1:
			reg48.AF_RX_Gain1 = 0x00;	// max gain
			break;
	}
	   
	//reg48.val=0x33CF; // as default per DS

	BK4819_WriteRegister(0x48,reg48.val);

}

bool BK4819_CheckSquelchStat(void)
{
REGx0C reg0C;
	if (gSettings.Squelch && !gMonitorMode) {
		reg0C.val = BK4819_ReadRegister(0x0C);
		return reg0C.SQL;
	}

	return true;
}

void BK4819_EnableTone1(bool bEnable)
{
	uint16_t Value;

	Value = BK4819_ReadRegister(0x70);
	Value = (Value & ~0x8000U) | (bEnable << 15);
	BK4819_WriteRegister(0x70, 0x4000 | Value);

	if (bEnable) {
		if (gRadioMode != RADIO_MODE_TX) {
			BK4819_WriteRegister(0x30, 0x0302);
			BK4819_SetAF(BK4819_AF_RXBEEP);
		} else {
			BK4819_WriteRegister(0x30, 0xC3FA);
			BK4819_SetAF(BK4819_AF_TXBEEP);
		}
	} else {
		if (gRadioMode == RADIO_MODE_TX) {
			BK4819_WriteRegister(0x30, 0xC1FE);
		} else {
			BK4819_WriteRegister(0x30, 0xBFF1);
		}
		if (gRadioMode != RADIO_MODE_RX) {
			BK4819_SetAF(BK4819_AF_MUTE);
		} else {
			OpenAudio(gMainVfo->gModulationType);/// gMainVfo->gBandWidth, 
			if (gMainVfo->gModulationType > MOD_FM) {
				BK4819_EnableScramble(0); // AM, SSB
			} else {
				BK4819_EnableScramble(gMainVfo->Scramble); // FM
			}
		}
	}
}

void BK4819_GenTail(bool bIsNarrow)
{
	if (gSettings.TailTone) {
		if (bIsNarrow) {
			BK4819_WriteRegister(0x51, gFrequencyBandInfo.CtcssTxGainNarrow | 0x9000);
		} else {
			BK4819_WriteRegister(0x51, gFrequencyBandInfo.CtcssTxGainWide | 0x9000);
		}
		if (gTxCodeType == CODE_TYPE_OFF || gTxCodeType == CODE_TYPE_CTCSS) {
			BK4819_WriteRegister(0x07, 0x046F);
		} else {
			BK4819_WriteRegister(0x52, 0x823F);
		}
		DELAY_WaitMS(250);
	}
}

void BK4819_SetupPowerAmplifier(uint8_t level, uint32_t frequency)
{
REGx36 reg36;
	
	reg36.PA_Gain1=(frequency < 28000000) ? 1:2;
	reg36.PA_Gain2=(frequency < 28000000) ? 2:4;
	reg36.PA_Enable=1;// this does not cut RF
	switch(level){
		case 0: // OFF
			reg36.PA_Bias=0; 
			reg36.PA_Gain1=0;
			reg36.PA_Gain2=0;
			break;
		case 1: // LOW
			reg36.PA_Bias=0x20; //  350 mW
			break;
		case 2: // MID
			reg36.PA_Bias=0x60; // 2800 mW
			break;
		case 3: // HIGH
			reg36.PA_Bias=0xB0; // 5600 mW
			break;
	}
	BK4819_WriteRegister(0x36, reg36.val);
}


void BK4819_SetRFTXDeviation(uint8_t level)
{
REGx40 reg40;
	
	switch(level){
		case 0:
			reg40.TX_Deviation=0x2D0;//Deviation = 0x2D0; // 6,25 k
			break;
		case 1:
			reg40.TX_Deviation=0x4D0;//Deviati.= 0x4D0; // 12,5 k
			break;
		case 2:
			reg40.TX_Deviation=0x540;//Deviation.0x540; // 25 k
			break;
		case 3:
			reg40.TX_Deviation=0x97F;//Deviation = 0.F; // 125 k
			break;
	}

	if (gMainVfo->Scramble) {
		reg40.TX_Deviation-=200;//=Deviation -= 200;
	}

	reg40.TX_Deviation_Enable=1;
	
	BK4819_WriteRegister(0x40, reg40.val);
}

void BK4819_SetMicSensitivityTuning(void)
{
	BK4819_WriteRegister(0x7D, 0xE940 | (gExtendedSettings.MicGainLevel & 0x1F));
}

void BK4819_EnableTX(bool bUseMic)
{
	BK4819_WriteRegister(0x37, 0x1D0F);
	DELAY_WaitMS(10);
	BK4819_WriteRegister(0x52, 0x028F);
	BK4819_WriteRegister(0x30, 0x0200);
	if (bUseMic) {
		BK4819_WriteRegister(0x30, 0xC1FE);
	} else {
		BK4819_WriteRegister(0x30, 0xC3FA);
	}
}

void BK4819_StartFrequencyScan(void)
{
	BK4819_WriteRegister(0x32, 0x0B01);
	DELAY_WaitMS(200);
}

void BK4819_StopFrequencyScan(void)
{
	BK4819_WriteRegister(0x32, 0x0000);
}

void BK4819_DisableAutoCssBW(void)
{
	BK4819_WriteRegister(0x51, 0x0300);
	DELAY_WaitMS(200);
	BK4819_EnableRX();
}

#ifdef ENABLE_SPECTRUM
void BK4819_SetRFFrequency(const uint32_t frequency, const bool trigger_update)
{
	BK4819_WriteRegister(0x38, (frequency >> 0) & 0xFFFF);
	BK4819_WriteRegister(0x39, (frequency >> 16) & 0xFFFF);

	if (trigger_update)
	{ // trigger a PLL/VCO update
		const uint16_t reg = BK4819_ReadRegister(0x30);
		BK4819_WriteRegister(0x30, reg & ~BK4819_REG_30_ENABLE_VCO_CALIB);
		BK4819_WriteRegister(0x30, reg);
	}
}
#endif
