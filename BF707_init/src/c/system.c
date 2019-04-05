/*
 * system.c
 *
 * ADSP-BF7xx system
 *
 * File Version 2.8.0.0
 *
 * Copyright (c) 2011-2017 Analog Devices, Inc. All Rights Reserved.
 */

/*****************************************************************************
 Include Files
******************************************************************************/
#include "system.h"


/*****************************************************************************
 Functions
******************************************************************************/

/****************************************************************************
 Name:          get_pllclk_hz
 Description:   get current PLL clock frequency in Hz
 Input:         uint32_t ulRegCgu0Ctl
 Return:        uint32_t PLLCLK [Hz]
*****************************************************************************/
uint32_t get_pllclk_hz(void)
{
    uint32_t ulMsel;
    uint32_t ulPllClkHz;

    cgu_config pllclk;

    pllclk.ulCGU_CTL = *pREG_CGU0_CTL;

    ulMsel = ( (pllclk.ulCGU_CTL & BITM_CGU_CTL_MSEL) >> BITP_CGU_CTL_MSEL );
    if ( ulMsel == 0uL ) {
       ulMsel = 128uL;
    }
    ulPllClkHz = ulMsel * (uint32_t) CLKIN_Hz;

    return ( ulPllClkHz >> (BITM_CGU_CTL_DF & pllclk.ulCGU_CTL) );
} /* get_pllclk_hz */


/****************************************************************************
 Name:          get_pllclk_ns
 Description:   get current PLL clock frequency in ns
 Input:         uint32_t ulRegCgu0Ctl
 Return:        uint32_t PLLCLK [ns]
*****************************************************************************/
uint32_t get_pllclk_ns(void)
{
    uint32_t ulMsel = 0uL;
    uint32_t ulPllClkNs = 0uL;
    cgu_config pllclk;

    pllclk.ulCGU_CTL = *pREG_CGU0_CTL;

    ulMsel = ( (pllclk.ulCGU_CTL & BITM_CGU_CTL_MSEL) >> BITP_CGU_CTL_MSEL );
    if ( ulMsel == 0uL ) {
       ulMsel = 128uL;
    }
    ulPllClkNs = (uint32_t) CLKIN_ns / ulMsel;

    return ( ulPllClkNs << (BITM_CGU_CTL_DF & pllclk.ulCGU_CTL) );
} /* get_pllclk_ns */


/****************************************************************************
 Name:          get_cclk_hz
 Description:   get current core clock frequency in Hz
 Input:         none
 Return:        uint32_t CCLK [Hz]
*****************************************************************************/
uint32_t get_cclk_hz(void)
{
    uint32_t ulCsel;
    cgu_config cclk;
    uint32_t ulCclk_hz;

    cclk.ulCGU_STAT = *pREG_CGU0_STAT;
    cclk.ulCGU_DIV =  *pREG_CGU0_DIV;
    if (cclk.ulCGU_STAT & BITM_CGU_STAT_PLLBP) {
       ulCclk_hz = (uint32_t)CLKIN_Hz;
    } else {
       ulCsel = ( (cclk.ulCGU_DIV & BITM_CGU_DIV_CSEL) >> BITP_CGU_DIV_CSEL );
       if (ulCsel == 0uL) {
          ulCclk_hz = 0uL;
       } else {
          ulCclk_hz = get_pllclk_hz() / ulCsel;
       }
    }
    return ulCclk_hz;
} /* get_cclk_hz */


/****************************************************************************
 Name:          get_cclk_ns
 Description:   get current core clock frequency in ns
 Input:         none
 Return:        uint32_t CCLK [ns]
*****************************************************************************/
uint32_t get_cclk_ns(void)
{
   uint32_t ulCsel;
   cgu_config cclk;
   uint32_t ulCclk_ns;

   cclk.ulCGU_STAT = *pREG_CGU0_STAT;
   cclk.ulCGU_DIV = *pREG_CGU0_DIV;

   if (cclk.ulCGU_STAT & BITM_CGU_STAT_PLLBP) {
      ulCclk_ns = (uint32_t) CLKIN_ns;
   } else {
      ulCsel = ( (cclk.ulCGU_DIV & BITM_CGU_DIV_CSEL) >> BITP_CGU_DIV_CSEL );
      if (ulCsel == 0uL) {
         ulCclk_ns = 0uL;
      } else {
         ulCclk_ns = get_pllclk_ns() * ulCsel ;
      }
   } return ulCclk_ns;
} /* get_cclk_ns */


/****************************************************************************
 Name:          get_sysclk_hz
 Description:   get current system clock frequency in Hz
 Input:         uint32_t ulRegCgu0Ctl, uint32_t ulRegCgu0Stat, uint32_t ulRegCgu0Div
 Return:        uint32_t SYSCLK [Hz]
*****************************************************************************/
uint32_t get_sysclk_hz(uint32_t ulRegCgu0Ctl, uint32_t ulRegCgu0Stat, uint32_t ulRegCgu0Div)
{
   uint32_t ulSysSel;
   cgu_config sysclk;
   uint32_t ulSysclk_hz;

   sysclk.ulCGU_CTL  = ulRegCgu0Ctl;
   sysclk.ulCGU_DIV  = ulRegCgu0Div;
   sysclk.ulCGU_STAT = ulRegCgu0Stat;

   if (sysclk.ulCGU_STAT & BITM_CGU_STAT_PLLBP) {
      ulSysclk_hz = (uint32_t)CLKIN_Hz;
   } else {
      ulSysSel = ( (sysclk.ulCGU_DIV & BITM_CGU_DIV_SYSSEL) >> BITP_CGU_DIV_SYSSEL );
      if (ulSysSel == 0uL) {
         ulSysclk_hz = 0uL;
      } else {
         ulSysclk_hz = ( (get_pllclk_hz()) / (ulSysSel) );
      }
   }
   return ulSysclk_hz;
} /* get_sysclk_hz */


