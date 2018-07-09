/*****************************************************************************
 * BF707_Wagner.c
 *****************************************************************************/

#include <stdio.h>
#include <sys/platform.h>

#include <drivers\uart\adi_uart.h>
#include <drivers\spi\adi_spi.h>
#include <services\pwr\adi_pwr.h>
#include <services/int/adi_sec.h>

#include "adi_initialize.h"
#include "BF707_Wagner.h"





/* Baud rate to be used for char echo */
#define BAUD_RATE           115200u

/* UART Device Number to test */
#define UART_DEVICE_NUM     0u

/* UART Handle */
static ADI_UART_HANDLE  ghUART;

/* Number of Tx and Rx buffers */
#define NUM_BUFFERS     2

/* Rx and Tx buffers */
static uint8_t RxBuffer[NUM_BUFFERS];

/* Memory required for operating UART in interrupt mode */
static uint8_t  gUARTMemory[ADI_UART_BIDIR_DMA_MEMORY_SIZE];





uint8_t spiMem[ADI_SPI_INT_MEMORY_SIZE]; //ADI_SPI_DMA_MEMORY_SIZE

/** 
 * If you want to use command program arguments, then place them in the following string. 
 */
char __argv_string[] = "";

/* Sets the Software controlled switches */
extern void ConfigSoftSwitches(void);

#define SPI_UNSELECT_SLAVE              do { *pREG_PORTB_DATA_SET = BITM_PORT_DATA_PX15; } while (0)
#define SPI_SELECT_SLAVE                do { *pREG_PORTB_DATA_CLR = BITM_PORT_DATA_PX15; } while (0)




/* Example result definitions */
#define FAILED              (-1)
#define PASSED              0

/* Macro for reporting errors */
#define REPORT_ERROR        printf


/* default power settings */
#define MHZTOHZ       (1000000)

#define DF_DEFAULT    (0x0)
#define MSEL_DEFAULT  (0x10)
#define SSEL_DEFAULT  (0x8)
#define CSEL_DEFAULT  (0x4)

#define CLKIN         (25 * MHZTOHZ)
#define CORE_MAX      (500 * MHZTOHZ)
#define SYSCLK_MAX    (250 * MHZTOHZ)
#define SCLK_MAX      (125 * MHZTOHZ)
#define VCO_MIN       (72 * MHZTOHZ)




