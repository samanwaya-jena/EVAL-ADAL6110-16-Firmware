/*
 **
 ** Source file generated on May 7, 2019 at 12:50:23.	
 **
 ** Copyright (C) 2011-2019 Analog Devices Inc., All Rights Reserved.
 **
 ** This file is generated automatically based upon the options selected in 
 ** the Pin Multiplexing configuration editor. Changes to the Pin Multiplexing
 ** configuration should be made by changing the appropriate options rather
 ** than editing this file.
 **
 ** Selected Peripherals
 ** --------------------
 ** SPI2 (CLK, MISO, MOSI, RDY, SEL1, SEL2, SEL3, D2, D3)
 **
 ** GPIO (unavailable)
 ** ------------------
 ** PA04, PB08, PB09, PB10, PB11, PB12, PB13, PB14, PB15
 */

#include <sys/platform.h>
#include <stdint.h>

#define SPI2_CLK_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<20))
#define SPI2_MISO_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<22))
#define SPI2_MOSI_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<24))
#define SPI2_RDY_PORTA_MUX  ((uint16_t) ((uint16_t) 2<<8))
#define SPI2_SEL1_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<30))
#define SPI2_SEL2_PORTB_MUX  ((uint32_t) ((uint32_t) 2<<16))
#define SPI2_SEL3_PORTB_MUX  ((uint32_t) ((uint32_t) 2<<18))
#define SPI2_D2_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<26))
#define SPI2_D3_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<28))

#define SPI2_CLK_PORTB_FER  ((uint32_t) ((uint32_t) 1<<10))
#define SPI2_MISO_PORTB_FER  ((uint32_t) ((uint32_t) 1<<11))
#define SPI2_MOSI_PORTB_FER  ((uint32_t) ((uint32_t) 1<<12))
#define SPI2_RDY_PORTA_FER  ((uint16_t) ((uint16_t) 1<<4))
#define SPI2_SEL1_PORTB_FER  ((uint32_t) ((uint32_t) 1<<15))
#define SPI2_SEL2_PORTB_FER  ((uint32_t) ((uint32_t) 1<<8))
#define SPI2_SEL3_PORTB_FER  ((uint32_t) ((uint32_t) 1<<9))
#define SPI2_D2_PORTB_FER  ((uint32_t) ((uint32_t) 1<<13))
#define SPI2_D3_PORTB_FER  ((uint32_t) ((uint32_t) 1<<14))

int32_t adi_initpinmux(void);

/*
 * Initialize the Port Control MUX and FER Registers
 */
int32_t adi_initpinmux(void) {
    /* PORTx_MUX registers */
    *pREG_PORTA_MUX = SPI2_RDY_PORTA_MUX;
    *pREG_PORTB_MUX = SPI2_CLK_PORTB_MUX | SPI2_MISO_PORTB_MUX
     | SPI2_MOSI_PORTB_MUX | SPI2_SEL1_PORTB_MUX | SPI2_SEL2_PORTB_MUX
     | SPI2_SEL3_PORTB_MUX | SPI2_D2_PORTB_MUX | SPI2_D3_PORTB_MUX;

    /* PORTx_FER registers */
    *pREG_PORTA_FER = SPI2_RDY_PORTA_FER;
    *pREG_PORTB_FER = SPI2_CLK_PORTB_FER | SPI2_MISO_PORTB_FER
     | SPI2_MOSI_PORTB_FER | SPI2_SEL1_PORTB_FER | SPI2_SEL2_PORTB_FER
     | SPI2_SEL3_PORTB_FER | SPI2_D2_PORTB_FER | SPI2_D3_PORTB_FER;
    return 0;
}

