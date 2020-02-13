/*
 * USB_msg.h
 *
 *  Created on: Feb 11, 2020
 *      Author: e.turenne
 */

#ifndef USB_MSG_H_
#define USB_MSG_H_

#include <stdint.h>


enum Msg_ID{
	// output messages, does not pass trough message queue
	msgID_ACK              = 0x53, // Command well received and understood
	msgID_NACK             = 0x54, // Command not received or not understood or timed out
	msgID_queueEmpty       = 0x56, // no message left to send (response to msgID_poll)
	// output messages, pushed in msg queue
	msgID_sensorStatus     = 0x01, // BIST
	msgID_sensorBoot       = 0x02, // alive, just to notice the host
	msgID_responseParameter= 0x52, // response to msgID_getParameter (0x51)
	msgID_transmitRaw      = 0xB0, // raw channel sampling
	msgID_trackInfo        = 0x0A, // track information (id, pixel, Intensity, confidence)
	msgID_trackValue       = 0x0B, // track information (id, distance, velocity, acceleration)
	msgID_FrameDone        = 0x09, // all the data in the current frame were sent
	//Input messages, answered from queue
	msgID_poll             = 0x55,  // get a message from the queue
	// input messages, ack/nack answer
	msgID_setparameter     = 0x50,  // send a parameter
	msgID_getParameter     = 0x51,  // get a parameter (0x52 response pushed in queue)
};


#define CAN_PAYLOAD_LENGTH 8    //in bytes
#define RAW_NUM_SAMPLES    100  // in 16b samples
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
