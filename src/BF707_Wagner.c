/*****************************************************************************
 * BF707_Wagner.c
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/platform.h>

#include "adi_initialize.h"

#include "Guardian_ADI.h"
#include "SoftConfig_BF707.h"

#include "user_bulk.h"
#include <ADSP-BF707_device.h>

#include "flash_params.h"

#include "PWR_Freq_Mode.h"
#include "post_debug.h"

#include "BF707_Wagner.h"



#define USE_USB



typedef enum
{
    MAIN_STATE_SYSTEM_INIT,
    MAIN_STATE_USER_INIT,
    MAIN_STATE_RUN,
    MAIN_STATE_ERROR
} Main_States;

int main(int argc, char *argv[])
{
//	DEBUG_HEADER( "Wagner ADSP-BF707 Eval Board" );

    pADI_PORTA->DIR_SET = (3 << 0);
    pADI_PORTB->DIR_SET = (1 << 1);

	/**
	 * Initialize managed drivers and/or services that have been added to 
	 * the project.
	 * @return zero on success 
	 */
	adi_initComponents();
	
    /* Initialize Power service */
    power_init();

	/* Set the Software controlled switches to default values */
	ConfigSoftSwitches(SS_DEFAULT, 0, NULL);

	Flash_Init();

	Lidar_InitADI();

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

                main_state = MAIN_STATE_USER_INIT;
            break;

            case MAIN_STATE_USER_INIT:
            {
#ifdef USE_USB
            	User_Bulk_Init_Return_Code rv = user_bulk_init();
                if (rv == USER_BULK_INIT_SUCCESS)
                {
                    main_state = MAIN_STATE_RUN;
                }
                else if (rv == USER_BULK_INIT_FAILED)
                {
                    main_state = MAIN_STATE_ERROR;
                }
#else //USE_USB
                main_state = MAIN_STATE_RUN;
#endif //USE_USB
                break;
            }

            case MAIN_STATE_RUN:
#ifdef USE_USB
                user_bulk_main();
#else //USE_USB
                {
                   	uint16_t banknum = 0;
					Lidar_Acq(&banknum);
                }
#endif //USE_USB
            break;

            case MAIN_STATE_ERROR:

            break;
        }
    }
}