int main(int argc, char *argv[])
{
	int dgg = 0;

	/**
	 * Initialize managed drivers and/or services that have been added to 
	 * the project.
	 * @return zero on success 
	 */
	adi_initComponents();
	
	/* Set the Software controlled switches to default values */
	ConfigSoftSwitches();

	SPI_UNSELECT_SLAVE;

    /* Initialize Power service */
#if defined (__ADSPBF707_FAMILY__) || defined (__ADSPSC589_FAMILY__)
    if(adi_pwr_Init(0, CLKIN) != ADI_PWR_SUCCESS)
    {
        REPORT_ERROR("Failed to initialize power service \n");
        return FAILED;
    }
#else
    if(adi_pwr_Init(CLKIN, CORE_MAX, SYSCLK_MAX, VCO_MIN) != ADI_PWR_SUCCESS)
    {
        REPORT_ERROR("Failed to initialize power service \n");
        return FAILED;
    }
#endif


    adi_sec_Enable(true);




    /* UART return code */
   	ADI_UART_RESULT    eResult;

    /*
     * Initialize UART
     */
    /* Open UART driver */
    if((eResult = adi_uart_Open(UART_DEVICE_NUM,
                                 ADI_UART_DIR_BIDIRECTION,
                                 gUARTMemory,
                                 ADI_UART_BIDIR_DMA_MEMORY_SIZE,
                                 &ghUART)) != ADI_UART_SUCCESS)
    {
    	REPORT_ERROR("Could not open UART Device 0x%08X \n", eResult);
    	return FAILED;
    }
    /* Set the UART Mode */
    if((eResult = adi_uart_SetMode(ghUART,
                                    ADI_UART_MODE_UART
                                   )) != ADI_UART_SUCCESS)
    {
    	REPORT_ERROR("Could not set the Mode 0x%08X \n", eResult);
    	return FAILED;
    }

    /* Set UART Baud Rate */
    if((eResult = adi_uart_SetBaudRate(ghUART,
    									BAUD_RATE
                                        )) != ADI_UART_SUCCESS)
    {
    	REPORT_ERROR("Could not set the Baud Rate 0x%08X \n", eResult);
    	return FAILED;
    }

    /* Set number of stop bits */
    if((eResult = adi_uart_SetNumStopBits(ghUART,
                                            ADI_UART_ONE_STOPBIT
                                         )) != ADI_UART_SUCCESS)
    {
    	REPORT_ERROR("Could not set the stop bits 0x%08X \n", eResult);
    	return FAILED;
    }

    /* Set number of stop bits */
    if((eResult = adi_uart_SetWordLen(ghUART,
                                          ADI_UART_WORDLEN_8BITS
                                         )) != ADI_UART_SUCCESS)
    {
    	REPORT_ERROR("Could not set word length 0x%08X \n", eResult);
    	return FAILED;
    }

    printf("Setup Hyperterminal as described in Readme file. \n");
    printf("Press any key on the key board and notice the character  being echoed to Hyperterminal. \n");
    printf("\n Press return key to stop the program.\n");

    eResult = adi_uart_Write(ghUART, "\r\n\r\nBF707-EZLITE\r\n", 18);


	eResult = adi_uart_SubmitRxBuffer(ghUART, &RxBuffer[0], 1);

	eResult = adi_uart_EnableRx(ghUART, true);

//	eResult = adi_uart_RegisterCallback(ghUART, UartCallback, NULL);





	/* command processing loop */
//	while ( 1 )
//	{
//	}











	ADI_SPI_HANDLE hSpi = NULL;

	ADI_SPI_RESULT result;

	result = adi_spi_Open(2, spiMem, ADI_SPI_DMA_MEMORY_SIZE, &hSpi);


	/* Set master */
	result = adi_spi_SetMaster( hSpi,true);

	/* Set slave select using hardware*/
	result = adi_spi_SetHwSlaveSelect( hSpi, true);

	/* Selecting slave1 as the device*/
	result = adi_spi_SetSlaveSelect( hSpi,ADI_SPI_SSEL_ENABLE6);

	/* Setting the word size of 8 bytes*/
	result = adi_spi_SetWordSize( hSpi, ADI_SPI_TRANSFER_16BIT);

	/* Setting the clock frequency of spi   The frequency of the SPI clock is calculated by SCLK / 2* (Baud=3)*/
	result = adi_spi_SetClock( hSpi, 20);

	/* No call backs required*/
	result = adi_spi_RegisterCallback(hSpi, NULL, NULL);

	/* Disable DMA */
	result = adi_spi_EnableDmaMode(hSpi, false);

//	result = adi_spi_SetTransceiverMode(hSpi, ADI_SPI_TXRX_MODE);

	result = adi_spi_SetClockPolarity(hSpi, true);

	result = adi_spi_SetClockPhase(hSpi, false);

//	result = adi_spi_SetFlowControl(hSpi, ADI_SPI_FLOWCONTROL_MASTER, ADI_SPI_WATERMARK_DISABLE);

//	result = adi_spi_SetInterruptMode(hSpi, ADI_SPI_HW_ERR_NONE, false);


//	result = adi_spi_SlaveSelect(hSpi, true);
//	result = adi_spi_SlaveSelect(hSpi, false);
//	result = adi_spi_SlaveSelect(hSpi, true);

	uint16_t _startAddress = 0;

	uint8_t tbuf[4];
	uint8_t rbuf[4];

	tbuf[1] = (_startAddress << 1) >> 8;
	tbuf[0] = (_startAddress << 1) | 1;

	ADI_SPI_TRANSCEIVER spiTx;

	spiTx.pPrologue = NULL;
	spiTx.PrologueBytes = 0;
	spiTx.pTransmitter = tbuf;
	spiTx.TransmitterBytes = 4;
	spiTx.pReceiver = rbuf;
	spiTx.ReceiverBytes = 4;

	//SPI_SELECT_SLAVE;
	//result = adi_spi_ReadWrite(hSpi, &spiTx);
	//SPI_UNSELECT_SLAVE;

	result = adi_spi_SubmitBuffer(hSpi, &spiTx);

	while (1)
	{
		bool bAvailUart = false;
		bool bAvailSpi = false;

		eResult = adi_uart_IsRxBufferAvailable (ghUART, &bAvailUart);

		if (bAvailUart)
		{
			void * ptr = NULL;

			eResult = adi_uart_GetRxBuffer(ghUART, &ptr);

//			ProcessChar(*((char*)ptr));

			eResult = adi_uart_SubmitRxBuffer(ghUART, ptr, 1);
		}


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

//	SPI_UNSELECT_SLAVE;

	return 0;
}

