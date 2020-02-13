/*
  Copyright 2018 Phantom Intelligence Inc.
*/

//TODO RENAME AWL TO SOMETHING ELSE

#ifndef CANMESSAGEDEF_H
#define CANMESSAGEDEF_H


// len
#define AWLCANMSG_LEN 8

// CAN Frame
typedef struct {
    uint32_t id;        // Message id
    uint32_t timestamp; // timestamp in milliseconds
    uint8_t  flags;     // [extended_id|1][RTR:1][reserver:6]
    uint8_t  len;       // Frame size (0.8)
    uint8_t  data[AWLCANMSG_LEN];   // Databytes 0..7
    uint8_t  pad1;      // Padding required so that sizeof(AWLCANMessage) be 20 on all platforms
    uint8_t  pad2;      // ...
} CANMessage;


// id

//status report
#define PICANMSG_ID_SENSORSTATUS                   1
#define PICANMSG_ID_SENSORBOOT                     2
#define PICANMSG_ID_COMPLETEDFRAME                 9

// obsolete
#define PICANMSG_ID_OBSTACLETRACK                  10
#define PICANMSG_ID_OBSTACLEVELOCITY               11
#define PICANMSG_ID_OBSTACLESIZE                   12
#define PICANMSG_ID_OBSTACLEANGULARPOSITION        13
#define PICANMSG_ID_CHANNELDISTANCE1_FIRST         20
#define PICANMSG_ID_CHANNELDISTANCE1_LAST          26
#define PICANMSG_ID_CHANNELDISTANCE2_FIRST         30
#define PICANMSG_ID_CHANNELDISTANCE2_LAST          36
#define PICANMSG_ID_CHANNELINTENSITY1_FIRST        40
#define PICANMSG_ID_CHANNELINTENSITY1_LAST         46
#define PICANMSG_ID_CHANNELINTENSITY2_FIRST        50
#define PICANMSG_ID_CHANNELINTENSITY2_LAST         56

// fused data message per pixel
#define PICANMSG_ID_CHANNELDISTANCEANDINTENSITY    60

// do stuff general command
#define PICANMSG_ID_COMMANDMESSAGE                 80

// New command id for Wagner/Guardian
#define PICANMSG_ID_GETDATA                        87
#define PICANMSG_ID_POLLMESSAGES                   88
#define PICANMSG_ID_LIDARQUERY                     89


// parameters
#define PICANMSG_ID_CMD_SET_PARAMETER            0xC0
#define PICANMSG_ID_CMD_QUERY_PARAMETER          0xC1
#define PICANMSG_ID_CMD_RESPONSE_PARAMETER       0xC2

// file management (no files in wagner)
#define PICANMSG_ID_CMD_PLAYBACK_RAW             0xD1
#define PICANMSG_ID_CMD_RECORD_CALIBRATION       0xDA

// ID qui ne correspondent pas a la structure CAN
#define PICANMSG_ID_CMD_TRANSMIT_RAW             0xE0
#define PICANMSG_ID_CMD_TRANSMIT_COOKED          0xE1

#define PICANMSG_ID_CMD_PARAM_ALGO_SELECTED      0x01
#define PICANMSG_ID_CMD_PARAM_ALGO_PARAMETER     0x02
#define PICANMSG_ID_CMD_PARAM_AWL_REGISTER       0x03
#define PICANMSG_ID_CMD_PARAM_BIAS               0x04
#define PICANMSG_ID_CMD_PARAM_ADC_REGISTER       0x05
#define PICANMSG_ID_CMD_PARAM_PRESET             0x06
#define PICANMSG_ID_CMD_PARAM_GLOBAL_PARAMETER   0x07
#define PICANMSG_ID_CMD_PARAM_GPIO_CONTROL       0x08
#define PICANMSG_ID_CMD_PARAM_TRACKER_SELECTED   0x11
#define PICANMSG_ID_CMD_PARAM_TRACKER_PARAMETER  0x12
#define PICANMSG_ID_CMD_PARAM_DATE_TIME          0x20
#define PICANMSG_ID_CMD_PARAM_RECORD_FILENAME    0xD0
#define PICANMSG_ID_CMD_PARAM_PLAYBACK_FILENAME  0xD1
#define PICANMSG_ID_CMD_PARAM_


#endif //CANMESSAGEDEF_H
