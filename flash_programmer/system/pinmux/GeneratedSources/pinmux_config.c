/*
 **
 ** Source file generated on February 27, 2019 at 20:01:35.	
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
 ** SPI2 (CLK, MISO, MOSI, SEL1, D2, D3)
 ** UART0 (TX, RX, RTS, CTS)
 **
 ** GPIO (unavailable)
 ** ------------------
 ** PB08, PB09, PB10, PB11, PB12, PB13, PB14, PB15, PC02, PC03
 */

#include <sys/platform.h>
#include <stdint.h>

#define SPI2_CLK_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<20))
#define SPI2_MISO_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<22))
#define SPI2_MOSI_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<24))
#define SPI2_SEL1_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<30))
#define SPI2_D2_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<26))
#define SPI2_D3_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<28))
#define UART0_TX_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<16))
#define UART0_RX_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<18))
#define UART0_RTS_PORTC_MUX  ((uint16_t) ((uint16_t) 0<<4))
#define UART0_CTS_PORTC_MUX  ((uint16_t) ((uint16_t) 0<<6))

#define SPI2_CLK_PORTB_FER  ((uint32_t) ((uint32_t) 1<<10))
#define SPI2_MISO_PORTB_FER  ((uint32_t) ((uint32_t) 1<<11))
#define SPI2_MOSI_PORTB_FER  ((uint32_t) ((uint32_t) 1<<12))
#define SPI2_SEL1_PORTB_FER  ((uint32_t) ((uint32_t) 1<<15))
#define SPI2_D2_PORTB_FER  ((uint32_t) ((uint32_t) 1<<13))
#define SPI2_D3_PORTB_FER  ((uint32_t) ((uint32_t) 1<<14))
#define UART0_TX_PORTB_FER  ((uint32_t) ((uint32_t) 1<<8))
#define UART0_RX_PORTB_FER  ((uint32_t) ((uint32_t) 1<<9))
#define UART0_RTS_PORTC_FER  ((uint16_t) ((uint16_t) 1<<2))
#define UART0_CTS_PORTC_FER  ((uint16_t) ((uint16_t) 1<<3))

int32_t adi_initpinmux(void);

/*
 * Initialize the Port Control MUX and FER Registers
 */
int32_t adi_initpinmux(void) {
    /* PORTx_MUX registers */
    *pREG_PORTB_MUX = SPI2_CLK_PORTB_MUX | SPI2_MISO_PORTB_MUX
     | SPI2_MOSI_PORTB_MUX | SPI2_SEL1_PORTB_MUX | SPI2_D2_PORTB_MUX
     | SPI2_D3_PORTB_MUX | UART0_TX_PORTB_MUX | UART0_RX_PORTB_MUX;
    *pREG_PORTC_MUX = UART0_RTS_PORTC_MUX | UART0_CTS_PORTC_MUX;

    /* PORTx_FER registers */
    *pREG_PORTB_FER = SPI2_CLK_PORTB_FER | SPI2_MISO_PORTB_FER
     | SPI2_MOSI_PORTB_FER | SPI2_SEL1_PORTB_FER | SPI2_D2_PORTB_FER
     | SPI2_D3_PORTB_FER | UART0_TX_PORTB_FER | UART0_RX_PORTB_FER;
    *pREG_PORTC_FER = UART0_RTS_PORTC_FER | UART0_CTS_PORTC_FER;
    return 0;
}

