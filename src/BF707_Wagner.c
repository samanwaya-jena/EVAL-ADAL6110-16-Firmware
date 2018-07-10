/*****************************************************************************
 * BF707_Wagner.c
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/platform.h>

#define USE_UART

#ifdef USE_UART
#include <drivers\uart\adi_uart.h>
#endif //USE_UART

#include <drivers\spi\adi_spi.h>
#include <services\pwr\adi_pwr.h>
#include <services/int/adi_sec.h>

#include "adi_initialize.h"
#include "BF707_Wagner.h"

#include "Guardian_ADI.h"


#ifdef USE_UART

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

#endif //USE_UART




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


bool bComplete = false;

/* SPI callback */
void SpiCallback(void* pHandle, uint32_t u32Arg, void* pArg)
{
    ADI_SPI_HANDLE pDevice = (ADI_SPI_HANDLE *)pHandle;
    ADI_SPI_EVENT event = (ADI_SPI_EVENT)u32Arg;
    uint16_t *data = (uint16_t*)pArg;

    switch (event) {
        case ADI_SPI_TRANSCEIVER_PROCESSED:
            bComplete = true;
            break;
    default:
        break;
    }
}

void ProcessChar(char curChar);

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



	InitADI();


//    adi_sec_Enable(true);


#ifdef USE_UART
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
	while ( 1 )
	{
	bool bAvailUart = false;
	eResult = adi_uart_IsRxBufferAvailable (ghUART, &bAvailUart);

	if (bAvailUart)
	{
		void * ptr = NULL;

		eResult = adi_uart_GetRxBuffer(ghUART, &ptr);

		ProcessChar(*((char*)ptr));

		eResult = adi_uart_SubmitRxBuffer(ghUART, ptr, 1);
	}

	}



#endif //USE_UART





//	return 0;
}





void ReadParamFromSPI(uint16_t _startAddress, uint16_t *_data);
void WriteParamToSPI(uint16_t _startAddress, uint16_t _data);



int Lidar_read_mid(uint8_t *mid, uint8_t *pid, uint8_t *rid)
{
	uint16_t data;

	ReadParamFromSPI(0, &data);

	*mid = (data >> 8) & 0xFF;
	*pid = (data >> 4) & 0x0F;
	*rid = (data & 0x0F);

	return 0;
}



void Lidar_PrintInfo(void)
{
	int i;
	char str[32];

	uint8_t mid = 0;
	uint8_t pid = 0;
	uint8_t rid = 0;

	int result = Lidar_read_mid(&mid, &pid, &rid);

	sprintf(str, "Lidar mid:%d pid:%d rid:%d\r\n", mid, pid, rid);
#ifdef USE_UART
	adi_uart_Write(ghUART, str, strlen(str));
#endif //USE_UART
}

void Lidar_PrintInfoLoop(void)
{
	int i;

	while (1)
	{
		Lidar_PrintInfo();

		for (i=0; i<3000000; i++)
		{
			asm("nop;");
		}
	}
}

const char * hex = "0123456789ABCDEF";

void Lidar_DumpRegs(void)
{
	uint8_t regs[] = { 0,1,2,3,4,7,0x4D,0x56,0xF1,0xF2 };
	int i, ch;
	char str[32];

	int num = sizeof(regs) / sizeof(regs[0]);

	for (i=0; i<num; i++)
	{
		uint16_t reg = regs[i];
		uint16_t data = 0xFFFF;

		ReadParamFromSPI(reg, &data);

		sprintf(str, "%c%c: %c%c%c%c\r\n", hex[(reg>>4)&0xF], hex[(reg>>0)&0xF],
				hex[(data>>12)&0xF], hex[(data>>8)&0xF], hex[(data>>4)&0xF], hex[(data>>0)&0xF]);

#ifdef USE_UART
		adi_uart_Write(ghUART, str, strlen(str));
#endif //USE_UART
	}

	sprintf(str, "Ch: E TIA BAL\r\n");
#ifdef USE_UART
	adi_uart_Write(ghUART, str, strlen(str));
#endif //USE_UART

	for (ch=0; ch<16; ch++)
	{
		uint16_t en, tia, bal;
		uint16_t data = 0xFFFF;

		ReadParamFromSPI(13 + 4 * ch, &data);
		en = (data & 0x001F);

		ReadParamFromSPI(13 + 4 * ch + 1, &data);
		tia = (data & 0x03FF);

		ReadParamFromSPI(13 + 4 * ch + 2, &data);
		bal = (data & 0x01FF);

		sprintf(str, "%02d: %d %c%c%c %d\r\n", ch, (en) ? 1 : 0,
				hex[(tia>>8)&0xF], hex[(tia>>4)&0xF], hex[(tia>>0)&0xF],
				bal);

#ifdef USE_UART
		adi_uart_Write(ghUART, str, strlen(str));
#endif //USE_UART
	}

}

uint16_t buf[1600];

