/*
 * USB_cmd.c
 *
 *  Created on: Feb 11, 2020
 *      Author: e.turenne
 *
 *      defines all USB commands that can be received
 *
 */
#include <stdint.h>
#include <string.h>

#include "USB_cmd.h"

#include "../adal6110_16.h"
#include "USB_msg.h"
#include "cld_bf70x_bulk_lib.h"

#include "../demo_app.h" // board definitions
#include "../error_handler.h"
#include "Msg_queue.h"

#include "../parameters.h"


uint32_t GetTime(void);

void SendACK(USB_CAN_message* cmd, USB_msg* ret_msg);
void SendNACK(USB_CAN_message* cmd, USB_msg* ret_msg);
void SendNext(USB_CAN_message* cmd, USB_msg* ret_msg);

uint8_t ProcessCommand(USB_CAN_message* cmd);

extern uint8_t gSendCooked;
extern uint8_t gSendRaw;


inline uint32_t GetTime()
{
	uint32_t clockTime = 0;
	clockTime = (uint32_t) (cld_time_get() & 0xFFFFFFFF);
	return(clockTime);
}

/*
 * Message preparation
 */

USB_msg msg_to_push;

void USB_pushStatus()
{
	USB_CAN_message* msg = (USB_CAN_message*) &msg_to_push;
	uint32_t errorFlags;

	errorFlags = GetError();

	msg->id = msgID_sensorStatus;
	msg->flags = 0;
	msg->timestamp = GetTime();
	msg->len = 8;
	// put error/status code in payload
	msg->data[0] = 0; // temperature
	msg->data[1] = 0; //
	msg->data[2] = 0; // Voltage
	msg->data[3] = 0; //
	msg->data[4] = 0; // frame rate
	msg->data[5] = (uint8_t)((errorFlags&0xFF0000) >>16); // hardware
	msg->data[6] = (uint8_t)((errorFlags&0x00FF00) >>8);  // software
	msg->data[7] = (uint8_t)((errorFlags&0x0000FF) >>0);  // sensor
	msg->pad1 = 0x00;
	msg->pad2 = 0x00;

	if( MsgQueue_Ok != msgQueuePush(&msg_to_push))
			SetError(error_SW_comm_fifo_full);
}
void USB_pushBoot()
{
	USB_CAN_message* msg = (USB_CAN_message*) &msg_to_push;

	msg->id = msgID_sensorBoot;
	msg->flags = 0;
	msg->timestamp = GetTime();
	msg->len = 4;
	msg->data[0] = FIRMWARE_MAJOR_REV;
	msg->data[1] = FIRMWARE_MINOR_REV;
	msg->data[2] = 0; // error CSM
	msg->data[3] = 0; // BIST Values
	msg->data[4] = 0;
	msg->data[5] = 0;
	msg->data[6] = 0;
	msg->data[7] = 0;
	msg->pad1 = 0x00;
	msg->pad2 = 0x00;

	if( MsgQueue_Ok != msgQueuePush(&msg_to_push))
		SetError(error_SW_comm_fifo_full);
}

void USB_pushParameter(uint16_t address, uint16_t value, uint8_t paramType)
{
	USB_CAN_message* msg = (USB_CAN_message*) &msg_to_push;

	msg->id = msgID_command;
	msg->flags = 0;
	msg->timestamp = GetTime();
	msg->len = 8;
	msg->data[0] = msgID_respParametercmd;
	msg->data[1] = paramType;
	msg->data[3] = (uint8_t) ((address&0xFF00)>>8);
	msg->data[2] = (uint8_t) (address&0x00FF);
	msg->data[7] = (uint8_t) ((value&0xFF000000)>>24);
	msg->data[6] = (uint8_t) ((value&0x00FF0000)>>16);
	msg->data[5] = (uint8_t) ((value&0x0000FF00)>>8);
	msg->data[4] = (uint8_t) ((value&0x000000FF));
	msg->pad1 = 0x00;
	msg->pad2 = 0x00;

	if( MsgQueue_Ok != msgQueuePush(&msg_to_push))
		SetError(error_SW_comm_fifo_full);
}

