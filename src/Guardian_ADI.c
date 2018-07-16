/*
 * Guardian_ADI.c
 *
 *  Created on: 2016-09-05
 *      Author: pc
 */

#include <stdint.h>
#include <string.h>

#include <drivers\spi\adi_spi.h>

#include <ADSP-BF707_device.h>

#include "Guardian_ADI.h"

//#include "Guardian_basics.h"
//#include "rawdata.h"
//#include "GPIO.h"

/**
 * Private constant and variables
 *
 */
enum ADI_REGISTER_INDEX {
	DeviceIDAddress = 0x00, // Read only
	Control0Address = 0x01,
	Control1Address = 0x02,
	DataControlAddress = 0x03,
	DelayBetweenFlashesAddress = 0x04,
	ChannelEnableAddress = 0x05,
	ReferenceControl0Address = 0x09,
	CH0ControlReg0Address = 0x0D,
	CH0ControlReg1Address = 0x0E,
	CH1ControlReg0Address = 0x11,
	CH1ControlReg1Address = 0x12,
	CH2ControlReg0Address = 0x15,
	CH2ControlReg1Address = 0x16,
	CH3ControlReg0Address = 0x19,
	CH3ControlReg1Address = 0x1A,
	CH4ControlReg0Address = 0x1D,
	CH4ControlReg1Address = 0x1E,
	CH5ControlReg0Address = 0x21,
	CH5ControlReg1Address = 0x22,
	CH6ControlReg0Address = 0x25,
	CH6ControlReg1Address = 0x26,
	CH7ControlReg0Address = 0x29,
	CH7ControlReg1Address = 0x2A,
	CH8ControlReg0Address = 0x2D,
	CH8ControlReg1Address = 0x2E,
	CH9ControlReg0Address = 0x31,
	CH9ControlReg1Address = 0x32,
	CH10ControlReg0Address = 0x35,
	CH10ControlReg1Address = 0x36,
	CH11ControlReg0Address = 0x39,
	CH11ControlReg1Address = 0x3A,
	CH12ControlReg0Address = 0x3D,
	CH12ControlReg1Address = 0x3E,
	CH13ControlReg0Address = 0x41,
	CH13ControlReg1Address = 0x42,
	CH14ControlReg0Address = 0x45,
	CH14ControlReg1Address = 0x46,
	CH15ControlReg0Address = 0x49,
	CH15ControlReg1Address = 0x4A,
	ADC0Controlreg0Address = 0x4D,
	ADC1Controlreg0Address = 0x56,
	ADCWorkingModeAddress = 0xF2,
	BankStatusAddress = 0xF6, // read only
	SRAMReadOutAddress = 0xFF // read only
};



uint8_t spiMem[ADI_SPI_INT_MEMORY_SIZE]; //ADI_SPI_DMA_MEMORY_SIZE

ADI_SPI_HANDLE hSpi = NULL;


/**
 *  @brief Writes param to SPI
 *
 *  @param uint16_t _startAddres: register address
 *  @param uint16 _data: data to send
 *
 */
