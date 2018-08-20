/*****************************************************************************
 * BF707_Wagner.h
 *****************************************************************************/

#ifndef __BF707_WAGNER_H__
#define __BF707_WAGNER_H__


#define LED3_ON()  pADI_PORTA->DATA_SET = (1 << 0)
#define LED3_OFF() pADI_PORTA->DATA_CLR = (1 << 0)
#define LED3_TGL() pADI_PORTA->DATA_TGL = (1 << 0)

#define LED4_ON()  pADI_PORTA->DATA_SET = (1 << 1)
#define LED4_OFF() pADI_PORTA->DATA_CLR = (1 << 1)
#define LED4_TGL() pADI_PORTA->DATA_TGL = (1 << 1)

#define LED5_ON()  pADI_PORTB->DATA_SET = (1 << 1)
#define LED5_OFF() pADI_PORTB->DATA_CLR = (1 << 1)
#define LED5_TGL() pADI_PORTB->DATA_TGL = (1 << 1)


#endif /* __BF707_WAGNER_H__ */
