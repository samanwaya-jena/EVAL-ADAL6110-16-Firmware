/*********************************************************************************

Copyright(c) 2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/
/*!
* @file     PWR_Freq_Mode.h
*
* @brief    Primary header file for power service example.
*
* @details  Primary header file power service example which contains the
*           processor specific defines.
*
*/

#ifndef _PWRFREQMODE_H_
#define _PWRFREQMODE_H_

uint32_t power_init(void);
uint32_t ChangeFreq( uint32_t, uint32_t );


/* default power settings */
#define DF_DEFAULT    (0x0u)
#define MSEL_DEFAULT  (0x10u)
#define SSEL_DEFAULT  (0x8u)
#define CSEL_DEFAULT  (0x4u)

#define MHZTOHZ       (1000000u)

#define CLKIN         (25 * MHZTOHZ)
#define CORE_MAX      (400 * MHZTOHZ)
#define SYSCLK_MAX    (200 * MHZTOHZ)
#define SCLK_MAX      (100 * MHZTOHZ)
#define VCO_MIN       (72 * MHZTOHZ)

#endif /* _PWRFREQMODE_H_ */
