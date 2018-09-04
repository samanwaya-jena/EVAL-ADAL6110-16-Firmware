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
#include "user_bulk.h"
#include "cld_bf70x_bulk_lib.h"

#include <ADSP-BF707_device.h>
#include "BF707_Wagner.h"

#include "Guardian_ADI.h"

#include "Serial_cmd.h"

#include "AWLCANMessageDef.h"


#define FIRMWARE_MAJOR_REV 0
#define FIRMWARE_MINOR_REV 5


static AWLCANMessage user_bulk_adi_loopback_buffer;

/* Function prototypes */
static CLD_USB_Transfer_Request_Return_Type user_bulk_bulk_out_data_received(CLD_USB_Transfer_Params * p_transfer_data);

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
    .uart_baud  = 115200,                               /* CLD Library CONSOLE print UART
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






//
// CAN messages FIFO out
//

volatile int iCANFifoHead = 0;
volatile int iCANFifoTail = 0;

#define CANFIFO_SIZE 128
#define CANFIFO_MASK (CANFIFO_SIZE - 1)

AWLCANMessage canFifo[CANFIFO_SIZE];


int CANFifoPushMsg(AWLCANMessage * pCanMsg)
{
	int nextHead = (iCANFifoHead + 1) & CANFIFO_MASK;

	if (nextHead != iCANFifoTail)
	{
		memcpy(&canFifo[iCANFifoHead], pCanMsg, sizeof(AWLCANMessage));

		iCANFifoHead = nextHead;

		return 0;
	}

	return 1;
}

int user_CANFifoPushCompletedFrame(void)
{
	AWLCANMessage canMsg;

	canMsg.id = AWLCANMSG_ID_COMPLETEDFRAME;

	return CANFifoPushMsg(&canMsg);
}

int user_CANFifoPushDetection(int ch, uint16_t dist, uint16_t vel)
{
	int ret;
	AWLCANMessage canMsg;

	canMsg.id = AWLCANMSG_ID_OBSTACLETRACK;
	canMsg.len = AWLCANMSG_LEN;
	canMsg.data[0] = ch + 1;      //trackID
	canMsg.data[1] = 0;           //...
	canMsg.data[2] = 0x01 << ch;  //track->trackChannels.byteData
	canMsg.data[3] = ch;          //track->trackMainChannel
	canMsg.data[4] = 0;           //...
	canMsg.data[5] = 99;          //track->probability
	canMsg.data[6] = 44;          //track->intensity
	canMsg.data[7] = 0;           //...

	ret = CANFifoPushMsg(&canMsg);

	if (ret)
		return ret;

	canMsg.id = AWLCANMSG_ID_OBSTACLEVELOCITY;
	canMsg.len = AWLCANMSG_LEN;
	canMsg.data[0] = ch + 1;      //trackID
	canMsg.data[1] = 0;           //...
	canMsg.data[2] = dist >> 0;   //Distance
	canMsg.data[3] = dist >> 8;   //...
	canMsg.data[4] = vel >> 0;    //Velocity
	canMsg.data[5] = vel >> 8;    //...
	canMsg.data[6] = 0;
	canMsg.data[7] = 0;

	return CANFifoPushMsg(&canMsg);
}

int user_CANFifoPushReadResp(uint16_t registerAddress, uint16_t data)
{
	AWLCANMessage canMsg;

	canMsg.id = AWLCANMSG_ID_COMMANDMESSAGE;
	canMsg.len = AWLCANMSG_LEN;
	canMsg.data[0] = AWLCANMSG_ID_CMD_RESPONSE_PARAMETER;
	canMsg.data[1] = AWLCANMSG_ID_CMD_PARAM_AWL_REGISTER;

	canMsg.data[2] = (unsigned char) (registerAddress >> 0);
	canMsg.data[3] = (unsigned char) (registerAddress >> 8);
	canMsg.data[4] = (unsigned char) (data >> 0);
	canMsg.data[5] = (unsigned char) (data >> 8);
	canMsg.data[6] = 0;
	canMsg.data[7] = 0;

	return CANFifoPushMsg(&canMsg);
}

int user_CANFifoPushSensorStatus(void)
{
	AWLCANMessage canMsg;

	canMsg.id = AWLCANMSG_ID_SENSORSTATUS;
	canMsg.len = AWLCANMSG_LEN;
	canMsg.data[0] = 234 >> 0;
	canMsg.data[1] = 234 >> 8;
	canMsg.data[2] = 5 >> 0;
	canMsg.data[3] = 5 >> 8;
	canMsg.data[4] = 100;
	canMsg.data[5] = 0;
	canMsg.data[6] = 0;
	canMsg.data[7] = 0;

	return CANFifoPushMsg(&canMsg);
}

int user_CANFifoPushSensorBoot(void)
{
	AWLCANMessage canMsg;

	canMsg.id = AWLCANMSG_ID_SENSORBOOT;
	canMsg.len = AWLCANMSG_LEN;
	canMsg.data[0] = FIRMWARE_MAJOR_REV;
	canMsg.data[1] = FIRMWARE_MINOR_REV;
	canMsg.data[2] = 0;
	canMsg.data[3] = 0;
	canMsg.data[4] = 0;
	canMsg.data[5] = 0;
	canMsg.data[6] = 0;
	canMsg.data[7] = 0;

	return CANFifoPushMsg(&canMsg);
}

static int PollCanCommandMsg(AWLCANMessage * pCanReq, AWLCANMessage * pCanResp, int * pNum)
{
	int num = pCanReq->data[0];
	int i;

	if (num > CANFIFO_MASK)
		num = CANFIFO_MASK;

	for(i=0; i<num; i++)
	{
		if (iCANFifoHead != iCANFifoTail)
		{
			memcpy(pCanResp++, &canFifo[iCANFifoTail], sizeof(AWLCANMessage));

			iCANFifoTail = (iCANFifoTail + 1) & CANFIFO_MASK;
		}
	}

	*pNum = num;

	return 0;
}

static int ProcessLidarQueryCanCommandMsg(AWLCANMessage * pCanReq, AWLCANMessage * pCanResp)
{
	int _iCANFifoHead = iCANFifoHead;
	int _iCANFifoTail = iCANFifoTail;

	tDataFifo * pData = NULL;
	uint16_t numPendingTemp = 0;

	Lidar_GetDataFromFifo(&pData, &numPendingTemp);

	int numCANMsg = 0;
	if (_iCANFifoTail < _iCANFifoHead)
		numCANMsg = _iCANFifoHead - _iCANFifoTail;
	else if (_iCANFifoTail > _iCANFifoHead)
		numCANMsg = (CANFIFO_SIZE - _iCANFifoTail) + _iCANFifoHead;

	pCanResp->id = AWLCANMSG_ID_LIDARQUERY;

	uint32_t * pNbrDataCycles = (uint32_t *) &pCanResp->data[0];
	*pNbrDataCycles = numPendingTemp;

	uint32_t * pNbrCANMsg = (uint32_t *) &pCanResp->data[4];
	*pNbrCANMsg = numCANMsg;

	return 0;
}

static CLD_USB_Data_Received_Return_Type ProcessGetData(AWLCANMessage * pCanReq)
{
	tDataFifo * pData = NULL;

	/* Parameters used to send Bulk IN data in response to the
	   current command. */
	static CLD_USB_Transfer_Params transfer_params =
	{
		.fp_transfer_aborted_callback = user_bulk_adi_loopback_device_transfer_error
	};

	++iUSBnum;

	Lidar_GetDataFromFifo(&pData, &numPending);

	if (numPending)
	{
		uint16_t numReq = pCanReq->data[0];

		if (numPending > numReq)
			numPending = numReq;

		iUSBnumOK += numPending;

		/* Configure the USB Bulk IN transfer to read the number of bytes
		   specified in the Memory Read command. */
		transfer_params.num_bytes = numPending * sizeof(tDataFifo);

		/* Set the Bulk In data buffer address to the starting address specified in the
		   Memory Read command. */
		transfer_params.p_data_buffer = (unsigned char*) pData;
	}
	else
	{
		++iUSBnumEmpty;

		transfer_params.num_bytes = 0;
		transfer_params.p_data_buffer = 0;
	}
	transfer_params.callback.fp_usb_in_transfer_complete = user_bulk_adi_loopback_bulk_in_transfer_complete;
	transfer_params.transfer_timeout_ms = 1000;

	CLD_USB_Data_Transmit_Return_Type res = cld_bf70x_bulk_lib_transmit_bulk_in_data(&transfer_params);

	if (res != CLD_USB_TRANSMIT_SUCCESSFUL)
		cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Error!");

    return CLD_USB_DATA_GOOD;
}


