/*
 * error_handler.c
 *
 *  Created on: Feb 14, 2020
 *      Author: e.turenne
 */

#include "error_handler.h"

#include <stdint.h>
#include "demo_app.h"
#include "Communications/cld_bf70x_bulk_lib.h"
#include "parameters.h"
#include "adal6110_16.h"

static uint16_t errorFlags;
static char* errText[error_numberOfErrors] = {"Bootup error",
		                                      "ADAL6110_16 malfunction",
											  "DSP error",
											  "Configuration fault",

											  "",
											  "",
											  "",
											  "",

											  "Outbound message Queue full",
											  "USB communication timeout",
											  "USB unknown incoming message",
											  "USB outbound message error"};

void SetError(error_type err)
{
	CLD_Time errTime;

	errTime = cld_time_get();

	if (err < error_numberOfErrors)
	{
		errorFlags |= 1<<err;
		if(LiDARParameters[param_console_log]&CONSOLE_MASK_DEBUG)
			cld_console(CLD_CONSOLE_RED, CLD_CONSOLE_BLACK, "@%08d error:%04X -- %s\n\r", errTime, errorFlags, errText[err]);
	}else
	{
		errorFlags = 0xFFFF;
		if(LiDARParameters[param_console_log]&CONSOLE_MASK_DEBUG)
			cld_console(CLD_CONSOLE_BLACK, CLD_CONSOLE_RED,
					"@%08d error:%04X -- Unknown error! please call your local EVAL-ADAL6110_16 maintenance crew\n\r", errTime, errorFlags);
	}

	LiDARParameters[param_error_status] = errorFlags;
}


uint16_t GetError(void)
{
	uint32_t err;

	if(errorFlags) LED_BC2R_ON();
	else LED_BC2R_OFF();


	err = errorFlags;
	errorFlags = 0x0000;
	LiDARParameters[param_error_status] = 0x0000;
	return(err);

}


int IsErrorSet(error_type err)
{
	return( (1<<err)&errorFlags);
}
