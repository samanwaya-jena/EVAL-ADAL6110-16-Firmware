/*****************************************************************************
 * BF707_Wagner.c
 *****************************************************************************/

#include <sys/platform.h>

#include <drivers\spi\adi_spi.h>

#include "adi_initialize.h"
#include "BF707_Wagner.h"

uint8_t spiMem[ADI_SPI_INT_MEMORY_SIZE]; //ADI_SPI_DMA_MEMORY_SIZE

/** 
 * If you want to use command program arguments, then place them in the following string. 
 */
char __argv_string[] = "";

int main(int argc, char *argv[])
{
	int dgg = 0;

	/**
	 * Initialize managed drivers and/or services that have been added to 
	 * the project.
	 * @return zero on success 
	 */
	adi_initComponents();
	
	ADI_SPI_HANDLE hSpi = NULL;

	ADI_SPI_RESULT result;

	result = adi_spi_Open(2, spiMem, ADI_SPI_DMA_MEMORY_SIZE, &hSpi);


	/* Set master */
	result = adi_spi_SetMaster( hSpi,true);

	/* Set slave select using hardware*/
	result = adi_spi_SetHwSlaveSelect( hSpi, true);

	/* Selecting slave1 as the device*/
	result = adi_spi_SetSlaveSelect( hSpi,ADI_SPI_SSEL_ENABLE2);

	/* Setting the word size of 8 bytes*/
	result = adi_spi_SetWordSize( hSpi, ADI_SPI_TRANSFER_8BIT);

	/* Setting the clock frequency of spi   The frequency of the SPI clock is calculated by SCLK / 2* (Baud=3)*/
	result = adi_spi_SetClock( hSpi, 20);

	/* No call backs required*/
	result = adi_spi_RegisterCallback(hSpi, NULL, NULL);

	/* Disable DMA */
	result = adi_spi_EnableDmaMode(hSpi, false);



	result = adi_spi_SlaveSelect(hSpi, true);
	result = adi_spi_SlaveSelect(hSpi, false);
	result = adi_spi_SlaveSelect(hSpi, true);

	uint16_t _startAddress = 0;

	uint8_t tbuf[2];
	uint8_t rbuf[2];

	tbuf[0] = (_startAddress << 1) >> 8;
	tbuf[1] = (_startAddress << 1) | 1;

	ADI_SPI_TRANSCEIVER spiTx;

	spiTx.pPrologue = NULL;
	spiTx.PrologueBytes = 0;
	spiTx.pTransmitter = tbuf;
	spiTx.TransmitterBytes = 2;
	spiTx.pReceiver = rbuf;
	spiTx.ReceiverBytes = 2;

	result = adi_spi_ReadWrite(hSpi, &spiTx);


	return 0;
}