void USB_pushRawData(uint16_t pixelID, uint16_t *buf)
{
	USB_raw_message* msg = (USB_raw_message*) &msg_to_push;

	msg->id = msgID_transmitRaw;
	msg->timestamp = GetTime();
	msg->pixelNumber = pixelID;
	msg->payloadsize = RAW_NUM_SAMPLES;
	msg->synch[0] = 0x7FFF;
	msg->synch[1] = 0x8000;
	memcpy((uint8_t*)(msg->data), (uint8_t*)buf, sizeof(msg->data));

	if( MsgQueue_Ok != msgQueuePush(&msg_to_push))
		SetError(error_SW_comm_fifo_full);
}

void USB_pushTrack(uint16_t trackID, int pixelID, float probability, float intensity,
		          float distance, float velocity, float acceleration)
{
	USB_CAN_message* msg = (USB_CAN_message*) &msg_to_push;
	int val;

	msg->timestamp = GetTime();
	msg->flags = 0xF0;
	msg->len = 8;
	msg->pad1 = 0xEA;
	msg->pad2 = 0x1C;

	// send info
	msg->id = msgID_trackInfo;
	msg->data[1] = (uint8_t)(trackID>>8);
	msg->data[0] = (uint8_t)trackID;
	msg->data[2] = 0x00;
	pixelID = pixelID<0?0:pixelID>15?15:pixelID;
	val = (int)pixelID;
	msg->data[4] = (uint8_t)(val>>8);
	msg->data[3] = (uint8_t)val;
	probability = probability<0?0:probability>100?100:probability;
	val = (int)probability;
	msg->data[5] = (uint8_t) val;
	intensity = intensity<-20?-20:intensity>108?108:intensity;
	intensity = 2*(intensity+20); // 1/2 dB with a 20dB offset
	val = (int)intensity;
	msg->data[7] = (uint8_t)(val>>8);
	msg->data[6] = (uint8_t)(val);

	if( MsgQueue_Ok != msgQueuePush(&msg_to_push))
		SetError(error_SW_comm_fifo_full);

	// send values
	msg->id = msgID_trackValue;
	msg->data[1] = (uint8_t)(trackID>>8);
	msg->data[0] = (uint8_t)trackID;
	distance = distance<0?0:distance>200?200:distance;
	distance *= 100; // expressed in cm
	val = (int)distance;
	msg->data[3] = (uint8_t) (val>>8);
	msg->data[2] = (uint8_t) (val);
	//velocity = velocity<-100?-100:velocity>100?100:velocity;
	velocity *= 100; // expressed in cm/s
	val = (int)velocity;
	msg->data[5] = (uint8_t)(val>>8);
	msg->data[4] = (uint8_t) (val);
	val = (int)acceleration; // not used
	msg->data[7] = (uint8_t)(val>>8);
	msg->data[6] = (uint8_t)(val);

	if( MsgQueue_Ok != msgQueuePush(&msg_to_push))
		SetError(error_SW_comm_fifo_full);

}

void USB_pushEndOfFrame(uint16_t frameID, uint16_t systemID, uint16_t numTrackSent)
{
	USB_CAN_message* msg = (USB_CAN_message*) &msg_to_push;


	msg->id = msgID_FrameDone;
	msg->timestamp = GetTime();
	msg->flags = 0x00;
	msg->len = 8;

	msg->data[1] = (uint8_t)(frameID>>8)&0xFF;
	msg->data[0] = (uint8_t)frameID&0xFF;
	msg->data[3] = (uint8_t)(systemID>>8)&0xFF;
	msg->data[2] = (uint8_t)systemID&0xFF;
	msg->data[4] = 0;
	msg->data[5] = 0;
	msg->data[7] = (uint8_t)(numTrackSent>>8)&0xFF;
	msg->data[6] = (uint8_t)numTrackSent&0xFF;

	msg->pad1 = 0x00;
	msg->pad2 = 0x00;

	if( MsgQueue_Ok != msgQueuePush(&msg_to_push))
		SetError(error_SW_comm_fifo_full);
}

/*
 *  USB interface
 */


