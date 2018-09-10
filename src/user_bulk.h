#ifndef __USER_BULK_
#define __USER_BULK_
/*==============================================================================
    FILE:           user_bulk.h

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

#include <stdint.h>

typedef enum
{
    USER_BULK_INIT_SUCCESS = 0,
    USER_BULK_INIT_ONGOING,
    USER_BULK_INIT_FAILED,
} User_Bulk_Init_Return_Code;

extern User_Bulk_Init_Return_Code user_bulk_init (void);
extern void user_bulk_main (void);

extern int user_CANFifoReset(void);
extern int user_CANFifoPushReadResp(uint16_t registerAddress, uint16_t data);
extern int user_CANFifoPushDetection(int ch, uint16_t dist, uint16_t vel);
extern int user_CANFifoPushCompletedFrame(void);

extern int user_CANFifoPushSensorStatus(void);
extern int user_CANFifoPushSensorBoot(void);

#endif /* __USER_BULK_ */
