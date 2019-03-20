/*
  Copyright 2018 Phantom Intelligence Inc.
*/

#include <math.h>

#include "algo.h"



static float minvalue(float* _data, int _NbData) {
	float min = _data[0];
	for (int i = 0; i < _NbData; i++) {
		if(_data[i] < min){
			min = _data[i];
		}
	}
	return min;
}

static float findmean(float* _data, int _NbData) {
	int i;
	float fMEAN = 0;

	for (i = 0; i < _NbData; i++) {
		fMEAN += _data[i];
	}
	fMEAN = fMEAN / _NbData;
	return (fMEAN);
}

#if 0
static inline float STD(const float* _buff, const int _length, const float _mean) {
	float stdTemp = 0.00;
	for (int i = 0; i < _length; i++) {
		stdTemp += (_buff[i] - _mean) * (_buff[i] - _mean);
	}
	return (sqrtf(stdTemp / _length));
}

static inline void threshold(detection_type* _detPtr, float* _buffer, int _ch)
{
	// Threshold
	float * sigPtr;
	float mean, stdvalue, i, dx = 0.5 * LIGHTSPEED * SAMPLE_TIME, std_factor = 2;
	int iNbDetected = 0, dead_zone = 8;

	sigPtr = _buffer;

	mean = findmean(sigPtr, dead_zone); // Find noise mean (DC value)
	stdvalue = STD(sigPtr+dead_zone, GUARDIAN_SAMPLING_LENGTH-2*dead_zone, mean); // Find complete signal STD, excluding start and last points

	for(i = dead_zone; i < GUARDIAN_SAMPLING_LENGTH-dead_zone; i++)
	{
		if(*sigPtr < mean - std_factor*stdvalue && iNbDetected < GUARDIAN_NUM_DET_PER_CH) {
			_detPtr[iNbDetected].distance = i*dx - DISTANCE_OFFSET;
			_detPtr[iNbDetected].intensity = *sigPtr;
			++iNbDetected;
		}
		sigPtr++;
	}

	for (; iNbDetected < GUARDIAN_NUM_DET_PER_CH; iNbDetected++) {
		_detPtr[iNbDetected].distance = 0.00;
		_detPtr[iNbDetected].intensity = 0.00;
	}

}
#endif

//void threshold2(detection_type* _detPtr, float* _buffer, int _ch)
//{
//	// Threshold
//	float * sigPtr;
//	float noisemean, minVal, i, threshold, dx = 0.5 * LIGHTSPEED * SAMPLE_TIME;
//	float intervalPercentage = 50; // %
//	int iNbDetected = 0, dead_zone = 8;
//
//	sigPtr = _buffer;
//
//	noisemean = findmean(sigPtr, dead_zone); // Find noise mean (DC value)
//	minVal = minvalue(sigPtr+dead_zone,GUARDIAN_SAMPLING_LENGTH-2*dead_zone);
//
//	threshold = (intervalPercentage/100)*(abs(minVal - noisemean));
//
//	sigPtr = _buffer + dead_zone;
//
//	for(i = dead_zone; i < GUARDIAN_SAMPLING_LENGTH-dead_zone; i++)
//	{
//		if(*sigPtr < (noisemean - threshold) && iNbDetected < GUARDIAN_NUM_DET_PER_CH) {
//			_detPtr[iNbDetected].distance = i*dx - DISTANCE_OFFSET;
//			_detPtr[iNbDetected].intensity = *sigPtr;
//			++iNbDetected;
//		}
//		sigPtr++;
//	}
//
//	for (; iNbDetected < GUARDIAN_NUM_DET_PER_CH; iNbDetected++) {
//		_detPtr[iNbDetected].distance = 0.00;
//		_detPtr[iNbDetected].intensity = 0.00;
//	}
//}

void threshold2(detection_type* _detPtr, float* _buffer, int _ch)
{
	// Threshold
	float * sigPtr;
	float noisemean, minVal, i, threshold, dx = 0.5 * LIGHTSPEED * SAMPLE_TIME;
	float intervalPercentage = 50; // %
	float minimum_Viable_Value = 200;
	int iNbDetected = 0, dead_zone = 100;

	sigPtr = _buffer;

	noisemean = findmean(sigPtr, GUARDIAN_SAMPLING_LENGTH); // Find noise mean (DC value)
	minVal = minvalue(sigPtr,GUARDIAN_SAMPLING_LENGTH);

	if(abs(minVal - noisemean) > minimum_Viable_Value){

		threshold = (intervalPercentage/100)*(abs(minVal - noisemean));

		for(i = 0; i < GUARDIAN_SAMPLING_LENGTH; i++)
		{
			if(*sigPtr < (noisemean - threshold) && iNbDetected < GUARDIAN_NUM_DET_PER_CH) {
				_detPtr[iNbDetected].distance = i*dx - DISTANCE_OFFSET;
				_detPtr[iNbDetected].intensity = *sigPtr;
				++iNbDetected;
			}
			sigPtr++;
		}

	}

	for (; iNbDetected < GUARDIAN_NUM_DET_PER_CH; iNbDetected++) {
		_detPtr[iNbDetected].distance = 0.00;
		_detPtr[iNbDetected].intensity = 0.00;
	}
}
