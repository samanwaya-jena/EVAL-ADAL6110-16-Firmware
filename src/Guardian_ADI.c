/*
  Copyright 2018 Phantom Intelligence Inc.
*/

#define USE_DMA
#define USE_ALGO
//#define USE_ACCUMULATION


#include <stdint.h>
#include <string.h>

#include <drivers\spi\adi_spi.h>

#include <ADSP-BF707_device.h>

#include "user_bulk.h"

#include "post_debug.h"

#ifdef USE_ALGO
#include "algo.h"
#endif //USE_ALGO

#include "flash_params.h"

#include "BF707_Wagner.h"

#include "Guardian_ADI.h"


//#include "Guardian_basics.h"
//#include "rawdata.h"
//#include "GPIO.h"

/**
 * Private constant and variables
 *
 */



uint16_t Lidar_InitValues[][2] =
{
		{ 9, 0x7A4F },
		{ TriggerOutAddress, 0x1811 },
		{ 95, 0x8FCC },
		{ 96, 0x8FCC },
		{ 97, 0x8FCC },
		{ 98, 0x8FCC },
		{ 99, 0x8FCC },
		{ 100, 0x8FCC },
		{ 101, 0x8FCC },
		{ 102, 0x8FCC },
		{ 144, 0x0420 },
		{ 145, 0x87B4 },
		{ 146, 0x0004 },
		{ 129, 0x0001 },
		{ 131, 0x0A40 },
		{132, 0xC201}
};

int aChIdxADI[16] = {
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15
};

int aChIdxArray[16] = {
  7,
  8,
  6,
  9,
  5,
  10,
  4,
  11,
  3,
  12,
  2,
  13,
  1,
  14,
  0,
  15
};

uint16_t Int_InitValues[][2] =
{
  { 1, 0x0 },
  { 2, 0x0 }
};

#ifdef USE_DMA
uint8_t spiMem[ADI_SPI_DMA_MEMORY_SIZE];
#else //USE_DMA
uint8_t spiMem[ADI_SPI_INT_MEMORY_SIZE];
#endif //USE_DMA

ADI_SPI_HANDLE hSpi = NULL;


#ifdef USE_ACCUMULATION
bool bAcqAccUpdate = 1;
uint16_t iAcqAccMaxNew = 16;

uint16_t iAcqAccMax = 0;
uint16_t iAcqAccShift = 0;
#endif //USE_ACCUMULATION

/*
 * Private function prototypes
 */
static void ClearSram(void);



/**
 *  @brief Writes param to SPI
 *
 *  @param uint16_t _startAddres: register address
 *  @param uint16 _data: data to send
 *
 */
void WriteParamToSPI(uint16_t _startAddress, uint16_t _data)
{
	uint8_t ProBuffer1[4];

	ProBuffer1[0] = (_startAddress << 3);
	ProBuffer1[1] = (_startAddress << 3) >> 8;
	ProBuffer1[2] = (_data >> 0);
	ProBuffer1[3] = (_data >> 8);

	ADI_SPI_TRANSCEIVER Xcv0  = {ProBuffer1, 4u, NULL, 0u, NULL, 0u};

	ADI_SPI_RESULT result = adi_spi_ReadWrite(hSpi, &Xcv0);
}

/**
 *  @brief Read a signle register from SPI port
 *
 *  @param uint16_t _startAddress: register address
 *  @param uint16 *_dataPtr: pointer to the receive data buffer
 *
 */
void ReadParamFromSPI(uint16_t _startAddress, uint16_t *_data)
{
	uint8_t ProBuffer1[2];
	uint8_t RxBuffer1[2];

	ProBuffer1[0] = (_startAddress << 3);
	ProBuffer1[1] = ((_startAddress << 3) >> 8) | 0x80;

	RxBuffer1[0] = 0;
	RxBuffer1[1] = 0;

	ADI_SPI_TRANSCEIVER Xcv0  = {ProBuffer1, 2u, NULL, 0u, RxBuffer1, 2u};

	ADI_SPI_RESULT result = adi_spi_ReadWrite(hSpi, &Xcv0);

	*_data = *((uint16_t*)RxBuffer1);
}

/**
 *  @brief Read the sensor data from SPI port
 *
 *  @param uint16 *_dataPtr: pointer to the receive data buffer
 *
 *  Suffucient space should be made for all the data in the block *_dataPtr
 *
 */
void ReadDataFromSPI(uint16_t * pData, int num)
{
	//TO CHANGE METHOD TO USE SRAMREADOUTADDR
	uint8_t ProBuffer1[2] = {0xF8, 0x87}; //{LSB, MSB}

	ADI_SPI_TRANSCEIVER Xcv0DMA;

	ADI_SPI_RESULT result;

	Xcv0DMA.pPrologue = ProBuffer1;
	Xcv0DMA.PrologueBytes = 2u;
	Xcv0DMA.pTransmitter = NULL;
	Xcv0DMA.TransmitterBytes = 0u;
	Xcv0DMA.pReceiver = (uint8_t*) pData;
	Xcv0DMA.ReceiverBytes = num * sizeof(uint16_t);

	/* Disable DMA */
//	result = adi_spi_EnableDmaMode(hSpi, true);

//	result = adi_spi_SetDmaTransferSize(hSpi, ADI_SPI_DMA_TRANSFER_16BIT);

	result = adi_spi_SubmitBuffer(hSpi, &Xcv0DMA);

	bool bAvailSpi = false;
	while (!bAvailSpi)
	{
		result = adi_spi_IsBufferAvailable(hSpi, &bAvailSpi);

		if (bAvailSpi)
		{
			ADI_SPI_TRANSCEIVER     *pTransceiver = NULL;

			result =  adi_spi_GetBuffer(hSpi, &pTransceiver);

			if (pTransceiver)
			{
				++pTransceiver->TransmitterBytes;
			}

		}
	}

	/* Disable DMA */
//	result = adi_spi_EnableDmaMode(hSpi, false);
}

