/*==============================================================================
    FILE:           user_bulk.c

    DESCRIPTION:    Uses the cld_bulk library to implement the bulk loopback
                    features of the hostapp.exe test program.

    Copyright (c) 2014 Closed Loop Design, LLC

    This software is supplied "AS IS" without any warranties, express, implied
    or statutory, including but not limited to the implied warranties of fitness
    for purpose, satisfactory quality and non-infringement. Closed Loop Design LLC
    extends you a royalty-free right to reproduce and distribute executable files
    created using this software for use on Analog Devices Blackfin family
    processors only. Nothing else gives you the right to use this software.

==============================================================================*/
#include <string.h>
#include <builtins.h>
#include "user_bulk.h"
#include "cld_bf70x_bulk_lib.h"

#include <ADSP-BF707_device.h>



#define FIRMWARE_MAJOR_REV 0
#define FIRMWARE_MINOR_REV 8


/* Function prototypes */
static void user_bulk_usb_event (CLD_USB_Event event);


/* Bulk IN endpoint parameters */
static CLD_Bulk_Endpoint_Params user_bulk_in_endpoint_params =
{
    .endpoint_number            = 1,
    .max_packet_size_full_speed = 64,
    .max_packet_size_high_speed = 512,
};

/* Bulk OUT endpoint parameters */
static CLD_Bulk_Endpoint_Params user_bulk_out_endpoint_params =
{
    .endpoint_number            = 1,
    .max_packet_size_full_speed = 64,
    .max_packet_size_high_speed = 512,
};

/* cld_bulk library initialization data. */
static CLD_BF70x_Bulk_Lib_Init_Params user_bulk_init_params =
{
    .timer_num  = CLD_TIMER_0,                          /* Timer used by the CLD Library */
    .uart_num   = CLD_UART_0,                           /* UART used by the CLD Library.
                                                            If the uart_num is set to
                                                            CLD_UART_DISABLE the CLD library
                                                            will not use a UART */
    .uart_baud  = 115200,                               /* CLD Library CONSOLE print UART
                                                           baudrate. */
    .sclk0      = 200000000u,                           /* Blackfin SCLK0 frequency */
    .fp_console_rx_byte = NULL,    /* Function called when a byte
                                                           is received by the CONSOLE
                                                           UART. */
    .vendor_id = 0x064b,                                /* Analog Devices Vendor ID */
    .product_id = 0x7823,                               /* Analog Devices Product ID
                                                           used by the hostapp program. */

    /* Bulk IN endpoint parameters */
    .p_bulk_in_endpoint_params = &user_bulk_in_endpoint_params,

    /* Bulk OUT endpoint parameters */
    .p_bulk_out_endpoint_params = &user_bulk_out_endpoint_params,

    /* Function called when bulk OUT data has been received. */
    .fp_bulk_out_data_received = NULL,

    .usb_bus_max_power = 0,                             /* The ADSP-BF707 EZ-Board is
                                                           self-powered (not USB bus-powered) */

    .device_descriptor_bcdDevice = 0x0100,              /* Set USB Device Descriptor
                                                           firmware version to 1.00 */
    /* USB string descriptors - Set to CLD_NULL if not required */
    .p_usb_string_manufacturer  = "Analog Devices Inc",
    .p_usb_string_product       = "BF707 Bulk Loopback Device",
    .p_usb_string_serial_number = CLD_NULL,
    .p_usb_string_configuration = CLD_NULL,
    .p_usb_string_interface     = "BF707 Bulk Loopback Demo",

    .usb_string_language_id     = 0x0409,               /* English (US) language ID */

    /* Function called when one of the following USB events occurs
        CLD_USB_CABLE_CONNECTED      - USB cable connected
        CLD_USB_CABLE_DISCONNECTED   - USB cable disconnected
        CLD_USB_ENUMERATED_CONFIGURED- USB Enumerated (USB Configuration set to non-zero value)
        CLD_USB_UN_CONFIGURED        - USB Configuration set to 0.
        CLD_USB_BUS_RESET            - USB Bus Reset received. */
    .fp_cld_usb_event_callback = user_bulk_usb_event,
};

static uint16_t numPending = 0;

static CLD_Time usb_time = 0;



#define DISABLE_INT() unsigned int _intm = cli()
#define ENABLE_INT() sti(_intm)






