/*
 * init_memory.c
 *
 * Initialize DMC
 *
 * File Version 2.2.0.1
 *
 * Copyright (c) 2011-2015 Analog Devices, Inc. All Rights Reserved.
 */

/*************************************************************************************************************************
                            Include Files
*************************************************************************************************************************/
#include "init_memory.h"

#if CONFIG_MEMORY

/*************************************************************************************************************************
                           Function Definitions
**************************************************************************************************************************/
void DMC_Init_DDR2(void)
{
   /* To store dynamic DMC parameters */
   static DMC_PARAM_LIST DMC_Param_List = {
            0.0f,                     /* fDDR_Speed_MHZ - set below */
            (float)PARAM_MEMORY_SIZE, /* fDMC_PARAM_DDR_SIZE_MB */
            (float)PARAM_TRCD_NS,     /* fDMC_PARAM_TRCD_NS */
            (float)PARAM_TWTR_NS,     /* fDMC_PARAM_TWTR_NS */
            (float)PARAM_TRP_NS,      /* fDMC_PARAM_TRP_NS */
            (float)PARAM_TRAS_NS,     /* fDMC_PARAM_TRAS_NS */
            (float)PARAM_TRC_NS,      /* fDMC_PARAM_TRC_NS */
            (float)PARAM_TMRD_TCK,    /* fDMC_PARAM_TMRD_TCK */
            (float)PARAM_TREF_NS,     /* fDMC_PARAM_TREF_NS */
            (float)PARAM_TRFC_NS,     /* fDMC_PARAM_TRFC_NS */
            (float)PARAM_TRRD_NS,     /* fDMC_PARAM_TRRD_NS */
            (float)PARAM_TFAW_NS,     /* fDMC_PARAM_TFAW_NS */
            (float)PARAM_TRTP_NS,     /* fDMC_PARAM_TRTP_NS */
            (float)PARAM_TWR_NS,      /* fDMC_PARAM_TWR_NS */
            (float)PARAM_TXP_TCK,     /* fDMC_PARAM_TXP_TCK */
            (float)PARAM_TCKE_TCK,    /* fDMC_PARAM_TCKE_TCK */
            (float)PARAM_CL,          /* fDMC_PARAM_CL_TCK */
            (float)PARAM_WRRECOV_TCK, /* fDMC_PARAM_WRRECOV_TCK */
            (float)PARAM_AL_TCK,      /* fDMC_PARAM_AL_TCK */
            (float)PARAM_BL,          /* fDMC_PARAM_BL */
            2.0f,                     /* fDMC_PARAM_RDTOWR_TCK */
            75.0f,                    /* fDMC_PARAM_DLLCALRDCNT_TCK */
            5.0f                      /* fDMC_PARAM_DATACYC_TCK */
   };
   DMC_Param_List.fDDR_Speed_MHZ = ((float)get_ddrclk_hz()/1000000.0f);
   DMC_Init(MEMTYPE_DDR2, &DMC_Param_List);
} /* DMC_Init_DDR2 */