void ReadDataFromSPI_Start(uint16_t * pData, int num)
{
	static uint8_t gProBuffer1[2] = {0xF8, 0x87};
	static ADI_SPI_TRANSCEIVER gXcv0DMA;

	ADI_SPI_RESULT result;

#ifdef USE_DMA
	/* Disable DMA */
	result = adi_spi_EnableDmaMode(hSpi, true);

	result = adi_spi_SetDmaTransferSize(hSpi, ADI_SPI_DMA_TRANSFER_16BIT);
#endif //USE_DMA

	gXcv0DMA.pPrologue = gProBuffer1;
	gXcv0DMA.PrologueBytes = 2u;
	gXcv0DMA.pTransmitter = NULL;
	gXcv0DMA.TransmitterBytes = 0u;
	gXcv0DMA.pReceiver = (uint8_t*) pData;
	gXcv0DMA.ReceiverBytes = num * sizeof(uint16_t);

	result = adi_spi_SubmitBuffer(hSpi, &gXcv0DMA);
}

bool ReadDataFromSPI_Check(void)
{
	ADI_SPI_RESULT result;

	bool bAvailSpi = false;

	result = adi_spi_IsBufferAvailable(hSpi, &bAvailSpi);

	if (bAvailSpi)
	{
		ADI_SPI_TRANSCEIVER     *pTransceiver = NULL;

		result =  adi_spi_GetBuffer(hSpi, &pTransceiver);

//		if (pTransceiver)
//		{
//			++pTransceiver->TransmitterBytes;
//		}
	}

	return bAvailSpi;
}

void ResetADI(void) {

	// TODO: Drive the reset pin when available
	//We dont have reset ctrl need to wait a certain delay
	int i = 1200000; // 1.5ms @ 200mhz
	DEBUG_HEADER( "Wait for Gordon RESET sequence" );


	while(i--){
	}

	i = 1200000;
	while(i--){
	}

	//Lidar_SPITriggerMode();

	//ClearSram();
}

/**
 *
 *  Public API
 *
 *
 *
 */

/**
 * @brief initialize SPI port for ADI communication
 */
