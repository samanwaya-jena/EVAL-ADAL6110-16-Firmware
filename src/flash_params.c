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


#if defined(__DEBUG_FILE__)
#include <string.h>
extern FILE *pDebugFile;				/* debug file when directing output to a file */
#endif


#define MAN_CODE		0xEF		/* winbond */
#define DEV_CODE		0x15		/* W25Q32BV (16Mbit SPI flash) */
#define NUM_SECTORS		64			/* sectors */


extern struct flash_info w25q32bv_info;


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






/* CRC Driver includes */
#include <drivers/crc/adi_crc.h>

/* Header file with definitions specific to CRC driver implementation */
#include "adi_crc_def.h"

/* CRC Device number to work on */
#define	CRC_DEV_NUM					(0u)

/* CRC Device Handle */
static ADI_CRC_HANDLE   hCrcDev;
/* Memory to handle CRC Device */
static uint8_t          CrcDevMem[ADI_CRC_CORE_MEMORY_SIZE];

/* Flag to register the current status of CRC device */
volatile static bool    bCrcInProgress;

/* CRC Polynomial to be used for this test */
#define CRC_POLYNOMIAL_VAL          (0xEDB88320u)

/* 32-bit Seed value used to generate test data for CRC Memory Scan Compute Compare */
#define CRC_RAND_SEED_VAL           (0x32765321u)

ADI_CRC_RESULT adi_crc_Start(
    ADI_CRC_HANDLE const    hDevice
    )
{
	/* CRC device to work on */
	ADI_CRC_INFO    *pCrcInfo = (ADI_CRC_INFO *)hCrcDev;

	/* Update CRC operation status */
	pCrcInfo->pDevice->eCrcOpStatus |= ADI_CRC_OP_IN_PROGRESS;

	/* Load CRC Control Register */
	pCrcInfo->pReg->Control = (pCrcInfo->pDevice->ControlReg | (uint32_t)pCrcInfo->pDevice->eCrcMode);

	/* Enable CRC Block */
	pCrcInfo->pReg->Control |= ENUM_CRC_CTL_BLKEN_EN;

	return ADI_CRC_SUCCESS;
}

ADI_CRC_RESULT adi_crc_Stop(
    ADI_CRC_HANDLE const    hDevice
    )
{
	/* CRC device to work on */
	ADI_CRC_INFO    *pCrcInfo = (ADI_CRC_INFO *)hCrcDev;

	pCrcInfo->pReg->Control &= ~ENUM_CRC_CTL_BLKEN_EN;

	return ADI_CRC_SUCCESS;
}

typedef struct
{
	uint32_t magic;
	uint32_t numParams;
} tFlashParams;