/**************************************************************************************************************************
  Function name : DMC_Init
  Description   : This function initializes the DMC based on given memory device dependent parameters
  Arguments     : Parameter                 | Description                      | Valid values
                  uiMemtype                 | Type of memory                   | MEMTYPE_DDR2
                  pDMC_Parameter_List       | DMC Parameter List               | Refer to DMC_PARAM_LIST struct definition

  Return value  : None.
**************************************************************************************************************************/
static
void DMC_Init(enum DMC_MEM_TYPE memtype, DMC_PARAM_LIST *pDMC_Parameter_List)
{
   /* Variables to store the DMC parameters in terms of DCLK cycles */
   uint32_t uiDMC_PARAM_DDR_SIZE_MB, uiDMC_PARAM_TRCD_TCK, uiDMC_PARAM_TWTR_TCK, uiDMC_PARAM_TRP_TCK;
   uint32_t uiDMC_PARAM_TRAS_TCK, uiDMC_PARAM_TRC_TCK, uiDMC_PARAM_TMRD_TCK, uiDMC_PARAM_TREF_TCK;
   uint32_t uiDMC_PARAM_TRFC_TCK, uiDMC_PARAM_TRRD_TCK, uiDMC_PARAM_TFAW_TCK, uiDMC_PARAM_TRTP_TCK;
   uint32_t uiDMC_PARAM_TWR_TCK, uiDMC_PARAM_TXP_TCK, uiDMC_PARAM_TCKE_TCK, uiDMC_PARAM_CL_TCK;
   uint32_t uiDMC_PARAM_WRRECOV_TCK, uiDMC_PARAM_AL_TCK, uiDMC_PARAM_BL, uiDMC_PARAM_RDTOWR_TCK;
   uint32_t uiDMC_PARAM_DLLCALRDCNT_TCK, uiDMC_PARAM_DATACYC_TCK, uiDMC_PARAM_CL0_TCK, uiDMC_PARAM_CL123_TCK;
   uint32_t uiDMC_PARAM_WL_TCK;

   uint32_t uiDMC_CFG_VALUE, uiDMC_TR0_VALUE, uiDMC_TR1_VALUE, uiDMC_TR2_VALUE;
   uint32_t uiDMC_MR_VALUE, uiDMC_EMR1_VALUE, uiDMC_EMR2_VALUE, uiDMC_CTL_VALUE;
   uint32_t uiDMC_DLLCTL_VALUE;

   /* PAD calibration */
   DMC_Configure_PADCTL_Registers(memtype, DMC0_PADCTL0_VALUE, DMC0_PADCTL2_VALUE);

   /* Convert all the DDR parameters in terms of DCLK period */
   uiDMC_PARAM_DDR_SIZE_MB=(uint32_t)(logf(pDMC_Parameter_List->fDMC_PARAM_DDR_SIZE_MB/64.0f)/logf(2.0f));
   uiDMC_PARAM_TRCD_TCK=(uint32_t)ceilf((pDMC_Parameter_List->fDMC_PARAM_TRCD_NS*pDMC_Parameter_List->fDDR_Speed_MHZ)/1000.0f);
   uiDMC_PARAM_TWTR_TCK=(uint32_t)ceilf((pDMC_Parameter_List->fDMC_PARAM_TWTR_NS*pDMC_Parameter_List->fDDR_Speed_MHZ)/1000.0f);
   uiDMC_PARAM_TRP_TCK=(uint32_t)ceilf((pDMC_Parameter_List->fDMC_PARAM_TRP_NS*pDMC_Parameter_List->fDDR_Speed_MHZ)/1000.0f);
   uiDMC_PARAM_TRAS_TCK=(uint32_t)ceilf((pDMC_Parameter_List->fDMC_PARAM_TRAS_NS*pDMC_Parameter_List->fDDR_Speed_MHZ)/1000.0f);
   uiDMC_PARAM_TRC_TCK=(uint32_t)ceilf((pDMC_Parameter_List->fDMC_PARAM_TRC_NS*pDMC_Parameter_List->fDDR_Speed_MHZ)/1000.0f);
   uiDMC_PARAM_TMRD_TCK=(uint32_t)pDMC_Parameter_List->fDMC_PARAM_TMRD_TCK;
   uiDMC_PARAM_TREF_TCK=(uint32_t)ceilf((pDMC_Parameter_List->fDMC_PARAM_TREF_NS*pDMC_Parameter_List->fDDR_Speed_MHZ)/1000.0f);
   uiDMC_PARAM_TRFC_TCK=(uint32_t)ceilf((pDMC_Parameter_List->fDMC_PARAM_TRFC_NS*pDMC_Parameter_List->fDDR_Speed_MHZ)/1000.0f);
   uiDMC_PARAM_TRRD_TCK=(uint32_t)ceilf((pDMC_Parameter_List->fDMC_PARAM_TRRD_NS*pDMC_Parameter_List->fDDR_Speed_MHZ)/1000.0f);
   uiDMC_PARAM_TFAW_TCK=(uint32_t)ceilf((pDMC_Parameter_List->fDMC_PARAM_TFAW_NS*pDMC_Parameter_List->fDDR_Speed_MHZ)/1000.0f);
   uiDMC_PARAM_TRTP_TCK=(uint32_t)ceilf((pDMC_Parameter_List->fDMC_PARAM_TRTP_NS*pDMC_Parameter_List->fDDR_Speed_MHZ)/1000.0f);
   uiDMC_PARAM_TWR_TCK=(uint32_t)ceilf((pDMC_Parameter_List->fDMC_PARAM_TWR_NS*pDMC_Parameter_List->fDDR_Speed_MHZ)/1000.0f);
   uiDMC_PARAM_TXP_TCK=(uint32_t)pDMC_Parameter_List->fDMC_PARAM_TXP_TCK;
   uiDMC_PARAM_TCKE_TCK=(uint32_t)pDMC_Parameter_List->fDMC_PARAM_TCKE_TCK;
   uiDMC_PARAM_CL_TCK=(uint32_t)pDMC_Parameter_List->fDMC_PARAM_CL_TCK;
   uiDMC_PARAM_WRRECOV_TCK=(uint32_t)pDMC_Parameter_List->fDMC_PARAM_WRRECOV_TCK-1uL;
   uiDMC_PARAM_AL_TCK=(uint32_t)pDMC_Parameter_List->fDMC_PARAM_AL_TCK;
   uiDMC_PARAM_BL=(uint32_t)pDMC_Parameter_List->fDMC_PARAM_BL;
   uiDMC_PARAM_RDTOWR_TCK=(uint32_t)pDMC_Parameter_List->fDMC_PARAM_RDTOWR_TCK;
   uiDMC_PARAM_DLLCALRDCNT_TCK=(uint32_t)pDMC_Parameter_List->fDMC_PARAM_DLLCALRDCNT_TCK;
   uiDMC_PARAM_DATACYC_TCK=(uint32_t)pDMC_Parameter_List->fDMC_PARAM_DATACYC_TCK;

   /* Calculate various register values here */
   uiDMC_CFG_VALUE = (  ENUM_DMC_CFG_IFWID16
                      | ENUM_DMC_CFG_SDRWID16
                      | (uiDMC_PARAM_DDR_SIZE_MB<<BITP_DMC_CFG_SDRSIZE)
                      | ENUM_DMC_CFG_EXTBANK1);

   uiDMC_TR0_VALUE = (  (uiDMC_PARAM_TRCD_TCK<<BITP_DMC_TR0_TRCD)
                      | (uiDMC_PARAM_TWTR_TCK<<BITP_DMC_TR0_TWTR)
                      | (uiDMC_PARAM_TRP_TCK<<BITP_DMC_TR0_TRP)
                      | (uiDMC_PARAM_TRAS_TCK<<BITP_DMC_TR0_TRAS)
                      | (uiDMC_PARAM_TRC_TCK<<BITP_DMC_TR0_TRC)
                      | (uiDMC_PARAM_TMRD_TCK<<BITP_DMC_TR0_TMRD));

   uiDMC_TR1_VALUE = (  (uiDMC_PARAM_TREF_TCK<<BITP_DMC_TR1_TREF)
                      | (uiDMC_PARAM_TRFC_TCK<<BITP_DMC_TR1_TRFC)
                      | (uiDMC_PARAM_TRRD_TCK<<BITP_DMC_TR1_TRRD));

   uiDMC_TR2_VALUE = (  (uiDMC_PARAM_TFAW_TCK<<BITP_DMC_TR2_TFAW)
                      | (uiDMC_PARAM_TRTP_TCK<<BITP_DMC_TR2_TRTP)
                      | (uiDMC_PARAM_TWR_TCK<<BITP_DMC_TR2_TWR)
                      | (uiDMC_PARAM_TXP_TCK<<BITP_DMC_TR2_TXP)
                      | (uiDMC_PARAM_TCKE_TCK<<BITP_DMC_TR2_TCKE));

   if (memtype==MEMTYPE_DDR2)
   {
      uiDMC_MR_VALUE   = (  (uiDMC_PARAM_BL/4uL+1uL)<<BITP_DMC_MR_BLEN)
                          | (uiDMC_PARAM_CL_TCK<<BITP_DMC_MR_CL)
                          | (uiDMC_PARAM_WRRECOV_TCK<<BITP_DMC_MR_WRRECOV);

      uiDMC_EMR1_VALUE = (uiDMC_PARAM_AL_TCK<<BITP_DMC_EMR1_AL);
      uiDMC_EMR2_VALUE = 0uL;
      uiDMC_CTL_VALUE  = (  (uiDMC_PARAM_RDTOWR_TCK<<BITP_DMC_CTL_RDTOWR)
                          | BITM_DMC_CTL_DLLCAL
                          | BITM_DMC_CTL_INIT);
   }

   uiDMC_DLLCTL_VALUE =   (uiDMC_PARAM_DATACYC_TCK<<BITP_DMC_DLLCTL_DATACYC)
                        | (uiDMC_PARAM_DLLCALRDCNT_TCK<<BITP_DMC_DLLCTL_DLLCALRDCNT);

   /* Now write the DMC registers and start the DMC Initialization */
   DMC_Configure_Controller_Registers(uiDMC_CTL_VALUE, uiDMC_CFG_VALUE, uiDMC_TR0_VALUE,
                                      uiDMC_TR1_VALUE, uiDMC_TR2_VALUE, uiDMC_MR_VALUE,
                                      uiDMC_EMR1_VALUE, uiDMC_EMR2_VALUE, uiDMC_DLLCTL_VALUE);
} /* DMC_Init */

