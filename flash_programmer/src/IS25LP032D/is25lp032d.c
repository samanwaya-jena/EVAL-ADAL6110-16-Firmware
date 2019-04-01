/********************************************************************************
Copyright(c) 2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "../common/spi.h"
#include "../common/flash.h"

/* The following three macros are for debug purpose */
/* #define DUMMY_IS_ZERO */
/* #define DUMMY_IS_ONE */
/* otherwise DUMMY_IS_DONT_CARE */

/* Instruction Set */

/* Instruction Set Table 1 (Erase, Program Instructions) */
#define WRITE_ENABLE							0x06
//#define WRITE_ENABLE_FOR_VOLATILE_STATUS_REGISTER 0x50
#define WRITE_DISABLE							0x04
#define READ_STATUS_REGISTER_1					0x05
//#define READ_STATUS_REGISTER_2					0x35
#define WRITE_STATUS_REGISTER					0x01
#define PAGE_PROGRAM							0x02
#define QUAD_PAGE_PROGRAM						0x32 /*	0x32 SPI - 0x38 QSPI*/
#define SECTOR_ERASE							0xD7 /* 0xD7 SPI - 0x20 QSPI*/
#define BLOCK_ERASE_32KB						0x52
#define BLOCK_ERASE_64KB						0xd8
#define CHIP_ERASE								0xc7 /* SPI 0xc7 or QSPI 0x60 */
#define ERASE_PROGRAM_SUSPEND					0x75 /* SPI 0x75 or QSPI 0xB0 */
#define ERASE_PROGRAM_RESUME					0x7a /* SPI 0x7a or QSPI 0x30 */
#define POWER_DOWN								0xb9
//#define CONTINUOUS_READ_MODE_RESET				0xff

/* Instruction Set Table 2 (Read Instructions) */

#define READ_DATA								0x03
#define FAST_READ								0x0b
#define FAST_READ_DUAL_OUTPUT					0x3b
#define FAST_READ_QUAD_OUTPUT					0x6b
#define FAST_READ_DUAL_IO						0xbb
#define FAST_READ_QUAD_IO						0xeb
//#define WORD_READ_QUAD_IO						0xe7
//#define OCTAL_WORD_READ_QUAD_IO					0xe3
//#define SET_BURST_WITH_WRAP						0x77
#define FAST_READ_DTR							0x0d
#define FAST_READ_DUAL_DTR						0xbd
#define FAST_READ_QUAD_DTR						0xed

/* Instruction Set Table 3 (ID, Security Instructions) */

#define RELEASE_POWER_DOWN_DEVICE_ID 			0xab
#define MANUFACTURER_ID_DEVICE_ID				0x90
//#define MANUFACTURER_ID_DEVICE_ID_BY_DUAL_IO	0x92
//#define MANUFACTURER_ID_DEVICE_ID_BY_QUAD_IO	0x94
#define JEDEC_ID								0x9f
#define READ_UNIQUE_ID							0x4b
#define READ_SFDP_REGISTER						0x5a
//#define ERASE_SECURITY_REGISTERS				0x44
#define PROGRAM_SECURITY_REGISTERS				0x42 //function register
#define READ_SECURITY_REGISTERS					0x48 //function register

/* CONTINUOUS_READ_MODE */

//#define CONTINUOUS_READ_MODE_OFF				0xff
//#define CONTINUOUS_READ_MODE_ON					0x10

/* STATUS bits */

#define STATUS1_BUSY							0x01
#define STATUS1_WEL								0x02
#define STATUS1_BP0								0x04
#define STATUS1_BP1								0x08
#define STATUS1_BP2								0x10
#define STATUS1_BP3								0x20
#define STATUS1_QE								0x40
#define STATUS1_SRWD							0x80

/* Various sizes */

#define PAGE_SIZE								0x100
#define SECTOR_SIZE								0x1000
#define BLOCK_32KB_SIZE							0x8000
#define BLOCK_64KB_SIZE							0x10000

#define _64K_BLOCKS_

