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
#include "USB_msg.h"
#include "cld_bf70x_bulk_lib.h"

#include "../demo_app.h" // board definitions
#include "Msg_queue.h"

inline void ProcessError(void);
inline uint32_t GetTime(void);
inline void SendToUSB(USB_msg *msg);
inline void SendACK(void);
inline void SendNACK(void);
inline void SendNext(void);

inline void ProcessError()
{
	// do something... like red LED... plus BIST error
	LED_BC2R_ON();
}

inline uint32_t GetTime()
{
	uint32_t clockTime = 0;
	clockTime = (uint32_t) (cld_time_get() & 0xFFFFFFFF);
	return(clockTime);
}

/*
 * Message preparation
 */

void USB_sendStatus()
{
	static USB_CAN_message msg;

	msg.id = msgID_sensorStatus;
	msg.flags = 0;
	msg.timestamp = GetTime();
	msg.len = 0;
	// put error/status code in payload

	if( MsgQueue_Ok != msgQueuePush((USB_msg*) &msg))
			ProcessError();
}
void USB_SendBoot()
{
	static USB_CAN_message msg;

	msg.id = msgID_sensorBoot;
	msg.flags = 0;
	msg.timestamp = GetTime();
	msg.len = 0;

	if( MsgQueue_Ok != msgQueuePush((USB_msg*) &msg))
			ProcessError();
}

void USB_sendParameter(uint16_t address, uint16_t value)
{
	static USB_CAN_message msg;

	msg.id = msgID_responseParameter;
	msg.flags = 0;
	msg.timestamp = GetTime();
	msg.len = 4;
	msg.data[0] = (uint8_t) (address&0xFF);
	msg.data[1] = (uint8_t) (address>>8)&0xFF;
	msg.data[2] = (uint8_t) (value&0xFF);
	msg.data[3] = (uint8_t) (value>>8)&0xFF;

	if( MsgQueue_Ok != msgQueuePush((USB_msg*) &msg))
			ProcessError();
}

void USB_SendRawData(uint16_t pixelID, uint16_t *buf)
{
	static USB_raw_message msg;

	msg.timestamp = GetTime();
	msg.pixelNumber = pixelID;
	msg.payloadsize = RAW_NUM_SAMPLES; // to be validated... if synch has to be included, and 8 or 16 bits...
	msg.synch[0] = 0x7FFF;
	msg.synch[1] = 0x8000;
	memcpy(msg.data, buf, sizeof(msg.data));

	if( MsgQueue_Ok != msgQueuePush((USB_msg*) &msg))
		ProcessError();
}

void USB_SendTrack(uint16_t trackID, int pixelID, float probability, float intensity,
		          float distance, float velocity, float acceleration)
{
	USB_CAN_message msg;

	msg.timestamp = GetTime();
	msg.flags = 0;
	msg.len = 8;
	msg.pad1 = 0;
	msg.pad2 = 0;

	// send info
	msg.id = msgID_trackInfo;
	msg.data[0] = (uint8_t)trackID&0xFF;
	msg.data[1] = (uint8_t)(trackID>>8)&0xFF;
	msg.data[2] = 0x00;
	pixelID = pixelID<0?0:pixelID>15?15:pixelID;
	msg.data[3] = (uint8_t)pixelID&0xFF;
	msg.data[4] = (uint8_t)(pixelID>>8)&0xFF;
	probability = probability<0?0:probability>100?100:probability;
	msg.data[5] = (uint8_t) ((int)probability)&0xFF;
	intensity = intensity<-20?-20:intensity>108?108:intensity;
	intensity = 2*(intensity+20); // 1/2 dB with a 20dB offset
	msg.data[6] = (uint8_t)((int)intensity)&0xFF;
	msg.data[7] = (uint8_t)(((int)intensity)>>8)&0xFF;

	if( MsgQueue_Ok != msgQueuePush((USB_msg*) &msg))
		ProcessError();

	// send values
	msg.id = msgID_trackValue;
	msg.data[0] = (uint8_t)trackID&0xFF;
	msg.data[1] = (uint8_t)(trackID>>8)&0xFF;
	distance = distance<0?0:distance>200?200:distance;
	distance *= 100; // expressed in cm
	msg.data[2] = (uint8_t) ((int)distance)&0xFF;
	msg.data[3] = (uint8_t) (((int)distance)>>8)&0xFF;
	velocity = velocity<-100?-100:velocity>100?100:velocity;
	velocity *= 100; // expressed in cm/s
	msg.data[4] = (uint8_t) ((int)velocity)&0xFF;
	msg.data[5] = (uint8_t)(((int)velocity)>>8)&0xFF;
	acceleration = 0; // not used
	msg.data[6] = (uint8_t)((int)acceleration)&0xFF;
	msg.data[7] = (uint8_t)(((int)acceleration)>>8)&0xFF;

	if( MsgQueue_Ok != msgQueuePush( (USB_msg*) &msg))
		ProcessError();

}

void USB_SendEndOfFrame(uint16_t frameID, uint16_t systemID, uint16_t numTrackSent)
{
	USB_CAN_message msg;

	msg.id = msgID_FrameDone;
	msg.timestamp = GetTime();
	msg.flags = 0x00;
	msg.len = 8;

	msg.data[0] = (uint8_t)frameID&0xFF;
	msg.data[1] = (uint8_t)(frameID>>8)&0xFF;
	msg.data[2] = (uint8_t)systemID&0xFF;
	msg.data[3] = (uint8_t)(systemID>>8)&0xFF;
	msg.data[4] = 0;
	msg.data[5] = 0;
	msg.data[6] = (uint8_t)numTrackSent&0xFF;
	msg.data[7] = (uint8_t)(numTrackSent>>8)&0xFF;

	msg.pad1 = 0;
	msg.pad2 = 0;

	if( MsgQueue_Ok != msgQueuePush( (USB_msg*) &msg))
		ProcessError();
}

/*
 *  USB interface
 */


inline void SendToUSB(USB_msg *msg)
{
	if (msg->CAN.id == msgID_transmitRaw)
	{
		// do USB stuff (uint8_t*)msg, sizeof(USB_raw_message)
	}
	else
	{
		// do USB stuff (uint8_t*)msg, sizeof(USB_CAN_message)
	}
}


inline void SendACK()
{
	static USB_CAN_message msg;

	msg.id = msgID_ACK;
	msg.timestamp = GetTime();
	msg.flags = 0;
	msg.len = 0;

	SendToUSB((USB_msg*) &msg);
}

inline void SendNACK()
{
	static USB_CAN_message msg;

	msg.id = msgID_NACK;
	msg.timestamp = GetTime();
	msg.flags = 0;
	msg.len = 0;

	SendToUSB((USB_msg*) &msg);
}

inline void SendNext()
{
	static USB_msg msg;

	if (MsgQueue_Ok != msgQueuePop(&msg))
	{
		msg.CAN.id = msgID_queueEmpty;
		msg.CAN.timestamp = GetTime();
		msg.CAN.flags = 0;
		msg.CAN.len = 0;
	}
	SendToUSB(&msg);
}

void USB_ReadCommand(USB_CAN_message* msg)
{

	switch (msg->id)
	{
	case msgID_setparameter:
		SendACK();
		// do what has to de done
		break;
	case msgID_getParameter:
		SendACK();
		// do what has to de done
		break;
	case msgID_poll:
		SendNext();
		break;
	default:
		SendNACK();
	}

}
