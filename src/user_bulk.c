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

#include "Serial_cmd.h"


/* ADI hostapp.exe commands */
typedef enum
{
    NO_COMMAND = 0,                                     /* nothing doing here... */
    GET_FW_VERSION,                                     /* get the firmware version */
    QUERY_SUPPORT,                                      /* query for support */
    QUERY_REPLY,                                        /* query reply */
    QUERY_USB_PORT,                                     /* Which USB port connection is made */
    LOOPBACK,                                           /* run loopback on the device */
    MEMORY_READ,                                        /* read from specified memory on the device */
    MEMORY_WRITE,                                       /* write to specified memory on the device */
	LIDAR_QUERY,
	LIDAR_GETDATA
} ADI_Loopback_Command_Function_Code;

#pragma pack (1)
/* ADI loopback test command */
typedef struct
{
    unsigned long command;                              /* ADI_Loopback_Command_Function_Code command */
    unsigned long cmd_data;                             /* Command parameters */
    unsigned long next_msg_length;                      /* Length of the next bulk in/out message */
} ADI_Bulk_Loopback_Command;

/* QUERY_SUPPORT command response */
typedef struct
{
    unsigned long command;                              /* Query response function code */
    unsigned long supported;                            /* 0x00000001 = Supported, 0x00000000 = Not supported */
    unsigned long next_msg_length;                      /* Set to 0 */
} ADI_Bulk_Loopback_Query_Response;

/* QUERY_USB_PORT command response */
typedef struct
{
    unsigned long command;                              /* Query USB Port function code */
    unsigned long usb_port;                             /* 0x00000000 = USB0, 0x00000001 = USB1 */
    unsigned long next_msg_length;                      /* Set to 0 */
} ADI_Bulk_Loopback_Query_USB_Response;


// CAN Frame
typedef struct {
    unsigned long id;        // Message id
    unsigned long timestamp; // timestamp in milliseconds
    unsigned char flags;     // [extended_id|1][RTR:1][reserver:6]
    unsigned char len;       // Frame size (0.8)
    unsigned char data[8]; // Databytes 0..7
    unsigned char pad1;
    unsigned char pad2;
} ADI_AWLCANMessage;

/* QUERY_SUPPORT command response */
typedef struct
{
    unsigned long command;                              /* Query response function code */
    unsigned long nbrCycles;
    unsigned long nbrBytes;
    unsigned long next_msg_length;                      /* Set to 0 */
} ADI_Bulk_Loopback_Lidar_Query_Response;


/* The first 4-bytes of the bulk loopback transfer describe the next bulk loopback
   transfer.
        bits 31-24 = If set to 0 stop loopback, otherwise continue loopback.
        bits 23-0 = Length of the next loopback transfer
 */
typedef struct
{
    unsigned long next_msg_length;
} ADI_Bulk_Loopback_Data_Msg_Header;
#pragma pack ()

#define FIRMWARE_MAJOR_REV 0
#define FIRMWARE_MINOR_REV 2

/* ADI hostapp expected version response. */
#define ADI_BULK_LOOPBACK_NUM_VERSION_STRINGS       5
#define ADI_BULK_LOOPBACK_MAX_VERSION_STRING_LEN    32
#pragma align 4
static const char user_bulk_adi_loopback_device_fw_version[ADI_BULK_LOOPBACK_NUM_VERSION_STRINGS][ADI_BULK_LOOPBACK_MAX_VERSION_STRING_LEN] = {
                                                                    __DATE__,       /* build date       */
                                                                    __TIME__,       /* build time       */
                                                                    "00.02",     /* version number   */
                                                                    "ADSP-BF707",   /* target processor */
                                                                    "Wagner"};    /* application name */


/* State variable used to track what loopback test is active. */
typedef enum
{
    ADI_BULK_LOOPBACK_DEVICE_STATE_IDLE = 0,
    ADI_BULK_LOOPBACK_DEVICE_STATE_LOOPBACK,            /* Loopback test active */
    ADI_BULK_LOOPBACK_DEVICE_STATE_WRITE_MEMORY,        /* Write memory test active */
} ADI_Bulk_Lookback_Device_State;

