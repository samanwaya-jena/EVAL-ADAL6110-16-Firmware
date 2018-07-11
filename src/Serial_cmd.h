/*
 * Serial_cmd.h
 *
 *  Created on: Jul 10, 2018
 *      Author: danie
 */

#ifndef SERIAL_CMD_H_
#define SERIAL_CMD_H_

extern uint16_t buf[1600];

void Serial_RxChar(char curChar);
void Serial_Process(void);
void GetADIData(uint16_t *pBank, uint16_t * pData);

#endif /* SERIAL_CMD_H_ */