void SendACK(USB_CAN_message* cmd, USB_msg* ret_msg)
{
	ret_msg->CAN.id = msgID_command;
	ret_msg->CAN.timestamp = GetTime();
	ret_msg->CAN.flags = 0xA;
	ret_msg->CAN.len = 8;
	ret_msg->CAN.data[0] = (cmd->data[0]==msgID_getParametercmd)?msgID_ACKGetcmd:msgID_ACKSetcmd;
	ret_msg->CAN.data[1] = cmd->data[1];
	ret_msg->CAN.data[2] = cmd->data[2];
	ret_msg->CAN.data[3] = cmd->data[3];
	ret_msg->CAN.data[4] = cmd->data[4];
	ret_msg->CAN.data[5] = cmd->data[5];
	ret_msg->CAN.data[6] = cmd->data[6];
	ret_msg->CAN.data[7] = cmd->data[7];
	ret_msg->CAN.pad1 = 0x00;
	ret_msg->CAN.pad2 = 0x00;
}

void SendNACK(USB_CAN_message* cmd, USB_msg* ret_msg)
{
	ret_msg->CAN.id = msgID_command;
	ret_msg->CAN.timestamp = GetTime();
	ret_msg->CAN.flags = 0xA0;
	ret_msg->CAN.len = 0;
	ret_msg->CAN.len = 8;
	ret_msg->CAN.data[0] = msgID_NACKcmd;
	ret_msg->CAN.data[1] = cmd->data[1];
	ret_msg->CAN.data[2] = cmd->data[2];
	ret_msg->CAN.data[3] = cmd->data[3];
	ret_msg->CAN.data[4] = cmd->data[4];
	ret_msg->CAN.data[5] = cmd->data[5];
	ret_msg->CAN.data[6] = cmd->data[6];
	ret_msg->CAN.data[7] = cmd->data[7];
	ret_msg->CAN.pad1 = 0x00;
	ret_msg->CAN.pad2 = 0x00;

}

void SendNext(USB_CAN_message* cmd, USB_msg* ret_msg)
{
	if (MsgQueue_Ok != msgQueuePop(ret_msg))
	{
		ret_msg->CAN.id = msgID_command;
		ret_msg->CAN.timestamp = GetTime();
		ret_msg->CAN.flags = 0;
		ret_msg->CAN.len = 1;
		ret_msg->CAN.data[0] = msgID_queueEmptycmd;
		ret_msg->CAN.data[1] = 0x00;
		ret_msg->CAN.data[2] = 0x00;
		ret_msg->CAN.data[3] = 0x00;
		ret_msg->CAN.data[4] = 0x00;
		ret_msg->CAN.data[5] = 0x00;
		ret_msg->CAN.data[6] = 0x00;
		ret_msg->CAN.data[7] = 0x00;
		ret_msg->CAN.pad1 = 0x00;
		ret_msg->CAN.pad2 = 0x00;
	}
}

void USB_ReadCommand(USB_CAN_message* cmd, USB_msg* ret_msg)
{

	switch (cmd->id)
	{
	case msgID_command:
		if( LiDARParameters[param_console_log] & CONSOLE_MASK_USB )
			cld_console(CLD_CONSOLE_YELLOW, CLD_CONSOLE_BLACK, "-->0x%04X (%02X %02X %02X %02X %02X %02X %02X %02X) \r\n",
								cmd->id, cmd->data[0], cmd->data[1], cmd->data[2], cmd->data[3],
								cmd->data[4], cmd->data[5], cmd->data[6], cmd->data[7]);
		if ( ProcessCommand(cmd) != 0) SendNACK(cmd,ret_msg);
		else SendACK(cmd,ret_msg);
		break;

	case msgID_poll:
		SendNext(cmd,ret_msg);
		if (ret_msg->RAW.id != msgID_transmitRaw)
		{
			if ((ret_msg->CAN.data[0] != msgID_queueEmptycmd) && (LiDARParameters[param_console_log] & CONSOLE_MASK_USB))
			{
				cld_console(CLD_CONSOLE_YELLOW, CLD_CONSOLE_BLACK, " <--0x%04X (%02X %02X %02X %02X %02X %02X %02X %02X) \r\n",
						ret_msg->CAN.id, ret_msg->CAN.data[0], ret_msg->CAN.data[1], ret_msg->CAN.data[2], ret_msg->CAN.data[3],
						ret_msg->CAN.data[4], ret_msg->CAN.data[5], ret_msg->CAN.data[6], ret_msg->CAN.data[7]);
			}
		}else if (LiDARParameters[param_console_log] & CONSOLE_MASK_USB)
		{
			cld_console(CLD_CONSOLE_YELLOW, CLD_CONSOLE_BLACK, " <--0x%02X (%02d:%3d)\r\n",ret_msg->RAW.id,ret_msg->RAW.pixelNumber,ret_msg->RAW.payloadsize);
		}
		break;

	default:
		SendNACK(cmd,ret_msg);
		SetError(error_SW_comm_unknown);
	}

}


