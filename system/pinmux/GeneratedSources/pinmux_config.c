/*
 **
 ** Source file generated on July 6, 2018 at 14:24:02.	
 **
 ** Copyright (C) 2011-2018 Analog Devices Inc., All Rights Reserved.
 **
 ** This file is generated automatically based upon the options selected in 
 ** the Pin Multiplexing configuration editor. Changes to the Pin Multiplexing
 ** configuration should be made by changing the appropriate options rather
 ** than editing this file.
 **
 ** Selected Peripherals
 ** --------------------
 */

#include <sys/platform.h>
#include <stdint.h>

int32_t adi_initpinmux(void);


#define SPI2_CLK_PORTB_MUX  ((uint16_t) ((uint16_t) 0<<20))
#define SPI2_MISO_PORTB_MUX  ((uint16_t) ((uint16_t) 0<<22))
#define SPI2_MOSI_PORTB_MUX  ((uint16_t) ((uint16_t) 0<<24))
#define SPI2_SEL1_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<30))
#define SPI2_D2_PORTB_MUX  ((uint16_t) ((uint16_t) 0<<26))
#define SPI2_D3_PORTB_MUX  ((uint16_t) ((uint16_t) 0<<28))
#define UART0_TX_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<16))
#define UART0_RX_PORTB_MUX  ((uint32_t) ((uint32_t) 0<<18))

#define SPI2_CLK_PORTB_FER  ((uint16_t) ((uint16_t) 1<<10))
#define SPI2_MISO_PORTB_FER  ((uint16_t) ((uint16_t) 1<<11))
#define SPI2_MOSI_PORTB_FER  ((uint16_t) ((uint16_t) 1<<12))
#define SPI2_SEL1_PORTB_FER  ((uint32_t) ((uint32_t) 1<<15))
#define SPI2_D2_PORTB_FER  ((uint16_t) ((uint16_t) 1<<13))
#define SPI2_D3_PORTB_FER  ((uint16_t) ((uint16_t) 1<<14))
#define UART0_TX_PORTB_FER  ((uint32_t) ((uint32_t) 1<<8))
#define UART0_RX_PORTB_FER  ((uint32_t) ((uint32_t) 1<<9))


/*
 * Initialize the Port Control MUX Registers
 */
int32_t adi_initpinmux(void) {

    /* PORTx_MUX registers */
    *pREG_PORTB_MUX = SPI2_CLK_PORTB_MUX | SPI2_MISO_PORTB_MUX
     | SPI2_MOSI_PORTB_MUX | SPI2_SEL1_PORTB_MUX | SPI2_D2_PORTB_MUX | SPI2_D3_PORTB_MUX | UART0_TX_PORTB_MUX
     | UART0_RX_PORTB_MUX;

    /* PORTx_FER registers */
    *pREG_PORTB_FER = SPI2_CLK_PORTB_FER | SPI2_MISO_PORTB_FER
     | SPI2_MOSI_PORTB_FER | SPI2_SEL1_PORTB_FER | SPI2_D2_PORTB_FER | SPI2_D3_PORTB_FER | UART0_TX_PORTB_FER
     | UART0_RX_PORTB_FER;

	/* We use GPIO PB15 as SPI flash /CS. */
//	*pREG_PORTB_FER_CLR = BITM_PORT_DATA_PX15;
//	*pREG_PORTB_DIR_SET = BITM_PORT_DATA_PX15;

    return 0;
}

