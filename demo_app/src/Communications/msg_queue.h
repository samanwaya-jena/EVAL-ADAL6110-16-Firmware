/*
 * msg_queue.h
 *
 *  Created on: Feb 11, 2020
 *      Author: e.turenne
 */

#ifndef MSG_QUEUE_H_
#define MSG_QUEUE_H_

#include "USB_msg.h"

typedef enum {
	MsgQueue_Ok = 0,
	MsgQueue_Full,
	MsgQueue_Empty
}MsgQueueError;

MsgQueueError msgQueueReset();
MsgQueueError msgQueuePush(USB_msg* msg);
MsgQueueError msgQueuePop(USB_msg* msg);

#endif /* MSG_QUEUE_H_ */
