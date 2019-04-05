BF7xx_init is the init code project that can be used for the ADSP-BF70x and
ADSP-BF71x family processors.

BF7xx_init and part specific copies of it for EZ-KIT support are included
in CCES in folder <install-dir>/Blackfin/ldr/init_code/BF7xx_init.

These init codes can:

  1. Disable instruction cache
  2. Configure CGU
  3. Configure DMC0 DDR2 memory (*)
  4. Auto baud calculate for UART Slave
  5. Change the SPI Baud

Each of these supported initcode features are guarded by macros that are
defined in the src/include/init_platform.h include file:

There are three build configurations for each BF7xx_init project that set the
support macros as follows:

  1. BF7xx_init
     CONFIG_CGU=1, CONFIG_MEMORY=1(*), CHANGE_SPI_BAUD=1, DISABLE_ICACHE=0

  2. BF7xx_init_icache_disable
     CONFIG_CGU=1, CONFIG_MEMORY=1(*), CHANGE_SPI_BAUD=1, DISABLE_ICACHE=1

  3. BF7xx_init_icache_disable_only
     CONFIG_CGU=0, CONFIG_MEMORY=0,    CHANGE_SPI_BAUD=0, DISABLE_ICACHE=1

(*) for parts that support DMC0 DDR2 only, i.e. not ADSP-BF70[0246] or
    ADSP-BF71x parts.

You will receive a link-time warning if you attempt to disable the instruction
cache when building an application for ADSP-BF7xx processors and using the CCES
supplied LDFs. This warning indicates that you require to use one of the
"icache_disable" configuration initcodes. The warning text is:

  [Warning pp0018] "..\system\startup_ldf\app.ldf":59 'A loader initialization
                   code to disable instruction cache during booting is
                   required. This message may be suppressed by defining the LDF
                   macro USING_ICACHE_DISABLE_INIT_CODE.'

This warning is emitted because these processors enable the instruction cache
during the boot process, for maximum boot performance, and leave it in the
enabled state. Any application that uses the L1 SRAM/ICache memory space in
SRAM mode will fail during booting when it comes to writing that section of
memory unless instruction cache is disabled in an initcode.

This issue might not be evident during application development, since the
emulator will disable instruction cache before loading the application, so the
conflict may remain undetected until application development reaches the
booting stage. The link-time warning against disabling instruction cache is
intended to highlight the conflict sooner.

The link-time warning can be disabled by defining the link-time macro
USING_ICACHE_DISABLE_INIT_CODE, either by:

  - Adding "-flags-link -MDUSING_ICACHE_DISABLE_INIT_CODE"  to the application
    build linker command-line switches.

  - Or by adding USING_ICACHE_DISABLE_INIT_CODE to
    Project > Properties
              > C/C++ Build
                > Tool Settings
                  > CrossCore Blackfin Linker
                    > Preprocessor
                      > Preprocessor Definitions

The BF7xx_init projects are used to build the following prebuilt CCES
supplied initcodes for ADSP-BF706, ADSP-BF707, ADSP-BF716 and ADSP-BF719:

  <install-dir>/Blackfin/ldr/BF706_init_icache_disable_v10.dxe
  <install-dir>/Blackfin/ldr/BF706_init_icache_disable_v11.dxe
  <install-dir>/Blackfin/ldr/BF706_init_only_icache_disable_v10.dxe
  <install-dir>/Blackfin/ldr/BF706_init_only_icache_disable_v11.dxe
  <install-dir>/Blackfin/ldr/BF706_init_v10.dxe
  <install-dir>/Blackfin/ldr/BF706_init_v11.dxe

  <install-dir>/Blackfin/ldr/BF707_init_icache_disable_v10.dxe
  <install-dir>/Blackfin/ldr/BF707_init_icache_disable_v11.dxe
  <install-dir>/Blackfin/ldr/BF707_init_only_icache_disable_v10.dxe
  <install-dir>/Blackfin/ldr/BF707_init_only_icache_disable_v11.dxe
  <install-dir>/Blackfin/ldr/BF707_init_v10.dxe
  <install-dir>/Blackfin/ldr/BF707_init_v11.dxe

  <install-dir>Blackfin/ldr/BF716_init.dxe
  <install-dir>Blackfin/ldr/BF716_init_icache_disable.dxe
  <install-dir>Blackfin/ldr/BF716_init_only_icache_disable.dxe

  <install-dir>Blackfin/ldr/BF719_init.dxe
  <install-dir>Blackfin/ldr/BF719_init_icache_disable.dxe
  <install-dir>Blackfin/ldr/BF719_init_only_icache_disable.dxe

Be aware that the project and supplied initcode DXE files make use of Utility
ROM defined functions and data when possible.

The CCES supplied BF7xx_init project will likely need the Processor Settings
to be updated to match your particular processor and silicon revision.

last updated
7th March 2017
for CCES 2.8.0