int Flash_LoadConfig(uint16_t * pParams, int * pNum)
{
	/* CRC Return code */
	ADI_CRC_RESULT	eResult = ADI_CRC_SUCCESS;

	int Result = 0;							/* result */
	int num;
	uint32_t ExpectedCrcVal = 0;
	uint32_t FinalCrcVal = 0;
	uint32_t * pParamsUint32 = (uint32_t*) pParams;

	*pNum = 0;

	ConfigSoftSwitches(SS_SPI, sizeof(SoftSwitch)/sizeof(SoftSwitch[0]), SoftSwitch);

	flash_info = &w25q32bv_info;

	flash_open(flash_info);

	flash_set_mode(flash_info, STANDARD);

	/* calculate offset based on sector */
	unsigned long ulOffset = 40 * 64*1024;	/* 64KB sectors */

	tFlashParams flashParams;

	flashParams.magic = 0;
	flashParams.numParams = 0;

	/* write a value to the flash */
	Result = flash_read(flash_info, ulOffset, (uint8_t*) &flashParams, sizeof(flashParams));
	ulOffset += sizeof(flashParams);

	if (flashParams.magic != 0x1C3B00DA)
		goto FAIL;

	num = flashParams.numParams;

	Result = flash_read(flash_info, ulOffset, (uint8_t*) pParams, num * 2 * sizeof(uint16_t));
	ulOffset += num * 2 * sizeof(uint16_t);

	Result = flash_read(flash_info, ulOffset, (uint8_t*) &ExpectedCrcVal, sizeof(ExpectedCrcVal));
	ulOffset += sizeof(ExpectedCrcVal);

	/* Open a CRC device instance */
	eResult = adi_crc_Open (CRC_DEV_NUM, &CrcDevMem[0], ADI_CRC_CORE_MEMORY_SIZE, &hCrcDev);

	/* Set CRC polynomial */
	eResult = adi_crc_SetPolynomialVal (hCrcDev, CRC_POLYNOMIAL_VAL);

	/* Set CRC operating mode as Data fill */
	eResult = adi_crc_SetOperatingMode (hCrcDev, ADI_CRC_MODE_SCAN_COMPUTE_COMPARE);

	eResult = adi_crc_SetDataCount (hCrcDev, num + 2, 0);

	eResult = adi_crc_Start (hCrcDev);

	eResult = adi_crc_CoreWrite (hCrcDev, flashParams.magic);

	eResult = adi_crc_CoreWrite (hCrcDev, flashParams.numParams);

	int i;

    for (i=0; i<num; i++)
    	eResult = adi_crc_CoreWrite (hCrcDev, *pParamsUint32++);

	eResult = adi_crc_GetFinalCrcVal (hCrcDev, &FinalCrcVal);

	if (FinalCrcVal == ExpectedCrcVal)
		*pNum = num;

FAIL:
	flash_close(flash_info);

	ConfigSoftSwitches(SS_DEFAULT, 0, NULL);

	return 0;
}

int Flash_SaveConfig(uint16_t * pParams, int num)
{
	int Result = 0;							/* result */

	ConfigSoftSwitches(SS_SPI, sizeof(SoftSwitch)/sizeof(SoftSwitch[0]), SoftSwitch);

	flash_info = &w25q32bv_info;

	flash_open(flash_info);

	flash_set_mode(flash_info, STANDARD);

	/* calculate offset based on sector */
	unsigned long ulOffset = 40 * 64*1024;	/* 64KB sectors */

	/* erase the sector */
	Result = flash_erase(flash_info, ulOffset, 64*1024);

	tFlashParams flashParams;

	flashParams.magic = 0x1C3B00DA;
	flashParams.numParams = num;

	/* write a value to the flash */
	Result = flash_program(flash_info, ulOffset, (uint8_t*) &flashParams, sizeof(flashParams));
	ulOffset += sizeof(flashParams);

	Result = flash_program(flash_info, ulOffset, (uint8_t*) pParams, num * 2 * sizeof(uint16_t));
	ulOffset += num * 2 * sizeof(uint16_t);

	/* CRC Return code */
	ADI_CRC_RESULT	eResult = ADI_CRC_SUCCESS;

	/* Open a CRC device instance */
	eResult = adi_crc_Open (CRC_DEV_NUM, &CrcDevMem[0], ADI_CRC_CORE_MEMORY_SIZE, &hCrcDev);

	/* Set CRC polynomial */
	eResult = adi_crc_SetPolynomialVal (hCrcDev, CRC_POLYNOMIAL_VAL);

	/* Set CRC operating mode as Data fill */
	eResult = adi_crc_SetOperatingMode (hCrcDev, ADI_CRC_MODE_SCAN_COMPUTE_COMPARE);

	eResult = adi_crc_SetDataCount (hCrcDev, num + 2, 0);

	eResult = adi_crc_Start (hCrcDev);

	eResult = adi_crc_CoreWrite (hCrcDev, flashParams.magic);

	eResult = adi_crc_CoreWrite (hCrcDev, flashParams.numParams);

	uint32_t * pParamsUint32 = (uint32_t*) pParams;
	int i;

    for (i=0; i<num; i++)
    	eResult = adi_crc_CoreWrite (hCrcDev, *pParamsUint32++);

	uint32_t                FinalCrcVal = 0;
	eResult = adi_crc_GetFinalCrcVal (hCrcDev, &FinalCrcVal);

	// Write the CRC
	Result = flash_program(flash_info, ulOffset, (uint8_t*) &FinalCrcVal, sizeof(uint32_t));

	flash_close(flash_info);

	ConfigSoftSwitches(SS_DEFAULT, 0, NULL);

	return 0;
}

