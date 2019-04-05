/*
 * uart.h
 *
 * ADSP-BF7xx UART
 *
 * File Version 2.8.0.0
 *
 * Copyright (c) 2011-2017 Analog Devices, Inc. All Rights Reserved.
 */

#ifndef UART_H
#define UART_H

#include "init_platform.h"

#if CONFIG_CGU

#include "system.h"
#include <sys/platform.h>
#include <stdint.h>
#include <builtins.h>

/*****************************************************************************
 Prototypes
******************************************************************************/

uint32_t UartGetBitrate(uint32_t UartNum);
int16_t UartSetBitrate(uint32_t UartNum, uint32_t UartBitrate);
uint32_t UartRxFifoClear(uint32_t UartNum, uint32_t UartBitrate);

#endif /* CONFIG_CGU */

#endif /* UART_H */
