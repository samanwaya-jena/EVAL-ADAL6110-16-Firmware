/*
  Copyright 2018 Phantom Intelligence Inc.
*/

#include <stdio.h>
#include <string.h>
#include <sys/platform.h>

#include "adi_initialize.h"
#include <cdefBF70x_rom.h>
#include <ADSP-BF707_device.h>

#include "Lidar_adal6110_16.h"
#include "user_bulk.h"
#include "flash_params.h"
#include "PWR_Freq_Mode.h"
#include "demo_app.h"

#define USE_USB

typedef enum
{
    MAIN_STATE_SYSTEM_INIT,
    MAIN_STATE_USER_INIT,
    MAIN_STATE_RUN,
    MAIN_STATE_ERROR
} Main_States;

void setup_gpio_state();

int main(int argc, char *argv[])
{

	//TODO REMOVE ALL REFERENCE TO GUARDIANs?
	//TODO REVIEWS GUARDIAN.C

	// SET IO DIRECTION
 	setup_gpio_state();

	/**
	 * Initialize managed drivers and/or services that have been added to 
	 * the project.
	 * @return zero on success 
	 */

	adi_initComponents();
	
    /* Initialize Power service */
    power_init();

    /* Initialize the Flash device */
	Flash_Init();

	 /* Initialize the Lidar Parameter */
	Lidar_InitADI();

    Main_States main_state = MAIN_STATE_SYSTEM_INIT;

    //TODO Disable Laser for safety purpose, re-enable those line when needed

    LP_DRIVER_POWER_ON();
    LASER_OUTPUT_ENABLE();
    LASER_PULSE1_ENABLE();
    LASER_PULSE2_ENABLE();

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


void setup_gpio_state()
{
    LASER_PULSE1_DISABLE();
    LASER_PULSE2_DISABLE();
    LASER_OUTPUT_DISABLE();
    LP_DRIVER_POWER_OFF();

    pADI_PORTA->DIR_SET = (1 << 8); // LASER_1
	pADI_PORTA->DIR_SET = (1 << 9); // LASER_2
	pADI_PORTB->DIR_SET = (1 << 5); // EN_P_N
	pADI_PORTB->DIR_SET = (1 << 6); // PD_P

    pADI_PORTC->DIR_SET = (1 << 7); 	//LED_BC2_R
    pADI_PORTC->DIR_SET = (1 << 8); 	//LED_BC2_G
    pADI_PORTC->DIR_SET = (1 << 11); 	//LED_BC3_R
    pADI_PORTC->DIR_SET = (1 << 12); 	//LED_BC3_G

    //SET Debug LED
    pADI_PORTC->DATA_CLR = (1 << 7);
    pADI_PORTC->DATA_SET = (1 << 8);
	pADI_PORTC->DATA_CLR = (1 << 11);
	pADI_PORTC->DATA_SET = (1 << 12);

	//TRIG IN, in output mode Set @ 1
	pADI_PORTA->DIR_SET = (1 << 15);
	pADI_PORTA->DATA_SET = (1 << 15);
}
