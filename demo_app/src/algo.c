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

static int minpos(float* _data, int _NbData) {
	float min = _data[0];
	int pos = 0;

	for (int i = 0; i < _NbData; i++) {
		if(_data[i] < min){
			min = _data[i];
			pos = i;
		}
	}
	return pos;
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

void threshold2(detection_type* _detPtr, float* _buffer, int _ch)
{
	// Threshold
	float * sigPtr;
	float noisemean, minVal, threshold, dx = 0.5 * LIGHTSPEED * SAMPLE_TIME;
	float intervalPercentage = 50; // %
	float minimum_Viable_Value = 1000;
	int iNbDetected = 0;
	int i;

	sigPtr = _buffer;

	noisemean = findmean(sigPtr, 100); // Find noise mean (DC value)
	minVal = minvalue(sigPtr,100);

	if(abs(minVal - noisemean) > minimum_Viable_Value){

		threshold = (intervalPercentage/100)*(abs(minVal - noisemean));

		for(i = 0; i < GUARDIAN_SAMPLING_LENGTH; i++)
		{
			if(i <= 19)
			{
				if(*sigPtr < (noisemean - threshold) && *sigPtr <= -3000  && iNbDetected < GUARDIAN_NUM_DET_PER_CH) {
					_detPtr[iNbDetected].distance = i*dx - DISTANCE_OFFSET;
					_detPtr[iNbDetected].intensity = sigPtr[i];
					++iNbDetected;
				}
			} else {
				if(*sigPtr < (noisemean - threshold)  && iNbDetected < GUARDIAN_NUM_DET_PER_CH) {
					_detPtr[iNbDetected].distance = i*dx - DISTANCE_OFFSET;
					_detPtr[iNbDetected].intensity = sigPtr[i];
					++iNbDetected;
				}
			}
			sigPtr++;
		}
	}

	for (; iNbDetected < GUARDIAN_NUM_DET_PER_CH; iNbDetected++) {
		_detPtr[iNbDetected].distance = 0.00;
		_detPtr[iNbDetected].intensity = 0.00;
	}
}

void threshold3(detection_type* _detPtr, float* _buffer, int _ch)
{
	// Threshold
	float * sigPtr;
	float noisemean, minVal, threshold, dx = 0.5 * LIGHTSPEED * SAMPLE_TIME;
	float minimum_Viable_Value = 1000;
	int iNbDetected = 0;
	int i, minPos;

	sigPtr = _buffer;

	noisemean = findmean(sigPtr, 100); // Find signal mean
	minVal = minvalue(sigPtr,100); // Find signal min value
	minPos = minpos(sigPtr, 100); // find signal min value pos


	if(abs(minVal - noisemean) > minimum_Viable_Value) {

		_detPtr[iNbDetected].distance = minPos*dx - DISTANCE_OFFSET;
		_detPtr[iNbDetected].intensity = minVal;
		++iNbDetected;

	}

	for (; iNbDetected < GUARDIAN_NUM_DET_PER_CH; iNbDetected++) {
		_detPtr[iNbDetected].distance = 0.00;
		_detPtr[iNbDetected].intensity = 0.00;
	}
}
