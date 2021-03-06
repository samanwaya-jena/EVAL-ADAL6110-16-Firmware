/*
  Copyright 2018 Phantom Intelligence Inc.
*/

//#define USE_DMA


#include <stdint.h>
#include <string.h>

#include <ADSP-BF707_device.h>
#include <drivers\spi\adi_spi.h>

//#include "Flash/flash_params.h"
#include "demo_app.h"
#include "adal6110_16.h"
//#include "Communications/user_bulk.h"
#include "Communications/USB_cmd.h"
#include "Communications/cld_bf70x_bulk_lib.h"
#include "post_debug.h"
#include "parameters.h"
//#include "parameters_default_values.h"
#include "algo.h"


/**
 * Private constant and variables
 */
uint16_t frame_ID;
static uint16_t BankInTransfer = 0;
static int16_t pAcqData[1682];

uint16_t ADAL_POR_Values[][2] =
{
		// According with Datasheet p.19
		{ Control1Address, 0x0040 }, // bit6 in control 1 is a startup register
		{ TriggerOutAddress, 0x1011 }, //12 ns
		{ Control3Adress, 0x00 },
		{ 0x0009, 0x7A4F },            //startup 2  (set to 0x7A5F for 2ns mode)
		{ 0x004D, 0xC23F },            //startup 3  (set to 0xC23F for signal stability)
		{ 0x0054, 0x2AAA },            //startup 4
		{ 0x0056, 0xC23F },            //startup 5  (set to 0xC23F for signal stability)
		{ 0x005D, 0x2AAA },            //startup 6
		{ 0x005F, 0x8FCC },            //startup 7
		{ 0x0060, 0x8FCC },            //startup 8
		{ 0x0063, 0x8FCC },            //startup 9
		{ 0x0062, 0x8FCC },            //startup 10
		{ 0x0063, 0x8FCC },            //startup 11
		{ 0x0064, 0x8FCC },            //startup 12
		{ 0x0065, 0x8FCC },            //startup 13
		{ 0x0066, 0x8FCC },            //startup 14
		{ 0x0081, 0x0001 },            //startup 15
		{ 0x0083, 0x0A40 },            //startup 16  (0x0840 in datasheet)
		{ 0x0084, 0xC201 },            //startup 17  (0x0420 in datasheet)
		{ 0x0090, 0x0420 },            //startup 18
		{ 0x0091, 0x87B4 }             //startup 19
};