void Lidar_InitADI(void) {
    int i;
    uint16_t dataToBeRead = 0;
    uint16_t waitTimer = 40000; //wait 200us @ 200mhz


	if (hSpi == NULL)
	{
		ADI_SPI_RESULT result;

		result = adi_spi_Open(GORDON_SPI_DEVICE, spiMem, sizeof(spiMem), &hSpi);
		//result = adi_spi_Open(2, spiMem, sizeof(spiMem), &hSpi);
		CHECK_RESULT(result, "adi_spi_Open");

		/* Set master */
		result = adi_spi_SetMaster( hSpi,true);
		CHECK_RESULT(result, "adi_spi_SetMaster");

		/* Set slave select using hardware*/
		result = adi_spi_SetHwSlaveSelect( hSpi, false);
		CHECK_RESULT(result, "adi_spi_SetHwSlaveSelect");

		result = adi_spi_SetTransmitUnderflow( hSpi, true);
		CHECK_RESULT(result, "adi_spi_SetTransmitUnderflow");

		result = adi_spi_SetClockPhase(hSpi, false);
		CHECK_RESULT(result, "adi_spi_SetClockPhase");

		/* Setting the clock frequency of spi   The frequency of the SPI clock is calculated by SCLK / 2* (Baud=3)*/
		result = adi_spi_SetClock( hSpi, 9);
		CHECK_RESULT(result, "adi_spi_SetClock");

		/* Selecting slave1 as the device*/
		result = adi_spi_SetSlaveSelect( hSpi, ADI_SPI_SSEL_ENABLE1);
		CHECK_RESULT(result, "adi_spi_SetSlaveSelect");

		/* Setting the word size of 8 bytes*/
		result = adi_spi_SetWordSize( hSpi, ADI_SPI_TRANSFER_16BIT);
		CHECK_RESULT(result, "adi_spi_SetWordSize");

		/* No call backs required*/
	//	result = adi_spi_RegisterCallback(hSpi, NULL, NULL);

	//	result = adi_spi_SetTransceiverMode(hSpi, ADI_SPI_TXRX_MODE);

		result = adi_spi_SetClockPolarity(hSpi, true);
		CHECK_RESULT(result, "adi_spi_SetClockPolarity");

		if (result == 0u)
		{
			/* generate tx data interrupt when watermark level breaches 50% level */
			/* DMA watermark levels are disabled because SPI is in interrupt mode */
			result = adi_spi_SetTxWatermark(hSpi,
													  ADI_SPI_WATERMARK_50,
													  ADI_SPI_WATERMARK_DISABLE,
													  ADI_SPI_WATERMARK_DISABLE);
			CHECK_RESULT(result, "adi_spi_SetTxWatermark");
		}
		if (result == 0u)
		{
			/* generate rx data interrupt when watermark level breaches 50% level */
			/* DMA watermark levels are disabled because SPI is in interrupt mode */
			result = adi_spi_SetRxWatermark(hSpi,
													  ADI_SPI_WATERMARK_50,
													  ADI_SPI_WATERMARK_DISABLE,
													  ADI_SPI_WATERMARK_DISABLE);
			CHECK_RESULT(result, "adi_spi_SetRxWatermark");
		}
	}

	ResetADI();

	int num = sizeof(Lidar_InitValues) / sizeof(Lidar_InitValues[0]);
	int numParams = num;


	Flash_LoadConfig(0, &Lidar_InitValues[0][0], &numParams);
	//numParams = 0;
	if (numParams)
		num = numParams;

	for (i=0; i<num; i++)
	{
		WriteParamToSPI(Lidar_InitValues[i][0], Lidar_InitValues[i][1]);
		ReadParamFromSPI(Lidar_InitValues[i][0], &dataToBeRead);
	}

	numParams = 2;

	//Flash_LoadConfig(1, &Int_InitValues[0][0], &numParams);

	for (i = 0; i < numParams; i++)
	{
		switch (Int_InitValues[i][0])
		{
#ifdef USE_ACCUMULATION
			case 1:
				iAcqAccMax = Int_InitValues[i][1];
				break;
			case 2:
				iAcqAccShift = Int_InitValues[i][1];
				break;
#endif //USE_ACCUMULATION
			default:
				break;
		}
	}


    WriteParamToSPI(CH0ControlReg0Address, 0x001F);
  	WriteParamToSPI(CH1ControlReg0Address, 0x001F);
  	WriteParamToSPI(CH2ControlReg0Address, 0x001F);
  	WriteParamToSPI(CH3ControlReg0Address, 0x001F);
  	WriteParamToSPI(CH4ControlReg0Address, 0x001F);
  	WriteParamToSPI(CH5ControlReg0Address, 0x001F);
  	WriteParamToSPI(CH6ControlReg0Address, 0x001F);
  	WriteParamToSPI(CH7ControlReg0Address, 0x001F);
  	WriteParamToSPI(CH8ControlReg0Address, 0x001F);
  	WriteParamToSPI(CH9ControlReg0Address, 0x001F);
  	WriteParamToSPI(CH10ControlReg0Address, 0x001F);
  	WriteParamToSPI(CH11ControlReg0Address, 0x001F);
  	WriteParamToSPI(CH12ControlReg0Address, 0x001F);
  	WriteParamToSPI(CH13ControlReg0Address, 0x001F);
  	WriteParamToSPI(CH14ControlReg0Address, 0x001F);
  	WriteParamToSPI(CH15ControlReg0Address, 0x001F);

  	dataToBeRead = 0;

  	while(!dataToBeRead) { // Wait until bit 4 of register 0x92 is set
  		ReadParamFromSPI(146, &dataToBeRead);
  		if (dataToBeRead == 0xffff){
  			dataToBeRead = 0;
  		}
  		dataToBeRead = dataToBeRead & 8;
  	}

  	WriteParamToSPI(LFSRSEEDL, 0x9190);
  	WriteParamToSPI(LFSRSEEDH, 0x0001);
  	WriteParamToSPI(Control0Address, 0x1F80); // 16 accum // 64 accum = 0x1F80 (ne pas dépasser)
  	WriteParamToSPI(Control1Address, 0x8040);
  	WriteParamToSPI(TriggerOutAddress, 0x1021);//0x1021
  	WriteParamToSPI(DataAcqMode, 0x0001);
  	WriteParamToSPI(DelayBetweenFlashesAddress, 0x8000); //0x8000 (52 us)
  	WriteParamToSPI(CH0ControlReg0Address, 0x2E1F); // 0x5F9F
  	WriteParamToSPI(CH1ControlReg0Address, 0x2E1F);
  	WriteParamToSPI(CH2ControlReg0Address, 0x2E1F);
  	WriteParamToSPI(CH3ControlReg0Address, 0x2E1F);
  	WriteParamToSPI(CH4ControlReg0Address, 0x2E1F);
  	WriteParamToSPI(CH5ControlReg0Address, 0x2E1F);
  	WriteParamToSPI(CH6ControlReg0Address, 0x2E1F);
  	WriteParamToSPI(CH7ControlReg0Address, 0x2E1F);
  	WriteParamToSPI(CH8ControlReg0Address, 0x2E1F);
  	WriteParamToSPI(CH9ControlReg0Address, 0x2E1F);
  	WriteParamToSPI(CH10ControlReg0Address, 0x2E1F);
  	WriteParamToSPI(CH11ControlReg0Address, 0x2E1F);
  	WriteParamToSPI(CH12ControlReg0Address, 0x2E1F);
  	WriteParamToSPI(CH13ControlReg0Address, 0x2E1F);
  	WriteParamToSPI(CH14ControlReg0Address, 0x2E1F);
  	WriteParamToSPI(CH15ControlReg0Address, 0x2E1F);

  	WriteParamToSPI(CH0ControlReg1Address, 0x0180); // 0x01C0
  	WriteParamToSPI(CH1ControlReg1Address, 0x0180);
  	WriteParamToSPI(CH2ControlReg1Address, 0x0180);
  	WriteParamToSPI(CH3ControlReg1Address, 0x0180);
  	WriteParamToSPI(CH4ControlReg1Address, 0x0180);
  	WriteParamToSPI(CH5ControlReg1Address, 0x0180);
  	WriteParamToSPI(CH6ControlReg1Address, 0x0180);
  	WriteParamToSPI(CH7ControlReg1Address, 0x0180);
  	WriteParamToSPI(CH8ControlReg1Address, 0x0180);
  	WriteParamToSPI(CH9ControlReg1Address, 0x0180);
  	WriteParamToSPI(CH10ControlReg1Address, 0x0180);
  	WriteParamToSPI(CH11ControlReg1Address, 0x0180);
  	WriteParamToSPI(CH12ControlReg1Address, 0x0180);
  	WriteParamToSPI(CH13ControlReg1Address, 0x0180);
  	WriteParamToSPI(CH14ControlReg1Address, 0x0180);
  	WriteParamToSPI(CH15ControlReg1Address, 0x0180);

  	WriteParamToSPI(CH0ControlReg2Address, 0x00FF);
  	WriteParamToSPI(CH1ControlReg2Address, 0x00FF);
  	WriteParamToSPI(CH2ControlReg2Address, 0x00FF);
  	WriteParamToSPI(CH3ControlReg2Address, 0x00FF);
  	WriteParamToSPI(CH4ControlReg2Address, 0x00FF);
  	WriteParamToSPI(CH5ControlReg2Address, 0x00FF);
  	WriteParamToSPI(CH6ControlReg2Address, 0x00FF);
  	WriteParamToSPI(CH7ControlReg2Address, 0x00FF);
  	WriteParamToSPI(CH8ControlReg2Address, 0x00FF);
  	WriteParamToSPI(CH9ControlReg2Address, 0x00FF);
  	WriteParamToSPI(CH10ControlReg2Address, 0x00FF);
  	WriteParamToSPI(CH11ControlReg2Address, 0x00FF);
  	WriteParamToSPI(CH12ControlReg2Address, 0x00FF);
  	WriteParamToSPI(CH13ControlReg2Address, 0x00FF);
  	WriteParamToSPI(CH14ControlReg2Address, 0x00FF);
  	WriteParamToSPI(CH15ControlReg2Address, 0x00FF);

  	WriteParamToSPI(FRAMEDELAY, 0xFFFF); // Slow it down to max (3.86 ms gordon frame period with 64 accumulation)
  	WriteParamToSPI(AGCDCBCTRL, 0x0104);

  	WriteParamToSPI(185, 0);
  	WriteParamToSPI(77, 0xC23F);
  	WriteParamToSPI(86, 0x823F);
  	WriteParamToSPI(ChannelEnableAddress, 0xFFFF);
  	WriteParamToSPI(AGCEN, 0x0000);
  	WriteParamToSPI(DCEN, 0xFFFF);
  	WriteParamToSPI(185, 1);

  	while(waitTimer--){ // Wait 200 us
  	}

  	WriteParamToSPI(Control0Address, 0x1F82); // Set system ready to 1

  	//TODO Reenable user_can_fifo_push
	user_CANFifoPushSensorStatus();
    user_CANFifoPushSensorBoot();

}


