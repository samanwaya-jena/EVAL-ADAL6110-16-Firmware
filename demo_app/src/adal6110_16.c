/*
  Copyright 2018 Phantom Intelligence Inc.
*/

//#define USE_DMA
#define USE_ALGO

#include <stdint.h>
#include <string.h>

#include <ADSP-BF707_device.h>
#include <drivers\spi\adi_spi.h>

//#include "Flash/flash_params.h"
#include "demo_app.h"
#include "adal6110_16.h"
//#include "Communications/user_bulk.h"
#include "Communications/USB_cmd.h"
#include "post_debug.h"
#include "parameters.h"

#ifdef USE_ALGO
#include "algo.h"
#endif //USE_ALGO




/**
 * Private constant and variables
 */


uint16_t frame_ID;


uint16_t Lidar_POR_Values[][2] =
{
		{ 9, 0x7A4F },                   // 0x7A5F  according to wag-214 code
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
		{132, 0xC201}                    // might not be needed according to wag-214 code
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

/*
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
*/

#ifdef USE_DMA
uint8_t spiMem[ADI_SPI_DMA_MEMORY_SIZE];
#else //USE_DMA
uint8_t spiMem[ADI_SPI_INT_MEMORY_SIZE];
#endif //USE_DMA

ADI_SPI_HANDLE hSpi = NULL;


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
void ADAL_WriteParamToSPI(uint16_t _startAddress, uint16_t _data)
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
void ADAL_ReadParamFromSPI(uint16_t _startAddress, uint16_t *_data)
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
void ADAL_ReadDataFromSPI(uint16_t * pData, int num)
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

	}

	return bAvailSpi;
}

