/********************************************************************************
Copyright(c) 2012 Analog Devices, Inc. All Rights Reserved.

This software is proprietary.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/

#include <sys/platform.h>

#include "../common/spi.h"
#include "../common/flash.h"
#include "../target.h"

/* Assume there will be no more than 8 SPI ports */

#ifndef pREG_SPI0_CTL
#define pREG_SPI0_CTL (volatile void *)0x1
#endif

#ifndef pREG_SPI1_CTL
#define pREG_SPI1_CTL (volatile void *)0x1
#endif

#ifndef pREG_SPI2_CTL
#define pREG_SPI2_CTL (volatile void *)0x1
#endif

#ifndef pREG_SPI3_CTL
#define pREG_SPI3_CTL (volatile void *)0x1
#endif

#ifndef pREG_SPI4_CTL
#define pREG_SPI4_CTL (volatile void *)0x1
#endif

#ifndef pREG_SPI5_CTL
#define pREG_SPI5_CTL (volatile void *)0x1
#endif

#ifndef pREG_SPI6_CTL
#define pREG_SPI6_CTL (volatile void *)0x1
#endif

#ifndef pREG_SPI7_CTL
#define pREG_SPI7_CTL (volatile void *)0x1
#endif

static volatile uint32_t *pREG_SPI_CTL;
static volatile uint32_t *pREG_SPI_RXCTL;
static volatile uint32_t *pREG_SPI_TXCTL;
static volatile uint32_t *pREG_SPI_CLK;
static volatile uint32_t *pREG_SPI_RWCR;
static volatile uint32_t *pREG_SPI_TWC;
static volatile uint32_t *pREG_SPI_TWCR;
static volatile uint32_t *pREG_SPI_STAT;
static volatile uint32_t *pREG_SPI_RFIFO;
static volatile uint32_t *pREG_SPI_TFIFO;

static int adi_spi_bf6xx_open(int spi_no)
{
	switch (spi_no)
	{
	case 0: pREG_SPI_CTL = pREG_SPI0_CTL; break;
	case 1: pREG_SPI_CTL = pREG_SPI1_CTL; break;
	case 2: pREG_SPI_CTL = pREG_SPI2_CTL; break;
	case 3: pREG_SPI_CTL = pREG_SPI3_CTL; break;
	case 4: pREG_SPI_CTL = pREG_SPI4_CTL; break;
	case 5: pREG_SPI_CTL = pREG_SPI5_CTL; break;
	case 6: pREG_SPI_CTL = pREG_SPI6_CTL; break;
	case 7: pREG_SPI_CTL = pREG_SPI7_CTL; break;
	default: pREG_SPI_CTL = (volatile void *)0x1; break;
	}

	if (pREG_SPI_CTL == (volatile void *)0x1)
		return -1;

	pREG_SPI_RXCTL	= pREG_SPI_CTL + (pREG_SPI0_RXCTL - pREG_SPI0_CTL);
	pREG_SPI_TXCTL	= pREG_SPI_CTL + (pREG_SPI0_TXCTL - pREG_SPI0_CTL);
	pREG_SPI_CLK	= pREG_SPI_CTL + (pREG_SPI0_CLK - pREG_SPI0_CTL);
	pREG_SPI_RWCR	= pREG_SPI_CTL + (pREG_SPI0_RWCR - pREG_SPI0_CTL);
	pREG_SPI_TWC	= pREG_SPI_CTL + (pREG_SPI0_TWC - pREG_SPI0_CTL);
	pREG_SPI_TWCR	= pREG_SPI_CTL + (pREG_SPI0_TWCR - pREG_SPI0_CTL);
	pREG_SPI_STAT	= pREG_SPI_CTL + (pREG_SPI0_STAT - pREG_SPI0_CTL);
	pREG_SPI_RFIFO	= pREG_SPI_CTL + (pREG_SPI0_RFIFO - pREG_SPI0_CTL);
	pREG_SPI_TFIFO	= pREG_SPI_CTL + (pREG_SPI0_TFIFO - pREG_SPI0_CTL);

	return 0;
}

static int adi_spi_bf6xx_close(void)
{
	return 0;
}

static int adi_spi_bf6xx_config(const struct flash_info *fi)
{
	uint32_t config;

	/* Set SPI CLOCK */

	*pREG_SPI_CLK = 0x4;

	/* Set SPI CONTROL */

	config = ENUM_SPI_CTL_MASTER;

	if (fi->cpha)
		config |= ENUM_SPI_CTL_SCKBEG;
	else
		config |= ENUM_SPI_CTL_SCKMID;

	if (fi->cpol)
		config |= ENUM_SPI_CTL_SCKLO;
	else
		config |= ENUM_SPI_CTL_SCKHI;

	config |= ENUM_SPI_CTL_SIZE08;

	if (fi->lsb_first)
		config |= ENUM_SPI_CTL_LSB_FIRST;
	else
		config |= ENUM_SPI_CTL_MSB_FIRST;

	if (fi->start_on_mosi)
		config |= ENUM_SPI_CTL_STMOSI;
	else
		config |= ENUM_SPI_CTL_STMISO;

	*pREG_SPI_CTL = config;

	/* Set SPI word count reload registers */

	*pREG_SPI_RWCR = 0;
	*pREG_SPI_TWCR = 0;

	return 0;
}

