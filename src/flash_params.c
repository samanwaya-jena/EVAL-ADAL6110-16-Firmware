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
#include "post_debug.h"
//#include "SoftConfig_BF707.h"
//#include "timer_isr.h"


#if defined(__DEBUG_FILE__)
#include <string.h>
extern FILE *pDebugFile;				/* debug file when directing output to a file */
#endif


#define MAN_CODE		0xEF		/* winbond */
#define DEV_CODE		0x15		/* W25Q32BV (16Mbit SPI flash) */
#define NUM_SECTORS		64			/* sectors */


extern struct flash_info w25q32bv_info;


/*******************************************************************
*   Function:    TEST_SPI_FLASH
*   Description: This test will test the SPI flash on the EZ-Board.
*				 Since an image may live in the start of SPI flash
*                we will not erase the entire flash during this test.
*                We will first verify the manufacturer and vendor IDs
*				 then we will perform operations on only a few sectors.
*******************************************************************/
int InitFlashParams(void)
{
	int Result = 0;							/* result */
	int nManCode = 0, nDevCode = 0;			/* man and device ids */
	uint8_t mid;							/* manufacturer id */
	uint8_t did;							/* device id */

	DEBUG_HEADER( "SPI Flash Test" );

	flash_info = &w25q32bv_info;

	flash_open(flash_info);

	flash_set_mode(flash_info, STANDARD);

	if (flash_info->modes & (QUAD_INPUT | QUAD_OUTPUT | QUAD_IO))
		flash_enable_quad_mode(flash_info);

	Result = flash_read_mid_did(flash_info, &mid, &did);
	if (Result == 0)
	{
		if (mid != flash_info->mid || did != flash_info->did)
			return 0;
	}

	DEBUG_PRINT( "\nDetected manufacturer code of 0x%x and device code of 0x%x\n", mid, did );

	/* if codes don't match what we expect then we should fail */
	if ( (MAN_CODE != mid) || (DEV_CODE != did) )
	{
		DEBUG_RESULT(TEST_FAIL, "Flash codes do not match what we expected");
		return 0;
	}

    char wrBuf[] = "Hello World!";

	uint8_t ucBuf[128];

	unsigned long ulOffset;

	/* calculate offset based on sector */
	ulOffset = 40 * 64*1024;	/* 64KB sectors */

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

	return 0;
}
