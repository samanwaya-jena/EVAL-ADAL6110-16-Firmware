/*
 * (C) Copyright 2012 - Analog Devices, Inc. All Rights Reserved.
 *
 * This software is proprietary and confidential.  By using this software
 * you agree to the terms of the associated Analog Devices License Agreement.
 *
 * Project Name:  	Power_On_Self_Test
 *
 * Hardware:		ADSP-BF609 EZ-Board
 *
 * Description:	This file tests the SPI flash on the EZ-Board.
 */

/*******************************************************************
*  include files
*******************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "common/flash_errors.h"
#include "common/flash.h"

//#include "common/spi.h"
//#include "post_debug.h"
//#include "timer_isr.h"

#include "calc_crc.h"


#if defined(__DEBUG_FILE__)
#include <string.h>
extern FILE *pDebugFile;				/* debug file when directing output to a file */
#endif


#define MAN_CODE		0xEF		/* winbond */
#define DEV_CODE		0x15		/* W25Q32BV (16Mbit SPI flash) */
#define NUM_SECTORS		64			/* sectors */

#define FLASH_MAGIC     0x1C3B00DA

#define FLASH_PARAM0_ADDR (40 * 64*1024)
#define FLASH_PARAM1_ADDR (41 * 64*1024)
#define FLASH_PARAM_SIZE (64*1024)

extern struct flash_info is25lp032d_info;


int Flash_Init(void)
{
	flash_info = &is25lp032d_info;

	return 0;
}

typedef struct
{
	uint32_t magic;
	uint32_t numParams;
	uint32_t params[1]; // N params
	//TODO REMOVE CRC
//	uint32_t crc;
} tFlashParams;

int Flash_LoadConfig(int idx, uint16_t * pParams, int * pNum)
{
	int Result = 0;							/* result */
	int success = 1;
	tFlashParams * pFlashParams = NULL;

	uint32_t maxNum = *pNum;

	*pNum = 0;

	flash_open(flash_info);

	/* calculate offset based on sector */
	unsigned long ulOffset = (idx != 0) ? FLASH_PARAM1_ADDR : FLASH_PARAM0_ADDR;

	uint32_t magic = 0;
	uint32_t numParams = 0;

	Result = flash_read(flash_info, ulOffset, (uint8_t*) &magic, sizeof(uint32_t));
	ulOffset += sizeof(uint32_t);

	if (magic != FLASH_MAGIC)
		goto FAIL;

	Result = flash_read(flash_info, ulOffset, (uint8_t*) &numParams, sizeof(uint32_t));
	ulOffset += sizeof(uint32_t);

	if (numParams > maxNum)
		goto FAIL;

	pFlashParams = (tFlashParams *) malloc((numParams + 2) * sizeof(uint32_t));

	if (pFlashParams)
	{
		pFlashParams->magic = magic;
		pFlashParams->numParams = numParams;

		uint32_t ExpectedCrcVal = 0;

		Result = flash_read(flash_info, ulOffset, (uint8_t*) pFlashParams->params, numParams * sizeof(uint32_t));
		ulOffset += numParams * sizeof(uint32_t);

		Result = flash_read(flash_info, ulOffset, (uint8_t*) &ExpectedCrcVal, sizeof(ExpectedCrcVal));
		ulOffset += sizeof(ExpectedCrcVal);

		uint32_t CalculatedCrcVal = CalcCRC((uint32_t*) pFlashParams, numParams + 2);

		if (CalculatedCrcVal == ExpectedCrcVal)
		{
			memcpy(pParams, pFlashParams->params, numParams * sizeof(uint32_t));
			*pNum = numParams;
		}

		free(pFlashParams);
		success = 0;
	}

FAIL:
	flash_close(flash_info);

	return success;
}

int Flash_SaveConfig(int idx, uint16_t * pParams, int num)
{
	int Result = 0;							/* result */
	tFlashParams * pFlashParams = NULL;
	int sizeFlashParams = (num + 2) * sizeof(uint32_t);

	flash_open(flash_info);

	/* calculate offset based on sector */
	unsigned long ulOffset = (idx != 0) ? FLASH_PARAM1_ADDR : FLASH_PARAM0_ADDR;

	pFlashParams = (tFlashParams *) malloc(sizeFlashParams);

	if (pFlashParams)
	{
		pFlashParams->magic = FLASH_MAGIC;
		pFlashParams->numParams = num;

		memcpy(pFlashParams->params, pParams, num * sizeof(uint32_t));

		uint32_t CalculatedCrcVal = CalcCRC((uint32_t*) pFlashParams, num + 2);

		/* erase the sector */
		Result = flash_erase(flash_info, ulOffset, FLASH_PARAM_SIZE);

		/* write a value to the flash */
		Result = flash_program(flash_info, ulOffset, (uint8_t*) pFlashParams, sizeFlashParams);
		ulOffset += sizeFlashParams;

		// Write the CRC
		Result = flash_program(flash_info, ulOffset, (uint8_t*) &CalculatedCrcVal, sizeof(uint32_t));
		ulOffset += sizeof(uint32_t);

		free(pFlashParams);
	}

	flash_close(flash_info);

	return 0;
}

int Flash_ResetToFactoryDefault(int idx)
{
	int Result = 0;							/* result */

	flash_open(flash_info);

	/* calculate offset based on sector */
	unsigned long ulOffset = (idx != 0) ? FLASH_PARAM1_ADDR : FLASH_PARAM0_ADDR;

	/* erase the sector */
	Result = flash_erase(flash_info, ulOffset, FLASH_PARAM_SIZE);

	flash_close(flash_info);

	return 0;
}

