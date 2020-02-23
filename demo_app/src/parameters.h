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
	param_deviceID = 0x00,
	param_serialNumber,
	param_sensor_enable,
	param_status_period,  //in ms

	param_acq_enable = 0x10,
	param_DSP_enable,
	param_console_log, // bitfield

	param_detection_algo = 0x20,
	param_tracking_algo,
	param_num_det,
	param_num_track,

	param_detection_config = 0x30,  // 16 values reserved -- see algo for specifics
	param_tracking_config = 0x40,   // 16 values reserved -- see algo for specifics


	param_det_msg_decimation = 0x50, //0 to disable comm.
	param_raw_msg_decimation,

	number_of_param
}param_index;



#define CONSOLE_MASK_LOG 0x01
#define CONSOLE_MASK_USB 0x02
#define CONSOLE_MASK

#define RW_WRITE_MASK    0x8000
#define RW_INTERNAL_MASK 0x4000



extern uint16_t LiDARParameters[number_of_param]; // value of all functional parameters
extern uint8_t  LiDARParamDir[number_of_param];   // 0 = read only, 1 = read/write


int ProcessReadWriteFifo(void);

int Lidar_ReadFifoPush(uint16_t _startAddress);
int Lidar_WriteFifoPush(uint16_t _startAddress, uint16_t data);



#endif /* PARAMETERS_H_ */
