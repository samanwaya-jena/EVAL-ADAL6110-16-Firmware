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
#include "user_bulk.h"

#include <string.h>
#include <builtins.h>

#include "../adal6110_16.h"
#include "cld_bf70x_bulk_lib.h"

#include "Serial_cmd.h"
#include "USB_cmd.h"

#include "../demo_app.h"
#include "../error_handler.h"
#include "../parameters.h"

#include "../post_debug.h"





static USB_CAN_message user_bulk_adi_loopback_buffer;

/* Function prototypes */
CLD_USB_Transfer_Request_Return_Type user_bulk_bulk_out_data_received(CLD_USB_Transfer_Params * p_transfer_data);

static CLD_USB_Data_Received_Return_Type user_bulk_adi_can_cmd_received (void);
static void user_bulk_adi_loopback_device_transfer_error (void);
static void user_bulk_adi_loopback_bulk_in_transfer_complete (void);
static void user_bulk_console_rx_byte (unsigned char byte);
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
    .uart_baud  = 1000000,/*115200,                               * CLD Library CONSOLE print UART
                                                           baudrate. */
    .sclk0      = 100000000u,                           /* Blackfin SCLK0 frequency */
    .fp_console_rx_byte = user_bulk_console_rx_byte,    /* Function called when a byte
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
    .fp_bulk_out_data_received = user_bulk_bulk_out_data_received,

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

    cld_rv = cld_bf70x_bulk_lib_init(&user_bulk_init_params);

    if (cld_rv == CLD_SUCCESS)
    {
    	cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "\nFirmware version: %02d.%03d\n\r",
    	    		FIRMWARE_MAJOR_REV, FIRMWARE_MINOR_REV );
        cld_lib_usb_connect();

        return USER_BULK_INIT_SUCCESS;
    }
    else if(cld_rv == CLD_FAIL )
    {
    	SetError(error_SW_Boot);
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

Description:    user main loop... called at the main() function, handles the main operation of the system...

Returns:        None.
==============================================================================*/
void user_bulk_main (void)
{
    cld_bf70x_bulk_lib_main();
    Serial_Process();

}

/*=============================================================================
Function:       user_bulk_bulk_out_data_received

Parameters:     p_transfer_data - Pointer to the bulk OUT transfer data.
                    p_transfer_data->num_bytes     - Number of received bulk OUT bytes.
                                                     This value can be set to the total
                                                     transfer size if more then one
                                                     packet is expected.
                    p_transfer_data->p_data_buffer - Set to the address where the
                                                     bulk data should be written.
                    p_transfer_data->callback.fp_usb_out_transfer_complete -
                                                     Function called when the
                                                     requested received bytes have
                                                     been written to p_data_buffer.
                    p_transfer_data->fp_transfer_aborted_callback -
                                                     Optional function that is
                                                     called if the transfer is
                                                     aborted.

Description:    This function is called by the cld_bulk library when data is
                received on the Bulk OUT endpoint. This function sets the
                p_transfer_data parameters to select where the received data
                should be stored, and what function should be called when the
                transfer is complete.

Returns:        CLD_USB_TRANSFER_ACCEPT - Store the bulk data using the p_transfer_data
                                          parameters.
                CLD_USB_TRANSFER_PAUSE - The device isn't ready to process this
                                         bulk out packet so pause the transfer
                                         until the cld_bulk_resume_paused_bulk_out_transfer
                                         function is called.
                CLD_USB_TRANSFER_DISCARD - Discard this bulk packet.
                CLD_USB_TRANSFER_STALL - Stall the bulk OUT endpoint.
==============================================================================*/
CLD_USB_Transfer_Request_Return_Type user_bulk_bulk_out_data_received(CLD_USB_Transfer_Params * p_transfer_data)
{
    CLD_USB_Transfer_Request_Return_Type rv = CLD_USB_TRANSFER_STALL;

	if (cld_time_passed_ms(usb_time) >= 25u)
	{
		usb_time = cld_time_get();
		SetError(error_SW_comm_timeout);
	}

    if (p_transfer_data->num_bytes == sizeof(USB_CAN_message))
	{
		/* Save the received data to the user_bulk_adi_loopback_data.cmd structure,
		   and call user_bulk_adi_loopback_cmd_received once the data has been received. */
		p_transfer_data->p_data_buffer = (unsigned char *) &user_bulk_adi_loopback_buffer;
		p_transfer_data->callback.fp_usb_out_transfer_complete = user_bulk_adi_can_cmd_received;
		p_transfer_data->fp_transfer_aborted_callback = user_bulk_adi_loopback_device_transfer_error;
		p_transfer_data->transfer_timeout_ms = 1000;
		rv = CLD_USB_TRANSFER_ACCEPT;
	}

    return rv;
}