int SaveConfigToFlash(int idx)
{
	if (idx == 0)
	{
		int num = sizeof(Lidar_InitValues) / sizeof(Lidar_InitValues[0]);
		int i;
		for (i = 0; i < num; i++)
		{
			ReadParamFromSPI(Lidar_InitValues[i][0], &Lidar_InitValues[i][1]);
		}

		return Flash_SaveConfig(idx, &Lidar_InitValues[0][0], num);
	}
	else
	{
		int num;

#ifdef USE_ACCUMULATION
		num = 2;
		Int_InitValues[0][0] = 1;
		Int_InitValues[0][1] = iAcqAccMax;
		Int_InitValues[1][0] = 2;
		Int_InitValues[1][1] = iAcqAccShift;
#else //USE_ACCUMULATION
		num = 1;
		Int_InitValues[0][0] = 0;
		Int_InitValues[0][1] = 0;
#endif //USE_ACCUMULATION

		return Flash_SaveConfig(idx, &Int_InitValues[0][0], num);
	}
}

void ForceGetADIData(uint16_t bankNum, uint16_t * pData) {
	//1. Poll the Bank_Status register (Address 0xF6) until
	//   register reads 0x1 or 0x2. This means data is ready at
	//   that bank.
	// N/A

	//2. (TC1 ONLY) Write bit[1] of register address 0xF1 to 0x1
	//WriteParamToSPI(UndocumentedF1Address, 0x01B0 | 0x0002);

	//3. Immediately write to the corresponding bit in the
	//   SRAM_READ register bit 0 for bank0 and bit 1 for
	//   bank1 (Address 0x3). This will tell the AFE you intend
	//   to transfer that data.
	WriteParamToSPI(DataControlAddress, bankNum);

	//4. Read 1600 words from the SRAM by performing the
	//   SRAM Read operation which is detailed in the figure
	//   below. The SPI FSM will be locked into operation until
	//   all 1600 words are read.
	ReadDataFromSPI(pData, FRAME_NUM_PTS);

	//5. (TC1 ONLY) Write bit[1] of register address 0xF1 to 0x0
	//WriteParamToSPI(UndocumentedF1Address, 0x01B0);

	//6. Write 0x0 to the SRAM_READ register (Address 0x3)
	//   to disengage the transfer intent.
	WriteParamToSPI(DataControlAddress, 0x0000);
}