static int ProcessCanCommandMsg(AWLCANMessage * pCanReq, AWLCANMessage * pCanResp)
{
	switch (pCanReq->id)
	{
	case AWLCANMSG_ID_LIDARQUERY:
		ProcessLidarQueryCanCommandMsg(pCanReq, pCanResp);
		break;
	case AWLCANMSG_ID_COMMANDMESSAGE:
		if (pCanReq->data[0] == AWLCANMSG_ID_CMD_SET_PARAMETER &&
			pCanReq->data[1] == AWLCANMSG_ID_CMD_PARAM_AWL_REGISTER)
		{
			uint16_t registerAddress = * (uint16_t *) &pCanReq->data[2];
			uint16_t data = * (uint16_t *) &pCanReq->data[4];

			if (pCanReq->data[1] == AWLCANMSG_ID_CMD_PARAM_ADC_REGISTER)
				registerAddress |= RW_INTERNAL_MASK;

			Lidar_WriteFifoPush(registerAddress, data);
		}
		else if (pCanReq->data[0] == AWLCANMSG_ID_CMD_QUERY_PARAMETER &&
				pCanReq->data[1] == AWLCANMSG_ID_CMD_PARAM_AWL_REGISTER)
		{
			uint16_t registerAddress = * (uint16_t *) &pCanReq->data[2];

			if (pCanReq->data[1] == AWLCANMSG_ID_CMD_PARAM_ADC_REGISTER)
				registerAddress |= RW_INTERNAL_MASK;

			Lidar_ReadFifoPush(registerAddress);
		}
		break;
	}

	return 0;
}

