/*
 * system.h
 *
 * ADSP-BF7xx system
 *
 * File Version 2.8.0.0
 *
 * Copyright (c) 2011-2017 Analog Devices, Inc. All Rights Reserved.
 */

#ifndef SYSTEM_H
#define SYSTEM_H

/*****************************************************************************
  Include Files
******************************************************************************/
#include "init_platform.h"

#include <stdint.h>
#include <stdbool.h>
#include <sys/platform.h>

#include <cplb.h>
#include <cplbtab.h>

/*****************************************************************************
 Local definitions of CGU structure
******************************************************************************/
typedef struct _cgu_config
{
    uint32_t ulCGU_CTL;         /*!< Content of Clock Control Register       */
    uint32_t ulCGU_STAT;        /*!< Content of CGU Status Register          */
    uint32_t ulCGU_DIV;         /*!< Content of Clock Divide Register        */
    uint32_t ulCGU_CLKOUTSEL;   /*!< Content of CLKOUT Mux Select Register   */
} cgu_config;                   /*!< Local CGU Config Structure */

/*****************************************************************************
 Prototypes
******************************************************************************/

uint32_t get_pllclk_hz(void);
uint32_t get_pllclk_ns(void);
uint32_t get_cclk_hz(void);
uint32_t get_cclk_ns(void);
uint32_t get_sysclk_hz(uint32_t ulRegCgu0Ctl, uint32_t ulRegCgu0Stat, uint32_t ulRegCgu0Div);
uint32_t get_s0clk_hz(void);
uint32_t get_s1clk_hz(void);
uint32_t get_ddrclk_hz(void);
uint32_t get_ddrclk_ns(void);

#pragma noreturn
void ErrorWithNoReturn(void);

/****************************************************************************
 EOF
*****************************************************************************/

#endif /* SYSTEM_H */