void Lidar_GetData(void)
{
	uint16_t banknum = 0;
	int i,j,ch;

	memset(buf, 0, sizeof(buf));
//	for(i=0; i<1600; i++)
//		buf[i] = 0x;

	GetADIData(&banknum, buf);

	uint16_t * pData = buf;

	if (banknum)
	{
#ifdef USE_UART
		char str[32];
		sprintf(str, "bank %d\r\n", banknum);

		adi_uart_Write(ghUART, str, strlen(str));

		for(ch=0; ch<3; ch++)
		{
			sprintf(str, "Ch %d\r\n", ch);
			adi_uart_Write(ghUART, str, strlen(str));

			for(j=0; j<10; j++)
			{
				for(i=0; i<10; i++)
				{
					uint16_t data = pData[ch*100 + j*10 + i];
					sprintf(str, " %c%c%c%c ",
						hex[(data>>12)&0xF], hex[(data>>8)&0xF], hex[(data>>4)&0xF], hex[(data>>0)&0xF]);

					adi_uart_Write(ghUART, str, strlen(str));
				}
				adi_uart_Write(ghUART, "\r\n", 2);
			}
		}
#endif //USE_UART
	}
	else
		adi_uart_Write(ghUART, "No data!\r\n", 10);
}

int chIdx[16] = {
100 * 0,
100 * 1,
100 * 2,
100 * 3,
100 * 4,
100 * 5,
100 * 6,
100 * 7,
100 * 15,
100 * 14,
100 * 13,
100 * 12,
100 * 11,
100 * 10,
100 * 9,
100 * 8
};

void Lidar_GetDataCSV(void)
{
	char str[256];
	uint16_t banknum = 0;
	int i,j,ch,sample;

	memset(buf, 0, sizeof(buf));
//	for(i=0; i<1600; i++)
//		buf[i] = 0x7ff0 + i;

	GetADIData(&banknum, buf);

	uint16_t * pData = buf;

	if (banknum)
	{
#ifdef USE_UART
		sprintf(str, "##, ch00 , ch01 , ch02 , ch03 , ch04 , ch05 , ch06 , ch07 , ch08 , ch09 , ch10 , ch11 , ch12 , ch13 , ch14 , ch15\r\n", banknum);
		adi_uart_Write(ghUART, str, strlen(str));

		for(sample=0; sample<100; sample++)
		{
			sprintf(str, "%2d,", sample);
			adi_uart_Write(ghUART, str, strlen(str));

			for(ch=0; ch<16; ch++)
			{
				int16_t data = pData[chIdx[ch] + sample];

				sprintf(str, "%6d,", data);

				adi_uart_Write(ghUART, str, strlen(str));
			}
			adi_uart_Write(ghUART, "\r\n", 2);
		}

#endif //USE_UART
	}
}




#define MAX_CMD_SIZE  16

static char debugCmdIdx = 0;
static char debugCmd[MAX_CMD_SIZE];

void ProcessChar(char curChar)
{
#ifdef USE_UART
	/* UART return code */
	ADI_UART_RESULT    eResult;
	eResult = adi_uart_Write(ghUART, &curChar, 1);

	if (curChar == '\r')
	{
		char tmpChar = '\n';
		eResult = adi_uart_Write(ghUART, &tmpChar, 1);
	}
#endif //USE_UART

  if (debugCmdIdx < MAX_CMD_SIZE)
    debugCmd[debugCmdIdx++] = curChar;

  if (curChar == '\r')
  {
    debugCmd[debugCmdIdx-1] = 0;

    switch (debugCmd[0])
    {
//    case 'I':
//    	PrintFlashInfo();
//    	break;

    case 'a':
   		Lidar_GetData();
    	break;

    case 'A':
#ifdef USE_UART
    	adi_uart_Write(ghUART, "\r\n", 2);
#endif //USE_UART
   		Lidar_GetDataCSV();
    	break;

    case 'i':
   		Lidar_PrintInfo();
    	break;

    case 'I':
    	InitADI();
    	break;

    case 'g':
    	Lidar_Trig();
    	break;

    case 's':
    	Lidar_SPITriggerMode();
    	break;

    case 'f':
    	Lidar_FreerunMode();
    	break;

    case 'd':
   		Lidar_DumpRegs();
    	break;

    case '1':
   		Lidar_ChannelEnable(0, 1);
    	break;

    case '!':
   		Lidar_ChannelEnable(0, 0);
    	break;

    case '2':
   		Lidar_ChannelEnable(1, 1);
    	break;

    case '@':
   		Lidar_ChannelEnable(1, 0);
    	break;

    case 'b':
    {
    	int i = 1;
    	uint16_t bal = 0;
    	int ch = 0;

    	while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
    		ch = ch * 10 + debugCmd[i++] - '0';

    	if (debugCmd[i] == ' ' || debugCmd[i] == '=')
    		i++;

    	while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
    		bal = bal * 10 + debugCmd[i++] - '0';

    	Lidar_ChannelDCBal(ch, bal);
    	break;
    }

    case 'F':
    {
    	int i = 1;
    	uint16_t feedback = 0;
    	int ch = 0;

    	while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
    		ch = ch * 10 + debugCmd[i++] - '0';

    	if (debugCmd[i] == ' ' || debugCmd[i] == '=')
    		i++;

    	while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
    		feedback = feedback * 10 + debugCmd[i++] - '0';

    	Lidar_ChannelTIAFeedback(ch, feedback);
    	break;
    }

    case 'r':
	{
		char str[32];
		uint16_t data = 0xFFFF;

		ReadParamFromSPI(1, &data);

		sprintf(str, "r 1: %d\r\n", data);
#ifdef USE_UART
		adi_uart_Write(ghUART, str, strlen(str));
#endif //USE_UART

		break;
	}

    case 'w':
	{
		char str[32];
		uint16_t data = 0x3220;

		WriteParamToSPI(1, data);

		sprintf(str, "w 1: %d\r\n", data);
#ifdef USE_UART
		adi_uart_Write(ghUART, str, strlen(str));
#endif //USE_UART

		break;
	}

    }


    debugCmdIdx = 0;
  }


}





