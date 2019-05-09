/*
  Copyright 2018 Phantom Intelligence Inc.
*/

#ifndef GUARDIAN_ADI_H_
#define GUARDIAN_ADI_H_

#include "algo.h"


#define DATA_NUM_PTS	(DEVICE_NUM_CHANNEL * DEVICE_SAMPLING_LENGTH)
#define FRAME_NUM_PTS	DATA_NUM_PTS + (DEVICE_NUM_CHANNEL * 5) + 16
//#define FRAME_NUM_PTS	DATA_NUM_PTS
#define RW_WRITE_MASK    0x8000
#define RW_INTERNAL_MASK 0x4000

typedef struct
{
	int16_t AcqFifo[FRAME_NUM_PTS];
} tDataFifo;


enum ADI_REGISTER_INDEX {
	DeviceIDAddress = 0x00, // Read only
	Control0Address = 0x01,
	Control1Address = 0x02,
	DataControlAddress = 0x03,
	DelayBetweenFlashesAddress = 0x04,
	ChannelEnableAddress = 0x05,
	DataAcqMode = 0x06,
	TriggerOutAddress = 0x07,
	CH0ControlReg0Address = 0x0D,
	CH0ControlReg1Address = 0x0E,
	CH0ControlReg2Address = 0x0F,
	CH1ControlReg0Address = 0x11,
	CH1ControlReg1Address = 0x12,
	CH1ControlReg2Address = 0x13,
	CH2ControlReg0Address = 0x15,
	CH2ControlReg1Address = 0x16,
	CH2ControlReg2Address = 0x17,
	CH3ControlReg0Address = 0x19,
	CH3ControlReg1Address = 0x1A,
	CH3ControlReg2Address = 0x1B,
	CH4ControlReg0Address = 0x1D,
	CH4ControlReg1Address = 0x1E,
	CH4ControlReg2Address = 0x1F,
	CH5ControlReg0Address = 0x21,
	CH5ControlReg1Address = 0x22,
	CH5ControlReg2Address = 0x23,
	CH6ControlReg0Address = 0x25,
	CH6ControlReg1Address = 0x26,
	CH6ControlReg2Address = 0x27,
	CH7ControlReg0Address = 0x29,
	CH7ControlReg1Address = 0x2A,
	CH7ControlReg2Address = 0x2B,
	CH8ControlReg0Address = 0x2D,
	CH8ControlReg1Address = 0x2E,
	CH8ControlReg2Address = 0x2F,
	CH9ControlReg0Address = 0x31,
	CH9ControlReg1Address = 0x32,
	CH9ControlReg2Address = 0x33,
	CH10ControlReg0Address = 0x35,
	CH10ControlReg1Address = 0x36,
	CH10ControlReg2Address = 0x37,
	CH11ControlReg0Address = 0x39,
	CH11ControlReg1Address = 0x3A,
	CH11ControlReg2Address = 0x3B,
	CH12ControlReg0Address = 0x3D,
	CH12ControlReg1Address = 0x3E,
	CH12ControlReg2Address = 0x3F,
	CH13ControlReg0Address = 0x41,
	CH13ControlReg1Address = 0x42,
	CH13ControlReg2Address = 0x43,
	CH14ControlReg0Address = 0x45,
	CH14ControlReg1Address = 0x46,
	CH14ControlReg2Address = 0x47,
	CH15ControlReg0Address = 0x49,
	CH15ControlReg1Address = 0x4A,
	CH15ControlReg2Address = 0x4B,
	GPIOCFG = 0x70,
	SPICFG = 0x73,
	THSMAX = 0xE1,
	THSMIN = 0xE2,
	AGCDCBCTRL = 0xE3,
	AGCEN = 0xE4,
	DCEN = 0xE5,
	AGCDCBPID0 = 0xE6,
	AGCDCBPID1 = 0xE7,
	FRAMEDELAY = 0xE8,
	STARTADDRPOINTER = 0xF5,
	SRAM_READY = 0xF6, // read only
	LFSRSEEDL = 0xF7,
	LFSRSEEDH = 0xF8,
	SRAM_DATA = 0xFF // read only
};



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

void LoadDefaultConfig(int idx);

extern int gAcq;

//
// Testing counters
//
extern int iUSBnum;
extern int iUSBnumOK;
extern int iUSBnumEmpty;

#endif /* GUARDIAN_ADI_H_ */
