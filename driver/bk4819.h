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

#ifndef DRIVER_BK4819_H
#define DRIVER_BK4819_H

#include <stdbool.h>
#include <stdint.h>

#define BK4819_REG_30_SHIFT_ENABLE_VCO_CALIB    15
#define BK4819_REG_30_ENABLE_VCO_CALIB 			( 1u << BK4819_REG_30_SHIFT_ENABLE_VCO_CALIB)

//-------------------------------------------------- READ-ONLY REGISTERS 00..0F

typedef union {						// 
	struct {
		uint16_t	Int			: 1;
		uint16_t	SQL			: 1;
		uint16_t	VOX			: 1;
		uint16_t	_res3		: 7;
		uint16_t	CTC			: 2;
		uint16_t	CTCSSps		: 2;
		uint16_t	CDCSS		: 2;
	};
	uint16_t val;
}REGx0C;

//------------------------------------------------- READ-WRITE REGISTERS 10..7F

typedef union {						// RX AGC Gain Table[0]
	struct {
		uint16_t	PGA_Gain	: 3;
		uint16_t	MIX_Gain	: 2;
		uint16_t	LNA_Gain	: 3;
		uint16_t	LNA_GainS	: 2;
		uint16_t	_res10		: 6;
	};
	uint16_t val;
}REGx10;
typedef union {						// RX AGC Gain Table[1]
	struct {
		uint16_t	PGA_Gain	: 3;
		uint16_t	MIX_Gain	: 2;
		uint16_t	LNA_Gain	: 3;
		uint16_t	LNA_GainS	: 2;
		uint16_t	_res10		: 6;
	};
	uint16_t val;
}REGx11;
typedef union {						// RX AGC Gain Table[2]
	struct {
		uint16_t	PGA_Gain	: 3;
		uint16_t	MIX_Gain	: 2;
		uint16_t	LNA_Gain	: 3;
		uint16_t	LNA_GainS	: 2;
		uint16_t	_res10		: 6;
	};
	uint16_t val;
}REGx12;
typedef union {						// RX AGC Gain Table[3]
	struct {
		uint16_t	PGA_Gain	: 3;
		uint16_t	MIX_Gain	: 2;
		uint16_t	LNA_Gain	: 3;
		uint16_t	LNA_GainS	: 2;
		uint16_t	_res10		: 6;
	};
	uint16_t val;
}REGx13;
typedef union {						// RX AGC Gain Table[-1]
	struct {
		uint16_t	PGA_Gain	: 3;
		uint16_t	MIX_Gain	: 2;
		uint16_t	LNA_Gain	: 3;
		uint16_t	LNA_GainS	: 2;
		uint16_t	_res10		: 6;
	};
	uint16_t val;
}REGx14;

typedef union {						// AF HPF
	struct {
		uint16_t	AFTX_Pree	: 1;
		uint16_t	AFTX_LPF1	: 1;
		uint16_t	AFTX_HPF300	: 1;
		uint16_t	_res3		: 5;
		uint16_t	AFRX_Dee	: 1;
		uint16_t	AFRX_LPF3K	: 1;
		uint16_t	AFRX_HPF300	: 1;
		uint16_t	_res11		: 3;
		uint16_t	AFRX_DCCan	: 1;
		uint16_t	AFRX_CTCan	: 1;
	};
	uint16_t val;
}REGx2B;

typedef union {						// GPIO
	struct {
		uint16_t	GPIO0_Val	: 1;
		uint16_t	GPIO1_Val	: 1;
		uint16_t	GPIO2_Val	: 1;
		uint16_t	GPIO3_Val	: 1;
		uint16_t	GPIO4_Val	: 1;
		uint16_t	GPIO5_Val	: 1;
		uint16_t	GPIO6_Val	: 1;
		uint16_t	_res7		: 1;
		uint16_t	GPIO0_Dis	: 1;
		uint16_t	GPIO1_Dis	: 1;
		uint16_t	GPIO2_Dis	: 1;
		uint16_t	GPIO3_Dis	: 1;
		uint16_t	GPIO4_Dis	: 1;
		uint16_t	GPIO5_Dis	: 1;
		uint16_t	GPIO6_Dis	: 1;
		uint16_t	_res15		: 1;
	};
	uint16_t val;
}REGx33;

typedef union {						// PA Control
	struct {
		uint16_t	PA_Gain2	: 3;
		uint16_t	PA_Gain1	: 3;
		uint16_t	_res6		: 1;
		uint16_t	PA_Enable	: 1;
		uint16_t	PA_Bias		: 8;
	};
	uint16_t val;
}REGx36;

typedef union {						// RF TX Deviation
	struct {
		uint16_t	TX_Deviation		:12;
		uint16_t	TX_Deviation_Enable	: 1;
		uint16_t	_res13				: 3;
	};
	uint16_t val;
}REGx40;

typedef union {						// Filter Bandwidth
	struct {
		uint16_t	_res0		: 2;
		uint16_t	GainAfter	: 1;		// Gain after FM demodulation
		uint16_t	_res3		: 1;
		uint16_t	BW_Mode		: 2;		// Bandwidth mode selection 00:12.5k 01:6.25k 10:25k/20k
		uint16_t	AFTX_LPF2	: 3;		// AF TX LPF2 filter bandwidth selection
		uint16_t	BWdW		: 3;		// RF filter bandwidth when the signal is weak
		uint16_t	BWd			: 3;		// RF filter bandwidth
		uint16_t	_res15		: 1;
	};
	uint16_t val;
}REGx43;