/**************************************************************************************************************************
   Function name : DMC_Configure_Controller_Registers
   Description   : This function configures the basic DMC controller registers and starts the DMC initialization sequences
   Arguments     : Parameter     | Description                                         | Valid values
                   uiDMC_CTL     | DMC control register                                | uint32_t 32 bit
                   uiDMC_CFG     | DMC configuration register                          | uint32_t 32 bit
                   uiDMC_TR0     | DMC timing control register 0                       | uint32_t 32 bit
                   uiDMC_TR1     | DMC timing control register 1                       | uint32_t 32 bit
                   uiDMC_TR2     | DMC timing control register 2                       | uint32_t 32 bit
                   uiDMC_MR      | DMC mode register                                   | uint32_t 32 bit
                   uiDMC_MR1     | DMC (extended -DDR2/LPDDR) mode control register 1  | uint32_t 32 bit
                   uiDMC_MR2     | DMC control register                                | uint32_t 32 bit
                   uiDMC_DLLCTL  | DMC control register                                | uint32_t 32 bit
   Return value   :  None.
**************************************************************************************************************************/
static
void DMC_Configure_Controller_Registers(uint32_t uiDMC_CTL, uint32_t uiDMC_CFG, uint32_t uiDMC_TR0,
                                        uint32_t uiDMC_TR1, uint32_t uiDMC_TR2, uint32_t uiDMC_MR,
                                        uint32_t uiDMC_MR1, uint32_t uiDMC_MR2, uint32_t uiDMC_DLLCTL)
{
   /* Configure the DMC registers */
   *pREG_DMC0_CFG = uiDMC_CFG;
   *pREG_DMC0_TR0 = uiDMC_TR0;
   *pREG_DMC0_TR1 = uiDMC_TR1;
   *pREG_DMC0_TR2 = uiDMC_TR2;
   *pREG_DMC0_MR  = uiDMC_MR;
   *pREG_DMC0_EMR1 = uiDMC_MR1;
   *pREG_DMC0_EMR2 = uiDMC_MR2;

   /* Configure the DMC_CTL register at the end including the INIT bit */
   *pREG_DMC0_CTL   = uiDMC_CTL;

   /* Wait till INITDONE is set */
   while((*pREG_DMC0_STAT&BITM_DMC_STAT_INITDONE)==0uL) { }

   /* Program the DLLCTL register now */
   *pREG_DMC0_DLLCTL = uiDMC_DLLCTL;
} /* DMC_Configure_Controller_Registers */

