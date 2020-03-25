/*
  Copyright 2018 Phantom Intelligence Inc.
*/

#include <stdio.h>
#include <string.h>
#include <sys/platform.h>

#include "adi_initialize.h"
#include <cdefBF70x_rom.h>
//#include <ADSP-BF707_device.h>

#include "Communications/user_bulk.h"
#include "Communications/USB_cmd.h"
#include "Communications/cld_bf70x_bulk_lib.h"
#include "flash/flash_params.h"
#include "PWR_Freq_Mode.h"
#include "demo_app.h"

#include "adal6110_16.h"
#include "parameters.h"

#define USE_USB

typedef enum
{
    MAIN_STATE_SYSTEM_INIT,
    MAIN_STATE_USER_INIT,
    MAIN_STATE_RUN,
    MAIN_STATE_ERROR
} Main_States;

Main_States main_state;

void setup_gpio_state(void);
inline void InitApp(void);
inline void DoMainStateRun(void);
inline void DoMainStateSystemInit(void);



int main(int argc, char *argv[])
{
	InitApp();

	while (1)
    {
        switch (main_state)
        {
            case MAIN_STATE_SYSTEM_INIT:
            	LED_BC3G_ON();LED_BC3R_ON(); //amber
            	DoMainStateSystemInit();
            	break;

            case MAIN_STATE_USER_INIT:
            {
            	LED_BC3G_ON();LED_BC3R_ON(); //amber
#ifdef USE_USB
            	User_Bulk_Init_Return_Code rv = user_bulk_init();
                if (rv == USER_BULK_INIT_SUCCESS)
                {
            		param_InitValues();
                	USB_pushBoot();
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
            	LED_BC3G_ON();LED_BC3R_OFF();//Green
#ifdef USE_USB
                DoMainStateRun();
#else //USE_USB
                {
                   	uint16_t banknum = 0;
					ADAL_Acq(&banknum);
                }
#endif //USE_USB
            break;

            case MAIN_STATE_ERROR:
            	LED_BC3G_OFF();LED_BC3R_ON();// red
            break;
        }
    }
}

void InitApp()
{
	Main_States main_state = MAIN_STATE_SYSTEM_INIT;

	setup_gpio_state();

	/**
	 * Initialize managed drivers and/or services that have been added to
	 * the project.
	 */
	if(adi_initComponents()) main_state = MAIN_STATE_ERROR;
	if(!power_init()) main_state = MAIN_STATE_ERROR;
	if(Flash_Init()) main_state = MAIN_STATE_ERROR;
	ADAL_InitADI();

	if (main_state == MAIN_STATE_SYSTEM_INIT)
	{
		LP_DRIVER_POWER_ON();
		LASER_OUTPUT_ENABLE();
		LASER_PULSE1_ENABLE();
		LASER_PULSE2_ENABLE();
	}
}

void DoMainStateRun()
{
	#define MINUTES(x)      (x/60000)
    #define SECONDS(x)      ((x/1000)%60)
    #define M_SECONDS(x)    (x%1000)

	static CLD_Time log_time = 0;
	static CLD_Time watchTime  = 0;

    static int iAcqNum = 0;
    static int iAcqNum1 = 0;
    static int iAcqNum2 = 0;
    static int iAcqNumX = 0;

	/*
	 * Status validation and board behavior
	 */
	if (cld_time_passed_ms(watchTime) >= LiDARParameters[param_status_period])//watchDelay)
	{
		watchTime = cld_time_get();
		LED_BC2G_TGL();
		USB_pushStatus();
	}
	// every second, log how many data has been logged
	if ((LiDARParameters[param_console_log] & CONSOLE_MASK_LOG) && (cld_time_passed_ms(log_time) >= 1000u))
	{
		log_time = cld_time_get();
		cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Acq: %d (bank 1:%d / bank 2:%d / unknown:%d) USB %d (OK:%d, Empty:%d)\r\n",
				     iAcqNum, iAcqNum1, iAcqNum2, iAcqNumX, iUSBnum, iUSBnumOK, iUSBnumEmpty);
		iAcqNum = iAcqNum1 = iAcqNum2 = iAcqNumX = 0;
		iUSBnum = iUSBnumOK = iUSBnumEmpty = 0;
	}
	/*
	 * Communications
	 */
	user_bulk_main();
	/*
	 * Lidar operations
	 */
	uint16_t banknum = 0;
	ADAL_Acq(&banknum);
	if (!banknum)// || !LiDARParameters[param_acq_enable])
		param_ProcessReadWriteFifo();

	if (banknum)
	{
		++iAcqNum;
		if (banknum == 1)
			++iAcqNum1;
		else if (banknum == 2)
			++iAcqNum2;
		else
			++iAcqNumX;
	}
}

inline void DoMainStateSystemInit() {
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
