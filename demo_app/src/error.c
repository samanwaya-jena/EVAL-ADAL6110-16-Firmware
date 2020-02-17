/*
 * error.c
 *
 *  Created on: Feb 14, 2020
 *      Author: e.turenne
 */


#include "error.h"
#include "Communications/cld_bf70x_bulk_lib.h"

static uint16_t errorFlags;
static char* errText[error_numberOfErrors] = {"ADAL6110_16 malfunction",
		                                      "DSP error",
											  "Configuration CRC fault",
											  "",
											  "Outbound message Queue full",
											  "USB communication timeout",
											  "USB unknown incoming message"};

inline void SetError(error_type err)
{
	CLD_Time errTime;

	errTime = cld_time_get();

	if (err < error_numberOfErrors)
	{
		errorFlags |= 1<<err;
		cld_console(CLD_CONSOLE_RED, CLD_CONSOLE_BLACK, "@%08d error:%04X -- %s\n\r", errTime, errorFlags, errText[err]);
	}else
	{
		errorFlags = 0xFFFF;
		cld_console(CLD_CONSOLE_BLACK, CLD_CONSOLE_RED,
				"@%08d error:%04X -- Unknown error! please call your local EVAL-ADAL6110_16 maintenance crew\n\r", errTime, errorFlags);
	}
}

inline error_type getError()
{
	error_type err;

	if(errorFlags) LED_BC2R_ON();
	else LED_BC2R_OFF();

	err = errorFlags;
	errorFlags = 0x0000;
	return(err);

}

