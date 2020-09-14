#ifndef _ALGO_H_
#define _ALGO_H_
/*
  Copyright 2018 Phantom Intelligence Inc.
*/

/**
 * data management constants
 */
#define DEVICE_SAMPLING_LENGTH       100
#define DEVICE_NUM_CHANNEL           16
#define DEVICE_NUM_RAWHISTORY        4
#define DEVICE_NUM_BANKS             2
#define DEVICE_NUM_DET_PER_CH        1
#define DEVICE_NUM_TRACK_PER_CH      4
#define DEVICE_NUM_ALARM             16
#define DEVICE_NUM_TRIGGER           2

/**
 * DSP constant
 */
#define LIGHTSPEED 299792458  // m/s

#define SAMPLE_TIME (1.0 / (250000000))

#define DISTANCE_OFFSET 6.4


/**
 * detection result
 */
typedef struct{
	float distance;
	float intensity;
} detection_type;

typedef struct{
	unsigned int timeStamp;
	detection_type detections[DEVICE_NUM_CHANNEL][DEVICE_NUM_DET_PER_CH];
} detection_frame_type;


void threshold2(detection_type* detPtr, float* buffer, int ch);
void threshold3(detection_type* detPtr, float* buffer, int ch);
void threshold4(detection_type* detPtr, float* buffer, int ch);


#endif //_ALGO_H_
