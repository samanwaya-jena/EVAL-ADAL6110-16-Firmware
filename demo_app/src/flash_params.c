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
#include "SoftConfig_BF707.h"
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


static SWITCH_CONFIG SwitchConfig0[] =
{
  { 0x12u, 0xF8u },
  { 0x13u, 0xFFu },
  { 0x0u, 0x40u },   /* Set IODIRA direction (bit 6 input, all others output) */
  { 0x1u, 0x03u },   /* Set IODIRB direction (bit 0, 1 input, all others output) */
};
static SWITCH_CONFIG SwitchConfig1[] =
{
  { 0x12u, 0x81u },
  { 0x13u, 0x03u },
  { 0x0u, 0x00u },    /* Set IODIRA direction (all output) */
  { 0x1u, 0x80u },    /* Set IODIRB direction (bit 7 input, all others output) */
};

static SOFT_SWITCH SoftSwitch[] =
{
  {    0u,    0x21u,    sizeof(SwitchConfig0)/sizeof(SWITCH_CONFIG),    SwitchConfig0  },
  {    0u,    0x22u,    sizeof(SwitchConfig1)/sizeof(SWITCH_CONFIG),    SwitchConfig1  }
};


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
//	uint32_t crc;
} tFlashParams;

int Flash_LoadConfig(int idx, uint16_t * pParams, int * pNum)
{
	int Result = 0;							/* result */
	tFlashParams * pFlashParams = NULL;

	uint32_t maxNum = *pNum;

	*pNum = 0;

#ifdef EZ_KIT
	ConfigSoftSwitches(SS_SPI, sizeof(SoftSwitch)/sizeof(SoftSwitch[0]), SoftSwitch);
#endif
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
	}

FAIL:
	flash_close(flash_info);
#ifdef EZ_KIT
	ConfigSoftSwitches(SS_DEFAULT, 0, NULL);
#endif
	return 0;
}

int Flash_SaveConfig(int idx, uint16_t * pParams, int num)
{
	int Result = 0;							/* result */
	tFlashParams * pFlashParams = NULL;
	int sizeFlashParams = (num + 2) * sizeof(uint32_t);
#ifdef EZ_KIT
	ConfigSoftSwitches(SS_SPI, sizeof(SoftSwitch)/sizeof(SoftSwitch[0]), SoftSwitch);
#endif
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
#ifdef EZ_KIT
	ConfigSoftSwitches(SS_DEFAULT, 0, NULL);
#endif
	return 0;
}

int Flash_ResetToFactoryDefault(int idx)
{
	int Result = 0;							/* result */
#ifdef EZ_KIT
	ConfigSoftSwitches(SS_SPI, sizeof(SoftSwitch)/sizeof(SoftSwitch[0]), SoftSwitch);
#endif
	flash_open(flash_info);

	/* calculate offset based on sector */
	unsigned long ulOffset = (idx != 0) ? FLASH_PARAM1_ADDR : FLASH_PARAM0_ADDR;

	/* erase the sector */
	Result = flash_erase(flash_info, ulOffset, FLASH_PARAM_SIZE);

	flash_close(flash_info);
#ifdef EZ_KIT
	ConfigSoftSwitches(SS_DEFAULT, 0, NULL);
#endif

	return 0;
}


#if 0
/*******************************************************************
*   Function:    TEST_SPI_FLASH
*   Description: This test will test the SPI flash on the EZ-Board.
*				 Since an image may live in the start of SPI flash
*                we will not erase the entire flash during this test.
*                We will first verify the manufacturer and vendor IDs
*				 then we will perform operations on only a few sectors.
*******************************************************************/
int testFlashParams(void)
{
	int Result = 0;							/* result */
	int nManCode = 0, nDevCode = 0;			/* man and device ids */
	uint8_t mid;							/* manufacturer id */
	uint8_t did;							/* device id */

//	DEBUG_HEADER( "SPI Flash Test" );
#ifdef EZ_KIT
	ConfigSoftSwitches(SS_SPI, sizeof(SoftSwitch)/sizeof(SoftSwitch[0]), SoftSwitch);
#endif
	flash_info = &w25q32bv_info;

	flash_open(flash_info);

	if (flash_info->modes & (QUAD_INPUT | QUAD_OUTPUT | QUAD_IO))
		flash_enable_quad_mode(flash_info);

	Result = flash_read_mid_did(flash_info, &mid, &did);
	if (Result == 0)
	{
		if (mid != flash_info->mid || did != flash_info->did)
			return 0;
	}

//	DEBUG_PRINT( "\nDetected manufacturer code of 0x%x and device code of 0x%x\n", mid, did );

	/* if codes don't match what we expect then we should fail */
	if ( (MAN_CODE != mid) || (DEV_CODE != did) )
	{
//		DEBUG_RESULT(TEST_FAIL, "Flash codes do not match what we expected");
		return 0;
	}

    char wrBuf[] = "Hello World!";

	uint8_t ucBuf[128];

	unsigned long ulOffset;

	/* calculate offset based on sector */
	ulOffset = FLASH_PARAM_ADDR;

	memset(ucBuf, 0, sizeof(ucBuf));

	/* now do a read */
	Result = flash_read(flash_info, ulOffset, ucBuf, 128);

	/* erase the sector */
	Result = flash_erase(flash_info, ulOffset, 128);

	/* now do a read */
	Result = flash_read(flash_info, ulOffset, ucBuf, 128);

	/* write a value to the flash */
	Result = flash_program(flash_info, ulOffset, (uint8_t*) wrBuf, sizeof(wrBuf));

	/* now do a read */
	Result = flash_read(flash_info, ulOffset, ucBuf, 128);

	flash_close(flash_info);
#ifdef EZ_KIT
	ConfigSoftSwitches(SS_DEFAULT, 0, NULL);
#endif
	return 0;
}
#endif