/****************************************************************************
 Name:          get_s0clk_hz
 Description:   get current s0 clock frequency in Hz
 Input:         none
 Return:        uint32_t S0CLK [Hz]
*****************************************************************************/
uint32_t get_s0clk_hz(void)
{
   uint32_t ulS0sel;
   cgu_config s0clk;
   uint32_t ulS0clk_hz;

   s0clk.ulCGU_CTL = *pREG_CGU0_CTL;
   s0clk.ulCGU_STAT = *pREG_CGU0_STAT;
   s0clk.ulCGU_DIV = *pREG_CGU0_DIV;

   if (s0clk.ulCGU_STAT & BITM_CGU_STAT_PLLBP) {
      ulS0clk_hz = (uint32_t) CLKIN_Hz;
   } else {
      ulS0sel = ( (s0clk.ulCGU_DIV & BITM_CGU_DIV_S0SEL) >> BITP_CGU_DIV_S0SEL );
      if (ulS0sel == 0uL) {
         ulS0clk_hz = 0uL;
      } else {
         ulS0clk_hz = get_sysclk_hz(s0clk.ulCGU_CTL, s0clk.ulCGU_STAT, s0clk.ulCGU_DIV) / ulS0sel;
      }
   }
   return ulS0clk_hz;
} /* get_s0clk_hz */


/****************************************************************************
 Name:          get_s1clk_hz
 Description:   get current s1 clock frequency in Hz
 Input:         none
 Return:        uint32_t S1CLK [Hz]
*****************************************************************************/
uint32_t get_s1clk_hz(void)
{
   uint32_t ulS1sel;
   cgu_config s1clk;
   uint32_t ulS1clk_hz;

   s1clk.ulCGU_CTL = *pREG_CGU0_CTL;
   s1clk.ulCGU_STAT = *pREG_CGU0_STAT;
   s1clk.ulCGU_DIV = *pREG_CGU0_DIV;

   if (s1clk.ulCGU_STAT & BITM_CGU_STAT_PLLBP) {
      ulS1clk_hz = (uint32_t) CLKIN_Hz;
   } else {
      ulS1sel = ( (s1clk.ulCGU_DIV & BITM_CGU_DIV_S1SEL) >> BITP_CGU_DIV_S1SEL );
      if (ulS1sel == 0uL) {
         ulS1clk_hz =  0uL;
      } else {
         ulS1clk_hz = get_sysclk_hz(s1clk.ulCGU_CTL, s1clk.ulCGU_STAT, s1clk.ulCGU_DIV) / ulS1sel;
      }
   }
   return ulS1clk_hz;
} /* get_s1clk_hz */


/****************************************************************************
 Name:          get_ddrclk_hz
 Description:   get current DDR clock frequency in Hz
 Input:         none
 Return:        uint32_t DDRCLK [Hz]
*****************************************************************************/
uint32_t get_ddrclk_hz(void)
{
   uint32_t ulDsel;
   cgu_config ddrclk;
   uint32_t ulDdrclk_hz;

   ddrclk.ulCGU_STAT = *pREG_CGU0_STAT;
   ddrclk.ulCGU_DIV = *pREG_CGU0_DIV;

   if (ddrclk.ulCGU_STAT & BITM_CGU_STAT_PLLBP) {
      ulDdrclk_hz = (uint32_t) CLKIN_Hz;
   } else {
      ulDsel = ( (ddrclk.ulCGU_DIV & BITM_CGU_DIV_DSEL) >> BITP_CGU_DIV_DSEL );
      if (ulDsel == 0uL) {
         ulDdrclk_hz = 0uL;
      } else {
         ulDdrclk_hz = get_pllclk_hz() / ulDsel;
      }
   }
   return ulDdrclk_hz;
} /* get_ddrclk_hz */


/****************************************************************************
 Name:          get_ddrclk_ns
 Description:   get current DDR clock frequency in ns
 Input:         none
 Return:        uint32_t DDRCLK [ns]
*****************************************************************************/
uint32_t get_ddrclk_ns(void)
{
   uint32_t ulDsel;
   cgu_config ddrclk;
   uint32_t ulDdrclk_ns;

   ddrclk.ulCGU_STAT = *pREG_CGU0_STAT;
   ddrclk.ulCGU_DIV = *pREG_CGU0_DIV;

   if (ddrclk.ulCGU_STAT & BITM_CGU_STAT_PLLBP) {
      ulDdrclk_ns = (uint32_t) CLKIN_ns;
   } else {
      ulDsel = ( (ddrclk.ulCGU_DIV & BITM_CGU_DIV_DSEL) >> BITP_CGU_DIV_DSEL );
      if (ulDsel == 0uL) {
         ulDdrclk_ns = 0uL;
      } else {
         ulDdrclk_ns = get_pllclk_ns() * ulDsel;
      }
   }
   return ulDdrclk_ns;
} /* get_ddrclk_ns */


/****************************************************************************
 Name:          ErrorWithNoReturn
 Description:   Kernel Panic!!!
 Input:         none
 Return:        none
*****************************************************************************/
#pragma noreturn
void ErrorWithNoReturn(void)
{
    while (true) {
       asm volatile("EMUEXCPT;");
       idle();
    }
}

/****************************************************************************
 EOF
*****************************************************************************/
