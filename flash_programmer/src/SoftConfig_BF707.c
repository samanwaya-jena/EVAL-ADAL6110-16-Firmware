/********************************************************************************
Copyright(c) 2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/

/*
 *  Software Switch Configuration for the ADSP-BF609-EZ-BOARD
 */

#include <drivers\twi\adi_twi.h>

/* TWI settings */
#define TWI_PRESCALE  (8u)
#define TWI_BITRATE   (100u)
#define TWI_DUTYCYCLE (50u)

#define BUFFER_SIZE (32u)

/* TWI hDevice handle */
static ADI_TWI_HANDLE hDevice;

/* TWI data buffer */
static uint8_t twiBuffer[BUFFER_SIZE];

/* TWI device memory */
uint8_t deviceMemory[ADI_TWI_MEMORY_SIZE];

/* switch register structure */
typedef struct {
	uint8_t Register;
	uint8_t Value;
} SWITCH_CONFIG;

/* switch parameter structure */
typedef struct {
	uint32_t TWIDevice;
	uint16_t HardwareAddress;
	uint32_t NumConfigSettings;
	SWITCH_CONFIG *ConfigSettings;
} SOFT_SWITCH;

/* prototype */
void ConfigSoftSwitches(void);

/* disable misra diagnostics as necessary */
#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_8_7:"Objects shall be defined at block scope")
#pragma diag(suppress:misra_rule_17_4:"Array indexing shall be the only allowed form of pointer arithmetic")
#endif /* _MISRA_RULES */


/************************SoftConfig Information********************************
	
	~ means the signal is active low
	
	Please see the ADSP-BF609 EZ-KIT manual for more information on using
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
    N Y Y N Y Y Y N                        |       X Y Y Y Y Y N N    ( Active Y or N )
    1 0 0 1 0 0 0 1                        |       0 0 0 0 0 0 1 1    ( value being set )
*/
  { 0x12u, 0xF1u },                               { 0x13u, 0x03u },

  /*
   * specify inputs/outputs
   */

  { 0x0u, 0x00u },    /* Set IODIRA direction (all output) */
  { 0x1u, 0x80u },    /* Set IODIRB direction (bit 7 input, all others output) */
};

/* switch configuration */
static SOFT_SWITCH SoftSwitch[] =
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
     
#if defined(ADI_DEBUG)
#include <stdio.h>
#define CHECK_RESULT(result, message) \
	do { \
		if((result) != ADI_TWI_SUCCESS) \
		{ \
			printf((message)); \
			printf("\n"); \
		} \
	} while (0)  /* do-while-zero needed for Misra Rule 19.4 */
#else
#define CHECK_RESULT(result, message)
#endif
 
void ConfigSoftSwitches(void)
{
	uint32_t switchNum;
	uint32_t configNum;
	uint32_t settings;
	uint32_t switches;

	SOFT_SWITCH *ss;
	SWITCH_CONFIG *configReg;
	ADI_TWI_RESULT result;

	switches = (uint32_t)(sizeof(SoftSwitch)/sizeof(SOFT_SWITCH));
	for (switchNum=0u; switchNum<switches; switchNum++)
	{
		ss = &SoftSwitch[switchNum];

		result = adi_twi_Open(ss->TWIDevice, ADI_TWI_MASTER, 
    		deviceMemory, ADI_TWI_MEMORY_SIZE, &hDevice);
		CHECK_RESULT(result, "adi_twi_Open failed");

		result = adi_twi_SetHardwareAddress(hDevice, ss->HardwareAddress);
		CHECK_RESULT(result, "adi_twi_SetHardwareAddress failed");

		result = adi_twi_SetPrescale(hDevice, TWI_PRESCALE);
		CHECK_RESULT(result, "adi_twi_Prescale failed");

		result = adi_twi_SetBitRate(hDevice, TWI_BITRATE);
		CHECK_RESULT(result, "adi_twi_SetBitRate failed");

		result = adi_twi_SetDutyCycle(hDevice, TWI_DUTYCYCLE);
		CHECK_RESULT(result, "adi_twi_SetDutyCycle failed");
		
		/* switch register settings */
		for (configNum=0u; configNum<ss->NumConfigSettings; configNum++)
		{
			configReg = &ss->ConfigSettings[configNum];

			/* write register value */
			twiBuffer[0] = configReg->Register;
			twiBuffer[1] = configReg->Value;
			result = adi_twi_Write(hDevice, twiBuffer, (uint32_t)2, false);
			CHECK_RESULT(result, "adi_twi_Write failed");
		}

		result = adi_twi_Close(hDevice);
		CHECK_RESULT(result, "adi_twi_Close failed");
	}
}

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */
 