/**
 * @brief Clear the sram by reading both banks
 */
int16_t AcqFifoReset[FRAME_NUM_PTS];

static void ClearSram(void) {

	ForceGetADIData(0x01, (uint16_t*) AcqFifoReset);
	ForceGetADIData(0x02, (uint16_t*) AcqFifoReset);

}

/**
 * @brief Retreive the data from the ADI device
 */

void Lidar_GetADIData(uint16_t *pBank, uint16_t * pData) {
	uint16_t bankStatus = 0;

	*pBank = 0;
	ReadParamFromSPI(BankStatusAddress, &bankStatus);
	if (bankStatus)
	{
		//1. Poll the Bank_Status register (Address 0xF6) until
		//   register reads 0x1 or 0x2. This means data is ready at
		//   that bank.
		*pBank = bankStatus;

		//2. (TC1 ONLY) Write bit[1] of register address 0xF1 to 0x1
		//WriteParamToSPI(UndocumentedF1Address, 0x01B0 | 0x0002);

        //3. Immediately write to the corresponding bit in the
		//   SRAM_READ register bit 0 for bank0 and bit 1 for
		//   bank1 (Address 0x3). This will tell the AFE you intend
		//   to transfer that data.
		WriteParamToSPI(DataControlAddress, bankStatus);

		//4. Read 1600 words from the SRAM by performing the
		//   SRAM Read operation which is detailed in the figure
		//   below. The SPI FSM will be locked into operation until
		//   all 1600 words are read.
		ReadDataFromSPI(pData, FRAME_NUM_PTS);

		//5. (TC1 ONLY) Write bit[1] of register address 0xF1 to 0x0
		//WriteParamToSPI(UndocumentedF1Address, 0x01B0);

		//6. Write 0x0 to the SRAM_READ register (Address 0x3)
		//   to disengage the transfer intent.
		WriteParamToSPI(DataControlAddress, 0x0000);
	}
}

#ifdef USE_FAKE_DATA
void GetADIData_Start(uint16_t *pBank, uint16_t * pData);
bool GetADIData_Check(void);

#else //USE_FAKE_DATA

void GetADIData_Start(uint16_t *pBank, uint16_t * pData) {


	uint16_t bankStatus = 0;

	*pBank = 0;
	//TODO DEBUG
	ReadParamFromSPI(BankStatusAddress, &bankStatus);
	if (bankStatus)
	{
		//1. Poll the Bank_Status register (Address 0xF6) until
		//   register reads 0x1 or 0x2. This means data is ready at
		//   that bank.
		*pBank = bankStatus;

		//2. (TC1 ONLY) Write bit[1] of register address 0xF1 to 0x1
		//WriteParamToSPI(UndocumentedF1Address, 0x01B0 | 0x0002);

        //3. Immediately write to the corresponding bit in the
		//   SRAM_READ register bit 0 for bank0 and bit 1 for
		//   bank1 (Address 0x3). This will tell the AFE you intend
		//   to transfer that data.
		WriteParamToSPI(DataControlAddress, bankStatus);

		//4. Read 1600 words from the SRAM by performing the
		//   SRAM Read operation which is detailed in the figure
		//   below. The SPI FSM will be locked into operation until
		//   all 1600 words are read.
		ReadDataFromSPI_Start(pData, FRAME_NUM_PTS);
	}
}

bool GetADIData_Check(void)
{
	return ReadDataFromSPI_Check();
}
#endif //USE_FAKE_DATA

void GetADIData_Stop(void)
{
#ifdef USE_DMA
	ADI_SPI_RESULT result;

	/* Disable DMA */
	result = adi_spi_EnableDmaMode(hSpi, false);
#endif //USE_DMA

	//5. (TC1 ONLY) Write bit[1] of register address 0xF1 to 0x0
	//WriteParamToSPI(UndocumentedF1Address, 0x01B0);

	//6. Write 0x0 to the SRAM_READ register (Address 0x3)
	//   to disengage the transfer intent.
	WriteParamToSPI(DataControlAddress, 0x0000);
}

void Lidar_Trig(void)
{
	uint16_t data;

	ReadParamFromSPI(Control1Address, &data);

	WriteParamToSPI(Control1Address, data | 0x4000);
}

void Lidar_SPITriggerMode(void)
{
	uint16_t data;

	ReadParamFromSPI(Control1Address, &data);

	WriteParamToSPI(Control1Address, (data & ~0x4000) & ~0x8000);
}

void Lidar_FreerunMode(void)
{
	uint16_t data;

	ReadParamFromSPI(Control1Address, &data);

	WriteParamToSPI(Control1Address, (data & ~0x4000) | 0x8000);
}

void Lidar_ChannelEnable(int ch, int enable)
{
	uint16_t data;

	ReadParamFromSPI(ChannelEnableAddress, &data);

	if (enable)
		data |=  (1 << ch);
	else
		data &= ~(1 << ch);

    WriteParamToSPI(ChannelEnableAddress, data);
}