typedef struct
{
    ADI_Bulk_Lookback_Device_State state;               /* loopback test state. */
    ADI_Bulk_Loopback_Command cmd;                      /* Current loopback command */
    unsigned long total_num_loopback_bytes;             /* Total number of loopback bytes in the
                                                           current loopback transfer */
    unsigned long next_transfer_total_num_bytes;        /* Total number of bytes in the next
                                                           data transfer. */
    unsigned long write_mem_addr;                       /* Memory address to store the data
                                                           during a MEMORY_WRITE command. */
} ADI_Bulk_Loopback_Data;

static ADI_Bulk_Loopback_Data user_bulk_adi_loopback_data =
{
    .state = ADI_BULK_LOOPBACK_DEVICE_STATE_IDLE,
};

/* Loopback data buffer */
#define ADI_BULK_LOOPBACK_BUFFER_SIZE   0x10000
static unsigned char user_bulk_adi_loopback_buffer[ADI_BULK_LOOPBACK_BUFFER_SIZE];

/* Function prototypes */
static CLD_USB_Transfer_Request_Return_Type user_bulk_bulk_out_data_received(CLD_USB_Transfer_Params * p_transfer_data);

static CLD_USB_Data_Received_Return_Type user_bulk_adi_loopback_cmd_received (void);
static CLD_USB_Data_Received_Return_Type user_bulk_adi_loopback_data_received (void);
static CLD_USB_Data_Received_Return_Type user_bulk_adi_write_memory_data_received (void);
static CLD_USB_Data_Received_Return_Type user_bulk_adi_can_cmd_received (void);
static unsigned long user_bulk_adi_loopback_device_check_cmd_supported (ADI_Loopback_Command_Function_Code cmd);
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







//
// CAN messages FIFO out
//

volatile int iCANFifoHead = 0;
volatile int iCANFifoTail = 0;

#define CANFIFO_SIZE 128
#define CANFIFO_MASK (CANFIFO_SIZE - 1)

ADI_AWLCANMessage canFifo[CANFIFO_SIZE];


int CANFifoPushMsg(ADI_AWLCANMessage * pCanMsg)
{
	int nextHead = (iCANFifoHead + 1) & CANFIFO_MASK;

	if (nextHead != iCANFifoTail)
	{
		memcpy(&canFifo[iCANFifoHead], pCanMsg, sizeof(ADI_AWLCANMessage));

		iCANFifoHead = nextHead;

		return 0;
	}

	return 1;
}

int user_CANFifoPushCompletedFrame(void)
{
	ADI_AWLCANMessage canMsg;

	canMsg.id = 9;

	return CANFifoPushMsg(&canMsg);
}

int user_CANFifoPushDetection(int ch, uint16_t dist, uint16_t vel)
{
	int ret;
	ADI_AWLCANMessage canMsg;

	canMsg.id = 10;
	canMsg.len = 8;
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

	canMsg.id = 11;
	canMsg.len = 8;
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
	ADI_AWLCANMessage canMsg;

	canMsg.id = 80;
	canMsg.len = 8;
	canMsg.data[0] = 0xC2;
	canMsg.data[1] = 0x03;
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
	ADI_AWLCANMessage canMsg;

	canMsg.id = 1;
	canMsg.len = 8;
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
	ADI_AWLCANMessage canMsg;

	canMsg.id = 2;
	canMsg.len = 8;
	canMsg.data[0] = 1;
	canMsg.data[1] = 0;
	canMsg.data[2] = 0;
	canMsg.data[3] = 0;
	canMsg.data[4] = 0;
	canMsg.data[5] = 0;
	canMsg.data[6] = 0;
	canMsg.data[7] = 0;

	return CANFifoPushMsg(&canMsg);
}


