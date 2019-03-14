/*
 **
 ** Source file generated on March 1, 2019 at 15:50:58.	
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
 ** CAN1 (RX, TX)
 ** SPI0 (CLK, MISO, MOSI, RDY, SEL1, D2, D3)
 ** SPI1 (CLK, MISO, MOSI, SEL1)
 ** SPI2 (CLK, MISO, MOSI, SEL1, D2, D3)
 ** TM0 (TMR2)
 ** UART0 (TX, RX)
 **
 ** GPIO (unavailable)
 ** ------------------
 ** PA00, PA01, PA02, PA04, PA05, PA06, PA07, PA12, PA13, PB00, PB01, PB02, PB03,
 ** PB08, PB09, PB10, PB11, PB12, PB13, PB14, PB15, PC09
 */

#include <sys/platform.h>
#include <stdint.h>

#define CAN1_RX_PORTA_MUX  ((uint32_t) ((uint32_t) 1<<24))
#define CAN1_TX_PORTA_MUX  ((uint32_t) ((uint32_t) 1<<26))
#define SPI0_CLK_PORTB_MUX  ((uint16_t) ((uint16_t) 2<<0))
#define SPI0_MISO_PORTB_MUX  ((uint16_t) ((uint16_t) 2<<2))
#define SPI0_MOSI_PORTB_MUX  ((uint16_t) ((uint16_t) 2<<4))
#define SPI0_RDY_PORTA_MUX  ((uint16_t) ((uint16_t) 2<<12))
#define SPI0_SEL1_PORTA_MUX  ((uint16_t) ((uint16_t) 1<<10))
#define SPI0_D2_PORTB_MUX  ((uint16_t) ((uint16_t) 2<<6))
#define SPI0_D3_PORTC_MUX  ((uint32_t) ((uint32_t) 1<<18))
#define SPI1_CLK_PORTA_MUX  ((uint16_t) ((uint16_t) 0<<0))
#define SPI1_MISO_PORTA_MUX  ((uint16_t) ((uint16_t) 0<<2))
#define SPI1_MOSI_PORTA_MUX  ((uint16_t) ((uint16_t) 0<<4))
#define SPI1_SEL1_PORTA_MUX  ((uint16_t) ((uint16_t) 0<<8))
#define SPI2_CLK_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<20))
#define SPI2_MISO_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<22))
#define SPI2_MOSI_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<24))
#define SPI2_SEL1_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<30))
#define SPI2_D2_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<26))
#define SPI2_D3_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<28))
#define TM0_TMR2_PORTA_MUX  ((uint16_t) ((uint16_t) 0<<14))
#define UART0_TX_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<16))
#define UART0_RX_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<18))

#define CAN1_RX_PORTA_FER  ((uint32_t) ((uint32_t) 1<<12))
#define CAN1_TX_PORTA_FER  ((uint32_t) ((uint32_t) 1<<13))
#define SPI0_CLK_PORTB_FER  ((uint16_t) ((uint16_t) 1<<0))
#define SPI0_MISO_PORTB_FER  ((uint16_t) ((uint16_t) 1<<1))
#define SPI0_MOSI_PORTB_FER  ((uint16_t) ((uint16_t) 1<<2))
#define SPI0_RDY_PORTA_FER  ((uint16_t) ((uint16_t) 1<<6))
#define SPI0_SEL1_PORTA_FER  ((uint16_t) ((uint16_t) 1<<5))
#define SPI0_D2_PORTB_FER  ((uint16_t) ((uint16_t) 1<<3))
#define SPI0_D3_PORTC_FER  ((uint32_t) ((uint32_t) 1<<9))
#define SPI1_CLK_PORTA_FER  ((uint16_t) ((uint16_t) 1<<0))
#define SPI1_MISO_PORTA_FER  ((uint16_t) ((uint16_t) 1<<1))
#define SPI1_MOSI_PORTA_FER  ((uint16_t) ((uint16_t) 1<<2))
#define SPI1_SEL1_PORTA_FER  ((uint16_t) ((uint16_t) 1<<4))
#define SPI2_CLK_PORTB_FER  ((uint32_t) ((uint32_t) 1<<10))
#define SPI2_MISO_PORTB_FER  ((uint32_t) ((uint32_t) 1<<11))
#define SPI2_MOSI_PORTB_FER  ((uint32_t) ((uint32_t) 1<<12))
#define SPI2_SEL1_PORTB_FER  ((uint32_t) ((uint32_t) 1<<15))
#define SPI2_D2_PORTB_FER  ((uint32_t) ((uint32_t) 1<<13))
#define SPI2_D3_PORTB_FER  ((uint32_t) ((uint32_t) 1<<14))
#define TM0_TMR2_PORTA_FER  ((uint16_t) ((uint16_t) 1<<7))
#define UART0_TX_PORTB_FER  ((uint32_t) ((uint32_t) 1<<8))
#define UART0_RX_PORTB_FER  ((uint32_t) ((uint32_t) 1<<9))

int32_t adi_initpinmux(void);

/*
 * Initialize the Port Control MUX and FER Registers
 */
int32_t adi_initpinmux(void) {
    /* PORTx_MUX registers */
    *pREG_PORTA_MUX = CAN1_RX_PORTA_MUX | CAN1_TX_PORTA_MUX
     | SPI0_RDY_PORTA_MUX | SPI0_SEL1_PORTA_MUX | SPI1_CLK_PORTA_MUX
     | SPI1_MISO_PORTA_MUX | SPI1_MOSI_PORTA_MUX | SPI1_SEL1_PORTA_MUX
     | TM0_TMR2_PORTA_MUX;
    *pREG_PORTB_MUX = SPI0_CLK_PORTB_MUX | SPI0_MISO_PORTB_MUX
     | SPI0_MOSI_PORTB_MUX | SPI0_D2_PORTB_MUX | SPI2_CLK_PORTB_MUX
     | SPI2_MISO_PORTB_MUX | SPI2_MOSI_PORTB_MUX | SPI2_SEL1_PORTB_MUX
     | SPI2_D2_PORTB_MUX | SPI2_D3_PORTB_MUX | UART0_TX_PORTB_MUX
     | UART0_RX_PORTB_MUX;
    *pREG_PORTC_MUX = SPI0_D3_PORTC_MUX;

    /* PORTx_FER registers */
    *pREG_PORTA_FER = CAN1_RX_PORTA_FER | CAN1_TX_PORTA_FER
     | SPI0_RDY_PORTA_FER | SPI0_SEL1_PORTA_FER | SPI1_CLK_PORTA_FER
     | SPI1_MISO_PORTA_FER | SPI1_MOSI_PORTA_FER | SPI1_SEL1_PORTA_FER
     | TM0_TMR2_PORTA_FER;
    *pREG_PORTB_FER = SPI0_CLK_PORTB_FER | SPI0_MISO_PORTB_FER
     | SPI0_MOSI_PORTB_FER | SPI0_D2_PORTB_FER | SPI2_CLK_PORTB_FER
     | SPI2_MISO_PORTB_FER | SPI2_MOSI_PORTB_FER | SPI2_SEL1_PORTB_FER
     | SPI2_D2_PORTB_FER | SPI2_D3_PORTB_FER | UART0_TX_PORTB_FER
     | UART0_RX_PORTB_FER;
    *pREG_PORTC_FER = SPI0_D3_PORTC_FER;
    return 0;
}

