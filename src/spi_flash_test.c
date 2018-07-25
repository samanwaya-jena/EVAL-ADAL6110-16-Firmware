/*
 * (C) Copyright 2012 - Analog Devices, Inc. All Rights Reserved.
 *
 * This software is proprietary and confidential.  By using this software
 * you agree to the terms of the associated Analog Devices License Agreement.
 *
 * Project Name:  	Power_On_Self_Test
 *
 * Hardware:		ADSP-BF609 EZ-Board
 *
 * Description:	This file tests the SPI flash on the EZ-Board.
 */

/*******************************************************************
*  include files
*******************************************************************/
#include <cdefbf707.h>
#include <defbf707.h>
#include <stdlib.h>
#include <stdio.h>
#include "common/flash_errors.h"
#include "common/flash.h"
#include "common/spi.h"
#include "post_debug.h"
#include "SoftConfig_BF707.h"
#include "timer_isr.h"

#define SPI_NO 2

#if defined(__DEBUG_FILE__)
#include <string.h>
extern FILE *pDebugFile;				/* debug file when directing output to a file */
#endif

/************************SoftConfig Information********************************
	
	~ means the signal is active low
	
	Please see the ADSP-BF707 EZ-BRD manual for more information on using
	Software-Controlled Switches(SoftConfig)
	
********************************************************************************/

/* switch 0 register settings */
static SWITCH_CONFIG SwitchConfig0[] =
{

 /*
       U39 Port A                                  U39 Port B
  7--------------- ~RF_SPI2_SEL2_EN       |       7--------------- ~CAN1_ERR_EN
  | 6------------- ~RF_SPI2_SEL1_EN       |       | 6------------- ~CAN0_ERR_EN
  | | 5----------- ~SD_WP_EN              |       | | 5----------- ~CAN1_STB
  | | | 4--------- ~SD_CD_EN              |       | | | 4---------  CAN1_EN
  | | | | 3------- ~RF_SPI2_SEL1_EN       |       | | | | 3------- ~CAN0_STB
  | | | | | 2----- ~SPIFLASH_D3_EN        |       | | | | | 2-----  CAN0_EN
  | | | | | | 1--- ~SPIFLASH_D2_EN        |       | | | | | | 1--- ~SD_WP_EN
  | | | | | | | 0- ~SPIFLASH_CS_EN        |       | | | | | | | 0- ~SD_CD_EN
  | | | | | | | |                         |       | | | | | | | |
  N X N N N N N N                         |       N N N Y N Y X X     ( Active Y or N )
  1 1 1 1 0 0 0 0                         |       1 1 1 1 1 1 1 1     ( value being set )
*/
  { 0x12u, 0xF8u },                               { 0x13u, 0xFFu },

 /*
  * specify inputs/outputs
  */

  { 0x0u, 0x40u },   /* Set IODIRA direction (bit 6 input, all others output) */
  { 0x1u, 0x03u },   /* Set IODIRB direction (bit 0, 1 input, all others output) */
};
/* switch 2 register settings */
static SWITCH_CONFIG SwitchConfig1[] =
{

/*
            U38 Port A                                    U38 Port B

    7--------------- ~UART0CTS_RTS_LPBK    |       7---------------  Not Used
    | 6------------- ~UART0CTS_EN          |       | 6------------- ~PUSHBUTTON2_EN
    | | 5----------- ~UART0RTS_EN          |       | | 5----------- ~PUSHBUTTON1_EN
    | | | 4--------- ~UART0_EN             |       | | | 4--------- ~LED3_GPIO3_EN
    | | | | 3------- ~CAN1_RX_EN           |       | | | | 3------- ~LED2_GPIO2_EN
    | | | | | 2----- ~CAN0_RX_EN           |       | | | | | 2----- ~LED1_GPIO1_EN
    | | | | | | 1--- ~CAN1_TX_EN           |       | | | | | | 1--- ~UART0CTS_146_EN
    | | | | | | | 0- ~CAN0_TX_EN           |       | | | | | | | 0- ~UART0CTS_RST_EN
    | | | | | | | |                        |       | | | | | | | |
    N N N Y Y Y Y N                        |       X Y Y Y Y Y N N    ( Active Y or N )
    1 1 1 0 0 0 0 1                        |       0 0 0 0 0 0 1 1    ( value being set )
*/
  { 0x12u, 0xE1u },                               { 0x13u, 0x03u },

  /*
   * specify inputs/outputs
   */

  { 0x0u, 0x00u },    /* Set IODIRA direction (all output) */
  { 0x1u, 0x80u },    /* Set IODIRB direction (bit 7 input, all others output) */
};