static int PollCanCommandMsg(ADI_AWLCANMessage * pCanReq, ADI_AWLCANMessage * pCanResp, int * pNum)
{
	int num = pCanReq->data[0];
	int i;

	if (num > CANFIFO_MASK)
		num = CANFIFO_MASK;

	for(i=0; i<num; i++)
	{
		if (iCANFifoHead != iCANFifoTail)
		{
			memcpy(pCanResp++, &canFifo[iCANFifoTail], sizeof(ADI_AWLCANMessage));

			iCANFifoTail = (iCANFifoTail + 1) & CANFIFO_MASK;
		}
	}

	*pNum = num;

	return 0;
}

static int ProcessCanCommandMsg(ADI_AWLCANMessage * pCanReq, ADI_AWLCANMessage * pCanResp)
{
	switch (pCanReq->id)
	{
	case 80:
		if (pCanReq->data[0] == 0xC0 && pCanReq->data[1] == 0x03)
		{
			uint16_t registerAddress = * (uint16_t *) &pCanReq->data[2];
			uint16_t data = * (uint16_t *) &pCanReq->data[4];

			Lidar_WriteFifoPush(registerAddress, data);
		}
		else if (pCanReq->data[0] == 0xC1 && pCanReq->data[1] == 0x03)
		{
			uint16_t registerAddress = * (uint16_t *) &pCanReq->data[2];

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

//    static CLD_Time main_time = 0;
//    static CLD_Time msg_time = 0;
//    static CLD_Time run_time = 0;
    static CLD_Time log_time = 0;
//    static CLD_Time acq_time = 0;

    static int iAcqNum = 0;
    static int iAcqNum1 = 0;
    static int iAcqNum2 = 0;
    static int iAcqNumX = 0;

    LED3_ON();

    cld_bf70x_bulk_lib_main();

//    if (cld_time_passed_ms(main_time) >= 250u)
//    {
//        main_time = cld_time_get();
//        pADI_PORTA->DATA_TGL = (1 << 0);
//    }

    Serial_Process();

//    if (cld_time_passed_ms(acq_time) >= 1u)
    {
    	uint16_t banknum = 0;
//    	acq_time = cld_time_get();

    	LED4_ON();
    	Lidar_Acq(&banknum);
    	LED4_OFF();

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

	LED3_OFF();

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

    switch(user_bulk_adi_loopback_data.state)
    {
    case ADI_BULK_LOOPBACK_DEVICE_STATE_IDLE:
        /* IF the received bulk packet is an ADI_Bulk_Loopback_Command. */
        if (p_transfer_data->num_bytes == sizeof(ADI_Bulk_Loopback_Command))
        {
            /* Save the received data to the user_bulk_adi_loopback_data.cmd structure,
               and call user_bulk_adi_loopback_cmd_received once the data has been received. */
            p_transfer_data->p_data_buffer = (unsigned char *)&user_bulk_adi_loopback_data.cmd;
            p_transfer_data->callback.fp_usb_out_transfer_complete = user_bulk_adi_loopback_cmd_received;
            p_transfer_data->fp_transfer_aborted_callback = user_bulk_adi_loopback_device_transfer_error;
            p_transfer_data->transfer_timeout_ms = 1000;
            rv = CLD_USB_TRANSFER_ACCEPT;
        }
        /* IF the received bulk packet is an ADI_Bulk_Loopback_Command. */
        else if (p_transfer_data->num_bytes == sizeof(ADI_AWLCANMessage))
    	{
    		/* Save the received data to the user_bulk_adi_loopback_data.cmd structure,
    		   and call user_bulk_adi_loopback_cmd_received once the data has been received. */
        	p_transfer_data->p_data_buffer = user_bulk_adi_loopback_buffer;
    		p_transfer_data->callback.fp_usb_out_transfer_complete = user_bulk_adi_can_cmd_received;
    		p_transfer_data->fp_transfer_aborted_callback = user_bulk_adi_loopback_device_transfer_error;
    		p_transfer_data->transfer_timeout_ms = 1000;
    		rv = CLD_USB_TRANSFER_ACCEPT;
    	}
    	break;

    /* If the loopback test is active. */
    case ADI_BULK_LOOPBACK_DEVICE_STATE_LOOPBACK:
        /* Store the entire Bulk OUT transfer in the user_bulk_adi_loopback_buffer,
           and call user_bulk_adi_loopback_data_received once the data has been written to the buffer. */
        p_transfer_data->p_data_buffer = (unsigned char *)&user_bulk_adi_loopback_buffer[0];
        /* Set the p_transfer_data->num_bytes to the total number of bytes expected in the
           Loopback transfer. */
        p_transfer_data->num_bytes = user_bulk_adi_loopback_data.total_num_loopback_bytes;
        p_transfer_data->callback.fp_usb_out_transfer_complete = user_bulk_adi_loopback_data_received;
        p_transfer_data->fp_transfer_aborted_callback = user_bulk_adi_loopback_device_transfer_error;
        p_transfer_data->transfer_timeout_ms = 1000;

        rv = CLD_USB_TRANSFER_ACCEPT;
    	break;

    /* If the memory write test is active. */
    case ADI_BULK_LOOPBACK_DEVICE_STATE_WRITE_MEMORY:
        /* Configure the USB Bulk OUT transfer to write the number of bytes
            specified in the Memory Write command. */
        p_transfer_data->num_bytes = user_bulk_adi_loopback_data.next_transfer_total_num_bytes;

        /* Set the Bulk Out data buffer address to the starting address specified in the
           Memory Write command. */
        p_transfer_data->p_data_buffer = (unsigned char *)user_bulk_adi_loopback_data.write_mem_addr;

        /* Call the user_bulk_adi_write_memory_data_received function once the
           Write Memory operation has finished. */
        p_transfer_data->callback.fp_usb_out_transfer_complete = user_bulk_adi_write_memory_data_received;
        p_transfer_data->fp_transfer_aborted_callback = user_bulk_adi_loopback_device_transfer_error;
        p_transfer_data->transfer_timeout_ms = 1000;

        rv = CLD_USB_TRANSFER_ACCEPT;
    	break;
    }

    return rv;
}

/*=============================================================================
Function:       user_bulk_adi_loopback_cmd_received

Parameters:     None.

Description:    This function is called when the user_bulk_adi_loopback_data.cmd
                structure has been populated.

Returns:        CLD_USB_DATA_GOOD - The received data is valid.
==============================================================================*/
static CLD_USB_Data_Received_Return_Type user_bulk_adi_loopback_cmd_received (void)
{
    /* Parameters used to send Bulk IN data in response to the
       current command. */
    static CLD_USB_Transfer_Params transfer_params =
    {
        .fp_transfer_aborted_callback = user_bulk_adi_loopback_device_transfer_error
    };
    ADI_Bulk_Loopback_Query_Response * p_query_resp;
    ADI_Bulk_Loopback_Query_USB_Response * p_query_usb_port;
    ADI_Bulk_Loopback_Lidar_Query_Response * p_lidar_query_resp;

    if (((ADI_Loopback_Command_Function_Code)user_bulk_adi_loopback_data.cmd.command != MEMORY_READ) &&
    	((ADI_Loopback_Command_Function_Code)user_bulk_adi_loopback_data.cmd.command != LIDAR_QUERY))
    	cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Bulk Command %d: ", user_bulk_adi_loopback_data.cmd.command);

    /* Process the received command */
    switch((ADI_Loopback_Command_Function_Code)user_bulk_adi_loopback_data.cmd.command)
    {
        case NO_COMMAND:
            /* Do nothing */
        break;

        case GET_FW_VERSION:
            /* Return the firmware version using the Bulk IN endpoint. */
            transfer_params.num_bytes = sizeof(user_bulk_adi_loopback_device_fw_version);
            transfer_params.p_data_buffer = (unsigned char*)user_bulk_adi_loopback_device_fw_version;
            transfer_params.callback.fp_usb_in_transfer_complete = user_bulk_adi_loopback_bulk_in_transfer_complete;
            transfer_params.transfer_timeout_ms = 1000;
            cld_bf70x_bulk_lib_transmit_bulk_in_data(&transfer_params);
            cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Get FW Version");
        break;

        case QUERY_SUPPORT:
            p_query_resp = (ADI_Bulk_Loopback_Query_Response *)user_bulk_adi_loopback_buffer;

            /* Set the Query response data. */
            p_query_resp->command = QUERY_REPLY;
            p_query_resp->supported = user_bulk_adi_loopback_device_check_cmd_supported((ADI_Loopback_Command_Function_Code)user_bulk_adi_loopback_data.cmd.cmd_data);
            p_query_resp->next_msg_length = 0;

            /* Return the query response using the Bulk IN endpoint. */
            transfer_params.num_bytes = sizeof(ADI_Bulk_Loopback_Query_Response);
            transfer_params.p_data_buffer = user_bulk_adi_loopback_buffer;
            transfer_params.callback.fp_usb_in_transfer_complete = user_bulk_adi_loopback_bulk_in_transfer_complete;
            transfer_params.transfer_timeout_ms = 1000;
            cld_bf70x_bulk_lib_transmit_bulk_in_data(&transfer_params);
            cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Query Cmd Support");
        break;

        case QUERY_USB_PORT:
            p_query_usb_port = (ADI_Bulk_Loopback_Query_USB_Response *)user_bulk_adi_loopback_buffer;

            /* Set the Query response data. */
            p_query_usb_port->command = QUERY_USB_PORT;
            p_query_usb_port->usb_port = 0;  /* The BF70x has a single USB port. */
            p_query_usb_port->next_msg_length = 0;

            /* Return the query response using the Bulk IN endpoint. */
            transfer_params.num_bytes = sizeof(ADI_Bulk_Loopback_Query_USB_Response);
            transfer_params.p_data_buffer = user_bulk_adi_loopback_buffer;
            transfer_params.callback.fp_usb_in_transfer_complete = user_bulk_adi_loopback_bulk_in_transfer_complete;
            transfer_params.transfer_timeout_ms = 1000;
            cld_bf70x_bulk_lib_transmit_bulk_in_data(&transfer_params);
            cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Query USB Cmd");
        break;

        case LOOPBACK:
            /* The loopback test is starting. */
            user_bulk_adi_loopback_data.state = ADI_BULK_LOOPBACK_DEVICE_STATE_LOOPBACK;
            /* Save the total number of loopback bytes in the first loopback
               transfer. */
            user_bulk_adi_loopback_data.total_num_loopback_bytes = user_bulk_adi_loopback_data.cmd.next_msg_length;
            cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Loopback Data");
        break;

        case MEMORY_READ:
        {
        	tDataFifo * pData = NULL;

        	++iUSBnum;

        	Lidar_GetDataFromFifo(&pData, &numPending);

        	if (numPending)
        	{
        		uint16_t numReq = (uint16_t) user_bulk_adi_loopback_data.cmd.cmd_data;

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

        break;
        }

        case MEMORY_WRITE:
            /* The write memory test is starting */
            user_bulk_adi_loopback_data.state = ADI_BULK_LOOPBACK_DEVICE_STATE_WRITE_MEMORY;
            /* Save the total number of bytes being written to memory. */
            user_bulk_adi_loopback_data.next_transfer_total_num_bytes = user_bulk_adi_loopback_data.cmd.next_msg_length;
            /* Save the specified memory starting address. */
            user_bulk_adi_loopback_data.write_mem_addr = user_bulk_adi_loopback_data.cmd.cmd_data;
            cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Write Memory");
        break;

        // Wagner
        case LIDAR_QUERY:
        {
        	int _iCANFifoHead = iCANFifoHead;
        	int _iCANFifoTail = iCANFifoTail;

        	p_lidar_query_resp = (ADI_Bulk_Loopback_Lidar_Query_Response *)user_bulk_adi_loopback_buffer;

        	tDataFifo * pData = NULL;
        	uint16_t numPendingTemp = 0;

        	Lidar_GetDataFromFifo(&pData, &numPendingTemp);

        	int numCANMsg = 0;
        	if (_iCANFifoTail < _iCANFifoHead)
        		numCANMsg = _iCANFifoHead - _iCANFifoTail;
   			else if (_iCANFifoTail > _iCANFifoHead)
  				numCANMsg = (CANFIFO_SIZE - _iCANFifoTail) + _iCANFifoHead;

			/* Set the Query response data. */
        	p_lidar_query_resp->command = LIDAR_QUERY;
        	p_lidar_query_resp->nbrCycles = numPendingTemp;
        	p_lidar_query_resp->nbrBytes = numCANMsg;
        	p_lidar_query_resp->next_msg_length = 0;

			/* Return the query response using the Bulk IN endpoint. */
			transfer_params.num_bytes = sizeof(ADI_Bulk_Loopback_Lidar_Query_Response);
			transfer_params.p_data_buffer = user_bulk_adi_loopback_buffer;
			transfer_params.callback.fp_usb_in_transfer_complete = user_bulk_adi_loopback_bulk_in_transfer_complete;
			transfer_params.transfer_timeout_ms = 1000;
			cld_bf70x_bulk_lib_transmit_bulk_in_data(&transfer_params);
			//cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Lidar Query");
        }
		break;

        default:
            /* Do nothing */
        break;
    }

    if (((ADI_Loopback_Command_Function_Code)user_bulk_adi_loopback_data.cmd.command != MEMORY_READ) &&
        	((ADI_Loopback_Command_Function_Code)user_bulk_adi_loopback_data.cmd.command != LIDAR_QUERY))
    	cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "\n\r");

    return CLD_USB_DATA_GOOD;
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

    ADI_AWLCANMessage * pCanMsg = (ADI_AWLCANMessage *) user_bulk_adi_loopback_buffer;
    ADI_AWLCANMessage canResp[CANFIFO_SIZE];

//    cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "CAN Msg %d: ", pCanMsg->id);

    memset(&canResp[0], 0, sizeof(ADI_AWLCANMessage));

    if (pCanMsg->id == 88)
    	PollCanCommandMsg(pCanMsg, canResp, &num);
    else
    	ProcessCanCommandMsg(pCanMsg, canResp);

	/* Return the firmware version using the Bulk IN endpoint. */
	transfer_params.num_bytes = num * sizeof(ADI_AWLCANMessage);
	transfer_params.p_data_buffer = (unsigned char*)canResp;
	transfer_params.callback.fp_usb_in_transfer_complete = user_bulk_adi_loopback_bulk_in_transfer_complete;
	transfer_params.transfer_timeout_ms = 1000;
	cld_bf70x_bulk_lib_transmit_bulk_in_data(&transfer_params);

//   	cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "\n\r");

    return CLD_USB_DATA_GOOD;
}

/*=============================================================================
Function:       user_bulk_adi_loopback_data_received

Parameters:     None.

Description:    This function is called when loopback data has been received.

Returns:        CLD_USB_DATA_GOOD - The received data is valid.
==============================================================================*/
static CLD_USB_Data_Received_Return_Type user_bulk_adi_loopback_data_received (void)
{
    ADI_Bulk_Loopback_Data_Msg_Header * p_loopback_hdr;
    /* Structure used to echo back the received Bulk OUT data on the Bulk IN
       endpoint. */
    static CLD_USB_Transfer_Params bulk_in_transfer_params =
    {
        .p_data_buffer = user_bulk_adi_loopback_buffer,
        .callback.fp_usb_in_transfer_complete = user_bulk_adi_loopback_bulk_in_transfer_complete,
        .fp_transfer_aborted_callback = user_bulk_adi_loopback_device_transfer_error,
        .transfer_timeout_ms = 1000
    };

    bulk_in_transfer_params.num_bytes = user_bulk_adi_loopback_data.total_num_loopback_bytes;
    cld_bf70x_bulk_lib_transmit_bulk_in_data(&bulk_in_transfer_params);

    /* Set p_loopback_hdr to address the first 4-bytes of the received
       loopback data. */
    p_loopback_hdr = (ADI_Bulk_Loopback_Data_Msg_Header *)user_bulk_adi_loopback_buffer;

    /* If the loopback test should continue. */
    if (p_loopback_hdr->next_msg_length & 0xff000000)
    {
        /* Save the number of bytes in the next loopback transfer. */
        user_bulk_adi_loopback_data.total_num_loopback_bytes = (p_loopback_hdr->next_msg_length & 0x00ffffff);
    }
    else
    {
        cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Data Loopback ended\n\r");
        /* Exit the loopback test mode. */
        user_bulk_adi_loopback_data.state = ADI_BULK_LOOPBACK_DEVICE_STATE_IDLE;
    }
    return CLD_USB_DATA_GOOD;
}

/*=============================================================================
Function:       user_bulk_adi_write_memory_data_received

Parameters:     None.

Description:    This function is called when memory write data has been received.

Returns:        CLD_USB_DATA_GOOD - The received data is valid.
==============================================================================*/
static CLD_USB_Data_Received_Return_Type user_bulk_adi_write_memory_data_received (void)
{
    user_bulk_adi_loopback_data.state = ADI_BULK_LOOPBACK_DEVICE_STATE_IDLE;
    return CLD_USB_DATA_GOOD;
}

/*=============================================================================
Function:       user_bulk_adi_loopback_device_check_cmd_supported

Parameters:     cmd - The received command function code.

Description:    This function tests if the received function code is supported
                by the firmware.

Returns:        1 = Supported, 0 = Not supported
==============================================================================*/
static unsigned long user_bulk_adi_loopback_device_check_cmd_supported (ADI_Loopback_Command_Function_Code cmd)
{
    if ((cmd == NO_COMMAND) ||
        (cmd == GET_FW_VERSION) ||
        (cmd == QUERY_SUPPORT) ||
        (cmd == QUERY_USB_PORT) ||
        (cmd == LOOPBACK) ||
        (cmd == MEMORY_READ) ||
        (cmd == MEMORY_WRITE))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*=============================================================================
Function:       user_bulk_adi_loopback_device_transfer_error

Parameters:     None.

Description:    This function is called if a requested transfer is aborted.

Returns:        None.
==============================================================================*/
static void user_bulk_adi_loopback_device_transfer_error (void)
{
    user_bulk_adi_loopback_data.state = ADI_BULK_LOOPBACK_DEVICE_STATE_IDLE;
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

//    static unsigned char cnt = 0;
//    cnt++;
//    if (cnt == 6)
//    {
//        cnt = 0;
//        pADI_PORTB->DATA_TGL = (1 << 1);
//    }
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

#if 0
    static const CLD_CONSOLE_COLOR cld_console_colors[CLD_CONSOLE_NUM_COLORS] =
    {
        CLD_CONSOLE_RED,
        CLD_CONSOLE_GREEN,
        CLD_CONSOLE_YELLOW,
        CLD_CONSOLE_BLUE,
        CLD_CONSOLE_PURPLE,
        CLD_CONSOLE_CYAN,
        CLD_CONSOLE_WHITE,
        CLD_CONSOLE_BLACK,
    };
    static const char * color_strings[CLD_CONSOLE_NUM_COLORS] =
    {
        "Red",
        "Green",
        "Yellow",
        "Blue",
        "Purple",
        "Cyan",
        "White",
        "Black",
    };
    unsigned char i;

    cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "\r\nUser received %c\r\n", byte);

    if (byte == '!')
    {
        cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "\n\r----- CLD Console Color Demo -----\n\r\n\r");
        for (i = 0; i < CLD_CONSOLE_NUM_COLORS; i++)
        {
            cld_console(cld_console_colors[i], cld_console_colors[CLD_CONSOLE_NUM_COLORS - i - 1], "%s text with a %s background\n\r",
                        color_strings[i], color_strings[CLD_CONSOLE_NUM_COLORS - i - 1]);
        }
        cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "\n\r----------------------------------\n\r\n\r");
    }
#endif
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
            user_bulk_adi_loopback_data.state = ADI_BULK_LOOPBACK_DEVICE_STATE_IDLE;
        break;
        case CLD_USB_BUS_SUSPEND:
            cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "CLD Bulk Device Suspend\n\r");
        break;
        case CLD_USB_BUS_RESUME:
            cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "CLD Bulk Device Resume\n\r");
        break;
    }
}
