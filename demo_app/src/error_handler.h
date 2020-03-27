/*
 * error_handler.h
 *
 *  Created on: Feb 14, 2020
 *      Author: e.turenne
 */
#include <stdint.h>


#ifndef SRC_ERROR_H_
#define SRC_ERROR_H_

typedef enum{
	error_SW_Boot = 0,
	error_SW_ADI,
	error_SW_DSP ,
	error_SW_flash ,

	error_SW_comm_fifo_full = 8,
	error_SW_comm_timeout,
	error_SW_comm_unknown,
	error_SW_comm_USB_send,
	error_numberOfErrors
} error_type;

void SetError(error_type err);
uint16_t GetError(void);
int IsErrorSet(error_type err);


#endif /* SRC_ERROR_H_ */