/* switch configuration */
static SOFT_SWITCH SpiSoftSwitch[] =
{
  {
    0u,
    0x21u,
    sizeof(SwitchConfig0)/sizeof(SWITCH_CONFIG),
    SwitchConfig0
  },
  {
    0u,
    0x22u,
    sizeof(SwitchConfig1)/sizeof(SWITCH_CONFIG),
    SwitchConfig1
  }
};

#define MAN_CODE		0xEF		/* winbond */
#define DEV_CODE		0x15		/* W25Q32BV (16Mbit SPI flash) */
#define NUM_SECTORS		64			/* sectors */


extern const struct spi_ctlr adi_spi_bf6xx_ctlr;
extern struct flash_info w25q32bv_info;

#define SPI2_CLK_PORTB_MUX  ((uint16_t) ((uint16_t) 0<<20))
#define SPI2_MISO_PORTB_MUX  ((uint16_t) ((uint16_t) 0<<22))
#define SPI2_MOSI_PORTB_MUX  ((uint16_t) ((uint16_t) 0<<24))
#define SPI2_D2_PORTB_MUX  ((uint16_t) ((uint16_t) 0<<26))
#define SPI2_D3_PORTB_MUX  ((uint16_t) ((uint16_t) 0<<28))

#define SPI2_CLK_PORTB_FER  ((uint16_t) ((uint16_t) 1<<10))
#define SPI2_MISO_PORTB_FER  ((uint16_t) ((uint16_t) 1<<11))
#define SPI2_MOSI_PORTB_FER  ((uint16_t) ((uint16_t) 1<<12))
#define SPI2_D2_PORTB_FER  ((uint16_t) ((uint16_t) 1<<13))
#define SPI2_D3_PORTB_FER  ((uint16_t) ((uint16_t) 1<<14))

static void dpia_init(void)
{
	spi_ctlr = &adi_spi_bf6xx_ctlr;
	flash_info = &w25q32bv_info;

    /* PORTx_MUX registers */
    *pREG_PORTB_MUX = SPI2_CLK_PORTB_MUX | SPI2_MISO_PORTB_MUX
     | SPI2_MOSI_PORTB_MUX | SPI2_D2_PORTB_MUX | SPI2_D3_PORTB_MUX;

    /* PORTx_FER registers */
    *pREG_PORTB_FER = SPI2_CLK_PORTB_FER | SPI2_MISO_PORTB_FER
     | SPI2_MOSI_PORTB_FER | SPI2_D2_PORTB_FER | SPI2_D3_PORTB_FER;

	/* We use GPIO PB15 as SPI flash /CS. */
	*pREG_PORTB_FER_CLR = BITM_PORT_DATA_PX15;
	*pREG_PORTB_DIR_SET = BITM_PORT_DATA_PX15;
}

