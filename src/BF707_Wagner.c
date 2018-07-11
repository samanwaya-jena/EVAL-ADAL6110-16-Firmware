/*****************************************************************************
 * BF707_Wagner.c
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/platform.h>

#include <drivers\spi\adi_spi.h>
#include <services\pwr\adi_pwr.h>
#include <services/int/adi_sec.h>

#include "adi_initialize.h"
#include "BF707_Wagner.h"

#include "Guardian_ADI.h"

#include "user_bulk.h"
#include <ADSP-BF707_device.h>





typedef enum
{
    MAIN_STATE_SYSTEM_INIT,
    MAIN_STATE_USER_INIT,
    MAIN_STATE_RUN,
    MAIN_STATE_ERROR
} Main_States;






/* Sets the Software controlled switches */
extern void ConfigSoftSwitches(void);


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


    Main_States main_state = MAIN_STATE_SYSTEM_INIT;

    while (1)
    {
        switch (main_state)
        {
            case MAIN_STATE_SYSTEM_INIT:
                /* Enable and Configure the SEC. */

                /* sec_gctl - unlock the global lock  */
                pADI_SEC0->GCTL &= ~BITM_SEC_GCTL_LOCK;
                /* sec_gctl - enable the SEC in */
                pADI_SEC0->GCTL |= BITM_SEC_GCTL_EN;
                /* sec_cctl[n] - unlock */
                pADI_SEC0->CB.CCTL &= ~BITM_SEC_CCTL_LOCK;
                /* sec_cctl[n] - reset sci to default */
                pADI_SEC0->CB.CCTL |= BITM_SEC_CCTL_RESET;
                /* sec_cctl[n] - enable interrupt to be sent to core */
                pADI_SEC0->CB.CCTL = BITM_SEC_CCTL_EN;
                pADI_PORTA->DIR_SET = (3 << 0);
                pADI_PORTB->DIR_SET = (1 << 1);

                main_state = MAIN_STATE_USER_INIT;
            break;

            case MAIN_STATE_USER_INIT:
            {
            	User_Bulk_Init_Return_Code rv = user_bulk_init();
                if (rv == USER_BULK_INIT_SUCCESS)
                {
                    main_state = MAIN_STATE_RUN;
                }
                else if (rv == USER_BULK_INIT_FAILED)
                {
                    main_state = MAIN_STATE_ERROR;
                }
                break;
            }

            case MAIN_STATE_RUN:
                 user_bulk_main();
            break;

            case MAIN_STATE_ERROR:

            break;
        }
    }
}