#if defined(_64K_BLOCKS_)
  #define NUM_SECTORS 64
  #define BLOCK_SIZE BLOCK_64KB_SIZE
  #define ERASE_TYPE ERASE_64KB
#elif defined(_32K_BLOCKS_)
  #define NUM_SECTORS 128
  #define BLOCK_SIZE BLOCK_32KB_SIZE
  #define ERASE_TYPE ERASE_32KB
#else
  #define NUM_SECTORS 1024
  #define BLOCK_SIZE SECTOR_SIZE
  #define ERASE_TYPE ERASE_4KB
#endif

static int is25lp032d_write_status(const struct flash_info *fi, uint8_t status1);

static int is25lp032d_open(struct flash_info *fi)
{
	fi->size = 0x400000;
	fi->number_of_regions = 1;
	fi->erase_block_regions = malloc(fi->number_of_regions * sizeof (struct erase_block_region));

	if (fi->erase_block_regions == NULL)
		return -1;

	fi->erase_block_regions[0].block_size = BLOCK_SIZE;
	fi->erase_block_regions[0].number_of_blocks = NUM_SECTORS;

	/* make sure to disable block protection at startup, otherwise we cannot write to flash */
	is25lp032d_write_status(fi, 0);
	
	return 0;
}

static int is25lp032d_close(struct flash_info *fi)
{
	free(fi->erase_block_regions);
	return 0;
}

static int is25lp032d_write_enable(const struct flash_info *fi)
{
	return generic_write_enable (fi, WRITE_ENABLE);
}

static int is25lp032d_write_disable(const struct flash_info *fi)
{
	return generic_write_disable (fi, WRITE_DISABLE);
}

static int is25lp032d_read_mid_did(struct flash_info *fi, uint8_t *mid, uint8_t *did)
{
	uint8_t tbuf[10];
	uint8_t rbuf[2];
	uint8_t insn;
	int count;

	select_flash();

	switch (fi->current_mode)
	{
	case STANDARD:
		insn = MANUFACTURER_ID_DEVICE_ID;
		break;

	case DUAL_IO:
		/* unknown or bad mode */
		return -1;

	case QUAD_IO:
		/* unknown or bad mode */
		return -1;

	default:
		/* unknown or bad mode */
		return -1;
	}

	count = 0;
	assign_instruction(fi, tbuf, insn, &count);
	assign_address(fi, tbuf, 0, &count);

/*
	if (fi->current_mode == DUAL_IO || fi->current_mode == QUAD_IO)
		tbuf[count++] = CONTINUOUS_READ_MODE_OFF;
*/
	/* 2 dummies */
	if (fi->current_mode == QUAD_IO)
	{
#ifdef DUMMY_IS_ZERO
		tbuf[count++] = 0;
		tbuf[count++] = 0;
#else
#ifdef DUMMY_IS_ONE
		tbuf[count++] = 0xff;
		tbuf[count++] = 0xff;
#else
		count += 2;
#endif
#endif
	}

	spi_send(tbuf, count);

	spi_recv(rbuf, 2);
	*mid = rbuf[0];
	*did = rbuf[1];

	unselect_flash();

	return 0;
}

static int is25lp032d_read_uid(const struct flash_info *fi, uint64_t *uid)
{
	uint8_t tbuf[5];
	uint8_t rbuf[8];
	int count;

	/* Only for STANDARD mode */
	if (fi->current_mode != STANDARD)
		return -1;

	select_flash();

	count = 0;
	assign_instruction(fi, tbuf, READ_UNIQUE_ID, &count);
	/* 4 dummies */
#ifdef DUMMY_IS_ZERO
	tbuf[count++] = 0;
	tbuf[count++] = 0;
	tbuf[count++] = 0;
	tbuf[count++] = 0;
#else
#ifdef DUMMY_IS_ONE
	tbuf[count++] = 0xff;
	tbuf[count++] = 0xff;
	tbuf[count++] = 0xff;
	tbuf[count++] = 0xff;
#else
	count += 4;
#endif
#endif
	spi_send(tbuf, count);
	spi_recv(rbuf, 8);
	*uid = ((uint64_t) rbuf[0]) << 56;
	*uid |= ((uint64_t) rbuf[1]) << 48;
	*uid |= ((uint64_t) rbuf[2]) << 40;
	*uid |= ((uint64_t) rbuf[3]) << 32;
	*uid |= rbuf[4] << 24;
	*uid |= rbuf[5] << 16;
	*uid |= rbuf[6] << 8;
	*uid |= rbuf[7];

	unselect_flash();

	return 0;
}

