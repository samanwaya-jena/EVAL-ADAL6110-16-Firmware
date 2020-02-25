/*
 * Msg_queue.c
 *
 *  Created on: Feb 11, 2020
 *      Author: e.turenne
 */
#include <string.h>
#include <builtins.h>

#include "msg_queue.h"
#include "USB_msg.h"


#define DISABLE_INT() unsigned int _intm = cli()
#define ENABLE_INT() sti(_intm)
#define QUEUE_LENGTH 256
#define QUEUE_POINTER_MASK 0xFF

/*
 * private interface
 */
static const USB_msg Queue[QUEUE_LENGTH];
static int head;
static int tail;


/*
 * Public interface
 */
MsgQueueError msgQueueReset()
{
	DISABLE_INT();
	head=0;
	tail=0;
	ENABLE_INT();
	return(MsgQueue_Ok);
}


MsgQueueError msgQueuePush(USB_msg* msg)
{
	int insert_pos;
	MsgQueueError retVal = MsgQueue_Full;
	//DISABLE_INT();

	insert_pos = (head+1) & QUEUE_POINTER_MASK;
	if (insert_pos != tail)
	{
		memcpy((void*)&Queue[insert_pos], msg, sizeof(USB_msg));
		head = insert_pos;
		retVal = MsgQueue_Ok;
	}

	//ENABLE_INT();
	return(retVal);
}


MsgQueueError msgQueuePop(USB_msg* msg)
{
	MsgQueueError retVal = MsgQueue_Empty;
	//DISABLE_INT();

	if (tail != head)
	{
		tail = (tail+1) & QUEUE_POINTER_MASK;
		memcpy(msg, &Queue[tail], sizeof(USB_msg));
		retVal = MsgQueue_Ok;
	}

	//ENABLE_INT();
	return(retVal);
}


MsgQueueError msgQueueStatus()
{
	if (head == tail)
		return(MsgQueue_Empty);
	else if (tail == (head+1) & QUEUE_POINTER_MASK)
		return(MsgQueue_Full);
	return(MsgQueue_Ok);
}