static CLD_USB_Data_Received_Return_Type user_bulk_adi_can_cmd_received (void)
{
    /* Parameters used to send Bulk IN data in response to the
       current command. */
    static CLD_USB_Transfer_Params transfer_params =
    {
        .fp_transfer_aborted_callback = user_bulk_adi_loopback_device_transfer_error
    };

    USB_CAN_message* usbCMDmsg = (USB_CAN_message *)&user_bulk_adi_loopback_buffer;
    USB_msg usbResp;
    if( LiDARParameters[param_console_log] & CONSOLE_MASK_USB)
    {
		cld_console(CLD_CONSOLE_YELLOW, CLD_CONSOLE_BLACK, "-->0x%04X (%02X %02X %02X %02X %02X %02X %02X %02X) ",
				usbCMDmsg->id, usbCMDmsg->data[0], usbCMDmsg->data[1], usbCMDmsg->data[2], usbCMDmsg->data[3], usbCMDmsg->data[4],
				usbCMDmsg->data[5], usbCMDmsg->data[6], usbCMDmsg->data[7]);
    }
    USB_ReadCommand(usbCMDmsg, &usbResp);
    if( LiDARParameters[param_console_log] & CONSOLE_MASK_USB)
    {
		if (usbResp.CAN.id != msgID_transmitRaw)
		{
			cld_console(CLD_CONSOLE_YELLOW, CLD_CONSOLE_BLACK, " <--0x%04X (%02X %02X %02X %02X %02X %02X %02X %02X)",
					usbResp.CAN.id, usbResp.CAN.data[0], usbResp.CAN.data[1], usbResp.CAN.data[2], usbResp.CAN.data[3], usbResp.CAN.data[4],
					usbResp.CAN.data[5], usbResp.CAN.data[6], usbResp.CAN.data[7]);
		}
		else
		{
			cld_console(CLD_CONSOLE_YELLOW, CLD_CONSOLE_BLACK, " <--0x%04X ", usbResp.CAN.id);
		}
    }
	/* return message callback*/
	transfer_params.num_bytes = (usbResp.CAN.id != msgID_transmitRaw)?sizeof(USB_raw_message):sizeof(USB_CAN_message);
	transfer_params.p_data_buffer = (unsigned char*)&usbResp;
	transfer_params.callback.fp_usb_in_transfer_complete = user_bulk_adi_loopback_bulk_in_transfer_complete;
	transfer_params.transfer_timeout_ms = 1000;
	cld_bf70x_bulk_lib_transmit_bulk_in_data(&transfer_params);
	if( LiDARParameters[param_console_log] & CONSOLE_MASK_USB)
	{
		cld_console(CLD_CONSOLE_YELLOW, CLD_CONSOLE_BLACK, "\n\r");
	}
    return CLD_USB_DATA_GOOD;
}


/*=============================================================================
Function:       user_bulk_adi_loopback_device_transfer_error

Parameters:     None.

Description:    This function is called if a requested transfer is aborted.

Returns:        None.
==============================================================================*/
static void user_bulk_adi_loopback_device_transfer_error (void)
{
	if( LiDARParameters[param_console_log] & CONSOLE_MASK_USB)
		cld_console(CLD_CONSOLE_RED, CLD_CONSOLE_BLACK, "CLD Bulk Device transfer error (aborted)!\n\r");
//    user_bulk_adi_loopback_data.state = ADI_BULK_LOOPBACK_DEVICE_STATE_IDLE;
}

/*=============================================================================
Function:       user_bulk_adi_loopback_bulk_in_transfer_complete

Parameters:     None.

Description:    This function is called when a Bulk IN transfer is complete.

Returns:        None.
==============================================================================*/
static void user_bulk_adi_loopback_bulk_in_transfer_complete (void)
{
	/*
	 * Cette routine est utile avec la vieille comm... Autrement on n'a rien a faire
	 * a la fin du transfert de donnees vers l'hote
	 */
	//todo: remove!
	if (numPending)
	{
    	ADAL_ReleaseDataToFifo(numPending);  // incremente la tail du fifo de numPending...
    	numPending = 0;
	}
}

/*=============================================================================
Function:       user_bulk_console_rx_byte

Parameters:

Description:

Returns:
==============================================================================*/
static void user_bulk_console_rx_byte (unsigned char byte)
{
	Serial_RxChar(byte);
}

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
        	break;
        case CLD_USB_CABLE_DISCONNECTED:
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
