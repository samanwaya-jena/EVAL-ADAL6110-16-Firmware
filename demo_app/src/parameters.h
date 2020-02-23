/*
 * parameters.h
 *
 *  Created on: Feb 22, 2020
 *      Author: e.turenne
 */

#ifndef SRC_PARAMETERS_H_
#define SRC_PARAMETERS_H_


int ProcessReadWriteFifo(void);
int Lidar_ReadFifoPush(uint16_t _startAddress);
int Lidar_WriteFifoPush(uint16_t _startAddress, uint16_t data);



#endif /* SRC_PARAMETERS_H_ */