//
// CAN Messages FIFO out
//



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

    cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "\nFirmware version: %d.%d\n\n",
    		FIRMWARE_MAJOR_REV, FIRMWARE_MINOR_REV );

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
        LED3_TGL();
    }

    if (usb_time)
    {
		if (cld_time_passed_ms(usb_time) >= 500u)
		{
			usb_time = 0;
			LED4_OFF();
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
static CLD_USB_Transfer_Request_Return_Type user_bulk_bulk_out_data_received(CLD_USB_Transfer_Params * p_transfer_data)
{
    CLD_USB_Transfer_Request_Return_Type rv = CLD_USB_TRANSFER_STALL;

	if (cld_time_passed_ms(usb_time) >= 25u)
	{
		usb_time = cld_time_get();
		LED4_TGL();
	}

    if (p_transfer_data->num_bytes == sizeof(AWLCANMessage))
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
	int num = 1;

    /* Parameters used to send Bulk IN data in response to the
       current command. */
    static CLD_USB_Transfer_Params transfer_params =
    {
        .fp_transfer_aborted_callback = user_bulk_adi_loopback_device_transfer_error
    };

    AWLCANMessage * pCanMsg = (AWLCANMessage *) &user_bulk_adi_loopback_buffer;
    AWLCANMessage canResp[CANFIFO_SIZE];

//    cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "CAN Msg %d: ", pCanMsg->id);

    if (pCanMsg->id == AWLCANMSG_ID_GETDATA)
    	return ProcessGetData(pCanMsg);
    else if (pCanMsg->id == AWLCANMSG_ID_POLLMESSAGES)
    	PollCanCommandMsg(pCanMsg, canResp, &num);
    else
    	ProcessCanCommandMsg(pCanMsg, canResp);

	/* Return the firmware version using the Bulk IN endpoint. */
	transfer_params.num_bytes = num * sizeof(AWLCANMessage);
	transfer_params.p_data_buffer = (unsigned char*)canResp;
	transfer_params.callback.fp_usb_in_transfer_complete = user_bulk_adi_loopback_bulk_in_transfer_complete;
	transfer_params.transfer_timeout_ms = 1000;
	cld_bf70x_bulk_lib_transmit_bulk_in_data(&transfer_params);

//   	cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "\n\r");

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
	if (numPending)
	{
    	Lidar_ReleaseDataToFifo(numPending);
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