static int is25lp032d_read_jedec_id(const struct flash_info *fi, uint8_t *mid,
		uint8_t *memory_type_id, uint8_t *capacity_id)
{
	uint8_t tbuf[1];
	uint8_t rbuf[3];
	int count;

	/* Only for STANDARD mode */
	if (fi->current_mode != STANDARD)
		return -1;

	select_flash();

	count = 0;
	assign_instruction(fi, tbuf, JEDEC_ID, &count);
	spi_send(tbuf, count);

	spi_recv(rbuf, 3);
	*mid = rbuf[0];
	*memory_type_id = rbuf[1];
	*capacity_id = rbuf[2];

	unselect_flash();

	return 0;
}

static int is25lp032d_read_status(const struct flash_info *fi, uint8_t *status)
{
	uint8_t tbuf[1];
	uint8_t rbuf[1];
	int count;

	/* Only for STANDARD mode */
	if (fi->current_mode != STANDARD)
		return -1;

	select_flash();

	count = 0;
	assign_instruction(fi, tbuf, READ_STATUS_REGISTER_1 , &count);
	spi_send(tbuf, count);

	spi_recv(rbuf, 1);
	*status = rbuf[0];

	unselect_flash();

	return 0;
}




static int is25lp032d_not_busy(const uint8_t status)
{
	return (status & STATUS1_BUSY) ? 0 : 1;
}

static int is25lp032d_wait_ready(const struct flash_info *fi)
{
	uint8_t tbuf[1];
	int count;

	/* Only for STANDARD mode */
	if (fi->current_mode != STANDARD)
		return -1;

	select_flash();

	count = 0;
	assign_instruction(fi, tbuf, READ_STATUS_REGISTER_1, &count);
	spi_send(tbuf, count);

	spi_recv_until(is25lp032d_not_busy);

	unselect_flash();

	return 0;
}

static int is25lp032d_write_status(const struct flash_info *fi, uint8_t status1)
{
	uint8_t tbuf[2];
	int count;
	uint8_t old_status1;

	/* Only for STANDARD mode */
	if (fi->current_mode != STANDARD)
		return -1;

	is25lp032d_write_enable(fi);

	is25lp032d_read_status(fi, &old_status1);
	if (!(old_status1 & STATUS1_WEL))
		return -1;

	select_flash();

	count = 0;
	assign_instruction(fi, tbuf, WRITE_STATUS_REGISTER, &count);
	tbuf[count++] = status1;
	spi_send(tbuf, count);

	unselect_flash();

	is25lp032d_wait_ready(fi);

	return 0;
}

static int is25lp032d_enable_quad_mode(const struct flash_info *fi)
{
	uint8_t status1;

	is25lp032d_read_status(fi, &status1);


	if (status1 & STATUS1_QE)
		return 0;

	status1 |= STATUS1_QE;

	is25lp032d_write_status(fi, status1);

	return 0;
}

static int is25lp032d_disable_quad_mode(const struct flash_info *fi)
{
	uint8_t status1;

	is25lp032d_read_status(fi, &status1);

	if (!(status1 & STATUS1_QE))
		return 0;

	status1 &= ~STATUS1_QE;

	is25lp032d_write_status(fi, status1);

	return 0;
}

