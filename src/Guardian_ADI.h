/*
 * Guardian_ADI.h
 *
 *  Created on: 2016-09-05
 *      Author: pc
 */

#ifndef GUARDIAN_ADI_H_
#define GUARDIAN_ADI_H_

#include "algo.h"


#define LED3_ON()  pADI_PORTA->DATA_SET = (1 << 0)
#define LED3_OFF() pADI_PORTA->DATA_CLR = (1 << 0)
#define LED3_TGL() pADI_PORTA->DATA_TGL = (1 << 0)

#define LED4_ON()  pADI_PORTA->DATA_SET = (1 << 1)
#define LED4_OFF() pADI_PORTA->DATA_CLR = (1 << 1)
#define LED4_TGL() pADI_PORTA->DATA_TGL = (1 << 1)

#define LED5_ON()  pADI_PORTB->DATA_SET = (1 << 1)
#define LED5_OFF() pADI_PORTB->DATA_CLR = (1 << 1)
#define LED5_TGL() pADI_PORTB->DATA_TGL = (1 << 1)



enum ADI_Status {
	ADI_Empty , ADI_DataPresent , ADI_locked
};

#define FRAME_NUM_PTS		(GUARDIAN_NUM_CHANNEL * GUARDIAN_SAMPLING_LENGTH)

#define NUM_FIFO			8
#define NUM_FIFO_MASK		(NUM_FIFO-1)



typedef struct
{
	int16_t AcqFifo[FRAME_NUM_PTS];
	detection_type detections[GUARDIAN_NUM_CHANNEL * GUARDIAN_NUM_DET_PER_CH];
} tDataFifo;

extern tDataFifo dataFifo[NUM_FIFO];


/**
 * initialize SPI port for ADI communication
 */
void InitADI(void);

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
void Lidar_GetDataFromFifo(tDataFifo ** pDataPtr, uint16_t * pNumFifo);
void Lidar_ReleaseDataToFifo(uint16_t numFifo);

void ReadParamFromSPI(uint16_t _startAddress, uint16_t *_data);
void WriteParamToSPI(uint16_t _startAddress, uint16_t _data);

int Lidar_ReadFifoPush(uint16_t _startAddress);
int Lidar_WriteFifoPush(uint16_t _startAddress, uint16_t data);

extern bool gAcq;

//
// Testing counters
//
extern int iUSBnum;
extern int iUSBnumOK;
extern int iUSBnumEmpty;

#endif /* GUARDIAN_ADI_H_ */