/*******************************************************************
*   Function:    TEST_SPI_FLASH
*   Description: This test will test the SPI flash on the EZ-Board.
*				 Since an image may live in the start of SPI flash
*                we will not erase the entire flash during this test.
*                We will first verify the manufacturer and vendor IDs
*				 then we will perform operations on only a few sectors.
*******************************************************************/
int TEST_SPI_FLASH(void)
{
	int i, j;								/* indexes */
	unsigned int iPassed = 1;				/* pass flag */
	int Result = 0;							/* result */
	unsigned short usRd = 0, usWr = 0;		/* storage for data */
	int nManCode = 0, nDevCode = 0;			/* man and device ids */
	uint8_t mid;							/* manufacturer id */
	uint8_t did;							/* device id */

	DEBUG_HEADER( "SPI Flash Test" );

	uint32_t uiSecTmr0 = *pREG_SEC0_SCTL12;
	*pREG_SEC0_SCTL12 = 0;
	EnableGPTimer(false);

	/* sectors to be tested: we chose one sector from a few different banks, making
	   sure we don't overwrite any sector of flash where the boot image lives */
	int pnTestSectors[] = { 20,		/* 0x000xx000 */
							30,		/* 0x000xx000 */
							40,		/* 0x000xx000 */
							-1};	/* must end with -1 */


	/* configure soft switches for the default */
	uint32_t nNumber = (uint32_t)(sizeof(SpiSoftSwitch)/sizeof(SOFT_SWITCH));
	ConfigSoftSwitches(SS_SPI, nNumber, (SOFT_SWITCH *)&SpiSoftSwitch );

	/* Do board specific initialization */
	dpia_init();

	Result = spi_open(SPI_NO);
	if (Result != 0)
		return 0;

	unselect_flash();

	spi_config(flash_info);
	flash_open(flash_info);

	flash_set_mode(flash_info, STANDARD);

	if (flash_info->modes & (QUAD_INPUT | QUAD_OUTPUT | QUAD_IO))
		flash_enable_quad_mode(flash_info);

	Result = flash_read_mid_did(flash_info, &mid, &did);
	if (Result == 0)
	{
		if (mid != flash_info->mid || did != flash_info->did)
			return 0;
	}

	DEBUG_PRINT( "\nDetected manufacturer code of 0x%x and device code of 0x%x\n", mid, did );

	/* if codes don't match what we expect then we should fail */
	if ( (MAN_CODE != mid) || (DEV_CODE != did) )
	{
		DEBUG_RESULT(TEST_FAIL, "Flash codes do not match what we expected");
		return 0;
	}

	/* for each test sector */
	for ( usWr = 0x1234, j = 0; (pnTestSectors[j] != -1) && (iPassed); j++ )
	{
		/* make sure it's a valid sector */
		if ( (pnTestSectors[j] < 0) || (pnTestSectors[j] >= NUM_SECTORS) )
		{
			DEBUG_RESULT(TEST_FAIL, "Sector selected is not supported");
			return 0;
		}

		/* erase the sector */
		Result = flash_erase_sector(flash_info, pnTestSectors[j]);
		
		/* if the erase passed, continue */
		if(NO_ERR == Result)
		{

			DEBUG_PRINT( "Testing sector %d...\n", pnTestSectors[j] );

			flash_set_mode(flash_info, QUAD_INPUT);

			/* write some data */
			for( i = 0; i < 0x8; i+=1, usWr+= 0x1111 )
			{
				unsigned long ulOffset;

				/* calculate offset based on sector */
				ulOffset = pnTestSectors[j]*64*1024 + i;	/* 64KB sectors */

				/* write a value to the flash */
				Result = flash_program(flash_info, ulOffset, (uint8_t *)&usWr, 0x1);

				if(NO_ERR == Result)
				{
					iPassed = 1;

					/* now do a read */
					Result = flash_read(flash_info, ulOffset, (uint8_t *)&usRd, 1);

					if(NO_ERR == Result)
					{
						/* compare the data */
						if( (usRd & 0xff) != (usWr & 0xff) )
						{
							iPassed = 0;
							break;
						}
					}
					else
					{
						iPassed = 0;
						break;
					}
				}
				else
				{
					iPassed = 0;
					break;
				}
			}
		}

		/* if erase failed, break out */
		else
		{
			iPassed = 0;
			break;
		}

		/* at this point the sector test passed, erase it again */
		flash_set_mode(flash_info, STANDARD);
		Result = flash_erase_sector(flash_info, pnTestSectors[j]);
	}

	/* close driver and return result */
	unselect_flash();

	Result = spi_close();
	if (Result != 0)
		return 0;

	flash_close(flash_info);

	EnableGPTimer(true);
	*pREG_SEC0_SCTL12 = uiSecTmr0;
	
	return iPassed;
}