uint16_t ADAL_Init_Values[][2] =
{
	{ LFSRSEEDL, 0x9190 },
	{ LFSRSEEDH, 0x0001 },
	{ Control0Address, 0x1000 },    // 8 accum = 0x0400, 16 accum = 0x0800 // 63 accum = 0x1F80 (ne pas d�passer)
	{ Control1Address, 0x8040 },
	{ TriggerOutAddress, 0x1011 }, //2ns: 0x8021, 4ns: 0x1021  // 0x1011 = 16 ns pulse in 4ns mode
	{ DataAcqMode, 0x0001 },       //2ns: 0x0000,  4ns :0x0001
	{ DelayBetweenFlashesAddress, 0x5ED8}, //0x5ED8 (1 frame period = 10 ms => 100 Hz @ 32 accum)

	{ CH0ControlReg0Address, 0x2E1F },
	{ CH1ControlReg0Address, 0x2E1F },
	{ CH2ControlReg0Address, 0x2E1F },
	{ CH3ControlReg0Address, 0x2E1F },
	{ CH4ControlReg0Address, 0x2E1F },
	{ CH5ControlReg0Address, 0x2E1F },
	{ CH6ControlReg0Address, 0x2E1F },
	{ CH7ControlReg0Address, 0x2E1F },
	{ CH8ControlReg0Address, 0x2E1F },
	{ CH9ControlReg0Address, 0x2E1F },
	{ CH10ControlReg0Address, 0x2E1F },
	{ CH11ControlReg0Address, 0x2E1F },
	{ CH12ControlReg0Address, 0x2E1F },
	{ CH13ControlReg0Address, 0x2E1F },
	{ CH14ControlReg0Address, 0x2E1F },
	{ CH15ControlReg0Address, 0x2E1F },

	{ CH0ControlReg1Address, 0x0B80},
	{ CH1ControlReg1Address, 0x0B80},
	{ CH2ControlReg1Address, 0x0B80},
	{ CH3ControlReg1Address, 0x0B80},
	{ CH4ControlReg1Address, 0x0B80},
	{ CH5ControlReg1Address, 0x0B80},
	{ CH6ControlReg1Address, 0x0B80},
	{ CH7ControlReg1Address, 0x0B80},
	{ CH8ControlReg1Address, 0x0B80},
	{ CH9ControlReg1Address, 0x0B80},
	{ CH10ControlReg1Address, 0x0B80},
	{ CH11ControlReg1Address, 0x0B80},
	{ CH12ControlReg1Address, 0x0B80},
	{ CH13ControlReg1Address, 0x0B80},
	{ CH14ControlReg1Address, 0x0B80},
	{ CH15ControlReg1Address, 0x0B80},

	{ CH0ControlReg2Address, 0x00FF },
	{ CH1ControlReg2Address, 0x00FF },
	{ CH2ControlReg2Address, 0x00FF },
	{ CH3ControlReg2Address, 0x00FF },
	{ CH4ControlReg2Address, 0x00FF },
	{ CH5ControlReg2Address, 0x00FF },
	{ CH6ControlReg2Address, 0x00FF },
	{ CH7ControlReg2Address, 0x00FF },
	{ CH8ControlReg2Address, 0x00FF },
	{ CH9ControlReg2Address, 0x00FF },
	{ CH10ControlReg2Address, 0x00FF },
	{ CH11ControlReg2Address, 0x00FF },
	{ CH12ControlReg2Address, 0x00FF },
	{ CH13ControlReg2Address, 0x00FF },
	{ CH14ControlReg2Address, 0x00FF },
	{ CH15ControlReg2Address, 0x00FF },

	{ FRAMEDELAY, 0xFFF8 }, // Slow it down to max
	{ AGCDCBCTRL, 0x0014 }, // Default: 0x0104, Enable AGC to change anything: 0x0115 //might be changed to 0x0000 according to wag-214 code

	{ Control3Adress, 0 },
	{ ChannelEnableAddress, 0xFFFF },
	{ AGCEN, 0x0000 },
	{ DCEN, 0xFFFF },
	{ Control3Adress, 1 }
};


#ifdef USE_DMA
uint8_t spiMem[ADI_SPI_DMA_MEMORY_SIZE];
#else //USE_DMA
uint8_t spiMem[ADI_SPI_INT_MEMORY_SIZE];
#endif //USE_DMA

ADI_SPI_HANDLE hSpi = NULL;

//
// USB stat
//

int iUSBnum = 0;       // log the number of end of frame
int iUSBnumCooked = 0; // number of detection sent
int iUSBnumRaw = 0;    // number of raw frame sent

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

	if (_startAddress == DelayBetweenFlashesAddress)
		if(_data < 500) _data = 500; // limit the laser frequency to 20kHz
	if (_startAddress == AGCEN || _startAddress == DCEN || _startAddress == ChannelEnableAddress)
		ADAL_WriteParamToSPI(Control3Adress,0);

	ProBuffer1[0] = (_startAddress << 3);
	ProBuffer1[1] = (_startAddress << 3) >> 8;
	ProBuffer1[2] = (_data >> 0);
	ProBuffer1[3] = (_data >> 8);

	ADI_SPI_TRANSCEIVER Xcv0  = {ProBuffer1, 4u, NULL, 0u, NULL, 0u};
	ADI_SPI_RESULT result = adi_spi_ReadWrite(hSpi, &Xcv0);

	if (_startAddress == AGCEN || _startAddress == DCEN || _startAddress == ChannelEnableAddress)
		ADAL_WriteParamToSPI(Control3Adress,1);
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