int Flash_ResetToFactoryDefault(void)
{
	int Result = 0;							/* result */

	ConfigSoftSwitches(SS_SPI, sizeof(SoftSwitch)/sizeof(SoftSwitch[0]), SoftSwitch);

	flash_info = &w25q32bv_info;

	flash_open(flash_info);

	flash_set_mode(flash_info, STANDARD);

	/* calculate offset based on sector */
	unsigned long ulOffset = 40 * 64*1024;	/* 64KB sectors */

	/* erase the sector */
	Result = flash_erase(flash_info, ulOffset, 64*1024);

	flash_close(flash_info);

	ConfigSoftSwitches(SS_DEFAULT, 0, NULL);

	return 0;
}



//
// Test functions
//
void TestCRC(void)
{
    /* CRC Return code */
    ADI_CRC_RESULT	eResult = ADI_CRC_SUCCESS;

    /* Open a CRC device instance */
    eResult = adi_crc_Open (CRC_DEV_NUM, &CrcDevMem[0], ADI_CRC_CORE_MEMORY_SIZE, &hCrcDev);

    /* Set CRC polynomial */
	eResult = adi_crc_SetPolynomialVal (hCrcDev, CRC_POLYNOMIAL_VAL);

    /* Set CRC operating mode as Data fill */
	eResult = adi_crc_SetOperatingMode (hCrcDev, ADI_CRC_MODE_SCAN_COMPUTE_COMPARE);

	uint32_t                DataCount = 4;
	uint32_t                ReloadDataCount = 0;

	eResult = adi_crc_SetDataCount (hCrcDev, DataCount, ReloadDataCount);

	uint32_t ExpectedVal = 0x5a26ea00;
	eResult = adi_crc_SetExpectedVal (hCrcDev, ExpectedVal);

	uint32_t CrcSeedVal = 0x48294756;
	eResult = adi_crc_SetCrcSeedVal (hCrcDev, CrcSeedVal);

	eResult = adi_crc_Start (hCrcDev);

	bool bCrcInProgress = false;

	uint32_t DataToCrcFifo = 0x12345678;
	eResult = adi_crc_CoreWrite (hCrcDev, DataToCrcFifo++);

	eResult = adi_crc_CoreWrite (hCrcDev, DataToCrcFifo++);

	eResult = adi_crc_CoreWrite (hCrcDev, DataToCrcFifo++);

	eResult = adi_crc_CoreWrite (hCrcDev, DataToCrcFifo++);

	eResult = adi_crc_IsCrcInProgress(hCrcDev, &bCrcInProgress);

	eResult = adi_crc_Stop (hCrcDev);

	uint32_t                FinalCrcVal = 0;
	eResult = adi_crc_GetFinalCrcVal (hCrcDev, &FinalCrcVal);

	if (FinalCrcVal == ExpectedVal)
		CrcSeedVal++;
	else
		CrcSeedVal--;

	uint32_t                CurrentCrcVal = 0;
	eResult = adi_crc_GetCurrentCrcVal (hCrcDev, &CurrentCrcVal);

}

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

	ConfigSoftSwitches(SS_SPI, sizeof(SoftSwitch)/sizeof(SoftSwitch[0]), SoftSwitch);

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

	ConfigSoftSwitches(SS_DEFAULT, 0, NULL);

	return 0;
}
