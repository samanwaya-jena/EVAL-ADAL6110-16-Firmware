/*
 * uart.c
 *
 * ADSP-BF7xx uart
 *
 * File Version 2.8.0.0
 *
 * Copyright (c) 2011-2017 Analog Devices, Inc. All Rights Reserved.
 */

#include "uart.h"

#if CONFIG_CGU

static void UartPutc(uint32_t UartNum, uint8_t c);

/*****************************************************************************
 Functions
******************************************************************************/

/*
 * Function Name : short UartPutc(unsigned char UartNum, char c)
 * Description   : This function transmits a character
 *                 by polling THRE bit in the UART Status register.
 *
 * Parameters    : The character to transmit, UART number
 * Returns       : none.
 * Globals       : none
 */
static
void UartPutc(uint32_t UartNum, uint8_t c)
{
   volatile uint32_t *pUartStat;
   volatile uint32_t *pUartThr;

    switch (UartNum) {
        case 0uL:
           pUartStat = pREG_UART0_STAT;
           pUartThr  = pREG_UART0_THR;
        break;
        case 1uL:
           pUartStat = pREG_UART1_STAT;
           pUartThr  = pREG_UART1_THR;
        break;
        default:
           ErrorWithNoReturn();
        break;
    }

    while ( (*pUartStat & BITM_UART_STAT_THRE) == ENUM_UART_STAT_THR_NOT_EMPTY ) { /* wait */ }
    *pUartThr = c;
}


/* Function Name : UartRxFifoClear
 *
 * Description   : This function checks Receive FIFO Count Status
 *                 wait for Receive FIFO to be filled and wait for at least one word
 *                 send status feedback to host
 * Parameters    : UART number, UartBitrate
 * Returns       : Restore value for UART_CTL register. If the UartRxFifoClear function
 *                 is unsuccessful, a negative value is returned.
 * Globals       : none
 */

uint32_t UartRxFifoClear(uint32_t UartNum, uint32_t UartBitrate)
{
   volatile uint32_t *pUartStat;
   volatile uint32_t *pUartCtl;
   uint32_t UartCtl;

    switch (UartNum) {
        case 0uL:
           pUartStat = pREG_UART0_STAT;
           pUartCtl  = pREG_UART0_CTL;
        break;
        case 1uL:
           pUartStat = pREG_UART1_STAT;
           pUartCtl  = pREG_UART1_CTL;
        break;
        default:
           ErrorWithNoReturn();
        break;
    }

    UartCtl = *pUartCtl;                 /* save UART_CTL register */
    *pUartCtl &= (~BITM_UART_CTL_ARTS);  /* clear ARTS bit */
    *pUartCtl &= (~BITM_UART_CTL_MRTS);  /* clear MRTS bit -> force pin UARTxRTS to its de-assertive state when ARTS=0 */

    /***********************************************************************************
     Especially in half duplex mode it may be necessary to wait here.
     A delay loop with the length of one data word ensures that the
     host has finished sending the very last word after RTS/host wait
     has been de-asserted.
     IMPORTANT: If the host is waiting to send the last data word
     than this delay may not be enough. The programmer has to take
     care about this!
    ************************************************************************************/
#if 0
    for (8*UartBitrate ; UartBitrate > 0 ; UartBitrate--) {
       NOP();
    }
#endif
    /* Signal Receive Buffer Status to the host before changing PLL.
     * Four bytes are transmitted back to the host:
     *   0xBF
     *   UARTx_STAT[ 7:0] value
     *   UARTx_STAT[15:8] value
     *   0x00 to terminate string
     */


    UartPutc(UartNum, 0xBFu);
    UartPutc(UartNum, (uint8_t)(*pUartStat & 0xFFuL));
    UartPutc(UartNum, (uint8_t)((*pUartStat & 0xFF00uL) >> 8));
    UartPutc(UartNum, 0x00u);

    while( (*pUartStat & BITM_UART_STAT_TEMT) == ENUM_UART_STAT_TX_NOT_EMPTY )
    {
       NOP(); /* wait */
    }

    return UartCtl;
} /* UartRxFifoClear */