void Lidar_ChannelTIAFeedback(int ch, uint16_t feedback)
{
	uint16_t data;
	uint16_t temp = 0;

	ReadParamFromSPI(CH0ControlReg1Address + ch * 4, &data);

	data &= ~0x00FF;
	temp = (uint8_t) feedback;
	data |= temp;

    WriteParamToSPI(CH0ControlReg1Address + ch * 4, data);
}

void Lidar_ChannelDCBal(int ch, uint16_t bal)
{
	uint16_t data;

	ReadParamFromSPI(CH0ControlReg2Address + ch * 4, &data);

	data &= ~0x01FF;
	data |= bal;

    WriteParamToSPI(CH0ControlReg2Address + ch * 4, data);
}

void Lidar_FlashGain(uint16_t flashGain)
{
	uint16_t data;

	ReadParamFromSPI(Control0Address, &data);

	data &= ~0xFF10;
	data |= (flashGain << 7) & 0xFF10;

    WriteParamToSPI(Control0Address, data);
}



//
// Simulation
//

void AddFakeData(void)
{
	static uint16_t dist = 1000;

	user_CANFifoPushDetection(0, dist, 100);
	user_CANFifoPushDetection(1, dist + 250, 100);
	user_CANFifoPushDetection(2, dist + 500, 100);
	user_CANFifoPushDetection(3, dist + 650, 100);
	user_CANFifoPushDetection(4, dist + 650, 100);
	user_CANFifoPushDetection(5, dist + 500, 100);
	user_CANFifoPushDetection(6, dist + 250, 100);
	user_CANFifoPushDetection(7, dist, 100);

	user_CANFifoPushCompletedFrame();

	dist += 100;

	if (dist > 2500)
		dist = 500;
}



//
// Read/Write FIFO
//

volatile int iReadWriteFifoHead = 0;
volatile int iReadWriteFifoTail = 0;

#define READWRITEFIFO_SIZE 8
#define READWRITEFIFO_MASK (READWRITEFIFO_SIZE - 1)
uint32_t readWriteFifo[READWRITEFIFO_SIZE];


int Lidar_ReadFifoPush(uint16_t _startAddress)
{
	int nextHead = (iReadWriteFifoHead + 1) & READWRITEFIFO_MASK;

	if (nextHead != iReadWriteFifoTail)
	{
		readWriteFifo[iReadWriteFifoHead] = ((uint32_t)(_startAddress & ~RW_WRITE_MASK)) << 16;
		iReadWriteFifoHead = nextHead;

		return 1;
	}

	return 0;
}

int Lidar_WriteFifoPush(uint16_t _startAddress, uint16_t data)
{
	int nextHead = (iReadWriteFifoHead + 1) & READWRITEFIFO_MASK;

	if (nextHead != iReadWriteFifoTail)
	{
		readWriteFifo[iReadWriteFifoHead] = ((uint32_t)(RW_WRITE_MASK | (_startAddress & ~RW_WRITE_MASK)) << 16) |
				data;
		iReadWriteFifoHead = nextHead;

		return 1;
	}

	return 0;
}



#ifdef USE_ACCUMULATION

uint16_t iAcqAccNum = 0;
int32_t AcqFifoAcc[FRAME_NUM_PTS];

uint16_t GetNextPowerOfTwo(uint16_t v)
{
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  //v |= v >> 16;
  v++;

  return v;
}

bool IsPowerOfTwo(uint16_t x)
{
  return (x != 0) && ((x & (x - 1)) == 0);
}

int GetMSBit(uint16_t n)
{
  int pos = 0;
  while (n)
  {
    n = n >> 1;
    pos++;
  }
  return pos;
}


bool DoAccumulation(int16_t * pAcqFifo)
{
	bool bAccDone = false;

	if (bAcqAccUpdate)
	{
    if (iAcqAccMaxNew == 0)
      iAcqAccMaxNew = 1;

    if (!IsPowerOfTwo(iAcqAccMaxNew))
    {
      iAcqAccMaxNew = GetNextPowerOfTwo(iAcqAccMaxNew);
    }

		iAcqAccMax = iAcqAccMaxNew;
		iAcqAccShift = GetMSBit(iAcqAccMaxNew) - 1;

		iAcqAccNum = 0;
		bAcqAccUpdate = 0;
	}

	if (iAcqAccMax == 0 || iAcqAccMax == 1)
	{
        // Special case: no accumulation
        bAccDone = true;
	}
	else if (iAcqAccNum == 0)
    {
        // First accumulation
        int i;
        for(i=0; i<FRAME_NUM_PTS; i++)
        {
            AcqFifoAcc[i] = pAcqFifo[i];
        }

        ++iAcqAccNum;
    }
    else if ((iAcqAccNum + 1) >= iAcqAccMax) // >= just in case
    {
        // Last accumulation
        int i;
        for(i=0; i<FRAME_NUM_PTS; i++)
        {
            int32_t data = AcqFifoAcc[i] + pAcqFifo[i];
            pAcqFifo[i] = (int16_t) (data >> iAcqAccShift);
        }

        iAcqAccNum = 0;
        bAccDone = true;
    }
    else //if (iAcqAccNum < iAcqAccMax)
	{
        // Intermediate accumulations
		int i;
		for(i=0; i<FRAME_NUM_PTS; i++)
		{
			AcqFifoAcc[i] = AcqFifoAcc[i] + pAcqFifo[i];
		}

		++iAcqAccNum;
	}

    return bAccDone;
}
#endif //USE_ACCUMULATION



