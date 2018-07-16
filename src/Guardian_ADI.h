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

#define NUM_OF_CH			16
#define NUM_PTS_PER_CH		100
#define FRAME_NUM_PTS		(NUM_OF_CH * NUM_PTS_PER_CH)
#define FRAME_SIZE_BYTES	(FRAME_NUM_PTS * 2)

#define NUM_FIFO			8
#define NUM_FIFO_MASK		(8-1)

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
void Lidar_FlashGain(uint16_t flashGain);

/*
 * Acquisition Control
 */
void Lidar_Acq(uint16_t *pBank);
void Lidar_GetDataFromFifo(uint16_t ** pDataPtr, uint16_t * pNumFifo);
void Lidar_ReleaseDataToFifo(uint16_t numFifo);

extern int iUSBnum;
extern int iUSBnumOK;
extern int iUSBnumEmpty;

#endif /* GUARDIAN_ADI_H_ */
