/*
  Copyright 2018 Phantom Intelligence Inc.
*/

#include <ADSP-BF707_device.h>


#ifndef __BF707_WAGNER_H__
#define __BF707_WAGNER_H__

#define FIRMWARE_MAJOR_REV 0
#define FIRMWARE_MINOR_REV 14


#define EXTERNAL_SPI_DEVICE 0
#define GORDON_SPI_DEVICE 1
#define FLASH_SPI_DEVICE 2


#define LED_BC2G_ON()  pADI_PORTC->DATA_SET = (1 << 8)
#define LED_BC2G_OFF() pADI_PORTC->DATA_CLR = (1 << 8)
#define LED_BC2G_TGL() pADI_PORTC->DATA_TGL = (1 << 8)
#define LED_BC2R_ON()  pADI_PORTC->DATA_SET = (1 << 7)
#define LED_BC2R_OFF() pADI_PORTC->DATA_CLR = (1 << 7)
#define LED_BC2R_TGL() pADI_PORTC->DATA_TGL = (1 << 7)

#define LED_BC3G_ON()  pADI_PORTC->DATA_SET = (1 << 12)
#define LED_BC3G_OFF() pADI_PORTC->DATA_CLR = (1 << 12)
#define LED_BC3G_TGL() pADI_PORTC->DATA_TGL = (1 << 12)
#define LED_BC3R_ON()  pADI_PORTC->DATA_SET = (1 << 11)
#define LED_BC3R_OFF() pADI_PORTC->DATA_CLR = (1 << 11)
#define LED_BC3R_TGL() pADI_PORTC->DATA_TGL = (1 << 11)

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