static int is25lp032d_read(const struct flash_info *fi, uint32_t addr, uint8_t *buf, int size)
{
	uint8_t read_insn;
	uint8_t tbuf[20];
	int count;

	/* TODO frequency will limit instruction chosen.  */
	switch (fi->current_mode)
	{
	case STANDARD:
		read_insn = READ_DATA;
		/* for high frequency we need use FAST_READ.  */
		break;

	case DUAL_OUTPUT:
		read_insn = FAST_READ_DUAL_OUTPUT;
		break;

	case DUAL_IO:
		read_insn = FAST_READ_DUAL_IO;
		break;

	case QUAD_OUTPUT:
		read_insn = FAST_READ_QUAD_OUTPUT;
		break;

	case QUAD_IO:
		read_insn = FAST_READ_QUAD_IO;
		/* we should be able to choose WORD_READ_QUAD_IO
		   or OCTAL_WORD_READ_QUAD_IO in some cases. */
		break;

	default:
		/* bad read instruction */
		return -1;
	}

	select_flash();

	count = 0;
	assign_instruction(fi, tbuf, read_insn, &count);
	assign_address(fi, tbuf, addr, &count);
/*
	if (fi->current_mode == DUAL_IO || fi->current_mode == QUAD_IO)
		tbuf[count++] = CONTINUOUS_READ_MODE_OFF;
*/
	/* Add dummies */
	switch (read_insn)
	{
	case READ_DATA:
	case FAST_READ_DUAL_IO:
//	case OCTAL_WORD_READ_QUAD_IO:
	default:
		break;

	case FAST_READ:
//	case WORD_READ_QUAD_IO:
#ifdef DUMMY_IS_ZERO
		tbuf[count++] = 0;
#else
#ifdef DUMMY_IS_ONE
		tbuf[count++] = 0xff;
#else
		count += 1;
#endif
#endif
		break;

	case FAST_READ_DUAL_OUTPUT:
	case FAST_READ_QUAD_IO:
#ifdef DUMMY_IS_ZERO
		tbuf[count++] = 0;
		tbuf[count++] = 0;
#else
#ifdef DUMMY_IS_ONE
		tbuf[count++] = 0xff;
		tbuf[count++] = 0xff;
#else
		count += 2;
#endif
#endif
		break;

	case FAST_READ_QUAD_OUTPUT:
#ifdef DUMMY_IS_ZERO
		tbuf[count++] = 0;
		tbuf[count++] = 0;
		tbuf[count++] = 0;
		tbuf[count++] = 0;
#else
#ifdef DUMMY_IS_ONE
		tbuf[count++] = 0xff;
		tbuf[count++] = 0xff;
		tbuf[count++] = 0xff;
		tbuf[count++] = 0xff;
#else
		count += 4;
#endif
#endif
		break;
	}
	spi_send(tbuf, count);

	spi_recv(buf, size);

	unselect_flash();

	return 0;
}

static int is25lp032d_erase_1(const struct flash_info *fi,
		uint32_t addr, erase_type_t erase_type)
{
	uint8_t tbuf[4];
	int count;
	uint8_t status1;
	uint8_t insn;
	uint32_t addr_mask;

	/* Only for STANDARD mode */
	if (fi->current_mode != STANDARD)
		return -1;

	switch (erase_type)
	{
	case ERASE_4KB:
		insn = SECTOR_ERASE;
		addr_mask = 0xfffff000;
		break;

	case ERASE_32KB:
		insn = BLOCK_ERASE_32KB;
		addr_mask = 0xffff8000;
		break;

	case ERASE_64KB:
		insn = BLOCK_ERASE_64KB;
		addr_mask = 0xffff0000;
		break;

	case ERASE_CHIP:
		insn = CHIP_ERASE;
		addr_mask = 0;
		break;

	default:
		/* bad erase type */
		return -1;
	}

	is25lp032d_write_enable(fi);

	is25lp032d_read_status(fi, &status1);
	if (!(status1 & STATUS1_WEL))
		return -1;

	select_flash();

	count = 0;
	assign_instruction(fi, tbuf, insn, &count);
	if (erase_type != ERASE_CHIP)
		assign_address(fi, tbuf, addr & addr_mask, &count);
	spi_send(tbuf, count);

	unselect_flash();

	is25lp032d_wait_ready(fi);

	return 0;
}