static int adi_spi_bf6xx_enable(void)
{
	uint32_t config;

	config = *pREG_SPI_CTL;
	config &= ~ BITM_SPI_CTL_EN;
	config |= ENUM_SPI_CTL_EN;
	*pREG_SPI_CTL = config;

	return 0;
}

static int adi_spi_bf6xx_disable(void)
{
	uint32_t config;

	config = *pREG_SPI_CTL;
	config &= ~ BITM_SPI_CTL_EN;
	config |= ENUM_SPI_CTL_DIS;
	*pREG_SPI_CTL = config;

	return 0;
}

static void adi_spi_bf6xx_wait_send_done(void)
{
	/* It's possible when we check REG_SPI2_STAT, the data has just been
	   moved from TFIFO to shift register.  At that time TFIFO is empty,
	   and SPIF is still set (from last transfer and not cleared yet since
	   the first bit has not been shifted out from the shift register).
	   So we can't simply use (TFIFO_EMPTY && SPIF) as the indication of
	   transfer finish.  We need to check for more times until

	   -- it holds continuously in checking 100 times, (100 is a somewhat
		  arbitrarily chosen number).

	   -- or, it goes through TRUE, FALSE, TRUE again.

	   This method was implemented as below.  But now it's replaced by a
	   new and better mothod.

	uint32_t stat;
	int times, was_true;

	times = 0;
	was_true = 0;
	do
	{
		stat = *pREG_SPI_STAT;

		if ((stat & BITM_SPI_STAT_TFS) == ENUM_SPI_STAT_TFIFO_EMPTY
				&& (stat & BITM_SPI_STAT_SPIF) == ENUM_SPI_STAT_SPIF_HI)
		{
			if (was_true)
				break;

			times++;
		}
		else if (times != 0)
			was_true = 1;
	}
	while (times < 100);


	   A much better way to check if send is done is setting TWC and
	   checking TF bit, as implemented below.  */

	uint32_t stat;
	do
	{
		stat = *pREG_SPI_STAT;
		if ((stat & BITM_SPI_STAT_TF) == ENUM_SPI_TF_HI)
			break;
	}
	while (1);
	/* clear the transfer finish indication */
	*pREG_SPI_STAT = ENUM_SPI_TF_HI;
}

static int adi_spi_bf6xx_send(uint8_t *buf, int count)
{
	int i;
	int word_count;

#ifndef USE_SLVSEL

	uint32_t config;

	word_count = count / 4;

	if (word_count > 0)
	{
		adi_spi_bf6xx_disable();
		config = *pREG_SPI_CTL;
		config &= ~ BITM_SPI_CTL_SIZE;
		config |= ENUM_SPI_CTL_SIZE32;
		*pREG_SPI_CTL = config;
		adi_spi_bf6xx_enable();
	}

	while (word_count > 0)
	{
		int twc;

		twc = word_count < BITM_SPI_TWC_VALUE ? word_count : BITM_SPI_TWC_VALUE;
		*pREG_SPI_TWC = twc;
		*pREG_SPI_TXCTL = ENUM_SPI_TXCTL_TX_EN | ENUM_SPI_TXCTL_TTI_EN | ENUM_SPI_TXCTL_TWC_EN;

		for (i = 0; i < twc; i++)
		{
			uint32_t data;

			data = (buf[i * 4] << 24 | buf[i * 4 + 1] << 16
					| buf[i * 4 + 2] << 8 | buf[i * 4 + 3]);
			while (*pREG_SPI_STAT & BITM_SPI_STAT_TFF)
				;
			*pREG_SPI_TFIFO = data;
		}

		adi_spi_bf6xx_wait_send_done();

		*pREG_SPI_TXCTL = 0;
		word_count -= twc;
		buf += twc * 4;
	}


	/* If we have set word transfer size to 32, set back to 8.  */

	if (count / 4 > 0)
	{
		adi_spi_bf6xx_disable();
		config = *pREG_SPI_CTL;
		config &= ~ BITM_SPI_CTL_SIZE;
		config |= ENUM_SPI_CTL_SIZE08;
		*pREG_SPI_CTL = config;
		adi_spi_bf6xx_enable();
	}

	count %= 4;

#endif

	word_count = count;
	while (word_count > 0)
	{
		int twc;

		twc = word_count < BITM_SPI_TWC_VALUE ? word_count : BITM_SPI_TWC_VALUE;
		*pREG_SPI_TWC = twc;
		*pREG_SPI_TXCTL = ENUM_SPI_TXCTL_TX_EN | ENUM_SPI_TXCTL_TTI_EN | ENUM_SPI_TXCTL_TWC_EN;

		for (i = 0; i < twc; i++)
		{
			while (*pREG_SPI_STAT & BITM_SPI_STAT_TFF)
				;
			*pREG_SPI_TFIFO = buf[i];
		}

		adi_spi_bf6xx_wait_send_done();

		*pREG_SPI_TXCTL = 0;
		word_count -= twc;
		buf += twc;
	}

	return 0;
}

