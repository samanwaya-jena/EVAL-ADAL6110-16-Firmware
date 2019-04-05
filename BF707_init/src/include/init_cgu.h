/*
 *  init_cgu.h
 *
 *  Initialize CGU programmatically
 *
 *  File Version: 2.8.0.0
 *
 *  Copyright (c) 2011-2017 Analog Devices, Inc. All Rights Reserved.
 */

#ifndef INIT_CGU_H
#define INIT_CGU_H

#include "init_platform.h"

#if CONFIG_CGU

#include <sys/platform.h>

/* CGU Function Declaration */

void init_cgu(void);

/* PLL Multiplier and Divisor Selections (Required Value, Bit Position) */

#define DF     ((0uL  << BITP_CGU_CTL_DF)     & BITM_CGU_CTL_DF)     /* Set this if DF is required */

#if defined(__ADSPBF700__) || defined(__ADSPBF701__)
/* BF70[01] */
#define MSEL   ((16uL << BITP_CGU_CTL_MSEL)   & BITM_CGU_CTL_MSEL)   /* PLL Multiplier Select [1-127]: PLLCLK = ((CLKIN x MSEL/DF+1)) = 400MHz       */
#define DSEL   ((4UL  << BITP_CGU_DIV_DSEL)   & BITM_CGU_DIV_DSEL)   /* DDR Clock Divisor Select [1-31]: (CLKIN x MSEL/DF+1)/DSEL = 100MHz(max)      */
#define OSEL   ((8UL  << BITP_CGU_DIV_OSEL)   & BITM_CGU_DIV_OSEL)   /* OUTCLK Divisor Select [1-127]: (CLKIN x MSEL/(DF+1))/OSEL = 50 MHz(max)      */
#define CSEL   ((2uL  << BITP_CGU_DIV_CSEL)   & BITM_CGU_DIV_CSEL)   /* Core Clock Divisor Select [1-31]: (CLKIN x MSEL/DF+1)/CSEL = 200MHz(max)     */
#define SYSSEL ((4uL  << BITP_CGU_DIV_SYSSEL) & BITM_CGU_DIV_SYSSEL) /* System Clock Divisor Select [1-31]: (CLKIN x MSEL/DF+1)/SYSSEL = 100MHz(max) */
#define S0SEL  ((1uL  << BITP_CGU_DIV_S0SEL)  & BITM_CGU_DIV_S0SEL)  /* SCLK0 Divisor Select [1-7]: SYSCLK/S0SEL = 100MHz(max)                       */
#define S1SEL  ((1uL  << BITP_CGU_DIV_S1SEL)  & BITM_CGU_DIV_S1SEL)  /* SCLK1 Divisor Select [1-7]: SYSLCK/S1SEL = 100MHz(max)                       */
#else
/* BF70[2-7], BF716 */
#define MSEL   ((32uL << BITP_CGU_CTL_MSEL)   & BITM_CGU_CTL_MSEL)   /* PLL Multiplier Select [1-127]: PLLCLK = ((CLKIN x MSEL/(DF+1)) = 800MHz(max) */
#define DSEL   ((4uL  << BITP_CGU_DIV_DSEL)   & BITM_CGU_DIV_DSEL)   /* DDR Clock Divisor Select [1-31]: (CLKIN x MSEL/(DF+1))/DSEL = 200MHz(max)    */
#define OSEL   ((16uL << BITP_CGU_DIV_OSEL)   & BITM_CGU_DIV_OSEL)   /* OUTCLK Divisor Select [1-127]: (CLKIN x MSEL/(DF+1))/OSEL = 50 MHz(max)     */
#define CSEL   ((2uL  << BITP_CGU_DIV_CSEL)   & BITM_CGU_DIV_CSEL)   /* Core Clock Divisor Select [1-31]: (CLKIN x MSEL/DF+1)/CSEL =  400MHz(max)    */
#define SYSSEL ((4uL  << BITP_CGU_DIV_SYSSEL) & BITM_CGU_DIV_SYSSEL) /* System Clock Divisor Select [1-31]: (CLKIN x MSEL/DF+1)/SYSSEL = 200MHz(max) */
#define S0SEL  ((2uL  << BITP_CGU_DIV_S0SEL)  & BITM_CGU_DIV_S0SEL)  /* SCLK0 Divisor Select [1-7]: SYSCLK/S0SEL = 100MHz(max)                       */
#define S1SEL  ((1uL  << BITP_CGU_DIV_S1SEL)  & BITM_CGU_DIV_S1SEL)  /* SCLK1 Divisor Select [1-7]: SYSLCK/S1SEL = 200MHz(max)                       */
#endif

#define UPDT   ((1UL  << BITP_CGU_DIV_UPDT)   & BITM_CGU_DIV_UPDT)

#endif /* CONFIG_CGU */

#endif /* INIT_CGU_H */