static int is25lp032d_erase(const struct flash_info *fi, uint32_t addr, int size)
{
	size += addr % BLOCK_SIZE;
	addr -= addr % BLOCK_SIZE;

	while (size > 0)
	{
		is25lp032d_erase_1(fi, addr, ERASE_TYPE);
		addr += BLOCK_SIZE;
		size -= BLOCK_SIZE;
	}

	return 0;
}

static int is25lp032d_erase_chip(const struct flash_info *fi)
{
	return is25lp032d_erase_1(fi, 0, ERASE_CHIP);
}

static int is25lp032d_page_program(struct flash_info *fi,
		uint32_t addr, const uint8_t *buf, int size)
{
	uint8_t tbuf[16 + size];
	uint8_t insn;
	int count;
	spi_mode_t old_mode;
	uint8_t old_status1;

	if ((addr % PAGE_SIZE) + size > PAGE_SIZE)
		return -1;

	old_mode = fi->current_mode;

	flash_set_mode(fi, STANDARD);

	is25lp032d_write_enable(fi);

	is25lp032d_read_status(fi, &old_status1);
	if (!(old_status1 & STATUS1_WEL))
		return -1;

	flash_set_mode(fi, old_mode);

	select_flash();

	switch (fi->current_mode)
	{
	case STANDARD:
		insn = PAGE_PROGRAM;
		break;

	case QUAD_INPUT:
		insn = QUAD_PAGE_PROGRAM;
		break;

	default:
		return -1;
	}

	count = 0;
	assign_instruction(fi, tbuf, insn, &count);
	assign_address(fi, tbuf, addr, &count);
	memcpy(tbuf + count, buf, size);
	spi_send(tbuf, count + size);

	unselect_flash();

	old_mode = fi->current_mode;

	flash_set_mode(fi, STANDARD);

	is25lp032d_wait_ready(fi);

	flash_set_mode(fi, old_mode);

	return 0;
}

/* Program arbitrary size */
static int is25lp032d_program(struct flash_info *fi,
		uint32_t addr, const uint8_t *buf, int size)
{
	int program_size;

	while (size > 0)
	{
		if (addr % PAGE_SIZE != 0)
			program_size = ((PAGE_SIZE - addr % PAGE_SIZE) < size
					? (PAGE_SIZE - addr % PAGE_SIZE) : size);
		else if (size >= PAGE_SIZE)
			program_size = PAGE_SIZE;
		else
			program_size = size;

		is25lp032d_page_program(fi, addr, buf, program_size);

		addr += program_size;
		buf += program_size;
		size -= program_size;
	}

	return 0;
}

static int is25lp032d_reset(const struct flash_info *fi)
{
	/* Not supported */
	return -1;
}

struct flash_info is25lp032d_info =
{
		"IS25LP032D",	/* name */
		"ISSI",	/* mname */
		0x9D,		/* manufacturer ID */
		0x15,		/* device ID */
		0x60,		/* memory type ID */
		0x16,		/* capacity ID */
	STANDARD,// | DUAL_OUTPUT | QUAD_OUTPUT | DUAL_IO | DUAL_OUTPUT, /* supported modes */


	/* The following three fields are better be
	   initialized in flash_open functions.  */
	0,			/* size in Byte */
	0,			/* number_of_regions */
	NULL,		/* erase_block_regions */
	104,		/* max_freq */
	80,			/* max_quad_freq */
	50,			/* max_read_data_freq */

	0,			/* cpha */
	0,			/* cpol */
	0,			/* lsb_first */
	0,			/* start_on_mosi */

	0,			/* frequency */
	STANDARD,	/* current_mode */

	is25lp032d_open,
	is25lp032d_close,

	is25lp032d_read_mid_did,
	is25lp032d_read_uid,
	is25lp032d_read_jedec_id,
	is25lp032d_read_status,
	0,
	is25lp032d_write_status,
	is25lp032d_enable_quad_mode,
	is25lp032d_disable_quad_mode,
	is25lp032d_read,
	is25lp032d_write_enable,
	is25lp032d_write_disable,
	is25lp032d_erase,
	is25lp032d_erase_chip,
	is25lp032d_program,
	is25lp032d_reset,
};