void WriteParamToSPI(uint16_t _startAddress, uint16_t _data)
{
	static uint8_t ProBuffer1[4];

	ProBuffer1[0] = (_startAddress << 1);
	ProBuffer1[1] = (_startAddress << 1) >> 8;
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
	static uint8_t ProBuffer1[2];
	static uint8_t RxBuffer1[2];

	ProBuffer1[0] = (_startAddress << 1) | 0x01;
	ProBuffer1[1] = (_startAddress << 1) >> 8;

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
	ADI_SPI_RESULT result;

	/* Disable DMA */
//	result = adi_spi_EnableDmaMode(hSpi, true);

//	result = adi_spi_SetDmaTransferSize(hSpi, ADI_SPI_DMA_TRANSFER_16BIT);

	uint8_t ProBuffer1[2] = {0xFF, 0x01};
	ADI_SPI_TRANSCEIVER Xcv0DMA  = {ProBuffer1, 2u, NULL, 0u, (uint8_t*) pData, num * sizeof(uint16_t)};

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

static uint8_t gProBuffer1[2] = {0xFF, 0x01};
static ADI_SPI_TRANSCEIVER gXcv0DMA;

void ReadDataFromSPI_Start(uint16_t * pData, int num)
{
	ADI_SPI_RESULT result;

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
#if 0
	int i = 30;
// P1.3 is used as GPIO for LED indication. Macros can be find in GPIO.h
	P1_3_set_mode(OUTPUT_PP_GP);
	P1_3_set_driver_strength(STRONG);
// switch these line according to polarity
	P1_3_reset();
	while (--i) {
	}
	P1_3_set();
#endif
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
void InitADI(void) {
    int i;

    uint16_t initValues[][2] =
	{
	{ 1, 0x0282 },
	{ 2, 0x4040 },
	{ 4, 0x8000 },
	{ 7, 0x103D },
	{ 9, 0x3A50 },
	{ 10, 0x0001 },
	{ 13, 0xCE7F },//0
	{ 14, 0x0C07 },
	{ 15, 0xF0FF },
	{ 17, 0xCE1F },//1
	{ 18, 0x0C07 },
	{ 19, 0xC0FF },
	{ 21, 0xCE1F },//2
	{ 22, 0x0C07 },
	{ 23, 0xC0FF },
	{ 25, 0xCE1F },//3
	{ 26, 0x0C07 },
	{ 27, 0x00FF },
	{ 29, 0xCE1F },//4
	{ 30, 0x0C07 },
	{ 31, 0x00FF },
	{ 33, 0xCE1F },//5
	{ 34, 0x0C07 },
	{ 35, 0xC0FF },
	{ 37, 0xCE1F },//6
	{ 38, 0x0C07 },
	{ 39, 0xC0FF },
	{ 41, 0xCE7F },//7
	{ 42, 0x0C07 },
	{ 43, 0xF0FF },
	{ 45, 0xCE7F },//8
	{ 46, 0x0C07 },
	{ 47, 0xF0FF },
	{ 49, 0xCE1F },//9
	{ 50, 0x0C07 },
	{ 51, 0xC0FF },
	{ 53, 0xCE1F },//10
	{ 54, 0x0C07 },
	{ 55, 0xC0FF },
	{ 57, 0xCE1F },//11
	{ 58, 0x0C07 },
	{ 59, 0x00FF },
	{ 61, 0xCE1F },//12
	{ 62, 0x0C07 },
	{ 63, 0x00FF },
	{ 65, 0xCE1F },//13
	{ 66, 0x0C07 },
	{ 67, 0xC0FF },
	{ 69, 0xCE1F },//14
	{ 70, 0x0C07 },
	{ 71, 0xC0FF },
	{ 73, 0xCE7F },//15
	{ 74, 0x0C07 },
	{ 75, 0xF0FF },
	{ 77, 0x0237 },
	{ 78, 0x3333 },
	{ 84, 0x2772 },
	{ 85, 0x0B14 },
	{ 86, 0x0420 },
	{ 87, 0x3333 },
	{ 93, 0x2772 },
	{ 94, 0x0314 },
	{ 241, 0x01B0 },
	{ 243, 0x5040 },
	{ 244, 0x00C2 },
	{ 246, 0x0001 }
	};

    uint16_t calib[16] = {
		230,
		180,
		190,
		160,
		260,
		300,
		190,
		245,
		240,
		170,
		190,
		160,
		150,
		220,
		200,
		200
    };

	int num = sizeof(initValues) / sizeof(initValues[0]);

	if (hSpi == NULL)
	{
		ADI_SPI_RESULT result;

		result = adi_spi_Open(2, spiMem, ADI_SPI_DMA_MEMORY_SIZE, &hSpi);

		/* Set master */
		result = adi_spi_SetMaster( hSpi,true);

		/* Set slave select using hardware*/
		result = adi_spi_SetHwSlaveSelect( hSpi, false);

		result = adi_spi_SetTransmitUnderflow( hSpi, true);

		result = adi_spi_SetClockPhase(hSpi, false);

		/* Setting the clock frequency of spi   The frequency of the SPI clock is calculated by SCLK / 2* (Baud=3)*/
		result = adi_spi_SetClock( hSpi, 9);

		/* Selecting slave1 as the device*/
		result = adi_spi_SetSlaveSelect( hSpi, ADI_SPI_SSEL_ENABLE1);

		/* Setting the word size of 8 bytes*/
		result = adi_spi_SetWordSize( hSpi, ADI_SPI_TRANSFER_16BIT);

		/* No call backs required*/
	//	result = adi_spi_RegisterCallback(hSpi, NULL, NULL);

	//	result = adi_spi_SetTransceiverMode(hSpi, ADI_SPI_TXRX_MODE);

		result = adi_spi_SetClockPolarity(hSpi, true);

		if (result == 0u)
		{
			/* generate tx data interrupt when watermark level breaches 50% level */
			/* DMA watermark levels are disabled because SPI is in interrupt mode */
			result = adi_spi_SetTxWatermark(hSpi,
													  ADI_SPI_WATERMARK_50,
													  ADI_SPI_WATERMARK_DISABLE,
													  ADI_SPI_WATERMARK_DISABLE);
		}
		if (result == 0u)
		{
			/* generate rx data interrupt when watermark level breaches 50% level */
			/* DMA watermark levels are disabled because SPI is in interrupt mode */
			result = adi_spi_SetRxWatermark(hSpi,
													  ADI_SPI_WATERMARK_50,
													  ADI_SPI_WATERMARK_DISABLE,
													  ADI_SPI_WATERMARK_DISABLE);
		}
	}

	ResetADI();

	for (i=0; i<num; i++)
	{
		WriteParamToSPI(initValues[i][0], initValues[i][1]);
	}

	for (i=0; i<16; i++)
	{
		Lidar_ChannelDCBal(i, calib[i]);
	}
}

/**
 * @brief Clear the sram by reading both banks
 */
void ClearSram(void) {

//	WriteParamToSPI(DataControlAddress, 0x01);
//	ReadDataFromSPI(&DataForSPITransaction[0]);
//	WriteParamToSPI(DataControlAddress, 0x00);

//	WriteParamToSPI(DataControlAddress, 0x02);
//	ReadDataFromSPI(&DataForSPITransaction[0]);
//	WriteParamToSPI(DataControlAddress, 0x00);

}

/**
 * @brief Get the current status
 */
const uint16_t bank0Full = 0x0001;
const uint16_t bank1Full = 0x0002;
const uint16_t maxNumberOfSameState = 20;
int GetADIStatus(void) {
	static uint16_t bankLastStatus = 0x00;
	static uint16_t numberOfSameState = 0;
	uint16_t bankCurrentStatus = 0x00;
	uint16_t returnValue;

	returnValue = ADI_Empty;

	ReadParamFromSPI(BankStatusAddress, &bankCurrentStatus);

	if (bankCurrentStatus == bankLastStatus) {
		if (++numberOfSameState >= maxNumberOfSameState) {
			numberOfSameState = 0;
			returnValue = ADI_locked;
		}
	} else {
		bankLastStatus = bankCurrentStatus;
	}
	if (bankCurrentStatus == bank0Full || bankCurrentStatus == bank1Full) {
		bankLastStatus = bankCurrentStatus;
		numberOfSameState = 0;
		returnValue = ADI_DataPresent;
	}
//	if (sensorParameter.DSP.debugMode & debugInsertionAcq) {
//		returnValue = ADI_DataPresent;
//	}
	return (returnValue);
}

/**
 * @brief Retreive the data from the ADI device
 */

void GetADIData(uint16_t *pBank, uint16_t * pData) {
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
		WriteParamToSPI(0xF1, 0x01B0 | 0x0002);

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
		WriteParamToSPI(0xF1, 0x01B0);

		//6. Write 0x0 to the SRAM_READ register (Address 0x3)
		//   to disengage the transfer intent.
		WriteParamToSPI(DataControlAddress, 0x0000);
	}
}

void GetADIData_Start(uint16_t *pBank, uint16_t * pData) {
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
		WriteParamToSPI(0xF1, 0x01B0 | 0x0002);

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

void GetADIData_Stop(void)
{
	//5. (TC1 ONLY) Write bit[1] of register address 0xF1 to 0x0
	WriteParamToSPI(0xF1, 0x01B0);

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

	ReadParamFromSPI(CH0ControlReg0Address + ch * 4, &data);

	if (enable)
		data |=  0x001F;
	else
		data &= ~0x001F;

    WriteParamToSPI(CH0ControlReg0Address + ch * 4, data);
}

void Lidar_ChannelTIAFeedback(int ch, uint16_t feedback)
{
	uint16_t data;

	ReadParamFromSPI(CH0ControlReg1Address + ch * 4, &data);

	data &= ~0x03FF;
	data |= feedback;

    WriteParamToSPI(CH0ControlReg1Address + ch * 4, data);
}

void Lidar_ChannelDCBal(int ch, uint16_t bal)
{
	uint16_t data;

	ReadParamFromSPI(CH0ControlReg0Address + ch * 4 + 2, &data);

	data &= ~0x01FF;
	data |= bal;

    WriteParamToSPI(CH0ControlReg0Address + ch * 4 + 2, data);
}

void Lidar_FlashGain(uint16_t flashGain)
{
	uint16_t data;

	ReadParamFromSPI(Control0Address, &data);

	data &= ~0x1F80;
	data |= (flashGain << 7) & 0x1F80;

    WriteParamToSPI(Control0Address, data);
}



//
// Acquisition control
//

static uint16_t BankInTransfer = 0;

static int iFifoHead = 0;
static int iFifoTail = 0;
static uint16_t AcqFifo[NUM_FIFO][FRAME_NUM_PTS];

void Lidar_Acq(uint16_t *pBank)
{
	*pBank = 0;

	if (BankInTransfer == 0)
	{
		GetADIData_Start(&BankInTransfer, AcqFifo[iFifoHead]);
	}
	else
	{
		if (GetADIData_Check())
		{
			*pBank = BankInTransfer;
			BankInTransfer = 0;

			GetADIData_Stop();

			int iFifoHeadNext = (iFifoHead + 1) & NUM_FIFO_MASK;

			if (iFifoHeadNext != iFifoTail)
			{
				iFifoHead = iFifoHeadNext;
				pADI_PORTB->DATA_CLR = (1 << 1);
			}
			else
				pADI_PORTB->DATA_SET = (1 << 1);
		}
	}
}

void Lidar_GetDataFromFifo(uint16_t ** pDataPtr, uint16_t * pNumFifo)
{
	if (iFifoHead != iFifoTail)
	{
		if (iFifoTail < iFifoHead)
			*pNumFifo = iFifoHead - iFifoTail;
		else
			*pNumFifo = NUM_FIFO - iFifoTail;

		*pDataPtr = AcqFifo[iFifoTail];
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
	pADI_PORTB->DATA_CLR = (1 << 1);
}


//
// USB
//

int iUSBnum = 0;
int iUSBnumOK = 0;
int iUSBnumEmpty = 0;