#ifdef USE_ALGO

static float tmpAcqFloat[GUARDIAN_SAMPLING_LENGTH];

int DoAlgo(int16_t * pAcqFifo)
{
	int ch;
    detection_type detections[GUARDIAN_NUM_CHANNEL * GUARDIAN_NUM_DET_PER_CH];

    for(ch=0; ch<GUARDIAN_NUM_CHANNEL; ++ch)
    {
        int i;


        int chIdxArray = aChIdxArray[ch];

        int chIdx = aChIdxADI[chIdxArray];

        detection_type* pDetections = &detections[ch * GUARDIAN_NUM_DET_PER_CH];

        for(i=0; i<GUARDIAN_SAMPLING_LENGTH; ++i)
            tmpAcqFloat[i] = (float) pAcqFifo[chIdx * GUARDIAN_SAMPLING_LENGTH + i];

        threshold2(pDetections, tmpAcqFloat, ch);

        if (pDetections->distance)
        {
            user_CANFifoPushDetection(ch, (uint16_t) (pDetections->distance * 100.0), 0);
        }
    }

    return 0;
}
#endif //USE_ALGO



inline int ProcessReadWriteFifo(void)
{
	uint32_t op = readWriteFifo[iReadWriteFifoTail];
	uint16_t addrPlusRW = op >> 16;
	uint16_t addr = addrPlusRW & ~RW_WRITE_MASK;
	uint16_t data = op & 0xFFFF;
	bool bWriteOp = ((addrPlusRW & RW_WRITE_MASK) != 0);

	if (op & 0x80000000)
	{
		switch (addr)
		{
    case 0x3FFE:
			SaveConfigToFlash(0);
			break;
		case 0x3FFF:
			Flash_ResetToFactoryDefault(0);
			//Lidar_Reset();
			break;
		case 0x4000:
			//Lidar_Reset();
			break;
#ifdef USE_ACCUMULATION
		case 0x4001:
			iAcqAccMaxNew = data;
			bAcqAccUpdate = 1;
			break;
#endif //USE_ACCUMULATION
    case 0x7FFE:
			SaveConfigToFlash(1);
			break;
		case 0x7FFF:
			Flash_ResetToFactoryDefault(1);
			//Lidar_Reset();
			break;
		default:
			WriteParamToSPI(addr, data);
			break;
		}
	}
	else
	{
		switch (addr)
		{
#ifdef USE_ACCUMULATION
		case 0x4001:
			data = iAcqAccMax;
			break;
#endif //USE_ACCUMULATION
		default:
			ReadParamFromSPI(addr, &data);
			break;
		}

		user_CANFifoPushReadResp(addr, data);
	}

	iReadWriteFifoTail = (iReadWriteFifoTail + 1) & READWRITEFIFO_MASK;

	return 0;
}



//
// Acquisition control
//

int gAcq = true;

static uint16_t BankInTransfer = 0;

volatile int iFifoHead = 0;
volatile int iFifoTail = 0;

#define NUM_FIFO			4
#define NUM_FIFO_MASK		(NUM_FIFO-1)

static tDataFifo dataFifo[NUM_FIFO];


void Lidar_Acq(uint16_t *pBank)
{
	*pBank = 0;

	if (BankInTransfer == 0)
	{
		if (iReadWriteFifoHead != iReadWriteFifoTail)
			ProcessReadWriteFifo();

		if (gAcq)
			GetADIData_Start(&BankInTransfer, (uint16_t*) dataFifo[iFifoHead].AcqFifo);
	}
	else
	{
		if (GetADIData_Check())
		{
			bool bAccDone = true;
			int16_t * pAcqFifo = dataFifo[iFifoHead].AcqFifo;

			*pBank = BankInTransfer;
			BankInTransfer = 0;

			GetADIData_Stop();

#ifdef USE_ACCUMULATION
			bAccDone = DoAccumulation(pAcqFifo);
#endif //USE_ACCUMULATION

			if (bAccDone)
			{
#ifdef USE_ALGO
				DoAlgo(pAcqFifo);
#endif //USE_ALGO
			    user_CANFifoPushCompletedFrame();
			}

            if (bAccDone)
            {
				int iFifoHeadNext = (iFifoHead + 1) & NUM_FIFO_MASK;

				if (iFifoHeadNext != iFifoTail)
				{
					iFifoHead = iFifoHeadNext;
				}
            }
		}
	}
}

void Lidar_GetDataFromFifo(tDataFifo ** pDataPtr, uint16_t * pNumFifo)
{
	if (iFifoHead != iFifoTail)
	{
		if (iFifoTail < iFifoHead)
			*pNumFifo = iFifoHead - iFifoTail;
		else
			*pNumFifo = NUM_FIFO - iFifoTail;

		*pDataPtr = &dataFifo[iFifoTail];
	}
	else
	{
		*pDataPtr = NULL;
	    *pNumFifo = 0;
	}
}

void Lidar_ReleaseDataToFifo(uint16_t numFifo)
{
	iFifoTail = (iFifoTail + numFifo) & NUM_FIFO_MASK;
//	LED5_OFF();
}

