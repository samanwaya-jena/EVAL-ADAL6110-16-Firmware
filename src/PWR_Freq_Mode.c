/*
 * Copyright(c) 2014 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary and confidential.  By using this software you agree
 * to the terms of the associated Analog Devices License Agreement.
 */

/*
 * Use the power service to change processor mode and frequency settings.
 */

#include <services\pwr\adi_pwr.h>
#include <ccblkfn.h>
#include <stdio.h>
#include "post_debug.h"

/* ADI initialization header */
#include "PWR_Freq_Mode.h"

static bool bError = false;

static void CheckResult(ADI_PWR_RESULT result, char *message);

/* the power service device number */
#define DEV_NUM   (0)

/*
 * Report error if result code is not success
 */
static void CheckResult(ADI_PWR_RESULT result, char *message)
{
#if !defined (__DEBUG_UART__)
	if (result != ADI_PWR_SUCCESS) {
		DEBUG_PRINT("%s failed\n", message);
		bError = true;
	}
#endif
}


/*
 * Power event Callback function
 * Suppress these as someone may want to use in the future
 */
static void PWRCallback(void *pCBParam, uint32_t Event, void *pArg)
{
	uint32_t sclk;
	ADI_PWR_MODE  mode;

	switch ( (ADI_PWR_EVENT)Event )
	{
    case ADI_PWR_EVENT_FREQ_PRE_CHANGE:
    	sclk = *(uint32_t*)(pArg);
    	break;
    case ADI_PWR_EVENT_FREQ_POST_CHANGE:
    	sclk = *(uint32_t*)(pArg);
    	break;
    case ADI_PWR_EVENT_MODE_PRE_CHANGE:
    	mode = (ADI_PWR_MODE)pArg;
    	break;
    case ADI_PWR_EVENT_MODE_POST_CHANGE:
    	mode = (ADI_PWR_MODE)pArg;
    	break;
    default:
    	break;
	}
}

/*
 * power_init
 */
uint32_t power_init(  )
{
	uint32_t   fcclk = 0u;
	uint32_t   fsysclk = 0u;
	uint32_t   fsclk0 = 0u;
	uint32_t   fsclk1 = 0u;
	uint32_t   coreclk;
	uint32_t   systemclk;

	ADI_PWR_RESULT result;
	ADI_PWR_MODE  mode;

	bError = false;

	result = adi_pwr_Init(DEV_NUM, CLKIN);
	CheckResult(result, "adi_pwr_Open");

	result = adi_pwr_InstallCallback(DEV_NUM, PWRCallback);
	CheckResult(result, "adi_pwr_InstallCallback");

	result = adi_pwr_SetClkDivideRegister(DEV_NUM, ADI_PWR_CLK_DIV_DSEL, 3);
	CheckResult(result, "adi_pwr_SetClkDivideRegister");

	/* set max freq */
	result = adi_pwr_SetFreq(DEV_NUM, CORE_MAX, SYSCLK_MAX);
	CheckResult(result, "adi_pwr_SetFreq");

	result = adi_pwr_GetCoreFreq(DEV_NUM, &fcclk);
	CheckResult(result, "adi_pwr_GetCoreFreq");

	result = adi_pwr_GetSystemFreq(DEV_NUM, &fsysclk, &fsclk0, &fsclk1);
	CheckResult(result, "adi_pwr_GetSystemFreq");
	if ((fcclk < (CORE_MAX-CLKIN)) || (fsysclk < (SYSCLK_MAX-CLKIN)))
	{
		bError = true;
	}

    return bError ? 0 : 1;
}

/*
 * Update the CCLK and SCLK for SRAM test
 */
uint32_t ChangeFreq( uint32_t  coreclk, uint32_t  systemclk)
{
	ADI_PWR_RESULT result;

	result = adi_pwr_SetFreq(DEV_NUM, coreclk, systemclk);
	CheckResult(result, "adi_pwr_SetFreq");

    return bError ? 0 : 1;
}