/**
 *
 *  Public API
 *
 *
 *
 */

void ADAL_Reset(void)
{
	// TODO: Drive the reset pin when available
	//We dont have reset ctrl. Need to wait a certain delay
	//Use software reset for now
	uint16_t reg = 0;
	uint32_t i = 600000; // 1.5ms @ 400mhz

	//READ Control0Address
	ADAL_ReadParamFromSPI(Control0Address, &reg);

	ADAL_WriteParamToSPI(Control0Address, reg |= 0x0001); //SET BIT 0
	for(int time=i;time;time--);

	ADAL_WriteParamToSPI(Control0Address, reg &= 0xFFFE);//CLEAR Bit 0
	for(int time=i;time;time--);
}


void ADAL_Start(void)
{
	uint16_t reg;

	ADAL_ReadParamFromSPI(Control0Address, &reg);
	ADAL_WriteParamToSPI(Control0Address, reg |= 0x0002); //SET BIT 0
}


void ADAL_Stop(void)
{
	uint16_t reg;

	ADAL_ReadParamFromSPI(Control0Address, &reg);
	ADAL_WriteParamToSPI(Control0Address, reg &= ~0x0002); //SET BIT 0
}


/**
 * @brief initialize SPI port for ADI communication
 */
uint16_t ADAL_SPI_init(void)
{
	ADI_SPI_RESULT result;

	if (hSpi == NULL)
	{
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
		//to have a 50 MHz SPI on a 100 MHz sclk, the divider should be set to 1 (100/(2+1) = 50)
		result = adi_spi_SetClock( hSpi, 1);//9);
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
	return((uint16_t) result);
}

void ADAL_InitADI(void) {
    int i;
    uint16_t dataToBeRead = 0;
    uint32_t waitTimer = 80000; //wait 200us @ 400mhz

    ADAL_SPI_init();

	BankInTransfer = 0;
	frame_ID = 0;

	int num = sizeof(ADAL_POR_Values) / sizeof(ADAL_POR_Values[0]);
	for (i=0; i<num; i++)
		ADAL_WriteParamToSPI(ADAL_POR_Values[i][0], ADAL_POR_Values[i][1]);

  	dataToBeRead = 0;
  	while(!dataToBeRead) {                    // Wait until valid clock (TC_STATUS bit 4)
  		ADAL_ReadParamFromSPI(TC_STATUS, &dataToBeRead);
  		if (dataToBeRead == 0xffff){
  			dataToBeRead = 0;
  		}
  		dataToBeRead = dataToBeRead & 8;
  	}

	num = sizeof(ADAL_Init_Values) / sizeof(ADAL_Init_Values[0]);
	for (i=0; i<num; i++)
		ADAL_WriteParamToSPI(ADAL_Init_Values[i][0], ADAL_Init_Values[i][1]);

	ADAL_Reset();
	ADAL_Start();
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
static void ClearSram(void)
{
	ForceGetADIData(0x01, (uint16_t*) pAcqData);
	ForceGetADIData(0x02, (uint16_t*) pAcqData);
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

void ADAL_ClearTrig(void)
{
	uint16_t data;

	ADAL_ReadParamFromSPI(Control1Address, &data);
	if (!(data&0x8000)) // trig mode?
		ADAL_WriteParamToSPI(Control1Address, data & ~0x4000); // reset the trigger bit
}

void ADAL_SPITriggerMode(void)
{
	uint16_t data;

	ADAL_ReadParamFromSPI(Control1Address, &data);
	ADAL_WriteParamToSPI(Control1Address, (data & ~0x4000) & ~0x8000); // Clear bit 15 and 14
}

void ADAL_FreerunMode(void)
{
	uint16_t data;
	ADAL_ReadParamFromSPI(Control1Address, &data);
	ADAL_WriteParamToSPI(Control1Address, (data & ~0x4000) | 0x8000); // clear bit 14, set bit 15
}
/*
void ADAL_ChannelEnable(int ch, int enable)
{
	uint16_t data;

	ADAL_ReadParamFromSPI(ChannelEnableAddress, &data);
	if (enable)	data |=  (1 << ch);
	else data &= ~(1 << ch);
    ADAL_WriteParamToSPI(ChannelEnableAddress, data);
}
*/
void ADAL_ChannelTIAFeedback(int ch, uint16_t feedback)
{
	uint16_t data;
	if (ch>=0 && ch<16)
	{
		ADAL_ReadParamFromSPI(CH0ControlReg1Address + ch * 4, &data);
		data &= ~0x00FF;
		data |= ((uint8_t)feedback) & 0x00FF;
		ADAL_WriteParamToSPI(CH0ControlReg1Address + ch * 4, data);
	}
    else
    {
    	for (ch=0;ch<16;ch++)
    	{
    		ADAL_ReadParamFromSPI(CH0ControlReg1Address + ch * 4, &data);
			data &= ~0x00FF;
			data |= ((uint8_t)feedback) & 0x00FF;
			ADAL_WriteParamToSPI(CH0ControlReg1Address + ch * 4, data);
    	}
    }
}

void ADAL_ChannelDCBal(int ch, uint16_t bal)
{
	uint16_t data;
	if (ch>=0 && ch<16)
	{
		ADAL_ReadParamFromSPI(CH0ControlReg2Address + ch * 4, &data);
		data &= ~0x01FF;
		data |= bal & 0x01FF;
		ADAL_WriteParamToSPI(CH0ControlReg2Address + ch * 4, data);
	}
	else
	{
		for (ch=0;ch<16;ch++)
		{
			ADAL_ReadParamFromSPI(CH0ControlReg2Address + ch * 4, &data);
			data &= ~0x01FF;
			data |= bal & 0x01FF;
			ADAL_WriteParamToSPI(CH0ControlReg2Address + ch * 4, data);
		}
	}
}

void ADAL_FlashGain(uint16_t flashGain)
{
	uint16_t data;

	ADAL_ReadParamFromSPI(Control0Address, &data);
	data &= ~0x1F80;
	data |= ((flashGain & 0x003F) << 7);
    ADAL_WriteParamToSPI(Control0Address, data);

}

float ADAL_SetFrameRate(uint16_t frame_rate)
{
	uint16_t CONTROL_regval, FLASHDLY_regval, FRAMEDLY_regval,FRAMEDLY_newval;
	uint16_t flash_gain, tr_delay, flash_delay, frame_delay;
	float flash_time,frame_time,period;

	ADAL_ReadParamFromSPI(Control0Address, &CONTROL_regval);
	ADAL_WriteParamToSPI(Control0Address, CONTROL_regval&0xFFFE); // stop operations
	ADAL_ReadParamFromSPI(DelayBetweenFlashesAddress, &FLASHDLY_regval);
	ADAL_ReadParamFromSPI(FRAMEDELAY, &FRAMEDLY_regval);
	flash_gain  = (CONTROL_regval  & 0x1F80) >>7;
	tr_delay    = (CONTROL_regval  & 0x0078) >>3;
	flash_delay = (FLASHDLY_regval & 0xFFF8) >>3;
	frame_delay = (FRAMEDLY_regval & 0x7FF8) >>3;

	frame_delay = 0xFFF; // always longest delay... (200Hz max rate)
	FRAMEDLY_regval = ((frame_delay&0x0FFF)<<3)|0x8000;

	period  = 1.0/frame_rate;
	//todo: put back the upper frame rate limit to its best value, it was switched for tests -- so far 467 without USB comm...
	//todo: validate the logic of keeping 10fps as a minimum as certain flash gain will not allow this slow frame rate (minimum 23 flashes)
	period = period<0.001?0.001:period>0.1?0.1:period; //limit: 50 to 250 Hz
	frame_time = 166100e-9 + (frame_delay*100e-9);  //(uint16_t)(((frame_time - flash_time) - 166100e-9)/100e-9);
	flash_time = period - frame_time;
	flash_time /= flash_gain;     // accumulation
	flash_time -= tr_delay*4e-9;  //skip length
	flash_delay = (uint16_t)(flash_time/100e-9); //scale
	flash_delay -= 5;           // minimum length
	flash_delay = flash_delay>0x1FFF?0x1FFF:flash_delay<500?500:flash_delay; //bound between the register limit and 20kHz
	FLASHDLY_regval = (flash_delay&0x1FFF)<<3;

	period = flash_gain*((tr_delay*4e-9) + (flash_delay*100e-9) + 500e-9) + (frame_delay * 100e-9) + 166100e-9;
	//cld_console(CLD_CONSOLE_YELLOW,CLD_CONSOLE_BLACK,"p=%f, fr=%e,fl=%e, g=%d tr=%e \r\n",period,frame_time,flash_time,flash_gain,tr_delay*4e-9);
	//cld_console(CLD_CONSOLE_YELLOW,CLD_CONSOLE_BLACK,"Fr_delay=%d, Fl_delay=%d, Tr_delay=%d, Flash_gain=%d\n\r",frame_delay,flash_delay,tr_delay,flash_gain);

	ADAL_WriteParamToSPI(DelayBetweenFlashesAddress,FLASHDLY_regval);
	ADAL_WriteParamToSPI(FRAMEDELAY,FRAMEDLY_regval);
	ADAL_WriteParamToSPI(Control0Address, CONTROL_regval); // restart with previous values

	return ( 1/period );
}

int ADAL_SetPulseWidth(uint16_t width)
{
	uint16_t regval;

	ADAL_ReadParamFromSPI(TriggerOutAddress, &regval);

	regval &= 0xFFC7; //reset bits 5..3

	if(width < 10) regval |= 0x0000;
	else if (width < 14) regval |= 0x0008;
	else if (width < 18) regval |= 0x0010;
	else if (width < 22) regval |= 0x0018;
	else regval |= 0x0020;

	ADAL_WriteParamToSPI(TriggerOutAddress,regval);

	return(8+((regval&0x0038)>>1));
}


//
// Simulation
//
/*
void AddFakeData(void)
{
	static uint16_t dist = 10;

	if(LiDARParameters[param_det_msg_decimation])
	{
		if (0 == frame_ID%LiDARParameters[param_det_msg_decimation])
		{
			USB_pushTrack(0, 0, 100, 44, dist, 0, 0);
			USB_pushTrack(0, 1, 100, 44, dist + 2.50, 0, 0);
			USB_pushTrack(0, 2, 100, 44, dist + 5.00, 0, 0);
			USB_pushTrack(0, 3, 100, 44, dist + 6.50, 0, 0);
			USB_pushTrack(0, 4, 100, 44, dist + 6.50, 0, 0);
			USB_pushTrack(0, 5, 100, 44, dist + 5.00, 0, 0);
			USB_pushTrack(0, 6, 100, 44, dist + 2.50, 0, 0);
			USB_pushTrack(0, 7, 100, 44, dist, 0, 0);
			USB_pushEndOfFrame(frame_ID, 0, 8);
		}
	}
	dist += 1;

	if (dist > 35)
		dist = 5;
}
*/




static float tmpAcqFloat[DEVICE_SAMPLING_LENGTH];

int DoAlgo(int16_t * pAcqFifo)
{
	int ch;
	int numDet;
    detection_type detections[DEVICE_NUM_CHANNEL * DEVICE_NUM_DET_PER_CH];
    float dist[DEVICE_NUM_CHANNEL];
    static CLD_Time last_time = 0;

    numDet = 0;
    for(ch=0; ch<DEVICE_NUM_CHANNEL; ++ch)
    {
        int i;
        int chIndex = LiDARParameters[param_channel_map_offset+ch];
        detection_type* pDetections = &detections[ch * DEVICE_NUM_DET_PER_CH];

        for(i=0; i<DEVICE_SAMPLING_LENGTH; ++i)
            tmpAcqFloat[i] = (float) pAcqFifo[ch * DEVICE_SAMPLING_LENGTH + i];

        //threshold2(pDetections, tmpAcqFloat, ch);
        //threshold3(pDetections, tmpAcqFloat, ch);
        threshold4(pDetections, tmpAcqFloat, ch);

        if (pDetections->distance && LiDARParameters[param_det_msg_decimation] && (LiDARParameters[param_det_msg_mask]&(1<<chIndex)))
		{
			if (0 == frame_ID%LiDARParameters[param_det_msg_decimation])
			{
				numDet++;
				iUSBnumCooked++;
				USB_pushTrack(numDet, chIndex , 100, pDetections->intensity, pDetections->distance, 0, 0);
			}
		}
    }
    if( (cld_time_passed_ms(last_time) > 500) && (LiDARParameters[param_console_log]&CONSOLE_MASK_DIST) )
    {
    	last_time = cld_time_get();
    	for(ch=0;ch<DEVICE_NUM_CHANNEL;ch++) dist[LiDARParameters[param_channel_map_offset+ch]]=detections[ch*DEVICE_NUM_DET_PER_CH].distance;
    	for(ch=0;ch<DEVICE_NUM_CHANNEL;ch++) cld_console(CLD_CONSOLE_CYAN,CLD_CONSOLE_BLACK,"%2d: %2.2f\t",ch,dist[ch]);
    	cld_console(CLD_CONSOLE_CYAN,CLD_CONSOLE_BLACK,"\r");
    }
    return numDet;
}



//
// Acquisition control
//


void ADAL_Acq(uint16_t *pBank)
{
	*pBank = 0;
	int numDet =0;
	int framedatasent;


	if (BankInTransfer == 0)
	{
		if (LiDARParameters[param_acq_enable])
			GetADIData_Start(&BankInTransfer, (uint16_t*) pAcqData);
	}
	else
	{
		LED_BC3R_ON();
		if (GetADIData_Check())
		{
			*pBank = BankInTransfer;
			BankInTransfer = 0;
			GetADIData_Stop();

			framedatasent = 0;
			frame_ID = (frame_ID+1) & 0xFFFF;
			if(LiDARParameters[param_raw_msg_decimation])
			{
				if (0 == frame_ID%LiDARParameters[param_raw_msg_decimation])
				{
					for(int ch=0; ch<DEVICE_NUM_CHANNEL; ++ch)
					{
						if (LiDARParameters[param_raw_msg_mask]&(1<<LiDARParameters[param_channel_map_offset+ch]))
						{
							iUSBnumRaw++;
							USB_pushRawData(LiDARParameters[param_channel_map_offset+ch], (uint16_t*) &pAcqData[ch*DEVICE_SAMPLING_LENGTH]);
							framedatasent++;
						}
					}
				}
			}
			if (LiDARParameters[param_DSP_enable])
			{
				numDet = DoAlgo(pAcqData);
				framedatasent += numDet;
			}
			if(framedatasent)//LiDARParameters[param_det_msg_decimation] || LiDARParameters[param_raw_msg_decimation])
			{
				//if (0 == frame_ID%LiDARParameters[param_det_msg_decimation] || 0 == frame_ID%LiDARParameters[param_raw_msg_decimation])
				//{
					iUSBnum++;
					USB_pushEndOfFrame(frame_ID, 0x0000, numDet);
				//}
			}
		}
		LED_BC3R_OFF();
	}
}


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