static int adi_spi_bf6xx_recv(uint8_t *buf, int count)
{
	int i = 0;

#ifndef USE_SLVSEL

	uint32_t config;

	if (count >= 4)
	{
		adi_spi_bf6xx_disable();
		config = *pREG_SPI_CTL;
		config &= ~ BITM_SPI_CTL_SIZE;
		config |= ENUM_SPI_CTL_SIZE32;
		*pREG_SPI_CTL = config;
		adi_spi_bf6xx_enable();
	}

	if (count >= 4)
	{
		*pREG_SPI_RXCTL = ENUM_SPI_RXCTL_RX_EN | ENUM_SPI_RXCTL_RTI_EN;

		for (; i + 4 <= count; i += 4)
		{
			uint32_t data;

			while (*pREG_SPI_STAT & BITM_SPI_STAT_RFE)
				;
			data = *pREG_SPI_RFIFO;

			buf[i] = (data >> 24) & 0xff;
			buf[i + 1] = (data >> 16) & 0xff;
			buf[i + 2] = (data >> 8) & 0xff;
			buf[i + 3] = data & 0xff;
		}

		*pREG_SPI_RXCTL = 0;
	}

	/* If we have set word transfer size to 32, set back to 8.  */

	if (count >= 4)
	{
		adi_spi_bf6xx_disable();
		config = *pREG_SPI_CTL;
		config &= ~ BITM_SPI_CTL_SIZE;
		config |= ENUM_SPI_CTL_SIZE08;
		*pREG_SPI_CTL = config;
		adi_spi_bf6xx_enable();
	}

#endif

	if (count > 0)
	{
		*pREG_SPI_RXCTL = ENUM_SPI_RXCTL_RX_EN | ENUM_SPI_RXCTL_RTI_EN;

		for (; i < count; i++)
		{
			while (*pREG_SPI_STAT & BITM_SPI_STAT_RFE)
				;
			buf[i] = *pREG_SPI_RFIFO;
		}

		*pREG_SPI_RXCTL = 0;
	}

	return 0;
}

static int adi_spi_bf6xx_recv_until (int (*condition)(const uint8_t))
{
	uint8_t data;

	*pREG_SPI_RXCTL = ENUM_SPI_RXCTL_RX_EN | ENUM_SPI_RXCTL_RTI_EN;

	do
	{
		while (*pREG_SPI_STAT & BITM_SPI_STAT_RFE)
			;
		data = *pREG_SPI_RFIFO;
	}
	while (!condition(data));

	*pREG_SPI_RXCTL = 0;

	return 0;
}

static int adi_spi_bf6xx_set_mode(spi_mode_t mode)
{
	uint32_t config;

	/* TODO check if SPI is quiescent */

	config = *pREG_SPI_CTL;
	config &= ~ BITM_SPI_CTL_MIOM;

	switch (mode)
	{
	case STANDARD:
		config |= ENUM_SPI_CTL_MIO_DIS;
		break;

	case DUAL_OUTPUT:
	case DUAL_IO:
		config |= ENUM_SPI_CTL_MIO_DUAL;
		break;

	case QUAD_OUTPUT:
	case QUAD_IO:
		config |= ENUM_SPI_CTL_MIO_QUAD;
		break;

	default:
		/* bad mode */
		return -1;
	}

	*pREG_SPI_CTL = config;

	return 0;
}

static int adi_spi_bf6xx_select_slave(int slave_no)
{
	/* currently we only handle slave_no == 1 */
	switch (slave_no)
	{
	case 0:
		SPI_UNSELECT_SLAVE;
		break;

	case 1:
		SPI_SELECT_SLAVE;
		break;

	default:
		return -1;
	}

	return 0;
}

const struct spi_ctlr adi_spi_bf6xx_ctlr =
{
	adi_spi_bf6xx_open,
	adi_spi_bf6xx_close,
	adi_spi_bf6xx_config,
	adi_spi_bf6xx_enable,
	adi_spi_bf6xx_disable,
	adi_spi_bf6xx_send,
	adi_spi_bf6xx_recv,
	adi_spi_bf6xx_recv_until,
	adi_spi_bf6xx_set_mode,
	adi_spi_bf6xx_select_slave,
};
