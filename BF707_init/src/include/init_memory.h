/*
 * init_memory.h
 *
 * Initialize DMC
 *
 * File Version 2.2.0.0
 *
 * Copyright (c) 2011-2015 Analog Devices, Inc. All Rights Reserved.
 */

#ifndef INIT_MEMORY_H
#define INIT_MEMORY_H

/*************************************************************************************************************************
                           Include Files
*************************************************************************************************************************/
#include "init_platform.h"

#if CONFIG_MEMORY

#include "system.h"

#include <sys/platform.h>
#include <builtins.h>
#include <math.h>
#include <stdint.h>

/*************************************************************************************************************************
                           Structures
**************************************************************************************************************************/

/* Structure to store the device parameters for DDR2 memories */

typedef struct _DMC_PARAM_LIST
{
   float fDDR_Speed_MHZ;
   float fDMC_PARAM_DDR_SIZE_MB;
   float fDMC_PARAM_TRCD_NS;
   float fDMC_PARAM_TWTR_NS;
   float fDMC_PARAM_TRP_NS;
   float fDMC_PARAM_TRAS_NS;
   float fDMC_PARAM_TRC_NS;
   float fDMC_PARAM_TMRD_TCK;
   float fDMC_PARAM_TREF_NS;
   float fDMC_PARAM_TRFC_NS;
   float fDMC_PARAM_TRRD_NS;
   float fDMC_PARAM_TFAW_NS;
   float fDMC_PARAM_TRTP_NS;
   float fDMC_PARAM_TWR_NS;
   float fDMC_PARAM_TXP_TCK;
   float fDMC_PARAM_TCKE_TCK;
   float fDMC_PARAM_CL_TCK;
   float fDMC_PARAM_WRRECOV_TCK;
   float fDMC_PARAM_AL_TCK;
   float fDMC_PARAM_BL;
   float fDMC_PARAM_RDTOWR_TCK;
   float fDMC_PARAM_DLLCALRDCNT_TCK;
   float fDMC_PARAM_DATACYC_TCK;
} DMC_PARAM_LIST;

/*************************************************************************************************************************
                           MACROS
**************************************************************************************************************************/
/* From memory device data sheet */
#define PARAM_MEMORY_SIZE       2048            /* DDR size in Mega Bits  */
#define PARAM_CL                3               /* CL in DCLK cycles      */
#define PARAM_AL_TCK            0               /* AL in DCLK cycles      */
#define PARAM_BL                4               /* Burst length (4/8)     */
#define PARAM_TRCD_NS           12.5            /* tRCD in ns             */
#define PARAM_TWTR_NS           7.5             /* tWTR in ns             */
#define PARAM_TRP_NS            12.5            /* tRP in ns              */
//#define PARAM_TRP_NS            15            /* tRP in ns              */
#define PARAM_TRAS_NS           40              /* tRAS in ns             */
#define PARAM_TRC_NS            55              /* tRC in ns              */
#define PARAM_TMRD_TCK          2               /* tMRD in DCLK cycles    */
#define PARAM_TREF_NS           7800            /* tREF in ns             */
//#define PARAM_TREF_NS           3900            /* tREF in ns             */
#define PARAM_TRFC_NS           195             /* tRFC in ns             */
#define PARAM_TRRD_NS           10              /* tRRD in ns             */
#define PARAM_TFAW_NS           45              /* tFAW in ns             */
#define PARAM_TRTP_NS           7.5             /* tRTP in ns             */
//#define PARAM_TRTP_NS           10             /* tRTP in ns             */
#define PARAM_TWR_NS            15              /* tWR in ns              */
#define PARAM_WRRECOV_TCK       3               /* tWR in DCLK cycles     */
#define PARAM_TXP_TCK           2               /* tXP in DCLK cycles     */
#define PARAM_TCKE_TCK          3               /* tCKE in DCLK cycles    */

/* DO NOT CHANGE PADCTL0 VALUES */
#define DMC0_PADCTL0_VALUE   (BITM_DMC_CAL_PADCTL0_PUCALEN | BITM_DMC_CAL_PADCTL0_PDCALEN | BITM_DMC_CAL_PADCTL0_RTTCALEN)
#define DMC0_PADCTL2_VALUE   	(0x0078283CuL)
//#define DMC0_PADCTL2_VALUE   	(0x0000000CuL)



/*************************************************************************************************************************
                           Enum definitions
**************************************************************************************************************************/
/* Use these definitions instead of numbers while passing as arguments */
enum DMC_MEM_TYPE
{
   MEMTYPE_DDR2
};

/*************************************************************************************************************************
                           Function Declarations
**************************************************************************************************************************/
static void DMC_Configure_Controller_Registers(uint32_t uiDMC_CTL, uint32_t uiDMC_CFG, uint32_t uiDMC_TR0, uint32_t uiDMC_TR1, uint32_t uiDMC_TR2, uint32_t uiDMC_MR, uint32_t uiDMC_MR1, uint32_t uiDMC_MR2, uint32_t uiDMC_DLLCTL);
static void DMC_Configure_PADCTL_Registers(enum DMC_MEM_TYPE memtype, uint32_t uiDMC_PADCTL0, uint32_t uiDMC_PADCTL2);
static void DMC_Init(enum DMC_MEM_TYPE memtype, DMC_PARAM_LIST *pDMC_Parameter_List);
void DMC_Init_DDR2(void);

#endif /* CONFIG_MEMORY */

#endif /* INIT_MEMORY_H */
