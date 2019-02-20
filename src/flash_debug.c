/*
 * flash_debug.c
 *
 *  Created on: Feb 15, 2019
 *      Author: JeanAlexis
 */

#include "flash_params.h"
#include "BF707_Wagner.h"
#include <drivers\spi\adi_spi.h>
#include "common\flash.h"
#include "post_debug.h"

ADI_SPI_HANDLE hSpiFlash_d;


#ifdef USE_DMA
uint8_t flashMem_d[ADI_SPI_DMA_MEMORY_SIZE];
#else //USE_DMA
uint8_t flashMem_d[ADI_SPI_INT_MEMORY_SIZE];
#endif //USE_DMA

struct flash_info * fl_info;
extern struct flash_info is25lp032d_info;


void flash_test()
{
	ADI_SPI_RESULT result;
	int jedec_id;
	uint8_t mid;
	uint8_t memory_type_id;
	uint8_t capacity_id;

	//Init flash mem struct
	//flash_info = &w25q32bv_info_debug;
	fl_info = &is25lp032d_info;

	//Configure SPI handler and setup transmit mode
	result = adi_spi_Open(FLASH_SPI_DEVICE, flashMem_d, sizeof(flashMem_d), &hSpiFlash_d);

	if(result != ADI_SPI_SUCCESS){
		printf("FAILDED TO OPEN FLASH SPI\n");
	}



	/* Set master */
	result = adi_spi_SetMaster( hSpiFlash_d,true);
//	CHECK_RESULT(result, "adi_spi_SetMaster");

	/* Set slave select using hardware*/
	result = adi_spi_SetHwSlaveSelect( hSpiFlash_d, false);
	//CHECK_RESULT(result, "adi_spi_SetHwSlaveSelect");

	result = adi_spi_SetLsbFirst( hSpiFlash_d, false);

	result = adi_spi_SetTransmitUnderflow( hSpiFlash_d, true);
	//CHECK_RESULT(result, "adi_spi_SetTransmitUnderflow");

	result = adi_spi_SetClockPhase(hSpiFlash_d, false);
	//CHECK_RESULT(result, "adi_spi_SetClockPhase");

	/* Setting the clock frequency of spi   The frequency of the SPI clock is calculated by SCLK / 2* (Baud=3)*/
	result = adi_spi_SetClock( hSpiFlash_d, 9);
	//CHECK_RESULT(result, "adi_spi_SetClock");

	/* Selecting slave1 as the device*/
	result = adi_spi_SetSlaveSelect( hSpiFlash_d, ADI_SPI_SSEL_ENABLE1);
	//CHECK_RESULT(result, "adi_spi_SetSlaveSelect");

	result = adi_spi_SetWordSize( hSpiFlash_d, ADI_SPI_TRANSFER_8BIT);

	result = adi_spi_SetInterruptMode(hSpiFlash_d, ADI_SPI_TRANSCEIVER_PROCESSED, true);

	//flash open
	fl_info->size = 0x400000;
	fl_info->number_of_regions = 1;
	fl_info->erase_block_regions = malloc(fl_info->number_of_regions * sizeof (struct erase_block_region));


	fl_info->erase_block_regions[0].block_size = 0x10000;
	fl_info->erase_block_regions[0].number_of_blocks = 64;


	//Test Read Register
	jedec_id = fl_info->flash_read_jedec_id(fl_info,&mid, &memory_type_id, &capacity_id);
	printf("mid: %d, mem_type: %d, capacity: %d", mid,memory_type_id,capacity_id);


	//Test write register
}