/**************************************************************************************************************************
  Function name   : DMC_Configure_PADCTL_Registers
  Description   : This function configures the basic DMC PAD control registers
  Arguments     : Parameter      | Description                    | Valid values
                  uiMemtype      | Memory type                    | MEMTYPE_DDR2
                  uiDMC_PADCTL0  | PAD control register 0         | uint32_t 32 bit
                  uiDMC_PADCTL2  | PAD control register 2         | uint32_t 32 bit
  Return value  : None.
**************************************************************************************************************************/

static
void DMC_Configure_PADCTL_Registers(enum DMC_MEM_TYPE memtype, uint32_t uiDMC_PADCTL0, uint32_t uiDMC_PADCTL2)
{
   int i;

   float fcclk = (float) get_cclk_hz();
   float fdclk = (float) get_ddrclk_hz();

   /* Calculate fCCLK to fDCLK ratio rounded up to next integral value for DCLK delay loops. */
   int32_t fcclk_to_fdclk_ratio = (int32_t)ceil(fcclk/fdclk);

   /* Select the memory type */
   if (memtype==MEMTYPE_DDR2)
   {
      *pREG_DMC0_PHY_CTL4 = ENUM_DMC_PHY_CTL4_DDR2;
      *pREG_DMC0_PHY_CTL3 = 0x0A0000C0uL;

      /* Program the PAD RTT and driver impedance values required here */
      *pREG_DMC0_CAL_PADCTL0 = DMC0_PADCTL0_VALUE;
      *pREG_DMC0_CAL_PADCTL2 = DMC0_PADCTL2_VALUE;

      /* Start calibration */
      *pREG_DMC0_CAL_PADCTL0 |= BITM_DMC_CAL_PADCTL0_CALSTRT;

      /* Wait for PAD calibration to complete - 300 DCLK cycle. */
      for (i=0; i<(300*fcclk_to_fdclk_ratio); i++)
      {
         NOP();
      }
   }

   /* Wait for DLL lock. Wait for at least 4500 DCLK cycles. */
   for (i=0; i<(4500*fcclk_to_fdclk_ratio); i++)
   {
      NOP();
   }
} /* DMC_Configure_PADCTL_Registers */

#else
/* suppress translation unit must contain at least one declaration warning */
#pragma diag(suppress:96)
#endif /* CONFIG_MEMORY */
