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

enum {
	GPIO_FILTER_UHF     = 1U << 5,
	GPIO_FILTER_VHF     = 1U << 6,
	GPIO_FILTER_UNKWOWN = 1U << 7,
};

static void Delay(volatile uint8_t Counter)
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
		Delay(10);
		gpio_bits_set(GPIOB, BOARD_GPIOB_BK4819_SCL);
		Data <<= 1;
		Delay(10);
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
		Delay(10);
		gpio_bits_reset(GPIOB, BOARD_GPIOB_BK4819_SCL);
		Delay(10);
	}

	return Data;
}

#if 0
static void DisableAGC(uint32_t Unknown)
{
	BK4819_WriteRegister(0x13, 0x03BE);
	BK4819_WriteRegister(0x12, 0x037B);
	BK4819_WriteRegister(0x11, 0x027B);
	BK4819_WriteRegister(0x10, 0x007A);
	BK4819_WriteRegister(0x14, 0x0019);
	BK4819_WriteRegister(0x49, 0x2A38);
	BK4819_WriteRegister(0x7B, 0x8420);
}
#endif

void OpenAudio(bool bIsNarrow, uint8_t gModulationType)
{
	switch(gModulationType) {
		case 0:								/// FM
			BK4819_SetAF(BK4819_AF_NORMAL);
			break;
		case 1:								/// AM
			BK4819_SetAF(BK4819_AF_7U);
			break;
		case 2:								/// LSB
			BK4819_SetAF(BK4819_AF_4U);
			break;
		case 3:								/// USB
			BK4819_SetAF(BK4819_AF_5U);
			break;
	}
	if (bIsNarrow) {
		BK4819_SetAfGain(gFrequencyBandInfo.RX_DAC_GainNarrow);
	} else {
		BK4819_SetAfGain(gFrequencyBandInfo.RX_DAC_GainWide);
	}
}

// Public

uint16_t BK4819_ReadRegister(uint8_t Reg)
{
	uint16_t Data;

	TMR1->ctrl1_bit.tmren = FALSE;

	SDA_SetOutput();

	gpio_bits_reset(GPIOB, BOARD_GPIOB_BK4819_SCL);
	gpio_bits_reset(GPIOB, BOARD_GPIOB_BK4819_CS);

	I2C_Send(0x80U | Reg);
	Delay(10);
	Data = I2C_RecvU16();

	gpio_bits_set(GPIOB, BOARD_GPIOB_BK4819_CS);

	TMR1->ctrl1_bit.tmren = TRUE;

	return Data;
}

void BK4819_WriteRegister(uint8_t Reg, uint16_t Data)
{
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
	return BK4819_ReadRegister(0x67) & 0x01FF;
}

void BK4819_Init(void)
{
	BK4819_WriteRegister(0x00, 0x8000);
	BK4819_WriteRegister(0x00, 0x0000);
	BK4819_WriteRegister(0x37, 0x1D0F);
BK4819_WriteRegister(0x36, 0x0022); /// from UV-K5
	// DisableAGC(0);
	BK4819_WriteRegister(0x33, 0x1F00);
	BK4819_WriteRegister(0x35, 0x0000);
	BK4819_WriteRegister(0x1E, 0x4C58);
	BK4819_WriteRegister(0x1F, 0xA656);
	BK4819_WriteRegister(0x3E, gCalibration.BandSelectionThreshold);
	BK4819_WriteRegister(0x3F, 0x0000);
	BK4819_WriteRegister(0x2A, 0x4F18);
	BK4819_WriteRegister(0x53, 0xE678);
	BK4819_WriteRegister(0x2C, 0x5705);
	BK4819_WriteRegister(0x4B, 0x7102);
	//BK4819_WriteRegister(0x77, 0x88EF);
	BK4819_WriteRegister(0x26, 0x13A0);
	BK4819_SetAFResponseCoefficients(false, true,  gCalibration.RX_3000Hz_Coefficient);
	BK4819_SetAFResponseCoefficients(false, false, gCalibration.RX_300Hz_Coefficient);
	BK4819_SetAFResponseCoefficients(true,  true,  gCalibration.TX_3000Hz_Coefficient);
	BK4819_SetAFResponseCoefficients(true,  false, gCalibration.TX_300Hz_Coefficient);
	BK4819_EnableRX();
	BK4819_SetAF(BK4819_AF_MUTE);
	BK4819_RestoreGainSettings();
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
	//BK4819_WriteRegister(0x47, 0x6040 | (Type << 8));		///WT:
	BK4819_WriteRegister(0x47,
						 			(0) 			// AF Tx Filter 0=Normal 1=Bypass All
						 			|(Type << 8) 	// AFOutputSelection
						 			|(1 << 13)		// AF Output Inverse Mode
						 			|(1 << 14)		// ?
						 			|(1 << 6)		// ?
						 );
						 			
	
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

void BK4819_SetSquelchGlitch(bool bIsNarrow)
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
	
	if (bIsNarrow) {
		BK4819_WriteRegister(0x4D, gSquelchGlitchLevel[gSettings.Squelch] + 0x9FFF);
		BK4819_WriteRegister(0x4E, gSquelchGlitchLevel[gSettings.Squelch] + 0x4DFE);
	} else {
		BK4819_WriteRegister(0x4D, gSquelchGlitchLevel[gSettings.Squelch] + 0xA000);
		BK4819_WriteRegister(0x4E, gSquelchGlitchLevel[gSettings.Squelch] + 0x4DFF);
	}
#endif

}

