/*
 * Guardian_ADI.c
 *
 *  Created on: 2016-09-05
 *      Author: pc
 */


#define USE_DMA
#define USE_ALGO
#define USE_ACCUMULATION


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


#ifdef USE_ALGO
uint16_t iAlgoNum = 1;
uint16_t iAlgoScaler = 1;
#endif //USE_ALGO

#ifdef USE_ACCUMULATION
uint16_t iAcqAccNum = 0;
uint16_t iAcqAccMax = 16;
uint16_t iAcqAccShift = 4;
int32_t AcqFifoAcc[FRAME_NUM_PTS];
#endif //USE_ACCUMULATION



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
	UndocumentedF1Address = 0xF1,
	ADCWorkingModeAddress = 0xF2,
	BankStatusAddress = 0xF6, // read only
	SRAMReadOutAddress = 0xFF // read only
};

uint16_t Lidar_InitValues[][2] =
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

	Lidar_SPITriggerMode();

	ClearSram();

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
void Lidar_InitADI(void) {
    int i;

#if 0
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
#endif

	if (hSpi == NULL)
	{
		ADI_SPI_RESULT result;

		result = adi_spi_Open(2, spiMem, sizeof(spiMem), &hSpi);
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

	Flash_LoadConfig(&Lidar_InitValues[0][0], &numParams);

	if (numParams)
		num = numParams;

	for (i=0; i<num; i++)
	{
		WriteParamToSPI(Lidar_InitValues[i][0], Lidar_InitValues[i][1]);
	}

	user_CANFifoPushSensorStatus();
    user_CANFifoPushSensorBoot();
}


int SaveConfigToFlash(void)
{
	int num = sizeof(Lidar_InitValues) / sizeof(Lidar_InitValues[0]);
	int i;
	for (i=0; i<num; i++)
	{
		ReadParamFromSPI(Lidar_InitValues[i][0], &Lidar_InitValues[i][1]);
	}

	return Flash_SaveConfig(&Lidar_InitValues[0][0], num);
}

int ResetToFactoryDefault(void)
{
	return Flash_ResetToFactoryDefault();
}

void ForceGetADIData(uint16_t bankNum, uint16_t * pData) {
	//1. Poll the Bank_Status register (Address 0xF6) until
	//   register reads 0x1 or 0x2. This means data is ready at
	//   that bank.
	// N/A

	//2. (TC1 ONLY) Write bit[1] of register address 0xF1 to 0x1
	WriteParamToSPI(UndocumentedF1Address, 0x01B0 | 0x0002);

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
	WriteParamToSPI(UndocumentedF1Address, 0x01B0);

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
		WriteParamToSPI(UndocumentedF1Address, 0x01B0 | 0x0002);

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
		WriteParamToSPI(UndocumentedF1Address, 0x01B0);

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
	ReadParamFromSPI(BankStatusAddress, &bankStatus);
	if (bankStatus)
	{
		//1. Poll the Bank_Status register (Address 0xF6) until
		//   register reads 0x1 or 0x2. This means data is ready at
		//   that bank.
		*pBank = bankStatus;

		//2. (TC1 ONLY) Write bit[1] of register address 0xF1 to 0x1
		WriteParamToSPI(UndocumentedF1Address, 0x01B0 | 0x0002);

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
	WriteParamToSPI(UndocumentedF1Address, 0x01B0);

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
		readWriteFifo[iReadWriteFifoHead] = ((uint32_t) _startAddress & 0x7FFF) << 16;
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
		readWriteFifo[iReadWriteFifoHead] = 0x80000000 | ((uint32_t) _startAddress & 0x7FFF) << 16 | data;
		iReadWriteFifoHead = nextHead;

		return 1;
	}

	return 0;
}

inline int ProcessReadWriteFifo(void)
{
	uint32_t op = readWriteFifo[iReadWriteFifoTail];
	uint16_t addr = (op >> 16) & 0x7FFF;
	uint16_t data =  op & 0xFFFF;

	if (op & 0x80000000)
	{
		switch (addr)
		{
		case 100:
			iAcqAccMax = data;
			break;
		case 101:
			iAcqAccShift = data;
			break;
		case 102:
			iAlgoScaler = data;
			break;
		case 997:
			Lidar_Reset();
			break;
		case 998:
			SaveConfigToFlash();
			break;
		case 999:
			ResetToFactoryDefault();
			break;
		default:
			WriteParamToSPI(addr, data);
			break;
		}

		// DGG: temp
		if (addr == 0)
			AddFakeData();
	}
	else
	{
		switch (addr)
		{
		case 100:
			data = iAcqAccMax;
			break;
		case 101:
			data = iAcqAccShift;
			break;
		case 102:
			data = iAlgoScaler;
			break;
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

bool gAcq = true;

static uint16_t BankInTransfer = 0;

volatile int iFifoHead = 0;
volatile int iFifoTail = 0;

#define NUM_FIFO			8
#define NUM_FIFO_MASK		(NUM_FIFO-1)

static tDataFifo dataFifo[NUM_FIFO];

#ifdef USE_ALGO
static float tmpAcqFloat[GUARDIAN_SAMPLING_LENGTH];
#endif //USE_ALGO

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
			bool bAccDone = false;
			int ch;

			*pBank = BankInTransfer;
			BankInTransfer = 0;

			GetADIData_Stop();

#ifdef USE_ACCUMULATION
			if (iAcqAccNum < iAcqAccMax)
			{
				int i;
				if (iAcqAccNum == 0)
					memset(AcqFifoAcc, 0, sizeof(AcqFifoAcc));

				for(i=0; i<FRAME_NUM_PTS; i++)
				{
					AcqFifoAcc[i] += dataFifo[iFifoHead].AcqFifo[i];
				}

				++iAcqAccNum;
			}
			else
			{
				int i;
				for(i=0; i<FRAME_NUM_PTS; i++)
				{
					int16_t data =
					dataFifo[iFifoHead].AcqFifo[i] = (int16_t) (AcqFifoAcc[i] >> iAcqAccShift);
				}

				iAcqAccNum = 0;
				bAccDone = true;
			}
#endif //USE_ACCUMULATION


#ifdef USE_ALGO
			if (bAccDone)
			{
				if (iAlgoScaler == 0)
				{
					iAlgoNum = 1;
				}
				else if (iAlgoNum < iAlgoScaler)
				{
					++iAlgoNum;
				}
				else
				{
					detection_type detections[GUARDIAN_NUM_CHANNEL * GUARDIAN_NUM_DET_PER_CH];
					bool bOneDetection = false;

					for(ch=0; ch<GUARDIAN_NUM_CHANNEL; ++ch)
					{
						int i;

						detection_type* pDetections = &detections[ch * GUARDIAN_NUM_DET_PER_CH];

						for(i=0; i<GUARDIAN_SAMPLING_LENGTH; ++i)
							tmpAcqFloat[i] = (float) dataFifo[iFifoHead].AcqFifo[ch * GUARDIAN_SAMPLING_LENGTH + i];

						threshold2(pDetections, tmpAcqFloat, ch);

						if (pDetections->distance)
						{
							bOneDetection = true;
							user_CANFifoPushDetection(ch, (uint16_t) (pDetections->distance * 100.0), 100);
						}
					}

					if (bOneDetection)
						user_CANFifoPushCompletedFrame();

					iAlgoNum = 1;
				}
			}
#endif //USE_ALGO

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
	Lidar_InitADI();

	iReadWriteFifoHead = 0;
	iReadWriteFifoTail = 0;

	iFifoHead = 0;
	iFifoTail = 0;
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

	if (cld_time_passed_ms(acq_time) >= 1000u)
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

