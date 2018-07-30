/*
 * calc_crc.c
 *
 *  Created on: Jul 30, 2018
 *      Author: danie
 */

/* CRC Driver includes */
#include <drivers/crc/adi_crc.h>

/* Header file with definitions specific to CRC driver implementation */
#include "adi_crc_def.h"

#include "calc_crc.h"



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

uint32_t CalcCRC(uint32_t * ptr, int num)
{
	/* CRC Return code */
	ADI_CRC_RESULT	eResult = ADI_CRC_SUCCESS;

	uint32_t FinalCrcVal = 0;

	/* Open a CRC device instance */
	eResult = adi_crc_Open (CRC_DEV_NUM, &CrcDevMem[0], ADI_CRC_CORE_MEMORY_SIZE, &hCrcDev);

	/* Set CRC polynomial */
	eResult = adi_crc_SetPolynomialVal (hCrcDev, CRC_POLYNOMIAL_VAL);

	/* Set CRC operating mode as Data fill */
	eResult = adi_crc_SetOperatingMode (hCrcDev, ADI_CRC_MODE_SCAN_COMPUTE_COMPARE);

	eResult = adi_crc_SetDataCount (hCrcDev, num, 0);

	eResult = adi_crc_Start (hCrcDev);

	int i;

    for (i=0; i<num; i++)
    	eResult = adi_crc_CoreWrite (hCrcDev, *ptr++);

	eResult = adi_crc_GetFinalCrcVal (hCrcDev, &FinalCrcVal);

	eResult = adi_crc_Close(hCrcDev);

	return FinalCrcVal;
}







#if 0
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
#endif