void Lidar_Reset(void)
{
  user_CANFifoReset();

	Lidar_InitADI();

	iFifoHead = 0;
	iFifoTail = 0;

	BankInTransfer = 0;

#ifdef USE_ACCUMULATION
	iAcqAccNum = 0;
#endif //USE_ACCUMULATION
}



//
// USB
//

int iUSBnum = 0;
int iUSBnumOK = 0;
int iUSBnumEmpty = 0;


#ifdef USE_FAKE_DATA

//
// Fake data
//

const int16_t fakeData[2][GUARDIAN_SAMPLING_LENGTH + 32] =
{
{
	-153,
	381,
	1261,
	347,
	-993,
	1215,
	-1201,
	-857,
	250,
	1594,
	762,
	1144,
	1091,
	719,
	-366,
	-719,
	-57,
	794,
	-1132,
	-1575,
	-1445,
	-1465,
	-540,
	1270,
	187,
	95,
	-179,
	-1379,
	-1344,
	530,
	-892,
	-1421,
	-1539,
	559,
	779,
	-968,
	-2000,
	-8000,
	-12000,
	-13000,
	-12000,
	-8000,
	-2000,
	53,
	1365,
	1127,
	-1453,
	-1490,
	-345,
	-823,
	-1627,
	663,
	-139,
	1546,
	201,
	-1390,
	-1323,
	177,
	1256,
	1134,
	468,
	326,
	256,
	-1617,
	1151,
	425,
	-700,
	377,
	1153,
	-2000,
	-4000,
	-5000,
	-5500,
	-5000,
	-4000,
	-2000,
	-234,
	663,
	-456,
	778,
	201,
	38,
	-1263,
	1141,
	-1091,
	-1339,
	-1145,
	27,
	-478,
	1158,
	1271,
	-1512,
	474,
	-1328,
	-1019,
	1588,
	-633,
	-407,
	-1184,
	-1204
	,
	-1091,
	-1339,
	-1145,
	27,
	-478,
	1158,
	1271,
	-1512,
	474,
	-1328,
	-1019,
	1588,
	-633,
	-407,
	-1184,
	-1204,
	-1091,
	-1339,
	-1145,
	27,
	-478,
	1158,
	1271,
	-1512,
	474,
	-1328,
	-1019,
	1588,
	-633,
	-407,
	-1184,
	-1204

},
{
203 ,
928  ,
1359 ,
-1524,
-323 ,
-805 ,
-1157,
-1545,
-1352,
14   ,
114  ,
1329 ,
592  ,
-1077,
-1218,
-231 ,
-865 ,
-850 ,
-1474,
632  ,
796  ,
1465 ,
-355 ,
-1600,
114  ,
-646 ,
1434 ,
-751 ,
1235 ,
858  ,
853  ,
-764 ,
538  ,
-1024,
-1241,
660  ,
-2000,
-8000,
-12000,
-13000,
-12000,
-8000,
-2000,
841  ,
-637 ,
693  ,
442  ,
1430 ,
-520 ,
1182 ,
-1490,
-1502,
-857 ,
-324 ,
507  ,
-1391,
516  ,
962  ,
-1633,
-1522,
-153 ,
-1334,
1422 ,
-30  ,
1457 ,
-245 ,
229  ,
-638 ,
-830 ,
-2000,
-4000,
-5000,
-5500,
-5000,
-4000,
-2000,
1501 ,
-752 ,
1600 ,
93   ,
-81  ,
-751 ,
1092 ,
-229 ,
276  ,
-516 ,
-103 ,
647  ,
-1058,
-1586,
-9   ,
1149 ,
206  ,
-1362,
687  ,
-1612,
-802 ,
-404 ,
1173 ,
293
,
276  ,
-516 ,
-103 ,
647  ,
-1058,
-1586,
-9   ,
1149 ,
206  ,
-1362,
687  ,
-1612,
-802 ,
-404 ,
1173 ,
293,
276  ,
-516 ,
-103 ,
647  ,
-1058,
-1586,
-9   ,
1149 ,
206  ,
-1362,
687  ,
-1612,
-802 ,
-404 ,
1173 ,
293

}

};

#include "cld_bf70x_bulk_lib.h"

int iFakeData_offsetPerCycles = 0;
int iFakeData_offsetPerChannel = 0;

void GetADIData_Start(uint16_t *pBank, uint16_t * pData)
{
	static CLD_Time acq_time = 0;
	static uint16_t nextBank = 0;

	static CLD_Time offsetPerCycles = 0;

	if (cld_time_passed_ms(acq_time) >= 500u)
	{
		int ch;

		acq_time = cld_time_get();

		*pBank = nextBank + 1;

		for(ch=0; ch<GUARDIAN_NUM_CHANNEL; ch++)
		{
			int offset = (iFakeData_offsetPerChannel) ? offsetPerCycles + ch : offsetPerCycles;
			memcpy(pData + ch * 100, &fakeData[nextBank][offset], GUARDIAN_SAMPLING_LENGTH * sizeof(int16_t));
		}

		nextBank = (nextBank + 1) & 0x1;
	}

	if (iFakeData_offsetPerCycles)
	{
		iFakeData_offsetPerCycles = 0;
		offsetPerCycles = (offsetPerCycles + 1) & (32-1);
	}
}

bool GetADIData_Check(void)
{
	return 1;
}
#endif //USE_FAKE_DATA

