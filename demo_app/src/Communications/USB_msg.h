/*
 * USB_msg.h
 *
 *  Created on: Feb 11, 2020
 *      Author: e.turenne
 */

#ifndef USB_MSG_H_
#define USB_MSG_H_

#include <stdint.h>
#include "../algo.h"


enum Msg_ID{
	// output messages, pushed in msg queue
	msgID_sensorStatus     = 0x01, // BIST
	msgID_sensorBoot       = 0x02, // alive, just to notice the host
	msgID_transmitRaw      = 0xB0, // raw channel sampling
	msgID_trackInfo        = 0x0A, // track information (id, pixel, Intensity, confidence)
	msgID_trackValue       = 0x0B, // track information (id, distance, velocity, acceleration)
	msgID_FrameDone        = 0x09, // all the data in the current frame were sent
	//msgID_Alarms           = 0x10, // send the alarms (if implemented)
	//msgID_distance_intensity=0x3C, // send detection distance and intensity (untracked)
	//Input messages, answered from queue, msgID_queueEmptycmd if queue is empty
	msgID_poll             = 0x56, // get a message from the queue
	// input messages, ack/nack answer
	msgID_command          = 0x50,
	// commands
	msgID_setparametercmd  = 0xC0, // send a parameter
	msgID_getParametercmd  = 0xC1, // get a parameter (0xC2 response pushed in queue)
	msgID_respParametercmd = 0xC2, // response to msgID_getParameter (0xC1)
	// command responses
	msgID_ACKSetcmd        = 0xC8, // Command well received and understood
	msgID_ACKGetcmd        = 0xC9, // Command well received and understood
	msgID_NACKcmd          = 0xC3, // Command not received or not understood or timed out
	msgID_queueEmptycmd    = 0xCA, // no message left to send (response to msgID_poll)
	msgID_requestCookedcmd = 0xE0,
	msgID_requestRawcmd    = 0xE1
};


enum Param_Type{
	cmdParam_DetectionAlgo = 0x01,
	cmdParam_DetectionParam = 0x02,
	cmdParam_SensorRegister = 0x03,
	cmdParam_ADCRegister = 0x05,
	cmdParam_GlobalParam =  0x07,
	cmdParam_GPIORegister = 0x08,
	cmdParam_TrackingAlgo = 0x11,
	cmdParam_TrackingParam = 0x12,
	cmdParam_Date_Time     = 0x20
};

#define CAN_PAYLOAD_LENGTH 8    //in bytes
#define RAW_NUM_SAMPLES    DEVICE_SAMPLING_LENGTH  // in 16b samples
#define RAW_PAYLOAD_LENGTH RAW_NUM_SAMPLES*2 + 4  //in bytes
#define RAW_SYNCH_VALUES {0x7fff, 0x8000}

typedef struct{
	uint32_t id;
    uint32_t timestamp; // timestamp in milliseconds (16 bits used)
    uint8_t  flags;     // [extended_id|1][RTR:1][reserver:6]
	uint8_t  len;       // Frame size (0.8)
    uint8_t  data[CAN_PAYLOAD_LENGTH];   // Databytes 0..7
    uint8_t  pad1;      // Padding required so that sizeof(AWLCANMessage) be 20 on all platforms
    uint8_t  pad2;      // ...
} USB_CAN_message;

typedef struct {
	uint8_t  id;
    uint8_t  reserved; // not used
    uint16_t pixelNumber; // logical pixel index in array
    uint16_t timestamp;   // timestamp in ms
    uint16_t payloadsize; // includes all of the following data (102 *2)
    uint16_t synch[2]; // infinite values to mark the start of the payload
	uint16_t data[RAW_NUM_SAMPLES];// payload of 16 bits values
} USB_raw_message;

typedef union{
	USB_CAN_message CAN;   // Can format
	USB_raw_message RAW;   // raw format
} USB_msg;

#endif /* USB_MSG_H_ */
