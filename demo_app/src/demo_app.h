/*
  Copyright 2018 Phantom Intelligence Inc.
*/

#ifndef __BF707_WAGNER_H__
#define __BF707_WAGNER_H__

#define EXTERNAL_SPI_DEVICE 0
#define GORDON_SPI_DEVICE 1
#define FLASH_SPI_DEVICE 2


#define LED_BC2_ON()  pADI_PORTC->DATA_SET = (1 << 8)
#define LED_BC2_OFF() pADI_PORTC->DATA_CLR = (1 << 8)
#define LED_BC2_TGL() pADI_PORTC->DATA_TGL = (1 << 8)

#define LED_BC3_ON()  pADI_PORTC->DATA_SET = (1 << 12)
#define LED_BC3_OFF() pADI_PORTC->DATA_CLR = (1 << 12)
#define LED_BC3_TGL() pADI_PORTC->DATA_TGL = (1 << 12)

#define GORDON_RESET() pADI_PORTC->DATA_SET = (1 << 8)
#define GORDON_SET()   pADI_PORTC->DATA_SET = (1 << 8)
#define GORDON_TGL()   pADI_PORTC->DATA_SET = (1 << 8)

#define LASER_OUTPUT_ENABLE() pADI_PORTB->DATA_CLR = (1 << 5)
#define LASER_OUTPUT_DISABLE() pADI_PORTB->DATA_SET = (1 << 5)

#define LASER_PULSE1_ENABLE() pADI_PORTA->DATA_CLR = (1 << 8)
#define LASER_PULSE1_DISABLE() pADI_PORTA->DATA_SET = (1 << 8)

#define LASER_PULSE2_ENABLE() pADI_PORTA->DATA_CLR = (1 << 9)
#define LASER_PULSE2_DISABLE() pADI_PORTA->DATA_SET = (1 << 9)

#define LP_DRIVER_POWER_ON() pADI_PORTB->DATA_CLR = (1 << 6)
#define LP_DRIVER_POWER_OFF() pADI_PORTB->DATA_SET = (1 << 6)




#endif /* __BF707_WAGNER_H__ */