void ResetADI(void) {

	// TODO: Drive the reset pin when available
	//We dont have reset ctrl. Need to wait a certain delay
	//Use software reset for now
	uint16_t reg = 0;
	uint32_t i = 600000; // 1.5ms @ 400mhz

	//READ Control0Address
	ADAL_ReadParamFromSPI(Control0Address, &reg);

	//CLEAR BIT 0
	reg &= 0xFFFE;
	ADAL_WriteParamToSPI(Control0Address, reg);

	while(i--){
	}

	//SET Bit 0
	reg |= 0x0001;
	ADAL_WriteParamToSPI(Control0Address, reg);
	i = 600000;

	while(i--){
	}


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
void ADAL_InitADI(void) {
    int i;
    uint16_t dataToBeRead = 0;
    uint32_t waitTimer = 80000; //wait 200us @ 400mhz


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

	int num = sizeof(Lidar_POR_Values) / sizeof(Lidar_POR_Values[0]);
	int numParams = num;

	for (i=0; i<num; i++)
	{
		ADAL_WriteParamToSPI(Lidar_POR_Values[i][0], Lidar_POR_Values[i][1]);
		//ReadParamFromSPI(Lidar_POR_Values[i][0], &dataToBeRead);
	}

	ADAL_WriteParamToSPI(CH0ControlReg0Address, 0x003F); //might not be needed according to wag-214 code
	ADAL_WriteParamToSPI(CH1ControlReg0Address, 0x003F);//might not be needed according to wag-214 code
	ADAL_WriteParamToSPI(CH2ControlReg0Address, 0x003F);//might not be needed according to wag-214 code
	ADAL_WriteParamToSPI(CH3ControlReg0Address, 0x003F);//might not be needed according to wag-214 code
	ADAL_WriteParamToSPI(CH4ControlReg0Address, 0x003F);//might not be needed according to wag-214 code
	ADAL_WriteParamToSPI(CH5ControlReg0Address, 0x003F);//might not be needed according to wag-214 code
	ADAL_WriteParamToSPI(CH6ControlReg0Address, 0x003F);//might not be needed according to wag-214 code
	ADAL_WriteParamToSPI(CH7ControlReg0Address, 0x003F);//might not be needed according to wag-214 code
	ADAL_WriteParamToSPI(CH8ControlReg0Address, 0x003F);//might not be needed according to wag-214 code
	ADAL_WriteParamToSPI(CH9ControlReg0Address, 0x003F);//might not be needed according to wag-214 code
	ADAL_WriteParamToSPI(CH10ControlReg0Address, 0x003F);//might not be needed according to wag-214 code
	ADAL_WriteParamToSPI(CH11ControlReg0Address, 0x003F);//might not be needed according to wag-214 code
	ADAL_WriteParamToSPI(CH12ControlReg0Address, 0x003F);//might not be needed according to wag-214 code
	ADAL_WriteParamToSPI(CH13ControlReg0Address, 0x003F);//might not be needed according to wag-214 code
	ADAL_WriteParamToSPI(CH14ControlReg0Address, 0x003F);//might not be needed according to wag-214 code
	ADAL_WriteParamToSPI(CH15ControlReg0Address, 0x003F);//might not be needed according to wag-214 code

  	dataToBeRead = 0;

  	while(!dataToBeRead) { // Wait until bit 4 of register 0x92 is set
  		ADAL_ReadParamFromSPI(146, &dataToBeRead);
  		if (dataToBeRead == 0xffff){
  			dataToBeRead = 0;
  		}
  		dataToBeRead = dataToBeRead & 8;
  	}

	ADAL_WriteParamToSPI(LFSRSEEDL, 0x9190);
	ADAL_WriteParamToSPI(LFSRSEEDH, 0x0001);
	ADAL_WriteParamToSPI(Control0Address, 0x1F80);    // 8 accum = 0x0400, 16 accum = 0x0800 // 64 accum = 0x1F80 (ne pas dépasser)
	ADAL_WriteParamToSPI(Control1Address, 0x8040);
	ADAL_WriteParamToSPI(TriggerOutAddress, 0x1021); //2ns: 0x8021, 4ns: 0x1021
	ADAL_WriteParamToSPI(DataAcqMode, 0x0001);       //2ns: 0x0000,  4ns :0x0001
	ADAL_WriteParamToSPI(DelayBetweenFlashesAddress, 0x4000); //0x4000 (1 frame period = 13.08 ms, 76.45 Hz MAX, measured with scope and FRAMEDELAY to minimum)

	ADAL_WriteParamToSPI(CH0ControlReg0Address, 0x3C3F);
	ADAL_WriteParamToSPI(CH1ControlReg0Address, 0x3C3F);
	ADAL_WriteParamToSPI(CH2ControlReg0Address, 0x3C3F);
	ADAL_WriteParamToSPI(CH3ControlReg0Address, 0x3C3F);
	ADAL_WriteParamToSPI(CH4ControlReg0Address, 0x3C3F);
	ADAL_WriteParamToSPI(CH5ControlReg0Address, 0x3C3F);
	ADAL_WriteParamToSPI(CH6ControlReg0Address, 0x3C3F);
	ADAL_WriteParamToSPI(CH7ControlReg0Address, 0x3C3F);
	ADAL_WriteParamToSPI(CH8ControlReg0Address, 0x3C3F);
	ADAL_WriteParamToSPI(CH9ControlReg0Address, 0x3C3F);
	ADAL_WriteParamToSPI(CH10ControlReg0Address, 0x3C3F);
	ADAL_WriteParamToSPI(CH11ControlReg0Address, 0x3C3F);
	ADAL_WriteParamToSPI(CH12ControlReg0Address, 0x3C3F);
	ADAL_WriteParamToSPI(CH13ControlReg0Address, 0x3C3F);
	ADAL_WriteParamToSPI(CH14ControlReg0Address, 0x3C3F);
	ADAL_WriteParamToSPI(CH15ControlReg0Address, 0x3C3F);

	// this block is set to 0x0B80 in documentation revB... Who's right?
	ADAL_WriteParamToSPI(CH0ControlReg1Address, 0x0180); //might be changed to 0x09A0 according to wag-214 code
	ADAL_WriteParamToSPI(CH1ControlReg1Address, 0x0180); //might be changed to 0x09A0 according to wag-214 code
	ADAL_WriteParamToSPI(CH2ControlReg1Address, 0x0180); //might be changed to 0x09A0 according to wag-214 code
	ADAL_WriteParamToSPI(CH3ControlReg1Address, 0x0180); //might be changed to 0x09A0 according to wag-214 code
	ADAL_WriteParamToSPI(CH4ControlReg1Address, 0x0180); //might be changed to 0x09A0 according to wag-214 code
	ADAL_WriteParamToSPI(CH5ControlReg1Address, 0x0180); //might be changed to 0x09A0 according to wag-214 code
	ADAL_WriteParamToSPI(CH6ControlReg1Address, 0x0180); //might be changed to 0x09A0 according to wag-214 code
	ADAL_WriteParamToSPI(CH7ControlReg1Address, 0x0180); //might be changed to 0x09A0 according to wag-214 code
	ADAL_WriteParamToSPI(CH8ControlReg1Address, 0x0180); //might be changed to 0x09A0 according to wag-214 code
	ADAL_WriteParamToSPI(CH9ControlReg1Address, 0x0180); //might be changed to 0x09A0 according to wag-214 code
	ADAL_WriteParamToSPI(CH10ControlReg1Address, 0x0180); //might be changed to 0x09A0 according to wag-214 code
	ADAL_WriteParamToSPI(CH11ControlReg1Address, 0x0180); //might be changed to 0x09A0 according to wag-214 code
	ADAL_WriteParamToSPI(CH12ControlReg1Address, 0x0180); //might be changed to 0x09A0 according to wag-214 code
	ADAL_WriteParamToSPI(CH13ControlReg1Address, 0x0180); //might be changed to 0x09A0 according to wag-214 code
	ADAL_WriteParamToSPI(CH14ControlReg1Address, 0x0180); //might be changed to 0x09A0 according to wag-214 code
	ADAL_WriteParamToSPI(CH15ControlReg1Address, 0x0180); //might be changed to 0x09A0 according to wag-214 code

  	ADAL_WriteParamToSPI(CH0ControlReg2Address, 0x00FF);
  	ADAL_WriteParamToSPI(CH1ControlReg2Address, 0x00FF);
  	ADAL_WriteParamToSPI(CH2ControlReg2Address, 0x00FF);
  	ADAL_WriteParamToSPI(CH3ControlReg2Address, 0x00FF);
  	ADAL_WriteParamToSPI(CH4ControlReg2Address, 0x00FF);
  	ADAL_WriteParamToSPI(CH5ControlReg2Address, 0x00FF);
  	ADAL_WriteParamToSPI(CH6ControlReg2Address, 0x00FF);
  	ADAL_WriteParamToSPI(CH7ControlReg2Address, 0x00FF);
  	ADAL_WriteParamToSPI(CH8ControlReg2Address, 0x00FF);
  	ADAL_WriteParamToSPI(CH9ControlReg2Address, 0x00FF);
  	ADAL_WriteParamToSPI(CH10ControlReg2Address, 0x00FF);
  	ADAL_WriteParamToSPI(CH11ControlReg2Address, 0x00FF);
  	ADAL_WriteParamToSPI(CH12ControlReg2Address, 0x00FF);
  	ADAL_WriteParamToSPI(CH13ControlReg2Address, 0x00FF);
  	ADAL_WriteParamToSPI(CH14ControlReg2Address, 0x00FF);
  	ADAL_WriteParamToSPI(CH15ControlReg2Address, 0x00FF);

  	ADAL_WriteParamToSPI(FRAMEDELAY, 0x8000); // Slow it down to max (3.86 ms gordon frame period with 64 accumulation)
  	ADAL_WriteParamToSPI(AGCDCBCTRL, 0x0014); // Default: 0x0104, Enable AGC to change anything: 0x0115 //might be changed to 0x0000 according to wag-214 code

  	ADAL_WriteParamToSPI(185, 0);
  	ADAL_WriteParamToSPI(77, 0xC23F);
  	ADAL_WriteParamToSPI(86, 0x823F);
  	ADAL_WriteParamToSPI(ChannelEnableAddress, 0xFFFF);
  	ADAL_WriteParamToSPI(AGCEN, 0x0000);
  	ADAL_WriteParamToSPI(DCEN, 0xFFFF);
  	ADAL_WriteParamToSPI(185, 1);

  	while(waitTimer--){ // Wait 200 us
  	}

  	ADAL_WriteParamToSPI(Control0Address, 0x1F81); // Set system ready to 1  //might not be needed according to wag-214 code
  	ADAL_WriteParamToSPI(Control0Address, 0x1F82); // Set system ready to 1


  	//Apply last used config from flash
  	//num = sizeof(Lidar_InitValues) / sizeof(Lidar_InitValues[0]);
	//numParams = num;

//	if(Flash_LoadConfig(0, &Lidar_InitValues[0][0], &numParams))
//	{
//		numParams = 0;
//		num= 0;
//	}
//	else
//	{
//		num = numParams;
//		for (i=0; i<num; i++)
//		{
//			WriteParamToSPI(Lidar_InitValues[i][0], Lidar_InitValues[i][1]);
//		}
//	}

}


void ForceGetADIData(uint16_t bankNum, uint16_t * pData) {
	//1. Poll the Bank_Status register (Address 0xF6) until
	//   register reads 0x1 or 0x2. This means data is ready at
	//   that bank.
	// N/A

	//2. (TC1 ONLY) Write bit[1] of register address 0xF1 to 0x1
	//ADAL_WriteParamToSPI(UndocumentedF1Address, 0x01B0 | 0x0002);

	//3. Immediately write to the corresponding bit in the
	//   SRAM_READ register bit 0 for bank0 and bit 1 for
	//   bank1 (Address 0x3). This will tell the AFE you intend
	//   to transfer that data.
	ADAL_WriteParamToSPI(DataControlAddress, bankNum);

	//4. Read 1600 words from the SRAM by performing the
	//   SRAM Read operation which is detailed in the figure
	//   below. The SPI FSM will be locked into operation until
	//   all 1600 words are read.
	ADAL_ReadDataFromSPI(pData, FRAME_NUM_PTS);

	//5. (TC1 ONLY) Write bit[1] of register address 0xF1 to 0x0
	//WriteParamToSPI(UndocumentedF1Address, 0x01B0);

	//6. Write 0x0 to the SRAM_READ register (Address 0x3)
	//   to disengage the transfer intent.
	ADAL_WriteParamToSPI(DataControlAddress, 0x0000);
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

void ADAL_GetADIData(uint16_t *pBank, uint16_t * pData) {
	uint16_t bankStatus = 0;

	*pBank = 0;
	ADAL_ReadParamFromSPI(SRAM_READY, &bankStatus);
	if (bankStatus)
	{
		//1. Poll the Bank_Status register (Address 0xF6) until
		//   register reads 0x1 or 0x2. This means data is ready at
		//   that bank.
		*pBank = bankStatus;

		//2. (TC1 ONLY) Write bit[1] of register address 0xF1 to 0x1
		//ADAL_WriteParamToSPI(UndocumentedF1Address, 0x01B0 | 0x0002);

        //3. Immediately write to the corresponding bit in the
		//   SRAM_READ register bit 0 for bank0 and bit 1 for
		//   bank1 (Address 0x3). This will tell the AFE you intend
		//   to transfer that data.
		ADAL_WriteParamToSPI(DataControlAddress, bankStatus);

		//4. Read 1600 words from the SRAM by performing the
		//   SRAM Read operation which is detailed in the figure
		//   below. The SPI FSM will be locked into operation until
		//   all 1600 words are read.
		ADAL_ReadDataFromSPI(pData, FRAME_NUM_PTS);

		//5. (TC1 ONLY) Write bit[1] of register address 0xF1 to 0x0
		//WriteParamToSPI(UndocumentedF1Address, 0x01B0);

		//6. Write 0x0 to the SRAM_READ register (Address 0x3)
		//   to disengage the transfer intent.
		ADAL_WriteParamToSPI(DataControlAddress, 0x0000);
	}
}

#ifdef USE_FAKE_DATA
void GetADIData_Start(uint16_t *pBank, uint16_t * pData);
bool GetADIData_Check(void);

#else //USE_FAKE_DATA

void GetADIData_Start(uint16_t *pBank, uint16_t * pData) {


	uint16_t bankStatus = 0;

	*pBank = 0;

	ADAL_ReadParamFromSPI(SRAM_READY, &bankStatus);
	if (bankStatus)
	{
		//1. Poll the Bank_Status register (Address 0xF6) until
		//   register reads 0x1 or 0x2. This means data is ready at
		//   that bank.
		*pBank = bankStatus;

		//2. (TC1 ONLY) Write bit[1] of register address 0xF1 to 0x1
		//ADAL_WriteParamToSPI(UndocumentedF1Address, 0x01B0 | 0x0002);

        //3. Immediately write to the corresponding bit in the
		//   SRAM_READ register bit 0 for bank0 and bit 1 for
		//   bank1 (Address 0x3). This will tell the AFE you intend
		//   to transfer that data.
		ADAL_WriteParamToSPI(DataControlAddress, bankStatus);

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
	//ADAL_WriteParamToSPI(UndocumentedF1Address, 0x01B0);

	//6. Write 0x0 to the SRAM_READ register (Address 0x3)
	//   to disengage the transfer intent.
	ADAL_WriteParamToSPI(DataControlAddress, 0x0000);
}

void ADAL_Trig(void)
{
	uint16_t data;

	ADAL_ReadParamFromSPI(Control1Address, &data);

	ADAL_WriteParamToSPI(Control1Address, data | 0x4000);
}

void ADAL_SPITriggerMode(void)
{
	uint16_t data;

	ADAL_ReadParamFromSPI(Control1Address, &data);

	ADAL_WriteParamToSPI(Control1Address, (data & ~0x4000) & ~0x8000);
}

void ADAL_FreerunMode(void)
{
	uint16_t data;

	ADAL_ReadParamFromSPI(Control1Address, &data);

	ADAL_WriteParamToSPI(Control1Address, (data & ~0x4000) | 0x8000);
}

void ADAL_ChannelEnable(int ch, int enable)
{
	uint16_t data;

	ADAL_ReadParamFromSPI(ChannelEnableAddress, &data);

	if (enable)
		data |=  (1 << ch);
	else
		data &= ~(1 << ch);

    ADAL_WriteParamToSPI(ChannelEnableAddress, data);
}

void ADAL_ChannelTIAFeedback(int ch, uint16_t feedback)
{
	uint16_t data;
	uint16_t temp = 0;

	ADAL_ReadParamFromSPI(CH0ControlReg1Address + ch * 4, &data);

	data &= ~0x00FF;
	temp = (uint8_t) feedback;
	data |= temp;

    ADAL_WriteParamToSPI(CH0ControlReg1Address + ch * 4, data);
}

void ADAL_ChannelDCBal(int ch, uint16_t bal)
{
	uint16_t data;

	ADAL_ReadParamFromSPI(CH0ControlReg2Address + ch * 4, &data);

	data &= ~0x01FF;
	data |= bal;

    ADAL_WriteParamToSPI(CH0ControlReg2Address + ch * 4, data);
}

void ADAL_FlashGain(uint16_t flashGain)
{
	uint16_t data;

	ADAL_ReadParamFromSPI(Control0Address, &data);

	data &= ~0xFF10;
	data |= (flashGain << 7) & 0xFF10;

    ADAL_WriteParamToSPI(Control0Address, data);
}



//
// Simulation
//

void AddFakeData(void)
{
	static uint16_t dist = 10;

	if(LiDARParameters[param_det_msg_decimation])
	{
		if (0 == frame_ID%LiDARParameters[param_det_msg_decimation])
		{
			USB_pushTrack(0, 0, 100, 44, dist, 0, 0);
			USB_pushTrack(0, 1, 100, 44, dist + 250, 0, 0);
			USB_pushTrack(0, 2, 100, 44, dist + 500, 0, 0);
			USB_pushTrack(0, 3, 100, 44, dist + 650, 0, 0);
			USB_pushTrack(0, 4, 100, 44, dist + 650, 0, 0);
			USB_pushTrack(0, 5, 100, 44, dist + 500, 0, 0);
			USB_pushTrack(0, 6, 100, 44, dist + 250, 0, 0);
			USB_pushTrack(0, 7, 100, 44, dist, 0, 0);
			USB_pushEndOfFrame(frame_ID, 0, 8);
		}
	}
	dist += 1;

	if (dist > 35)
		dist = 5;
}




#ifdef USE_ALGO

static float tmpAcqFloat[DEVICE_SAMPLING_LENGTH];

int DoAlgo(int16_t * pAcqFifo)
{
	int ch;
	int numDet;
    detection_type detections[DEVICE_NUM_CHANNEL * DEVICE_NUM_DET_PER_CH];

    numDet = 0;
    for(ch=0; ch<DEVICE_NUM_CHANNEL; ++ch)
    {
        int i;
        int chIdxArray = LiDARParameters[param_channel_map_offset+ch];
        int chIdx = aChIdxADI[chIdxArray];
        detection_type* pDetections = &detections[ch * DEVICE_NUM_DET_PER_CH];

        for(i=0; i<DEVICE_SAMPLING_LENGTH; ++i)
            tmpAcqFloat[i] = (float) pAcqFifo[chIdx * DEVICE_SAMPLING_LENGTH + i];

        //threshold2(pDetections, tmpAcqFloat, ch);
        threshold3(pDetections, tmpAcqFloat, ch);

        if (pDetections->distance && LiDARParameters[param_det_msg_decimation] && (LiDARParameters[param_det_msg_mask]&(1<<chIdxArray)))
		{
			if (0 == frame_ID%LiDARParameters[param_det_msg_decimation])
			{
				numDet++;
				//USB_pushTrack(0x01, chIdxArray , 100, pDetections->intensity, pDetections->distance, 0x00, 0x00);
			}
		}
    }

    return numDet;
}
#endif //USE_ALGO






//
// Acquisition control
//

static uint16_t BankInTransfer = 0;

volatile int iFifoHead = 0;
volatile int iFifoTail = 0;

#define NUM_FIFO			4
#define NUM_FIFO_MASK		(NUM_FIFO-1)

static tDataFifo dataFifo[NUM_FIFO];

void ADAL_Acq(uint16_t *pBank)
{
	*pBank = 0;
	int numDet =0;

	if (BankInTransfer == 0)
	{
		if (LiDARParameters[param_acq_enable])
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

			if (bAccDone)
			{
				frame_ID = (frame_ID+1) & 0xFFFF;
				if(LiDARParameters[param_raw_msg_decimation])
				{
					if (0 == frame_ID%LiDARParameters[param_raw_msg_decimation])
					{
						for(int ch=0; ch<DEVICE_NUM_CHANNEL; ++ch)
						{
							if (LiDARParameters[param_raw_msg_mask]&(1<<LiDARParameters[param_channel_map_offset+ch]))
							{
								USB_pushRawData(LiDARParameters[param_channel_map_offset+ch], (uint16_t*) pAcqFifo[ch*DEVICE_SAMPLING_LENGTH]);
							}
						}
					}
				}
				if (LiDARParameters[param_DSP_enable])
				{
#ifdef USE_ALGO
					numDet = DoAlgo(pAcqFifo);
#endif //USE_ALGO
					if(LiDARParameters[param_det_msg_decimation])
					{
						if (0 == frame_ID%LiDARParameters[param_det_msg_decimation])
						{
							USB_pushEndOfFrame(frame_ID, 0x0000, numDet);
						}
					}
				}
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

/*
void ADAL_GetDataFromFifo(tDataFifo ** pDataPtr, uint16_t * pNumFifo)
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

void ADAL_ReleaseDataToFifo(uint16_t numFifo)
{
	iFifoTail = (iFifoTail + numFifo) & NUM_FIFO_MASK;
}
*/

void ADAL_Reset(void)
{
	//TODO DEBUG why it reboot the cpu
	//ADAL_InitADI();

	iFifoHead = 0;
	iFifoTail = 0;

	BankInTransfer = 0;

	frame_ID = 0;
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

const int16_t fakeData[2][DEVICE_SAMPLING_LENGTH + 32] =
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

		for(ch=0; ch<DEVICE_NUM_CHANNEL; ch++)
		{
			int offset = (iFakeData_offsetPerChannel) ? offsetPerCycles + ch : offsetPerCycles;
			memcpy(pData + ch * 100, &fakeData[nextBank][offset], DEVICE_SAMPLING_LENGTH * sizeof(int16_t));
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

