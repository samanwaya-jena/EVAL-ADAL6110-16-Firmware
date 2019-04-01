/*
 * flash_debug.h
 *
 *  Created on: Feb 15, 2019
 *      Author: JeanAlexis
 */

#ifndef FLASH_DEBUG_H_
#define FLASH_DEBUG_H_

void flash_test();

void ReadFlashSPI(uint16_t _startAddress, uint16_t *_data);
void WriteFlashSPI(uint16_t _startAddress, uint16_t _data);



#endif /* FLASH_DEBUG_H_ */
