/*
 * parameters.c
 *
 *  Created on: Feb 22, 2020
 *      Author: e.turenne
 */

#include <stdint.h>
#include "parameters.h"
#include "Lidar_adal6110_16.h"
#include "Communications/USB_cmd.h"
#include "Flash/flash_params.h"


//
// Read/Write FIFO
//

volatile int iReadWriteFifoHead = 0;
volatile int iReadWriteFifoTail = 0;

#define READWRITEFIFO_SIZE 8
#define READWRITEFIFO_MASK (READWRITEFIFO_SIZE - 1)
uint32_t readWriteFifo[READWRITEFIFO_SIZE];


int Lidar_ReadFifoPush(uint16_t _startAddress)
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

int Lidar_WriteFifoPush(uint16_t _startAddress, uint16_t data)
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

int ProcessReadWriteFifo(void)
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
				SaveConfigToFlash(0);
				break;
			case 0x3FFF:
				//Flash_ResetToFactoryDefault(0);
				LoadDefaultConfig(0);
				//Lidar_Reset();
				break;
			case 0x4000:
				Lidar_Reset();
				break;
			case 0x4001:
				//Lidar_Reset();
				break;
			case 0x7FFE:
				SaveConfigToFlash(1);
				break;
			case 0x7FFF:
				Flash_ResetToFactoryDefault(1);
				//Lidar_Reset();
				break;
			default:
				WriteParamToSPI(addr, data);
				break;
			}
		}
		else
		{
			switch (addr)
			{
			case 0x4000:
				break;
			case 0x4001:
				break;
			default:
				ReadParamFromSPI(addr, &data);
				break;
			}
			USB_pushParameter(addr,data,type);
		}
	}
	iReadWriteFifoTail = (iReadWriteFifoTail + 1) & READWRITEFIFO_MASK;

	return 0;
}


