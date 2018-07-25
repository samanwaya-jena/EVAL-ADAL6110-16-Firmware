/*
 * (C) Copyright 2014 - Analog Devices, Inc. All Rights Reserved.
 *
 * This software is proprietary and confidential.  By using this software
 * you agree to the terms of the associated Analog Devices License Agreement.
 *
 * Project Name:  	Power_On_Self_Test
 *
 * Hardware:		ADSP-BF707 EZ-Board
 *
 * Description:    Software Switch Configuration for the ADSP-BF707-EZ-BOARD
 */

#include <drivers\twi\adi_twi.h>

#include "SoftConfig_BF707.h"

#include "post_debug.h"

#if defined(__DEBUG_FILE__)
#include <string.h>
extern FILE *pDebugFile;				/* debug file when directing output to a file */
#endif

//#include "timer_isr.h"

/* TWI settings */
#define TWI_PRESCALE  (10u)
#define TWI_BITRATE   (100u)
#define TWI_DUTYCYCLE (50u)

#define BUFFER_SIZE (32u)

/* TWI hDevice handle */
static ADI_TWI_HANDLE hDevice;

/* TWI data buffer */
static uint8_t twiBuffer[BUFFER_SIZE];

/* TWI device memory */
uint8_t deviceMemory[ADI_TWI_MEMORY_SIZE];

void ConfigSoftSwitches(eSOFTSWITCH, uint32_t, SOFT_SWITCH *);

/* disable misra diagnostics as necessary */
#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_8_7:"Objects shall be defined at block scope")
#pragma diag(suppress:misra_rule_17_4:"Array indexing shall be the only allowed form of pointer arithmetic")
#endif /* _MISRA_RULES */


#define ADI_DEBUG 1
#if defined(ADI_DEBUG)
#include <stdio.h>
#define CHECK_RESULT(result, message) \
	do { \
		if((result) != ADI_TWI_SUCCESS) \
		{ \
			DEBUG_PRINT("Error %d calling %s\n", result, message); \
		} \
	} while (0)  /* do-while-zero needed for Misra Rule 19.4 */
#else
#define CHECK_RESULT(result, message)
#endif


void ConfigSoftSwitches(eSOFTSWITCH SS_WHO, uint32_t nNumber, SOFT_SWITCH *SoftSwitch )
{
	uint32_t switchNum;
	uint32_t configNum;
	uint32_t switches;

	SOFT_SWITCH *ss;

	SWITCH_CONFIG *configReg;
	ADI_TWI_RESULT result;

	// disable the GP timer
//	if(!EnableGPTimer(false))
//		return;

	if(SS_WHO == SS_DEFAULT)
	{
		SoftSwitch = &DefSoftSwitch[0];
		nNumber = (uint32_t)(sizeof(DefSoftSwitch)/sizeof(SOFT_SWITCH));
	}
	switches = nNumber;

	for (switchNum=0u; switchNum<switches; switchNum++)
	{

		ss = &SoftSwitch[switchNum];

		result = adi_twi_Open(ss->TWIDevice, ADI_TWI_MASTER, 
    		deviceMemory, ADI_TWI_MEMORY_SIZE, &hDevice);
		CHECK_RESULT(result, "adi_twi_Open failed");

		result = adi_twi_SetHardwareAddress(hDevice, ss->HardwareAddress);
		CHECK_RESULT(result, "adi_twi_SetHardwareAddress failed");

		result = adi_twi_SetPrescale(hDevice, TWI_PRESCALE);
		CHECK_RESULT(result, "adi_twi_Prescale failed");

		result = adi_twi_SetBitRate(hDevice, TWI_BITRATE);
		CHECK_RESULT(result, "adi_twi_SetBitRate failed");

		result = adi_twi_SetDutyCycle(hDevice, TWI_DUTYCYCLE);
		CHECK_RESULT(result, "adi_twi_SetDutyCycle failed");
		
		/* switch register settings */
		for (configNum=0u; configNum<ss->NumConfigSettings; configNum++)
		{
			configReg = &ss->ConfigSettings[configNum];

			/* write register value */
			twiBuffer[0] = configReg->Register;
			twiBuffer[1] = configReg->Value;
			result = adi_twi_Write(hDevice, twiBuffer, (uint32_t)2, false);
			CHECK_RESULT(result, "adi_twi_Write failed");
		}

		result = adi_twi_Close(hDevice);
		CHECK_RESULT(result, "adi_twi_Close failed");
	}

//	EnableGPTimer(true);
}

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */
 
