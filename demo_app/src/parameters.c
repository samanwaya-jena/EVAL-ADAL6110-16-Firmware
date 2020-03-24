/*
 * parameters.c
 *
 *  Created on: Feb 22, 2020
 *      Author: e.turenne
 */

#include <stdint.h>
#include <string.h>
#include "parameters.h"
#include "parameters_default_values.h"
#include "error_handler.h"
#include "Flash/flash_params.h"

#include "adal6110_16.h"
#include "Communications/USB_cmd.h"
#include "Communications/cld_bf70x_bulk_lib.h"




uint16_t LiDARParameters[number_of_param]; // value of all functional parameters
uint8_t  LiDARParamDir[number_of_param];   // 0 = read only, 1 = read/write

uint16_t ADAL_currentValues[][2] =
{

    {Control0Address , 0x1F80},
    {Control1Address , 0x8040},
    //{DataControlAddress , },
    {DelayBetweenFlashesAddress , 0x4000},
    {ChannelEnableAddress , 0xFFFF},
    {DataAcqMode , 0x0001},
    {TriggerOutAddress , 0x1021},
    {CH0ControlReg0Address , 0x2E1F},
    {CH0ControlReg1Address , 0x0180},
    {CH0ControlReg2Address , 0x00FF},
    {CH1ControlReg0Address , 0x2E1F},
    {CH1ControlReg1Address , 0x0180},
    {CH1ControlReg2Address , 0x00FF},
    {CH2ControlReg0Address , 0x2E1F},
    {CH2ControlReg1Address , 0x0180},
    {CH2ControlReg2Address , 0x00FF},
    {CH3ControlReg0Address , 0x2E1F},
    {CH3ControlReg1Address , 0x0180},
    {CH3ControlReg2Address , 0x00FF},
    {CH4ControlReg0Address , 0x2E1F},
    {CH4ControlReg1Address , 0x0180},
    {CH4ControlReg2Address , 0x00FF},
    {CH5ControlReg0Address , 0x2E1F},
    {CH5ControlReg1Address , 0x0180},
    {CH5ControlReg2Address , 0x00FF},
    {CH6ControlReg0Address , 0x2E1F},
    {CH6ControlReg1Address , 0x0180},
    {CH6ControlReg2Address , 0x00FF},
    {CH7ControlReg0Address , 0x2E1F},
    {CH7ControlReg1Address , 0x0180},
    {CH7ControlReg2Address , 0x00FF},
    {CH8ControlReg0Address , 0x2E1F},
    {CH8ControlReg1Address , 0x0180},
    {CH8ControlReg2Address , 0x00FF},
    {CH9ControlReg0Address , 0x2E1F},
    {CH9ControlReg1Address , 0x0180},
    {CH9ControlReg2Address , 0x00FF},
    {CH10ControlReg0Address , 0x2E1F},
    {CH10ControlReg1Address , 0x0180},
    {CH10ControlReg2Address , 0x00FF},
    {CH11ControlReg0Address , 0x2E1F},
    {CH11ControlReg1Address , 0x0180},
    {CH11ControlReg2Address , 0x00FF},
    {CH12ControlReg0Address , 0x2E1F},
    {CH12ControlReg1Address , 0x0180},
    {CH12ControlReg2Address , 0x00FF},
    {CH13ControlReg0Address , 0x2E1F},
    {CH13ControlReg1Address , 0x0180},
    {CH13ControlReg2Address , 0x00FF},
    {CH14ControlReg0Address , 0x2E1F},
    {CH14ControlReg1Address , 0x0180},
    {CH14ControlReg2Address , 0x00FF},
    {CH15ControlReg0Address , 0x2E1F},
    {CH15ControlReg1Address , 0x0180},
    {CH15ControlReg2Address , 0x00FF},
    //{GPIOCFG , },
    //{SPICFG , },
    //{THSMAX , },
    //{THSMIN , },
    {AGCDCBCTRL , 0x0104},
    {AGCEN , 0x0000},
    {DCEN , 0xFFFF},
    {AGCDCBPID0, 0x0032},
    {AGCDCBPID1 ,0x03e8},
    {FRAMEDELAY , 0x8000},
    //{STARTADDRPOINTER , },
    //{SRAM_READY , },
    {LFSRSEEDL , 0x9190},
    {LFSRSEEDH , 0x0001},
    //{SRAM_DATA , }*/
	{Control0Address , 0x1F81},
	{Control0Address , 0x1F82},
};

void param_ResetFactoryDefault()
{
	uint16_t serNum = LiDARParameters[param_serialNumber];
	uint16_t date   = LiDARParameters[param_manufDate];

	memcpy((char*)LiDARParameters,(char*)param_default,sizeof(LiDARParameters));

	LiDARParameters[param_serialNumber] = serNum; // keeps the serial number;
	LiDARParameters[param_manufDate] = date; // and date;

	memcpy((char*)ADAL_currentValues,(char*)ADAL_DefaultValues,sizeof(ADAL_DefaultValues));

	int num = sizeof(ADAL_currentValues) / sizeof(ADAL_currentValues[0]);
	int i;
	for (i=0; i<num; i++)
	{
		ADAL_WriteParamToSPI(ADAL_currentValues[i][0], ADAL_currentValues[i][1]);
	}
}

void param_InitValues(void)
{
	int i,num;

	memcpy((char*)LiDARParamDir,(char*)param_dir_values,sizeof(LiDARParamDir));

	param_LoadConfig();
}


