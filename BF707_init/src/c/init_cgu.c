/*
 * init_cgu.c
 *
 * Initialize CGU programmatically
 *
 * File Version 2.2.0.1
 *
 * Copyright (c) 2011-2015 Analog Devices, Inc. All Rights Reserved.
 */

#include "init_cgu.h"

#if CONFIG_CGU

void init_cgu (void) {

   /* Check if the new MSEL is the same as current MSEL value. If it is, just
    * update the CGU_DIV register.
    */

   uint32_t      ctl = (MSEL | DF);
   uint32_t curr_ctl = (*pREG_CGU0_CTL & (BITM_CGU_CTL_MSEL|BITM_CGU_CTL_DF));

   if (curr_ctl == ctl) {

      /* Verify that the CGU_STAT.CLKSALGN bit=0 (clocks aligned). */
      while((*pREG_CGU0_STAT & BITM_CGU_STAT_CLKSALGN) != 0uL) { }

      /* Write the new CGU_DIV field values with the CGU_DIV.UPDT bit=1. */
      *pREG_CGU0_DIV = (UPDT | OSEL | DSEL | S1SEL | S0SEL | SYSSEL | CSEL) ;

      /* Poll the CGU_STAT.CLKSALGN bit till it is 1 when clocks are aligned
       * indicating the end of the update sequence.
       */
      while((*pREG_CGU0_STAT & BITM_CGU_STAT_CLKSALGN) != 0uL) { }

   } else {

      /* Read CGU_STAT register to verify that:
       *  - The CGU_STAT.PLLEN bit=1 (PLL enabled).
       *  - The CGU_STAT.PLOCK bit=1 (PLL is not locking).
       *  - The CGU_STAT.CLKSALGN bit=0 (clocks aligned).
       */
      uint32_t cgu_stat;
      do {
         cgu_stat = *pREG_CGU0_STAT;
      } while (   ((cgu_stat & BITM_CGU_STAT_PLLEN) == 0uL)
               || ((cgu_stat & BITM_CGU_STAT_PLOCK) == 0uL)
               || ((cgu_stat & BITM_CGU_STAT_CLKSALGN) != 0uL));

      /* Write the new values to the CGU_DIV register’s clock divisor select
       * fields with the CGU_DIV.UPDT bit=0.
       */
      *pREG_CGU0_DIV =  (OSEL | DSEL | S1SEL | S0SEL | SYSSEL | CSEL);

      /* Write the new values to the CGU_CTL.DF and CGU_CTL.MSEL fields. */
      *pREG_CGU0_CTL = ctl;

      /* Poll CGU_STAT bits to determine the end of the update sequence
       * when the PLL is locked and the clocks are aligned. This is done 
       * by checking for when the CGU_STAT.PLOCK bit=1, CGU_STAT.PLLBP bit=0
       * and CGU_STAT.CLKSALGN bit=0.
       */
      do {
         cgu_stat = *pREG_CGU0_STAT;
      } while (   ((cgu_stat & BITM_CGU_STAT_PLOCK) == 0uL)
               || ((cgu_stat & BITM_CGU_STAT_PLLBP) != 0uL)
               || ((cgu_stat & BITM_CGU_STAT_CLKSALGN) != 0uL));

   }

  *pREG_CGU0_CLKOUTSEL = ENUM_CGU_CLKOUTSEL_CCLKDIV16;

} /* init_cgu */

#else
/* suppress translation unit must contain at least one declaration warning */
#pragma diag(suppress:96)
#endif
