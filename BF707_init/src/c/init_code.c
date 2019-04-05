/*
 * init_code.c
 *
 * Run initialization code to configure ADSP-BF7xx processors
 *
 * File Version 2.8.0.0
 *
 * Copyright (c) 2011-2017 Analog Devices, Inc. All Rights Reserved.
 */

#include "init_platform.h"
#if !DISABLE_ICACHE && !CONFIG_CGU && !CONFIG_MEMORY && !CHANGE_SPI_BAUD
  #error no configuration macros are set
#endif

#include "init_cgu.h"
#include "init_memory.h"
#include "uart.h"
#include <sys/platform.h>
#include <bfrom.h>
#include <builtins.h>
#include <stdbool.h>

uint32_t test_ddr();

#pragma retain_name /* retain resolved initcode entry */
void initcode(ADI_ROM_BOOT_CONFIG* pBootStruct);

#pragma retain_name /* retain resolved initcode entry */
void initcode(ADI_ROM_BOOT_CONFIG* pBootStruct)
{
#if DISABLE_ICACHE
   /* Set the L1 instruction cache/SRAM block as SRAM */
   ssync();
   *pREG_L1IM_ICTL &= ~(BITM_L1IM_ICTL_ENCPLB | BITM_L1IM_ICTL_CFG);
   ssync();
#endif /* DISABLE_ICACHE */

#if CONFIG_CGU
   {
      uint32_t udUART_BIT_RATE = 0uL;
      uint32_t udUART_CTL = 0uL;
      bool isBmodeThree = (*pREG_RCU0_STAT & BITM_RCU_STAT_BMODE) == (3uL << BITP_RCU_STAT_BMODE);

      /*
       * Mode 'Boot from UART host (slave mode)'
       * Save current BIT RATE value and Force transfer stop before changing CGU
       */

      if(isBmodeThree)
      {
         udUART_BIT_RATE = UartGetBitrate(0uL);
         udUART_CTL = UartRxFifoClear(0uL, udUART_BIT_RATE);
      }

#if WORKAROUND_DLLLOCK_ANOM_19000051
      /* Set bit 11 as per anomaly workaround description. */
      *pREG_DMC0_PHY_CTL0 |= 0x800uL;
#endif /* WORKAROUND_DLLLOCK_ANOM_19000051 */

      init_cgu();

#if WORKAROUND_DLLLOCK_ANOM_19000051
      /* Clear bit 11 as per anomaly workaround description. */
      *pREG_DMC0_PHY_CTL0 &= (~0x800uL);
#endif /* WORKAROUND_DLLLOCK_ANOM_19000051 */

      /*
       * Mode 'Boot from UART host (slave mode)'
       * Update UART Clock register according to new clock frequencies
       * and restore UART Control Register
       */

      if(isBmodeThree)
      {
         UartSetBitrate(0uL, udUART_BIT_RATE);
         *pREG_UART0_CTL = udUART_CTL;
      }
   }
#endif /* CONFIG_CGU */

#if CONFIG_MEMORY
   DMC_Init_DDR2();
#endif /* CONFIG_MEMORY */

   /*
    * BMODE 1: 'Boot from SPI memory (Master mode)'
    * modify SPIx_BAUD register to speed up booting from SPI memory
    */
#if CHANGE_SPI_BAUD
   if ( (*pREG_RCU0_STAT & BITM_RCU_STAT_BMODE) == (1uL << BITP_RCU_STAT_BMODE)  )
   {
      *pREG_SPI2_CLK = (uint32_t) SPI_BAUD_VAL;
   }
#endif /* CHANGE_SPI_BAUD */

   //test_ddr();
} /* initcode */



uint32_t test_ddr()
{
	static uint8_t test_done = 1;
	uint32_t temp_value ;

	if(test_done == 1)
	{
		uint32_t * ddrBaseAddr;

		ddrBaseAddr = 0x80000000;
		for(int i=0; i<1024; i++)
		{
			*(ddrBaseAddr) = 2*i;
			ddrBaseAddr += 1;
		}
		//Check for REad error
		ddrBaseAddr = 0x80000000;
		for(int i=0; i<1024; i++)
		{
			temp_value = ddrBaseAddr;
			if(*(ddrBaseAddr) != 2*i)
			{
				break;
			}
			ddrBaseAddr += 1;
		}

		test_done = 0;
	}
	return temp_value;
}
