/*
 * Guardian_ADI.h
 *
 *  Created on: 2016-09-05
 *      Author: pc
 */

#ifndef GUARDIAN_ADI_H_
#define GUARDIAN_ADI_H_

enum ADI_Status {
	ADI_Empty , ADI_DataPresent , ADI_locked
};

/**
 * initialize SPI port for ADI communication
 */
void InitADI(void);

/**
 * Get the current status
 */
int GetADIStatus(void);

/**
 * Retreive the data from the ADI device
 */
void GetADIData(uint16_t *pBank, uint16_t * pData);

/**
 * clear both memory on the ADI device
 */
void ClearSram(void);

void Lidar_ChannelEnable(int ch, int enable);
void Lidar_ChannelTIAFeedback(int ch, uint16_t feedback);
void Lidar_ChannelDCBal(int ch, uint16_t bal);

void Lidar_Trig(void);
void Lidar_SPITriggerMode(void);
void Lidar_FreerunMode(void);

#endif /* GUARDIAN_ADI_H_ */
