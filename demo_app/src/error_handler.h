/*
 * error.h
 *
 *  Created on: Feb 14, 2020
 *      Author: e.turenne
 */


#ifndef SRC_ERROR_H_
#define SRC_ERROR_H_

typedef enum{
	error_SW_ADI = 0,
	error_SW_DSP ,
	error_SW_flash ,

	error_SW_comm_fifo_full = 4,
	error_SW_comm_timeout,
	error_SW_comm_unknown,
	error_numberOfErrors
} error_type;

inline void SetError(error_type err);
inline error_type getError();

#endif /* SRC_ERROR_H_ */