typedef union {						// AF Output
	struct {
		uint16_t	AF_Byp		: 1;
		uint16_t	_res1		: 7;
		uint16_t	AF_Out		: 4;
		uint16_t	_res12		: 1;
		uint16_t	AF_Inv		: 1;
		uint16_t	_res14		: 2;		
	};
	uint16_t val;
}REGx47;
		
typedef union {
	struct {
		uint16_t	AF_DAC_Gain			: 4;
		uint16_t 	AF_RX_Gain2			: 6;
		uint16_t	AF_RX_Gain1			: 2;
		uint16_t	_res12				: 4;	
	};
	uint16_t val;
}REGx48;

typedef union {						// AGC Control
	struct {
		uint16_t	AGC_LowThres	: 7;
		uint16_t	AGC_HighThres	: 7;
		uint16_t	Sel_Auto		: 2;
	};
	uint16_t val;
}REGx49;

typedef union  {					// RSSI
	struct {
		uint16_t	RSSI			: 9;
		uint16_t	_res9			: 7;
	};
	uint16_t val;
}REGx67;

typedef union  {					// AGC Control
	struct {
		uint16_t	RX_IF_BW		: 3;
		uint16_t	TX_MIC_BW		: 3;
		uint16_t	_res6			: 6;
		 int16_t	AGC_Index		: 3;
		uint16_t	AGC_Mode		: 1;
	};
	uint16_t val;
}REGx7E;


enum BK4819_AF_Type_t {
	BK4819_AF_MUTE = 0U,	/// Mute
	BK4819_AF_NORMAL = 1U,	/// Normal AF Out
	BK4819_AF_RXBEEP = 2U,	/// Tone Out for Rx (Should enable Tone1 first)
	BK4819_AF_TXBEEP = 3U,	/// Beep Out for Tx (Should enable Tone1 first and set REG_03[9]=1 to enable AF)
	BK4819_AF_4U  = 4U,		/// undocumented
	BK4819_AF_5U  = 5U,		/// undocumented
	BK4819_AF_CTCO = 6U,	/// CTCSS/CDCSS Out for Rx Test	
	BK4819_AF_7U   = 7U,	/// undocumented
	BK4819_AF_FSKO = 8U,	/// FSK Out for Rx Test
};

typedef enum BK4819_AF_Type_t BK4819_AF_Type_t;

void OpenAudio(uint8_t gModulationType);
uint16_t BK4819_ReadRegister(uint8_t Reg);
uint16_t BK4819_GetRSSI();
int16_t BK4819_GetRSSI_DBM(void);
void BK4819_WriteRegister(uint8_t Reg, uint16_t Data);

void BK4819_Init(void);
void BK4819_SetAFResponseCoefficients(bool bTx, bool bLowPass, uint8_t Index);
void BK4819_EnableRX(void);
void BK4819_SetAF(BK4819_AF_Type_t Type);
void BK4819_SetFrequency(uint32_t Frequency);
void BK4819_SetSquelchMode(void);
void BK4819_SetSquelchGlitch(uint8_t BWidth);
void BK4819_SetSquelchNoise(uint8_t BWidth);
void BK4819_SetSquelchRSSI(uint8_t BWidth);
/*void BK4819_ToggleAGCMode(void);*/
void BK4819_SetAGCMode(uint8_t lvl);
void BK4819_SetAGCLevel(uint8_t lvl);
void BK4819_SetDefaultGainTables(void);
void BK4819_SetOptGainTables(void);
void BK4819_SetFilterBandwidth(uint8_t BWidth, bool weak);
void BK4819_SelectRFPath(uint32_t Frequency);
void BK4819_EnableScramble(uint8_t Scramble);
void BK4819_EnableCompander(bool bEnable);
void BK4819_EnableVox(bool bEnable);
void BK4819_SetToneFrequency(bool Tone, uint16_t freq);
void BK4819_EnableFFSK1200(bool bEnable);
void BK4819_ResetFSK(void);
void BK4819_StartAudio(void);
void BK4819_SetAfGain(uint16_t Gain);
void BK4819_InitDTMF(void);
bool BK4819_CheckSquelchStat(void);
void BK4819_EnableTone1(bool bEnable);
void BK4819_GenTail(bool bIsNarrow);
void BK4819_SetupPowerAmplifier(uint8_t level, uint32_t frequency);
//void BK4819_EnableRFTXDeviation(void);
void BK4819_SetRFTXDeviation(uint8_t level);
void BK4819_SetMicSensitivityTuning(void);
void BK4819_EnableTX(bool bUseMic);
void BK4819_StartFrequencyScan(void);
void BK4819_StopFrequencyScan(void);
void BK4819_DisableAutoCssBW(void);
#ifdef ENABLE_SPECTRUM
void BK4819_SetRFFrequency(const uint32_t frequency, const bool trigger_update);
#endif

#endif

