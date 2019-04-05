/*
 *   init_platform.h
 *
 *   Configures the ADSP-BF7xx initcode DXE via macro defines. The various
 *   configuration macros are defined to default values below. Alternative
 *   configurations can be achieved by defining the macros as required in the
 *   project options or by modifying the definitions in this file.
 *
 *   For further information, see readme.txt.
 *
 *   File Version 2.8.0.1
 *
 *   Copyright (c) 2011-2018 Analog Devices, Inc. All Rights Reserved.
 */

#ifndef INIT_PLATFORM_H
#define INIT_PLATFORM_H

#if !defined(__ADSPBF7xx__)
  #error "Expected target processor to be a ADSP-BF7xx family part"
#endif

#define CLKIN_Hz              25000000  /* CLKIN [Hz]   */
#define CLKIN_ns (1000000000/CLKIN_Hz)  /* CLKIN [ns]   */

/*
 *  CGU initialization is controlled by the definition of macro CONFIG_CGU.
 */

#if !defined(CONFIG_CGU)
  #define CONFIG_CGU 1 /* CGU initialization is done by default */
#endif

/*
 *  DDR2 memory initialization is controlled by the definition of macro CONFIG_MEMORY.
 */

#if !defined(CONFIG_MEMORY)
  #if defined(__ADSPBF700__) || defined(__ADSPBF702__) || \
      defined(__ADSPBF704__) || defined(__ADSPBF706__) || \
      defined(__ADSPBF71x__)
    #define CONFIG_MEMORY 0 /* DDR2 memory isn't supported */
  #else
    #define CONFIG_MEMORY 1 /* DDR2 memory initialization is done by default */
  #endif
#endif

/*
 * For Boot from SPI memory, this definition will modify the SPI_BAUD
 * register value and can increase or decrease the boot speed.
 *
 * To activate, define CHANGE_SPI_BAUD to 1.
 * Boot ROM default: 0x1F
 */

#if !defined(CHANGE_SPI_BAUD)
  #define CHANGE_SPI_BAUD 1 /* done by default */
#endif

#if CHANGE_SPI_BAUD && !defined(SPI_BAUD_VAL)
  #define SPI_BAUD_VAL 0x1 /* SPI Clock max is 50MHz. */
#endif

/*
 * Instruction Cache is enabled by the Boot ROM. If your application doesn't
 * make use of instruction cache, then it must be disabled before loading by
 * the initcode.
 *
 * To configure an initcode to disable instruction cache define DISABLE_ICACHE to 1.
 */

#if !defined(DISABLE_ICACHE)
  #define DISABLE_ICACHE 0 /* instruction cache used by default */
#endif

/* Silicon anomaly workaround configuration for DMC PHY DLL locking
 * issue 19-00-0051 "DLL in the DMC PHY may not lock to the new DCLK frequency".
 */

#if !defined(WORKAROUND_DLLLOCK_ANOM_19000051) && CONFIG_MEMORY
  #define WORKAROUND_DLLLOCK_ANOM_19000051 1 /* workaround enabled by default */
#endif

#endif /* INIT_PLATFORM_H */