uint8_t ProcessCommand(USB_CAN_message* cmd)
{
	/*
	 * command format
	 * Byte | description
	 * -----+---------------------------
	 *   0     cmd_ID
	 *   1     cmd_type
	 *   2-3   address
	 *   4-7   Value
	 */
	uint8_t ID = cmd->data[0];
	uint8_t type= cmd->data[1];
	// Big endian... MSB is last
	uint16_t add = (uint16_t) ((cmd->data[3] << 8) + cmd->data[2]);
	uint32_t val = (uint32_t) ((cmd->data[7] << 24) + (cmd->data[6] << 16) + (cmd->data[5] << 8) + cmd->data[4]);

	switch(ID)
	{
	case msgID_setparametercmd :
		switch(type)
		{
		case cmdParam_SensorRegister:
			add |= RW_INTERNAL_MASK;
		case cmdParam_ADCRegister:
			if(!param_WriteFifoPush( (add), (uint16_t) val)) SetError(error_SW_ADI);
			cld_console(CLD_CONSOLE_PURPLE,CLD_CONSOLE_BLACK,"Write request of parameter: 0x%04X to value 0x%04X\r\n",add,val);
			break;
		default:
			return(2);
		}
		break;
	case msgID_getParametercmd :
		switch(type)
		{
		case cmdParam_SensorRegister:
			add |= RW_INTERNAL_MASK;
		case cmdParam_ADCRegister:
			if(!param_ReadFifoPush(add)) SetError(error_SW_ADI);
			cld_console(CLD_CONSOLE_PURPLE,CLD_CONSOLE_BLACK,"Read request of parameter: 0x%04X\r\n",add,val);
			break;
		default:
			return(2);
		}
		break;
	case msgID_requestCookedcmd :
		LiDARParameters[param_det_msg_decimation] = cmd->data[3];
		LiDARParameters[param_det_msg_mask] = cmd->data[1] + (cmd->data[2]<<8);
		if( LiDARParameters[param_det_msg_decimation] )
		{
			LiDARParameters[param_DSP_enable] = 1; // force detection...
			cld_console(CLD_CONSOLE_PURPLE,CLD_CONSOLE_BLACK,"Cooked data requested at 1:%d decimation with 0x%04X mask\r\n",
					LiDARParameters[param_det_msg_decimation],LiDARParameters[param_det_msg_mask]);
		}else
		{
			LiDARParameters[param_DSP_enable] = LiDARParameters[param_raw_msg_decimation]?1:0; // stop detection...
			cld_console(CLD_CONSOLE_PURPLE,CLD_CONSOLE_BLACK,"Cooked data cancelled\r\n");
		}
		break;
	case msgID_requestRawcmd :
		LiDARParameters[param_raw_msg_decimation] = cmd->data[3];
		LiDARParameters[param_raw_msg_mask] = cmd->data[1] + (cmd->data[2]<<8);
		if( LiDARParameters[param_raw_msg_decimation] )
		{
			LiDARParameters[param_DSP_enable] = 1; // force detection...
			cld_console(CLD_CONSOLE_PURPLE,CLD_CONSOLE_BLACK,"Raw data requested at 1:%d decimation with 0x%04X mask\r\n",
					LiDARParameters[param_raw_msg_decimation],LiDARParameters[param_raw_msg_mask]);
		}else
		{
			LiDARParameters[param_DSP_enable] = LiDARParameters[param_det_msg_decimation]?1:0; // stop detection...
			cld_console(CLD_CONSOLE_PURPLE,CLD_CONSOLE_BLACK,"Cooked data cancelled\r\n");
		}
		break;
	default:
		SetError(error_SW_comm_unknown);
		return(1);
	}
	return(0);
}
