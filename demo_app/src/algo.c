/*
  Copyright 2018 Phantom Intelligence Inc.
*/

#include <math.h>
#include "algo.h"
#include "parameters.h"



static float minvalue(float* data, int NbData) {
	float min = data[0];
	for (int i = 0; i < NbData; i++) {
		if(data[i] < min){
			min = data[i];
		}
	}
	return min;
}

static int minpos(float* data, int NbData) {
	float min = data[0];
	int pos = 0;

	for (int i = 0; i < NbData; i++) {
		if(data[i] < min){
			min = data[i];
			pos = i;
		}
	}
	return pos;
}

static float findmean(float* data, int NbData) {
	int i;
	float fMEAN = 0;

	for (i = 0; i < NbData; i++) {
		fMEAN += data[i];
	}
	fMEAN = fMEAN / NbData;
	return (fMEAN);
}
static float findSNR(float* sigptr, int siglength, int minpos, int minwidth)
{
	float SNR = 0.0;
	float sigRMS = 0.0;
	float noiseRMS = 0.0;
	int pulsestart = 0;
	int pulsestop = siglength;
	int pos;
	float *dataptr;

	pulsestart = minpos - (int)((minwidth-1)/2);
	pulsestop = minpos + (int)((minwidth-1)/2);

	for(pos=0,dataptr=sigptr; pos<siglength; pos++,dataptr++)
	{
		if(pos>=pulsestart && pos<=pulsestop)
			sigRMS += pow(*dataptr,2);
		else if (pos<=pulsestart-minwidth || pos>=pulsestop+minwidth)
			noiseRMS += pow(*dataptr, 2);
	}

	noiseRMS = sqrt(noiseRMS);
	sigRMS = sqrt(sigRMS);

	SNR = 20*log(sigRMS/noiseRMS);

	return(SNR);
}

void threshold2(detection_type* detPtr, float* buffer, int ch)
{
	// Threshold
	float * sigPtr;
	float noisemean, minVal, threshold, dx = 0.5 * LIGHTSPEED * SAMPLE_TIME;
	float intervalPercentage = 50; // %
	float minimum_Viable_Value = 1000;
	int iNbDetected = 0;
	int i;

	sigPtr = buffer;

	noisemean = findmean(sigPtr, 100); // Find noise mean (DC value)
	minVal = minvalue(sigPtr,100);

	if(abs(minVal - noisemean) > minimum_Viable_Value){

		threshold = (intervalPercentage/100)*(abs(minVal - noisemean));

		for(i = 0; i < DEVICE_SAMPLING_LENGTH; i++)
		{
			if(i <= 19)
			{
				if(*sigPtr < (noisemean - threshold) && *sigPtr <= -3000  && iNbDetected < DEVICE_NUM_DET_PER_CH) {
					detPtr[iNbDetected].distance = i*dx - DISTANCE_OFFSET;
					detPtr[iNbDetected].intensity = sigPtr[i];
					++iNbDetected;
				}
			} else {
				if(*sigPtr < (noisemean - threshold)  && iNbDetected < DEVICE_NUM_DET_PER_CH) {
					detPtr[iNbDetected].distance = i*dx - DISTANCE_OFFSET;
					detPtr[iNbDetected].intensity = sigPtr[i];
					++iNbDetected;
				}
			}
			sigPtr++;
		}
	}

	for (; iNbDetected < DEVICE_NUM_DET_PER_CH; iNbDetected++) {
		detPtr[iNbDetected].distance = 0.00;
		detPtr[iNbDetected].intensity = 0.00;
	}
}

void threshold3(detection_type* detPtr, float* buffer, int ch)
{
	// Threshold
	float * sigPtr;
	float noisemean, minVal, threshold, dx = 0.5 * LIGHTSPEED * SAMPLE_TIME;
	float minimum_Viable_Value = 1000;
	int iNbDetected = 0;
	int i, minPos;
	const int siglength = 100; // ADAL6110-16 signal length
	const int minwidth = 11;   // set the window width for the signal

	sigPtr = buffer;

	noisemean = findmean(sigPtr, siglength); // Find signal mean
	minVal = minvalue(sigPtr,siglength);     // Find signal min value
	minPos = minpos(sigPtr, siglength);      // find signal min value pos


	if(abs(minVal - noisemean) > minimum_Viable_Value) {

		detPtr[iNbDetected].distance = minPos*dx - DISTANCE_OFFSET;
		detPtr[iNbDetected].intensity = findSNR(sigPtr,siglength,minPos,minwidth); // value in dB should be between -21 and 100 dB
		++iNbDetected;

	}

	for (; iNbDetected < DEVICE_NUM_DET_PER_CH; iNbDetected++) {
		detPtr[iNbDetected].distance = 0.00;
		detPtr[iNbDetected].intensity = 0.00;
	}
}

typedef union{
	uint16_t param[16];
	struct{
		uint16_t SNR_min;
		uint16_t pulse_width;
		uint16_t gap_width;
	};
}threshold4_param;

void threshold4(detection_type* detPtr, float* buffer, int ch)
{
	float * sigptr;
	int minPos,pos;
	float mean=0.0;
	int gapstart,gapstop;
	float RMSnoise = 0.0;
	float RMSsig = 0.0;
	float SNR=0.0;
	int iNbDetected = 0;

	const float dx = 0.5 * LIGHTSPEED * SAMPLE_TIME;
	const int siglength = 100;
	const threshold4_param* algo_param = &LiDARParameters[param_detection_config];

	sigptr = buffer;

	minPos = minpos(sigptr,siglength);
	gapstart = minPos-algo_param->gap_width;
	gapstart = gapstart<0?0:gapstart;
	gapstop  = minPos+algo_param->gap_width;
	gapstop  = gapstop>siglength?siglength:gapstop;

	for(pos=0;pos<siglength;pos++)
		if(pos<gapstart || pos>gapstop)
			mean += sigptr[pos];
	mean /= siglength - (gapstop-gapstart);

	for(pos=0;pos<siglength;pos++)
	{
		if(pos<gapstart || pos>gapstop)
			RMSnoise += pow((sigptr[pos]-mean),2);
		else if ( pos>(minPos-algo_param->pulse_width) && pos<(minPos+algo_param->pulse_width) )
			RMSsig += pow((sigptr[pos]-mean),2);
	}

	RMSnoise = sqrt(RMSnoise/(siglength-(gapstop-gapstart)));
	RMSsig = sqrt(RMSsig/(gapstop-gapstart));
	SNR = 20*log(RMSsig/RMSnoise);

	if(SNR > algo_param->SNR_min) {
			detPtr[iNbDetected].distance = minPos*dx - DISTANCE_OFFSET;
			detPtr[iNbDetected].intensity = SNR;
			++iNbDetected;
	}

	for (; iNbDetected < DEVICE_NUM_DET_PER_CH; iNbDetected++) {
		detPtr[iNbDetected].distance = 0.00;
		detPtr[iNbDetected].intensity = 0.00;
	}
}
