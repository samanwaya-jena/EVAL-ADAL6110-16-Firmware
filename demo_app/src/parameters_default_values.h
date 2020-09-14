/*
 * parameters_default_values.h
 *
 *  Created on: Feb 23, 2020
 *      Author: e.turenne
 */
#include <stdint.h>
#include "parameters.h"
#include "adal6110_16.h"

#ifndef PARAMETERS_DEFAULT_VALUES_H_
#define PARAMETERS_DEFAULT_VALUES_H_

#define READONLY  0
#define READWRITE 1

#define DEVICEID   0x0A03  // PI's device ID 0-9 are internal, A is EVAL-ADAL6110-16, configuration 3 (production hardware and configuration)
#define DATENULL   0x0000  //0x1402 or 5122 for 2020-02
#define SERIALNULL 0x0000


uint16_t param_default [number_of_param]=
{DEVICEID, DATENULL , SERIALNULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       // 0x00 sensor information
0, 1, 0, 100, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                      // 0x10 Enable switches
1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                        // 0x20 algo selection and general params
10, 5, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                      // 0x30 detection algo param
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                        // 0x40 tracking algo param
14,  12,  10,  8,  6,  4,  2,  0,  1,  3,  5,  7,  9,  11,  13,  15,   // 0x50 channel mapping
0, 0, 0, 0, 1000                                                       // 0x60 communication
};

uint8_t param_dir_values[number_of_param]=
{READONLY, READONLY, READONLY, READWRITE, READONLY, READONLY, READONLY, READONLY, // 0x00 sensor
 READONLY, READONLY, READONLY, READONLY, READONLY, READONLY, READONLY, READONLY,
 READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,// 0x10 Enable switches
 READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,
 READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,// 0x20 algo selection and general params
 READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,
 READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,// 0x30 detection algo param
 READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,
 READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,// 0x40 tracking algo param
 READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,
 READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,// 0x50 channel mapping
 READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,READWRITE,
 READWRITE,READWRITE,READWRITE,READWRITE,READWRITE                               // 0x60 communication
};

uint16_t ADAL_DefaultValues[][2] =
{
    {Control0Address , 0x1000},  //32 flash gain
    {Control1Address , 0x8040},
    //{DataControlAddress , },
    {DelayBetweenFlashesAddress , 0x5ED8},
    {DataAcqMode , 0x0001},
    {TriggerOutAddress , 0x1011},
    {CH0ControlReg0Address , 0x3C3F}, // datasheet p.26 010 0|1111|10 0 1|1111 = 0x4F9F
    {CH0ControlReg1Address , 0x0380},
    {CH0ControlReg2Address , 0x00FF},
    {CH1ControlReg0Address , 0x3C3F},
    {CH1ControlReg1Address , 0x0380},
    {CH1ControlReg2Address , 0x00FF},
    {CH2ControlReg0Address , 0x3C3F},
    {CH2ControlReg1Address , 0x0380},
    {CH2ControlReg2Address , 0x00FF},
    {CH3ControlReg0Address , 0x3C3F},
    {CH3ControlReg1Address , 0x0380},
    {CH3ControlReg2Address , 0x00FF},
    {CH4ControlReg0Address , 0x3C3F},
    {CH4ControlReg1Address , 0x0380},
    {CH4ControlReg2Address , 0x00FF},
    {CH5ControlReg0Address , 0x3C3F},
    {CH5ControlReg1Address , 0x0380},
    {CH5ControlReg2Address , 0x00FF},
    {CH6ControlReg0Address , 0x3C3F},
    {CH6ControlReg1Address , 0x0380},
    {CH6ControlReg2Address , 0x00FF},
    {CH7ControlReg0Address , 0x3C3F},
    {CH7ControlReg1Address , 0x0380},
    {CH7ControlReg2Address , 0x00FF},
    {CH8ControlReg0Address , 0x3C3F},
    {CH8ControlReg1Address , 0x0380},
    {CH8ControlReg2Address , 0x00FF},
    {CH9ControlReg0Address , 0x3C3F},
    {CH9ControlReg1Address , 0x0380},
    {CH9ControlReg2Address , 0x00FF},
    {CH10ControlReg0Address , 0x3C3F},
    {CH10ControlReg1Address , 0x0380},
    {CH10ControlReg2Address , 0x00FF},
    {CH11ControlReg0Address , 0x3C3F},
    {CH11ControlReg1Address , 0x0380},
    {CH11ControlReg2Address , 0x00FF},
    {CH12ControlReg0Address , 0x3C3F},
    {CH12ControlReg1Address , 0x0380},
    {CH12ControlReg2Address , 0x00FF},
    {CH13ControlReg0Address , 0x3C3F},
    {CH13ControlReg1Address , 0x0380},
    {CH13ControlReg2Address , 0x00FF},
    {CH14ControlReg0Address , 0x3C3F},
    {CH14ControlReg1Address , 0x0380},
    {CH14ControlReg2Address , 0x00FF},
    {CH15ControlReg0Address , 0x3C3F},
    {CH15ControlReg1Address , 0x0380},
    {CH15ControlReg2Address , 0x00FF},
    //{GPIOCFG , },
    //{SPICFG , },
    //{THSMAX , },
    //{THSMIN , },
    {AGCDCBCTRL , 0x0014},

	{Control3Adress, 0},
    {ChannelEnableAddress , 0xFFFF},
    {AGCEN , 0x0000},
    {DCEN , 0xFFFF},
	{Control3Adress, 1},
    //{AGCDCBPID0, },
    //{AGCDCBPID1 , },
    {FRAMEDELAY , 0xFFF8},
    //{STARTADDRPOINTER , },
    //{SRAM_READY , },
    {LFSRSEEDL , 0x9190},
    {LFSRSEEDH , 0x0001}//,
    //{SRAM_DATA , }*/
	//{Control0Address , 0x0F81},
	//{Control0Address , 0x0F82}
};

#endif /* PARAMETERS_DEFAULT_VALUES_H_ */
