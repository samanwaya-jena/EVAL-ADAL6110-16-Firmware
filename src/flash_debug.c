/*
 * flash_debug.c
 *
 *  Created on: Feb 15, 2019
 *      Author: JeanAlexis
 */

#include <stdint.h>
#include <string.h>

#include "flash_params.h"
#include "BF707_Wagner.h"
#include <drivers\spi\adi_spi.h>
#include "common\flash.h"
#include "post_debug.h"
#include "cld_bf70x_bulk_lib.h"

//ADI_SPI_HANDLE hSpiFlash_d;
extern ADI_SPI_HANDLE hSpiFlash;



#ifdef USE_DMA
uint8_t flashMem_d[ADI_SPI_DMA_MEMORY_SIZE];
#else //USE_DMA
uint8_t flashMem_d[ADI_SPI_INT_MEMORY_SIZE];
#endif //USE_DMA

struct flash_info * fl_info;
extern struct flash_info is25lp032d_info;

uint16_t Init_test_array[][2] =
{
		{ 0x0, 0xDEAD },
		{ 0x1, 0xBEEF },
		{ 0x2, 0xCAFE },
		{ 0x3, 0xFADE },
		{ 0x9, 0xBAAD },
		{ 0xA, 0xF00D },
		{ 0xB, 0xDEAD },
		{ 0xC, 0xC0DE },
		{ 0x10, 0x0D15 },
		{ 0x14, 0xEA5E },
		{ 0x16, 0x0420 },
		{ 0x17, 0x87B4 },
		{ 0x24, 0x0004 },
		{ 0x27, 0x0001 },
		{ 0x33, 0x0A40 },
		{ 0x38, 0xC201}
};

uint8_t init_page[32];


void flash_test()
{
	ADI_SPI_RESULT result;
	int jedec_id;
	uint8_t mid;
	uint8_t memory_type_id;
	uint8_t capacity_id;
	uint16_t mem_value;

	//Init flash mem struct
	//flash_info = &w25q32bv_info_debug;
	fl_info = &is25lp032d_info;

	//Configure SPI handler and setup transmit mode
	//result = adi_spi_Open(FLASH_SPI_DEVICE, flashMem_d, sizeof(flashMem_d), &hSpiFlash);
	  result = adi_spi_Open(FLASH_SPI_DEVICE, flashMem_d, sizeof(flashMem_d), &hSpiFlash);
		if(result != ADI_SPI_SUCCESS){
			printf("FAILED TO OPEN FLASH SPI\n");
		}

		/* Set master */
		result = adi_spi_SetMaster( hSpiFlash,true);
	//	CHECK_RESULT(result, "adi_spi_SetMaster");

		/* Set slave select using hardware*/
		result = adi_spi_SetHwSlaveSelect( hSpiFlash, false);
		//CHECK_RESULT(result, "adi_spi_SetHwSlaveSelect");

		result = adi_spi_SetLsbFirst( hSpiFlash, false);

		result = adi_spi_SetTransmitUnderflow( hSpiFlash, true);
		//CHECK_RESULT(result, "adi_spi_SetTransmitUnderflow");

		result = adi_spi_SetClockPhase(hSpiFlash, false);
		//CHECK_RESULT(result, "adi_spi_SetClockPhase");

		/* Setting the clock frequency of spi   The frequency of the SPI clock is calculated by SCLK / 2* (Baud=3)*/
		result = adi_spi_SetClock( hSpiFlash, 9);
		//CHECK_RESULT(result, "adi_spi_SetClock");

		/* Selecting slave1 as the device*/
		result = adi_spi_SetSlaveSelect( hSpiFlash, ADI_SPI_SSEL_ENABLE1);
		//CHECK_RESULT(result, "adi_spi_SetSlaveSelect");

		result = adi_spi_SetWordSize( hSpiFlash, ADI_SPI_TRANSFER_8BIT);

		/* No call backs required*/
	//	result = adi_spi_RegisterCallback(hSpi, NULL, NULL);

	//	result = adi_spi_SetTransceiverMode(hSpi, ADI_SPI_TXRX_MODE);

		result = adi_spi_SetClockPolarity(hSpiFlash, true);


		if (result == 0u)
		{
			/* generate tx data interrupt when watermark level breaches 50% level */
			/* DMA watermark levels are disabled because SPI is in interrupt mode */
			result = adi_spi_SetTxWatermark(hSpiFlash,
													  ADI_SPI_WATERMARK_50,
													  ADI_SPI_WATERMARK_DISABLE,
													  ADI_SPI_WATERMARK_DISABLE);

		}
		if (result == 0u)
		{
			/* generate rx data interrupt when watermark level breaches 50% level */
			/* DMA watermark levels are disabled because SPI is in interrupt mode */
			result = adi_spi_SetRxWatermark(hSpiFlash,
													  ADI_SPI_WATERMARK_50,
													  ADI_SPI_WATERMARK_DISABLE,
													  ADI_SPI_WATERMARK_DISABLE);

		}
	//flash open

	fl_info->flash_open(fl_info);

	//Test Read Register
	printf("MEMORY BEFORE FLASH WRITE\n\n\n");
	for(int i=0;i<128;i++)
	{
		//ReadFlashSPI(i,&mem_value);
		fl_info->flash_read(fl_info, i, &mem_value, sizeof(mem_value));
		printf("Memory Addr : 0x%x Memory Value:%x\n",i,mem_value );
	}

	memset(&init_page,0,sizeof(init_page));
	//Write FLASH MeMOry

	fl_info->flash_write_enable(fl_info);
	fl_info->flash_program(fl_info,0,&init_page,32);
	fl_info->flash_write_disable(fl_info);

	printf("MEMORY AFTER FLASH WRITE\n\n\n");
	for(int i=0;i<128;i++)
	{
		//WriteFlashSPI(Init_test_array[i][0], Init_test_array[i][1]);
		fl_info->flash_read(fl_info, i, &mem_value, sizeof(mem_value));
		printf("Memory Addr : 0x%x Memory Value:%x\n",i,mem_value );
	}


	//Test write register
}

void ReadFlashSPI(uint16_t _startAddress, uint16_t *_data)
{
	uint8_t ProBuffer1[2];
	uint8_t RxBuffer1[2];

	ProBuffer1[0] = (_startAddress << 3);
	ProBuffer1[1] = ((_startAddress << 3) >> 8) | 0x80;

	RxBuffer1[0] = 0;
	RxBuffer1[1] = 0;

	ADI_SPI_TRANSCEIVER Xcv0  = {ProBuffer1, 2u, NULL, 0u, RxBuffer1, 2u};

	ADI_SPI_RESULT result = adi_spi_ReadWrite(hSpiFlash, &Xcv0);

	*_data = *((uint16_t*)RxBuffer1);
}

void WriteFlashSPI(uint16_t _startAddress, uint16_t _data)
{
	uint8_t ProBuffer1[4];

	ProBuffer1[0] = (_startAddress << 3);
	ProBuffer1[1] = (_startAddress << 3) >> 8;
	ProBuffer1[2] = (_data >> 0);
	ProBuffer1[3] = (_data >> 8);

	ADI_SPI_TRANSCEIVER Xcv0  = {ProBuffer1, 4u, NULL, 0u, NULL, 0u};

	ADI_SPI_RESULT result = adi_spi_ReadWrite(hSpiFlash, &Xcv0);
}
