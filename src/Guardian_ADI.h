/*
  Copyright 2018 Phantom Intelligence Inc.
*/

#ifndef GUARDIAN_ADI_H_
#define GUARDIAN_ADI_H_

#include "algo.h"


#define FRAME_NUM_PTS		(GUARDIAN_NUM_CHANNEL * GUARDIAN_SAMPLING_LENGTH)

typedef struct
{
	int16_t AcqFifo[FRAME_NUM_PTS];
} tDataFifo;


/**
 * initialize SPI port for ADI communication
 */
void Lidar_InitADI(void);

/**
 * Retreive the data from the ADI device
 */
void Lidar_GetADIData(uint16_t *pBank, uint16_t * pData);

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
void Lidar_Reset(void);

/*
 * Private read/write SPI functions
 */
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