/* Function Name : UartGetBitrate
 *
 * Description   : This function calculates and returns the current UART bitrate.
 *
 * Parameters    : UART number
 * Returns       : Current UART Bitrate.
 * Globals       : none
 */
uint32_t UartGetBitrate(uint32_t UartNum)
{
    volatile uint32_t *pUartClk;
    uint32_t UartBitrate;

    switch (UartNum) {
        case 0uL:
           pUartClk = pREG_UART0_CLK;
        break;
        case 1uL:
           pUartClk = pREG_UART1_CLK;
        break;
        default:
           ErrorWithNoReturn();
        break;
    }

    if ( (*pUartClk & BITM_UART_CLK_DIV) == 0uL )
    {
       UartBitrate = 0uL;
    } else
    {
       UartBitrate = ( get_s0clk_hz() / (*pUartClk & BITM_UART_CLK_DIV) );
       if ( (*pUartClk & BITM_UART_CLK_EDBO) == ENUM_UART_CLK_DIS_DIV_BY_ONE )
       {
          /* Divide by clock prescaler value 16, rounding to nearest. */
          UartBitrate += 8uL;
          UartBitrate >>= 4;
       }
    }
    return UartBitrate;
} /* UartGetBitrate */

/* Function Name : UartSetBitrate
 * Description   : This function sets the UART bitrate.
 *
 * Parameters    : UART number, UART Bitrate
 * Returns       : 0 if the UartSetBitrate function is successful and -1 otherwise.
 * Globals       : none
 */
int16_t UartSetBitrate(uint32_t UartNum, uint32_t UartBitrate)
{
   volatile uint32_t *pUartStat;
   volatile uint32_t *pUartCtl;
   volatile uint32_t *pUartClk;
   int16_t rtn;

    switch (UartNum) {
        case 0uL:
           pUartStat = pREG_UART0_STAT;
           pUartCtl  = pREG_UART0_CTL;
           pUartClk  = pREG_UART0_CLK;
        break;
        case 1uL:
           pUartStat = pREG_UART1_STAT;
           pUartCtl  = pREG_UART1_CTL;
           pUartClk  = pREG_UART1_CLK;
        break;
        default:
           ErrorWithNoReturn();
        break;
    }

    if ( UartBitrate == 0uL )
    {
       rtn = -1;
    } else
    {
       uint32_t UartDivisor = ( get_s0clk_hz() / UartBitrate );

       if ( (*pUartClk & BITM_UART_CLK_EDBO) == ENUM_UART_CLK_DIS_DIV_BY_ONE )
       {
          /* Bit clock prescaler = 16 */
          UartDivisor += 8uL;
          UartDivisor >>= 4;
       }

       *pUartClk &= (~BITM_UART_CLK_DIV);
       *pUartClk |= UartDivisor;

        /* Signal the completion of the autobaud detection to the host.
         * Four bytes are transmitted back to the host:
         *   0xBF
         *   UART_CLK[ 7:0] value
         *   UART_CLK[15:8] value
         *   0x00 to terminate string
         */

       UartPutc(UartNum, 0xBFu);
       UartPutc(UartNum, (uint8_t)(*pUartClk & 0xFFuL));
       UartPutc(UartNum, (uint8_t)((*pUartClk & 0xFF00uL) >> 8));
       UartPutc(UartNum, 0x00u);
       while( (*pUartStat & BITM_UART_STAT_TEMT) == ENUM_UART_STAT_TX_NOT_EMPTY )
       {
          NOP(); /* wait */
       }
       rtn = 0;
    }

    return rtn;
} /* UartSetBitrate */
#else
/* suppress translation unit must contain at least one declaration warning */
#pragma diag(suppress : 96)
#endif /* CONFIG_CGU */

/**********************************************************************************************
 EOF
**********************************************************************************************/