void param_LoadConfig(void)
{
	int num,i;

	num = number_of_param/2; // saved in 32bits instead of 16
	if( !Flash_LoadConfig(0, (uint16_t*)LiDARParameters, &num))
	{
		if (num!=number_of_param/2)
			SetError(error_SW_flash);
	}else
		SetError(error_SW_flash);

	num = sizeof(ADAL_currentValues) / sizeof(ADAL_currentValues[0]); // number of 16bits pair of values in array
	if(!Flash_LoadConfig(1, (uint16_t*)ADAL_currentValues, &num))
	{
		if (num == sizeof(ADAL_currentValues) / sizeof(ADAL_currentValues[0]))
		{
			for (i = 0; i < num; i++)
				ADAL_WriteParamToSPI(ADAL_currentValues[i][0], ADAL_currentValues[i][1]);
		}else
			SetError(error_SW_flash);
	}else
		SetError(error_SW_flash);

	if (IsErrorSet(error_SW_flash))
		param_ResetFactoryDefault();

}

void param_SaveConfig(void)
{
	int num,i;

	num = sizeof(ADAL_currentValues) / sizeof(ADAL_currentValues[0]);
	for (i = 0; i < num; i++)
	{
		ADAL_ReadParamFromSPI(ADAL_currentValues[i][0], &ADAL_currentValues[i][1]);
	}
	Flash_SaveConfig(1, (uint16_t*)ADAL_currentValues, num);
	Flash_SaveConfig(0, (uint16_t*)LiDARParameters, number_of_param/2);
}


//
// Read/Write FIFO
//

volatile int iReadWriteFifoHead = 0;
volatile int iReadWriteFifoTail = 0;

#define READWRITEFIFO_SIZE 8
#define READWRITEFIFO_MASK (READWRITEFIFO_SIZE - 1)
uint32_t readWriteFifo[READWRITEFIFO_SIZE];


int param_ReadFifoPush(uint16_t _startAddress)
{
	int nextHead = (iReadWriteFifoHead + 1) & READWRITEFIFO_MASK;

	if (nextHead != iReadWriteFifoTail)
	{
		readWriteFifo[iReadWriteFifoHead] = ((uint32_t)(_startAddress & ~RW_WRITE_MASK)) << 16;
		iReadWriteFifoHead = nextHead;

		return 1;
	}

	return 0;
}

int param_WriteFifoPush(uint16_t _startAddress, uint16_t data)
{
	int nextHead = (iReadWriteFifoHead + 1) & READWRITEFIFO_MASK;

	if (nextHead != iReadWriteFifoTail)
	{
		readWriteFifo[iReadWriteFifoHead] = ((uint32_t)(RW_WRITE_MASK | (_startAddress & ~RW_WRITE_MASK)) << 16) |
				data;
		iReadWriteFifoHead = nextHead;

		return 1;
	}

	return 0;
}

int param_ProcessReadWriteFifo(void)
{
	uint32_t op = readWriteFifo[iReadWriteFifoTail];
	uint16_t addrPlusRW = op >> 16;
	uint16_t addr = addrPlusRW & ~RW_WRITE_MASK;
	uint16_t data = op & 0xFFFF;
	uint8_t type = (addrPlusRW & RW_INTERNAL_MASK)?cmdParam_SensorRegister:cmdParam_ADCRegister;
	bool bWriteOp = ((addrPlusRW & RW_WRITE_MASK) != 0);

	if (iReadWriteFifoHead != iReadWriteFifoTail)
	{
		if (op & 0x80000000) // write param
		{
			switch (addr)
			{
			case 0x3FFE:
			case 0x7FFE:
				param_SaveConfig();
				cld_console(CLD_CONSOLE_PURPLE,CLD_CONSOLE_BLACK,"Parameters saved to flash\r\n");
				break;
			case 0x3FFF:
			case 0x7FFF:
				param_ResetFactoryDefault();
				cld_console(CLD_CONSOLE_PURPLE,CLD_CONSOLE_BLACK,"All parameter reseted to default values\r\n");
				//ADAL_Reset();
				break;
			case RW_INTERNAL_MASK+param_deviceID://0x4000: // device ID
				ADAL_Reset();
				break;
			default:
				if (type == cmdParam_SensorRegister & LiDARParamDir[addr&~RW_INTERNAL_MASK]!=READONLY)
				{
					LiDARParameters[addr&~RW_INTERNAL_MASK] = data;
					cld_console(CLD_CONSOLE_PURPLE,CLD_CONSOLE_BLACK,"software parameter 0x%04X set to 0x%04X\r\n",addr&~RW_INTERNAL_MASK,data);
				}
				else
				{
					//figure how to keep local copy
					ADAL_WriteParamToSPI(addr, data);
					cld_console(CLD_CONSOLE_PURPLE,CLD_CONSOLE_BLACK,"ADAL6110 internal register 0x%04X set to 0x%04X\r\n",addr,data);
				}
				break;
			}
		}
		else // read
		{
			if(addr & RW_INTERNAL_MASK)
			{
				data = LiDARParameters[addr&~RW_INTERNAL_MASK];
				cld_console(CLD_CONSOLE_PURPLE,CLD_CONSOLE_BLACK,"software parameter 0x%04X read with value 0x%04X\r\n",addr&~RW_INTERNAL_MASK,data);
			}else
			{
				ADAL_ReadParamFromSPI(addr, &data);
				cld_console(CLD_CONSOLE_PURPLE,CLD_CONSOLE_BLACK,"ADAL6110 internal register 0x%04X read with value 0x%04X\r\n",addr,data);
			}
			USB_pushParameter(addr,data,type);
		}
		iReadWriteFifoTail = (iReadWriteFifoTail + 1) & READWRITEFIFO_MASK;
	}

	return 0;
}
