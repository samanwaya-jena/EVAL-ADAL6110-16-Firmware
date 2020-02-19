/*
 * USB_cmd.h
 *
 *  Created on: Feb 11, 2020
 *      Author: e.turenne
 *
 *      defines all USB commands that can be received
 *
 */

#ifndef USB_CMD_H_
#define USB_CMD_H_
#include <stdint.h>
#include "USB_msg.h"

// System called functions
void USB_pushStatus(void);
void USB_pushBoot(void);
void USB_pushParameter(uint16_t address, uint16_t value);
// DSP called functions
void USB_pushRawData(uint16_t pixelID, uint16_t *buf);
void USB_pushTrack(uint16_t trackID, int pixelID, float probability, float intensity,
        float distance, float velocity, float acceleration);
void USB_pushEndOfFrame(uint16_t frameID, uint16_t systemID, uint16_t numTrackSent);
// incoming message handling
void USB_ReadCommand(USB_CAN_message* cmd, USB_msg* ret_msg);

#endif /* USB_CMD_H_ */