void BK4819_SetSquelchNoise(bool bIsNarrow)
{
#ifdef ENABLE_ALT_SQUELCH

	uint16_t Value;

	if (gSettings.Squelch == 0){
		Value = (127 << 8) | 127;
	} else {
		Value = gExtendedSettings.SqNoiseBase - (gSettings.Squelch * 6);
		if (gExtendedSettings.SqNoiseBase > 230) {
			Value = (255 << 8) | Value;
		} else if (bIsNarrow) {
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
	if (bIsNarrow) {
		Value = ((gSquelchNoiseNarrow + 12 + Level) << 8) | (gSquelchNoiseNarrow + 6 + Level);
	} else {
		Value = ((gSquelchNoiseWide   + 12 + Level) << 8) | (gSquelchNoiseWide   - 6 + Level);
	}

	BK4819_WriteRegister(0x4F, Value);

#endif

}

void BK4819_SetSquelchRSSI(bool bIsNarrow)
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
	if (bIsNarrow) {
		Value = ((gSquelchRSSINarrow - 8 + Level) << 8) | (gSquelchRSSINarrow - 14 + Level);
	} else {
		Value = ((gSquelchRSSIWide   - 8 + Level) << 8) | (gSquelchRSSIWide   - 14 + Level);
	}

	BK4819_WriteRegister(0x78, Value);

#endif

}

#if 0
void BK4819_SetFilterBandwidth(bool bIsNarrow)
{
	// Check if modulation is FM
	if (gMainVfo->gModulationType == 0) { // if FM
#ifndef ENABLE_REGISTER_EDIT
		if (bIsNarrow) {
			BK4819_WriteRegister(0x43, 0x1008); // as per DS default for 12,5 kHz
			//BK4819_WriteRegister(0x43, 0x4048); //stock
			//BK4819_WriteRegister(0x43, 0x7B08); //kamil/fagci
			//BK4819_WriteRegister(0x43, 0x1408);//OEFW
			//BK4819_WriteRegister(0x43, 0x4408); //egzumer
		} else {
			BK4819_WriteRegister(0x43, 0x2020); // as per DS default for 25 kHz
			//BK4819_WriteRegister(0x43, 0x3028); //stock
			//BK4819_WriteRegister(0x43, 0x7B08); //kamil/fagci
			//BK4819_WriteRegister(0x43, 0x1408);//OEFW
			//BK4819_WriteRegister(0x43, 0x45A8); //egzumer
		}
#else
		uint16_t Value = BK4819_ReadRegister(0x43); 
		if (bIsNarrow) {
			BK4819_WriteRegister(0x43, (Value & ~0x30) | 0);
		} else {
			BK4819_WriteRegister(0x43, (Value & ~0x30) | 32);
		}
#endif
	}
}
#else
//void BK4819_SetFilterBandwidth(const BK4819_FilterBandwidth_t Bandwidth, const bool weak_no_different)
void BK4819_SetFilterBandwidth(bool Bandwidth, bool weak)
{
	
	// REG_43
	// <15>    0 ???
	//
	// <14:12> 4 RF filter bandwidth
	//         0 = 1.7  kHz
	//         1 = 2.0  kHz
	//         2 = 2.5  kHz
	//         3 = 3.0  kHz
	//         4 = 3.75 kHz
	//         5 = 4.0  kHz
	//         6 = 4.25 kHz
	//         7 = 4.5  kHz
	// if <5> == 1, RF filter bandwidth * 2
	//
	// <11:9>  0 RF filter bandwidth when signal is weak
	//         0 = 1.7  kHz
	//         1 = 2.0  kHz
	//         2 = 2.5  kHz
	//         3 = 3.0  kHz
	//         4 = 3.75 kHz
	//         5 = 4.0  kHz
	//         6 = 4.25 kHz
	//         7 = 4.5  kHz
	// if <5> == 1, RF filter bandwidth * 2
	//
	// <8:6>   1 AFTxLPF2 filter Band Width
	//         1 = 2.5  kHz (for 12.5k channel space)
	//         2 = 2.75 kHz
	//         0 = 3.0  kHz (for 25k   channel space)
	//         3 = 3.5  kHz
	//         4 = 4.5  kHz
	//         5 = 4.25 kHz
	//         6 = 4.0  kHz
	//         7 = 3.75 kHz
	//
	// <5:4>   0 BW Mode Selection
	//         0 = 12.5k
	//         1 =  6.25k
	//         2 = 25k/20k
	//
	// <3>     1 ???
	//
	// <2>     0 Gain after FM Demodulation
	//         0 = 0dB
	//         1 = 6dB
	//
	// <1:0>   0 ???

	uint16_t val = 0;
	switch (Bandwidth)
	{
		default:
		//case BK4819_FILTER_BW_WIDE:	// 25kHz
		if(Bandwidth==0){
			val = (4u << 12) |     // *3 RF filter bandwidth
				  (6u <<  6) |     // *0 AFTxLPF2 filter Band Width
				  (2u <<  4) |     //  2 BW Mode Selection
				  (1u <<  3) |     //  1
				  (0u <<  2);     //  0 Gain after FM Demodulation

			if (weak) {
				/// with weak RX signals the RX bandwidth is reduced
				val |= (2u <<  9);     // *0 RF filter bandwidth when signal is weak
			} else {
				// make the RX bandwidth the same with weak signals
				val |= (4u <<  9);     // *0 RF filter bandwidth when signal is weak
			}

		}//break;

		//case BK4819_FILTER_BW_NARROW:	// 12.5kHz
		else{
			val = (4u << 12) |     // *4 RF filter bandwidth
				  (0u <<  6) |     // *1 AFTxLPF2 filter Band Width
				  (0u <<  4) |     //  0 BW Mode Selection
				  (1u <<  3) |     //  1
				  (0u <<  2);      //  0 Gain after FM Demodulation

			if (weak) {
				val |= (2u <<  9);
			} else {
				val |= (4u <<  9);     // *0 RF filter bandwidth when signal is weak
			}

		}//break;
/*
		case BK4819_FILTER_BW_NARROWER:	// 6.25kHz
			val = (3u << 12) |     //  3 RF filter bandwidth
				  (3u <<  9) |     // *0 RF filter bandwidth when signal is weak
				  (1u <<  6) |     //  1 AFTxLPF2 filter Band Width
				  (1u <<  4) |     //  1 BW Mode Selection
				  (1u <<  3) |     //  1
				  (0u <<  2);      //  0 Gain after FM Demodulation

			if (weak) {
				val |= (0u <<  9);     //  0 RF filter bandwidth when signal is weak
			} else {
				val |= (3u <<  9);
			}
			break;*/
	}
//IFDBG UART_printf(1,"Wid: %d/r/n",val); narrow= 0x4808 wide=0x49A8
	BK4819_WriteRegister(0x43, val);
}

#endif


void BK4819_EnableFilter(bool bEnable)
{
	uint16_t Value;

	Value = BK4819_ReadRegister(0x33);
	Value |= 0
		| GPIO_FILTER_UHF
		| GPIO_FILTER_VHF
		| GPIO_FILTER_UNKWOWN;

	if (bEnable) {
		if (gCalibration.BandSelectionThreshold == 0xAAAA) {
			if (!gUseUhfFilter) {
				Value &= ~GPIO_FILTER_UNKWOWN;
			} else {
				Value &= ~GPIO_FILTER_VHF;
			}
		}
		else if (!gUseUhfFilter) {
			Value &= ~GPIO_FILTER_VHF;
		} else {
			Value &= ~GPIO_FILTER_UHF;
		}
	}
	BK4819_WriteRegister(0x33, Value);
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

void BK4819_RestoreGainSettings()
{
	/* Keep for now, may be useful for register editing
	const uint8_t orig_lna_short = 2;
            const uint8_t orig_lna = 5;
            const uint8_t orig_mixer = 2;
            const uint8_t orig_pga = 5;
			if(BK4819_ReadRegister(0x13) != ((orig_lna_short << 8) | (orig_lna << 5) | (orig_mixer << 3) | (orig_pga << 0))) {
            	BK4819_WriteRegister(0x13, (orig_lna_short << 8) | (orig_lna << 5) | (orig_mixer << 3) | (orig_pga << 0));
			}
	*/
	
	//Default values
	BK4819_WriteRegister(0x10, 0x0038);  
	BK4819_WriteRegister(0x11, 0x025a);  
	BK4819_WriteRegister(0x12, 0x037b); 
	BK4819_WriteRegister(0x13, 0x03de);
	BK4819_WriteRegister(0x14, 0x0000); 
}

void BK4819_ToggleAGCMode()
{
	// REG_7E[15] - AGC Mode
	// 1 - Fixed, 0 - Auto

	uint16_t Value = BK4819_ReadRegister(0x7E);
	uint16_t AGCIndex = (Value & 0x7000) >> 12; // Extract bits 14, 13 and 12

	if (!(Value & 0x8000U) || AGCIndex == 4){ //Bit 15 = 0 (AGC on) or 14:12 = 110 (index 4/min)
		Value ^= 0x8000U; //Toggle AGC
	} else {
		Value |= 0x8000U; //Turn AGC off
		AGCIndex = (AGCIndex + 7) % 8; //Decrement ACG Index
		Value =  (Value & 0x8FFFU) | (AGCIndex << 12); // Set bits 14:12 (AGC Index)
	}
	BK4819_WriteRegister(0x7E, Value);
}

void BK4819_SetToneFrequency(bool Tone2, uint16_t Tone)
{
	BK4819_WriteRegister(0x71 + Tone2, (Tone * 103U) / 10U);
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
	//BK4819_WriteRegister(0x30, 0x0000);
}

void BK4819_StartAudio(void)
{
	gpio_bits_set(GPIOA, BOARD_GPIOA_LED_GREEN);
	gRadioMode = RADIO_MODE_RX;
	OpenAudio(gMainVfo->bIsNarrow, gMainVfo->gModulationType);
///	if (gMainVfo->gModulationType == 0) {								// FM
	if(1){
		BK4819_WriteRegister(0x4D, 0xA080); /// Glitch threshold for Squelch
		BK4819_WriteRegister(0x4E, 0x6F7C); /// Squelch delay

		BK4819_EnableScramble(gMainVfo->Scramble);
		BK4819_EnableCompander(true);
		// BK4819_WriteRegister(0x43, 0x3028); // restore filter just in case -
											// this gets overwritten by sane defaults anyway.
		// Unset bit 4 of register 73 (Auto Frequency Control Disable)
		uint16_t reg_73 = BK4819_ReadRegister(0x73);
		BK4819_WriteRegister(0x73, reg_73 & ~0x10U); /// AFC settings
		if (gMainVfo->Scramble == 0) {
			BK4819_SetAFResponseCoefficients(false, true, gCalibration.RX_3000Hz_Coefficient);
		} else {
			BK4819_SetAFResponseCoefficients(false, true, 4);
		}
	}
	else {													// AM, SSB
		//BK4819_EnableScramble(0);															///WT: previously uncommented
		BK4819_EnableCompander(false);														///WT: check here AM audio level
		// Set bit 4 of register 73 (Auto Frequency Control Disable)
		uint16_t reg_73 = BK4819_ReadRegister(0x73);
		BK4819_WriteRegister(0x73, reg_73 | 0x10U);
		BK4819_WriteRegister(0x43, 0b0100000001011000); // Filter 6.25KHz					///WT: previously commented
		if (gMainVfo->gModulationType > 1) { // if SSB
			BK4819_WriteRegister(0x43, 0b0010000001011000); // Filter 6.25KHz
			BK4819_WriteRegister(0x37, 0b0001011000001111);
    		BK4819_WriteRegister(0x3D, 0b0010101101000101);
    		BK4819_WriteRegister(0x48, 0b0000001110101000);
		}
	}
	if (!gReceptionMode) {
		BK4819_EnableFFSK1200(true);
	}
	SPEAKER_TurnOn(SPEAKER_OWNER_RX);
}

void BK4819_SetAfGain(uint16_t Gain)
{

	if (gMainVfo->gModulationType) {			// AM, SSB
		//if ((Gain & 15) > 4) { ///WT: testing
		//	Gain -= 4;
		//}
		BK4819_WriteRegister(0x48, Gain);
	} else {									// FM
		switch (BK4819_ReadRegister(0x01)) {
		case 0:
		case 4:
			BK4819_WriteRegister(0x48, Gain);
			break;
		case 2: case 3: case 6: case 7:
			if ((Gain & 15) > 3) {
				Gain -= 3;
			}
			BK4819_WriteRegister(0x48, Gain);
			break;
		default:
			BK4819_WriteRegister(0x48, Gain);
			break;
		}
	}

	BK4819_WriteRegister(0x48,	//  0xB3A8);     // 1011 00 111010 1000
		(11u << 12) |     // ??? 0..15
		( 0u << 10) |     // AF Rx Gain-1
		(58u <<  4) |     // AF Rx Gain-2
		( 8u <<  0));     // AF DAC Gain (after Gain-1 and Gain-2)

}

bool BK4819_CheckSquelchLink(void)
{
	if (gSettings.Squelch && !gMonitorMode) {
		return ((BK4819_ReadRegister(0x0C) >> 1) & 1);
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
			OpenAudio(gMainVfo->bIsNarrow, gMainVfo->gModulationType);
			if (gMainVfo->gModulationType > 0) {
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
			BK4819_WriteRegister(0x07, 1135);
		} else {
			BK4819_WriteRegister(0x52, 0x823F);
		}
		DELAY_WaitMS(250);
	}
}

void BK4819_SetupPowerAmplifier(uint8_t Bias, uint32_t frequency)
{
	uint16_t Value;

#if 0
	Value = (Bias << 10) | 0x7F;
	if (Bias) {
		Value |= 0x80;
	}
	BK4819_WriteRegister(0x36, Value);
// Bias is 15 for LOW  33 for HIGH
// resulting Value is 0x3CFF for LOW 0x84FF for HIGH	
#else

	// REG_36 <15:8> 0 PA Bias output 0 ~ 3.2V
	//               255 = 3.2V
	//                 0 = 0V
	//
	// REG_36 <7>    0
	//               1 = Enable PA-CTL output
	//               0 = Disable (Output 0 V)
	//
	// REG_36 <5:3>  7 PA gain 1 tuning
	//               7 = max
	//               0 = min
	//
	// REG_36 <2:0>  7 PA gain 2 tuning
	//               7 = max
	//               0 = min
	//

	const uint8_t gain   = (frequency < 28000000) ? (1u << 3) | // PA Gain1 Tuning. 0(min)->7(max)  def. 1u
													(1u << 0) : // PA Gain2 Tuning. 0(min)->7(max)       0u
	
													(4u << 3) | // PA Gain1 Tuning. 0(min)->7(max)       4u
													(4u << 0);	// PA Gain2 Tuning. 0(min)->7(max)       2u

	Value = (Bias   << 8) |										// PA Biasoutput 0x00=0V 0xFF=3.2V
			(1      << 7) |										// 1=Enable PACTLoutput; 0=Disable(Output 0 V)
			(gain   << 0);
	BK4819_WriteRegister(0x36, Value);
	
#endif
IFDBG UART_printf(1,"Pwr Freq: %d\r\n",frequency);
IFDBG UART_printf(1,"Pwr Bias: %d\r\n",Bias);
IFDBG UART_printf(1,"Pwr val: %X\r\n",Value);

}

void BK4819_EnableRfTxDeviation(void)
{
	uint16_t Deviation;

	//Deviation = gMainVfo->bIsNarrow ? gFrequencyBandInfo.TxDeviationNarrow : gFrequencyBandInfo.TxDeviationWide;// 0x34EC
	// 0x4D0 -  12 kHz
	// 0x6FF -  25 kHz
	// 0x7FF -  40 kHz
	// 0x840 -  55 kHz
	// 0x940 - 200 kHz
	Deviation = gMainVfo->bIsNarrow ? 0x440 : 0x540;
	if (gMainVfo->Scramble) {
		Deviation -= 200;
	}
IFDBG UART_printf(1,"Dev: 0x%X\r\n",Deviation);

	Deviation |=	(   1u << 12);     // Enable
	
	//BK4819_WriteRegister(0x40, 0x14D0); /// default on DS
	BK4819_WriteRegister(0x40, Deviation);
	

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
void BK4819_set_rf_frequency(const uint32_t frequency, const bool trigger_update)
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
