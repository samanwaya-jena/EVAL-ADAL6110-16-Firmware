
/*
 * parameters.h
 *
 *  Created on: Feb 22, 2020
 *      Author: e.turenne
 */

#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#include <stdint.h>

typedef enum
{
	param_deviceID = 0x00,  // byte1:family, byte2:configuration
	param_manufDate,        // byte1:year (2000-2255), byte2: month (1-12)
	param_serialNumber,     // unique number (if rollover, use in conjunction with date)

	param_acq_enable = 0x10,
	param_DSP_enable,
	param_console_log,      // bitfield

	param_detection_algo = 0x20,
	param_tracking_algo,
	param_num_det,
	param_num_track,

	param_detection_config = 0x30,  // 16 values reserved -- see algo for specifics
	param_tracking_config = 0x40,   // 16 values reserved -- see algo for specifics
	param_channel_map_offset = 0x50,

	param_det_msg_decimation = 0x60, //0 to disable comm.
	param_det_msg_mask,
	param_raw_msg_decimation,
	param_raw_msg_mask,
	param_status_period, // in ms

	param_last_address = 0x6F,
	number_of_param
}param_index;

#define CONSOLE_MASK_LOG  0x01
#define CONSOLE_MASK_USB  0x02
#define CONSOLE_MASK_DIST 0x04
#define CONSOLE_MASK

#define RW_WRITE_MASK    0x8000
#define RW_INTERNAL_MASK 0x4000


extern uint16_t LiDARParameters[number_of_param]; // value of all functional parameters -- 32 bits flash registers -- also easier to cast in float


void param_InitValues(void);
void param_ResetFactoryDefault(void);

void param_LoadConfig(void);
void param_SaveConfig(void);

int param_ProcessReadWriteFifo(void);
int param_ReadFifoPush(uint16_t _startAddress);
int param_WriteFifoPush(uint16_t _startAddress, uint16_t data);


#endif /* PARAMETERS_H_ */