/*=============================================================================
Function:       user_bulk_init

Parameters:     None.

Description:    Initializes the CLD Bulk library.

Returns:        USER_BULK_INIT_SUCCESS/USER_BULK_INIT_ONGOING/USER_BULK_INIT_FAILED.
==============================================================================*/
User_Bulk_Init_Return_Code user_bulk_init (void)
{
    static unsigned char user_init_state = 0;
    CLD_RV cld_rv = CLD_ONGOING;

    /* Initalize the CLD Bulk Library */
    cld_rv = cld_bf70x_bulk_lib_init(&user_bulk_init_params);

    /*cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "\nFirmware version: %d.%d\n\n",
    		FIRMWARE_MAJOR_REV, FIRMWARE_MINOR_REV );
*/
    if (cld_rv == CLD_SUCCESS)
    {
        /* Connect to the USB Host */
        cld_lib_usb_connect();

        return USER_BULK_INIT_SUCCESS;
    }
    else if (cld_rv == CLD_FAIL)
    {
        return USER_BULK_INIT_FAILED;
    }
    else
    {
        return USER_BULK_INIT_ONGOING;
    }
}

/*=============================================================================
Function:       user_bulk_main

Parameters:     None.

Description:    Example user mainline.

Returns:        None.
==============================================================================*/

/*
 *
void user_bulk_main (void)
{
    #define MINUTES(x)      (x/60000)
    #define SECONDS(x)      ((x/1000)%60)
    #define M_SECONDS(x)    (x%1000)

    static CLD_Time main_time = 0;
//    static CLD_Time msg_time = 0;
//    static CLD_Time run_time = 0;
    static CLD_Time log_time = 0;
//    static CLD_Time acq_time = 0;

    static int iAcqNum = 0;
    static int iAcqNum1 = 0;
    static int iAcqNum2 = 0;
    static int iAcqNumX = 0;

    cld_bf70x_bulk_lib_main();

    if (cld_time_passed_ms(main_time) >= 250u)
    {
        main_time = cld_time_get();
        LED_BC2_TGL();
    }

    if (usb_time)
    {
		if (cld_time_passed_ms(usb_time) >= 500u)
		{
			usb_time = 0;
			LED_BC3_OFF();
		}
    }

    Serial_Process();

//    if (cld_time_passed_ms(acq_time) >= 1u)
    {
    	uint16_t banknum = 0;
//    	acq_time = cld_time_get();

    	Lidar_Acq(&banknum);

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

	if (gLogData & 1)
	{
		if (cld_time_passed_ms(log_time) >= 1000u)
		{
			log_time = cld_time_get();
			cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Acq: %d (%d,%d,%d) USB %d (%d,%d)\r\n", iAcqNum, iAcqNum1, iAcqNum2, iAcqNumX, iUSBnum, iUSBnumOK, iUSBnumEmpty);
			iAcqNum = iAcqNum1 = iAcqNum2 = iAcqNumX = 0;
			iUSBnum = iUSBnumOK = iUSBnumEmpty = 0;
			//pADI_PORTA->DATA_TGL = (1 << 8); //TODO laser 1 test REMOVE in release code
			//pADI_PORTA->DATA_TGL = (1 << 9); //TODO laser 1 test REMOVE in release code
		}
	}

//    if (cld_time_passed_ms(msg_time) >= 237u)
//    {
//        msg_time = cld_time_get();
//        run_time = cld_time_passed_ms(0);
//        cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Run time: %2.2d:%2.2d:%3.3d\r",
//            MINUTES(run_time),
//            SECONDS(run_time),
//            M_SECONDS(run_time));
//    }
}

*/


/*=============================================================================
Function:       user_bulk_usb_event

Parameters:     event - identifies which USB event has occurred.

Description:    Function Called when a USB event occurs.

Returns:        None.
==============================================================================*/
static void user_bulk_usb_event (CLD_USB_Event event)
{
    switch (event)
    {
        case CLD_USB_CABLE_CONNECTED:
//            pADI_PORTA->DATA_SET = (1 << 1);
        break;
        case CLD_USB_CABLE_DISCONNECTED:
//            pADI_PORTA->DATA_CLR = (1 << 1);
        break;
        case CLD_USB_ENUMERATED_CONFIGURED:
            cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "CLD Bulk Device Enumerated\n\r");
        break;
        case CLD_USB_UN_CONFIGURED:
        break;
        case CLD_USB_BUS_RESET:
//            user_bulk_adi_loopback_data.state = ADI_BULK_LOOPBACK_DEVICE_STATE_IDLE;
        break;
        case CLD_USB_BUS_SUSPEND:
            cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "CLD Bulk Device Suspend\n\r");
        break;
        case CLD_USB_BUS_RESUME:
            cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "CLD Bulk Device Resume\n\r");
        break;
    }
}

