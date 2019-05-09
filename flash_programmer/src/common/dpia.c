/*********************************************************************************

Copyright(c) 2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

 *********************************************************************************/

/*
 * This examples demonstrates how to interface to a flash device
 * from the flash programmer.
 *
 * This is meant only as an example and may not be fully optimized
 * to access flash as efficiently as possible.
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#include <sys/platform.h>

#include "spi.h"

#include "adi_initialize.h"
#include "flash_errors.h"
#include "flash.h"
#include "dpia.h"


/*
 * The buffer size can be altered to increase performance provided the heap
 * is large enough.
 */

#define BUFFER_SIZE			0x800

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* Flash programmer commands */
typedef enum
{
	FLASH_NO_COMMAND,		/* 0 */
	FLASH_GET_CODES,		/* 1 */
	FLASH_RESET,			/* 2 */
	FLASH_WRITE,			/* 3 */
	FLASH_FILL,				/* 4 */
	FLASH_ERASE_ALL,		/* 5 */
	FLASH_ERASE_SECT,		/* 6 */
	FLASH_READ,				/* 7 */
	FLASH_GET_SECTNUM,		/* 8 */
	FLASH_GET_SECSTARTEND,	/* 9 */
}enProgCmds;

/* structure for flash sector information */
typedef struct _SECTORLOCATION
{
	unsigned long ulStartOff;
	unsigned long ulEndOff;
}SECTORLOCATION;

/* Globals */
const char 		*AFP_Title ;							/* EzKit info */
const char 		*AFP_Description;						/* Device Description */
const char		*AFP_DeviceCompany;						/* Device Company */
char 			*AFP_DrvVersion		= "1.00.0";			/* Driver Version */
char			*AFP_BuildDate		= __DATE__;			/* Driver Build Date */
enProgCmds 		AFP_Command 		= FLASH_NO_COMMAND;	/* command sent down from the programmer */
int 			AFP_ManCode 		= -1;				/* 0x20 = Numonyx */
int 			AFP_DevCode 		= -1;				/* 0x15 = w25q32bv */
unsigned long 	AFP_Offset 			= 0x0;				/* offset into flash */
int 			*AFP_Buffer;							/* buffer used to read and write flash */
long 			AFP_Size 			= BUFFER_SIZE;		/* buffer size */
long 			AFP_Count 			= -1;				/* count of locations to be read or written */
long 			AFP_Stride 			= -1;				/* stride used when reading or writing */

/* The size of value in bytes (1, 2, or 4).  Generally it can be any size, like 3 or 5, but
 * 1, 2, 4 should be the most useful and 4-byte data is the largest value can be passed in
 * as the fill data value.  Normally stride should be larger than or equal to the value size.
 * But to be compatible with the old implementation, value size is set to 2 by default.
 * So we just ignore the value size and treat it as 1 if the stride is 1.  If the stride is
 * larger than 1, it should be also larger than or equal to the value size.  */
int				AFP_ValueSize		= 2;

														/* ignored when it's larger than AFP_Stride */
int 			AFP_NumSectors 		= -1;				/* number of sectors in the flash device */
int 			AFP_Sector 			= -1;				/* sector number */
int 			AFP_Error 			= NO_ERR;			/* contains last error encountered */
bool			AFP_Verify 			= FALSE;			/* verify writes or not */
unsigned long 	AFP_StartOff 		= 0x0;				/* sector start offset */
unsigned long 	AFP_EndOff 			= 0x0;				/* sector end offset */
int				AFP_FlashWidth		= 0x8;				/* width of the flash device */
int 			*AFP_SectorInfo;

/* Locals */
static bool bExit = FALSE;
static SECTORLOCATION *pSectorInfo;

/* external functions */
#ifdef USE_SOFT_SWITCHES
extern void ConfigSoftSwitches(void);
#endif

/* internal functions */
static ERROR_CODE GetFlashInfo(void);


static ERROR_CODE GetNumSectors(void);
static ERROR_CODE AllocateAFPBuffer(void);
static void FreeAFPBuffer(void);
static ERROR_CODE GetSectorMap(SECTORLOCATION *pSectInfo);

static ERROR_CODE ProcessCommand(void);
static ERROR_CODE FillData(unsigned long ulStart, long lCount, long lStride, int *pnData, int ValueSize);
static ERROR_CODE ReadData(unsigned long ulStart, long lCount, long lStride, int *pnData, int ValueSize);
static ERROR_CODE WriteData(unsigned long ulStart, long lCount, long lStride, int *pnData, int ValueSize);

#ifndef SPI_NO
#define SPI_NO 2
#endif

static const char *pEzKitTitle = "ADSP-BF707 EZ-Board";

extern const struct spi_ctlr adi_spi_bf6xx_ctlr;
extern struct flash_info is25lp032d_info;

//#define SPI2_CLK_PORTB_MUX  ((uint16_t) ((uint16_t) 0<<20))
//#define SPI2_MISO_PORTB_MUX  ((uint16_t) ((uint16_t) 0<<22))
//#define SPI2_MOSI_PORTB_MUX  ((uint16_t) ((uint16_t) 0<<24))
//#define SPI2_D2_PORTB_MUX  ((uint16_t) ((uint16_t) 0<<26))
//#define SPI2_D3_PORTB_MUX  ((uint16_t) ((uint16_t) 0<<28))
//
//#define SPI2_CLK_PORTB_FER  ((uint16_t) ((uint16_t) 1<<10))
//#define SPI2_MISO_PORTB_FER  ((uint16_t) ((uint16_t) 1<<11))
//#define SPI2_MOSI_PORTB_FER  ((uint16_t) ((uint16_t) 1<<12))
//#define SPI2_D2_PORTB_FER  ((uint16_t) ((uint16_t) 1<<13))
//#define SPI2_D3_PORTB_FER  ((uint16_t) ((uint16_t) 1<<14))
//
//
//static void dpia_init(void)
//{
//
//
//    /* PORTx_MUX registers */
//    *pREG_PORTB_MUX = SPI2_CLK_PORTB_MUX | SPI2_MISO_PORTB_MUX
//     | SPI2_MOSI_PORTB_MUX | SPI2_D2_PORTB_MUX | SPI2_D3_PORTB_MUX;
//
//    /* PORTx_FER registers */
//    *pREG_PORTB_FER = SPI2_CLK_PORTB_FER | SPI2_MISO_PORTB_FER
//     | SPI2_MOSI_PORTB_FER | SPI2_D2_PORTB_FER | SPI2_D3_PORTB_FER;
//
//	/* We use GPIO PB15 as SPI flash /CS. */
//	*pREG_PORTB_FER_CLR = BITM_PORT_DATA_PX15;
//	*pREG_PORTB_DIR_SET = BITM_PORT_DATA_PX15;
//}



/**
 *****************************************************************************
 * Program entry point
 *
 * @return				True of False depending on if the function is
 * 						successful
 */
int main(void)
{
	uint32_t Result;

	/* Do board specific initialization */
	//dpia_init();
	spi_ctlr = &adi_spi_bf6xx_ctlr;
	flash_info = &is25lp032d_info;

	Result = adi_initComponents();
	if (Result == 0)
	{
#ifdef USE_SOFT_SWITCHES
		ConfigSoftSwitches();
#endif
	}
	else
	{
		return FALSE;
	}
	*pREG_PORTB_FER_CLR = BITM_PORT_DATA_PX15;
	*pREG_PORTB_DIR_SET = BITM_PORT_DATA_PX15;





	//cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "TEST PRINT \n");

	Result = spi_open(SPI_NO);
	if (Result != 0)
		return FALSE;

	unselect_flash();

	spi_config(flash_info);
	flash_open(flash_info);

	flash_set_mode(flash_info, STANDARD);

	if (flash_info->modes & (QUAD_INPUT | QUAD_OUTPUT | QUAD_IO))
		flash_enable_quad_mode(flash_info);

	/* get flash manufacturer & device codes, title & desc */
	if( AFP_Error == NO_ERR )
	{
		AFP_Error = GetFlashInfo();
	}

	/* get the number of sectors for this device */
	if( AFP_Error == NO_ERR )
	{
		AFP_Error = GetNumSectors();
	}

	if( AFP_Error == NO_ERR )
	{
		/* malloc enough space to hold our start and end offsets */
		pSectorInfo = (SECTORLOCATION *)malloc(AFP_NumSectors * sizeof(SECTORLOCATION));
	}

	/* allocate AFP_Buffer */
	if( AFP_Error == NO_ERR )
	{
		AFP_Error = AllocateAFPBuffer();
	}

	/* get sector map */
	if( AFP_Error == NO_ERR )
	{
		AFP_Error = GetSectorMap(pSectorInfo);
	}

	/* point AFP_SectorInfo to our sector info structure */
	if( AFP_Error == NO_ERR )
	{
		AFP_SectorInfo = (int*)pSectorInfo;
	}

	/* command processing loop */
	while ( !bExit )
	{
		/*
		 * the programmer will set a breakpoint at "AFP_BreakReady" so it knows
		 * when we are ready for a new command because the processor will halt
		 *
		 * the jump is used so that the label will be part of the debug
		 * information in the driver image otherwise it may be left out
		 * since the label is not referenced anywhere
		 */
		asm("AFP_BreakReady:");
		asm("nop;");
		if ( FALSE )
			asm("jump AFP_BreakReady;");

		/* Make a call to the ProcessCommand */
		AFP_Error = ProcessCommand();
	}

	/* Clear the AFP_Buffer */
	FreeAFPBuffer();

	if( pSectorInfo )
	{
		free(pSectorInfo);
		pSectorInfo = 0;
	}

	/* Close the Device */
	//	AFP_Error = w25q32bv_Close();

	if (AFP_Error != NO_ERR)
		return FALSE;

	return TRUE;
}

/**
 *****************************************************************************
 * Get the manufacturer code and device code
 *
 * @return				value if any error occurs getting flash info
 */
static ERROR_CODE GetFlashInfo(void)
{
	uint8_t mid;
	uint8_t did;
	int result;

	result = flash_read_mid_did(flash_info, &mid, &did);
	if (result == 0)
	{
		AFP_ManCode = mid;
		AFP_DevCode = did;

		if (mid != flash_info->mid || did != flash_info->did)
			return SETUP_ERROR;
	}
	else
	{
		uint8_t mtype;
		uint8_t mcap;

		result = flash_read_jedec_id(flash_info, &mid, &mtype, &mcap);
		if (result)
			return SETUP_ERROR;

		AFP_ManCode = mid;
		AFP_DevCode = mtype << 8 | mcap;
		if (mid != flash_info->mid
				|| mtype != flash_info->memory_type_id
				|| mcap != flash_info->capacity_id)
			return SETUP_ERROR;
	}

	AFP_Title = pEzKitTitle;
	flash_name(flash_info, &AFP_Description);
	flash_company(flash_info, &AFP_DeviceCompany);

	return NO_ERR;
}

/**
 *****************************************************************************
 * Get the number of sectors for this device.
 *
 * @return				value if any error occurs getting number of sectors
 */
static ERROR_CODE GetNumSectors(void)
{
	flash_num_of_sectors(flash_info, &AFP_NumSectors);
	return NO_ERR;
}

/**
 *****************************************************************************
 * Allocate memory for the AFP_Buffer
 *
 * @return				value if any error occurs allocating memory
 */
static ERROR_CODE AllocateAFPBuffer(void)
{

	ERROR_CODE ErrorCode = NO_ERR;	//return error code

	/*
	 * by making AFP_Buffer as big as possible the plug-in can send and
	 * receive more data at a time making the data transfer quicker
	 *
	 * by allocating it on the heap the compiler does not create an
	 * initialized array therefore making the driver image smaller
	 * and faster to load
	 *
	 * The linker description file (LDF) could be modified so that
	 * the heap is larger, therefore allowing the BUFFER_SIZE to increase.
	 *
	 */

	/*
	 * the data type of the data being sent from the programmer
	 * is in bytes but we store the data as integers to make data
	 * manipulation easier when actually programming the data.  This is why
	 * BUFFER_SIZE bytes are being allocated rather than BUFFER_SIZE * sizeof(int).
	 */
	AFP_Buffer = malloc(BUFFER_SIZE);

	/* AFP_Buffer will be NULL if we could not allocate storage for the buffer */
	if ( AFP_Buffer == 0 )
	{
		/* tell programmer that our buffer was not initialized */
		ErrorCode = BUFFER_IS_NULL;
	}

	return(ErrorCode);
}

/**
 *****************************************************************************
 * Free the AFP_Buffer
 */
static void FreeAFPBuffer(void)
{
	/* free the buffer if we were able to allocate one */
	if ( AFP_Buffer )
		free( AFP_Buffer );

}

/**
 *****************************************************************************
 * Get the start and end offset for each sector in the flash.
 *
 * @param	pSectInfo	pointer to the SECTORLOCATION struct
 *
 * @return				value if any error occurs getting sector map
 */
static ERROR_CODE GetSectorMap(SECTORLOCATION *pSectInfo)
{
	int i;

	/* initiate sector information structures */
	for(i = 0; i < AFP_NumSectors; i++)
	{
		uint32_t start, end;
		flash_sector_start_end(flash_info, i, &start, &end);
		pSectInfo[i].ulStartOff = start;
		pSectInfo[i].ulEndOff = end;
	}

	return NO_ERR;
}

/**
 *****************************************************************************
 * Process each command sent by the programmer based on the value
 * in AFP_Command
 *
 * @return				any error trying to process a command
 */
static ERROR_CODE ProcessCommand(void)
{
	ERROR_CODE ErrorCode = NO_ERR;

	switch (AFP_Command)
	{
	/* erase all */
	case FLASH_ERASE_ALL:
	{
		int result;
		result = flash_erase_chip(flash_info);
		if (result) ErrorCode = PROCESS_COMMAND_ERR;
		//cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "FLASH_ERASE_ALL\n");
		break;
	}
	/* erase sector */
	case FLASH_ERASE_SECT:
	{
		int result;
		result = flash_erase_sector(flash_info, AFP_Sector);
		if (result) ErrorCode = PROCESS_COMMAND_ERR;
		//cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "FLASH_ERASE_SECT\n");
		break;
	}
	/* fill */
	case FLASH_FILL:
		ErrorCode = FillData(AFP_Offset, AFP_Count, AFP_Stride, AFP_Buffer, AFP_ValueSize);
		//cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "FLASH_FILL\n");
		break;

		/* get manufacturer and device codes */
	case FLASH_GET_CODES:
		/* AFP_ManCode and AFP_DevCode should have already been initialized */
		break;

		/* get sector number based on address */
	case FLASH_GET_SECTNUM:
	{
		int result;
		result = flash_sector_number(flash_info, AFP_Offset, &AFP_Sector);
		if (result) ErrorCode = PROCESS_COMMAND_ERR;
		//cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "FLASH_GET_SECTNUM\n");
		break;
	}
	/* get sector number start and end offset */
	case FLASH_GET_SECSTARTEND:
	{
		uint32_t start, end;
		int result;
		result = flash_sector_start_end(flash_info, AFP_Sector, &start, &end);
		if (result)
		{
			ErrorCode = INVALID_SECTOR;
		}
		else
		{
			AFP_StartOff = start;
			AFP_EndOff = end;
		}
		//cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "FLASH_GET_SECSTARTEND\n");
		break;
	}
	/* read */
	case FLASH_READ:
		ErrorCode = ReadData(AFP_Offset, AFP_Count, AFP_Stride, AFP_Buffer, AFP_ValueSize);
		//cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "FLASH_READ\n");
		break;
		/* reset */
	case FLASH_RESET:
	{
		int result;
		result = flash_reset(flash_info);
		if (result) ErrorCode = PROCESS_COMMAND_ERR;
		//cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "FLASH_RESET\n");
		break;
	}
	/* write */
	case FLASH_WRITE:
		ErrorCode = WriteData(AFP_Offset, AFP_Count, AFP_Stride, AFP_Buffer, AFP_ValueSize);
		//cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "FLASH_WRITE");
		break;

		/* no command or unknown command do nothing */
	case FLASH_NO_COMMAND:
	default:
		/* set our error */

		ErrorCode = UNKNOWN_COMMAND;
		break;
	}

	/* clear the command */
	AFP_Command = FLASH_NO_COMMAND;

	return(ErrorCode);
}

/**
 *****************************************************************************
 * Fill flash device with a value.
 *
 * @param	ulStart		Address in flash to start the writes at
 * @param	lCount		Number of elements to write, in this case bytes
 * @param	lStride		Number of locations to skip between writes
 * @param	*pnData		Pointer to data buffer(written to by the programmer)
 * @param	ValueSize	Size of the value in bytes (1, 2, or 4)
 *
 * @return				value if any error occurs during fill
 */
static ERROR_CODE FillData(unsigned long ulStart, long lCount, long lStride, int* pnData, int ValueSize)
{
	/* Save value so we can reuse the buffer instead of allocate another buffer.
	   This should be helpful for processors with small internal memory.  */
	int value = *pnData;
	uint8_t *buf = (uint8_t *) pnData;
	uint32_t addr = ulStart;
	long BufferCount = BUFFER_SIZE / ValueSize;
	long i;
	ERROR_CODE Result;

	/* See the comment for AFP_ValueSize.  */

	if (lStride == 1)
		ValueSize = 1;

	if (lStride < ValueSize)
		return WRITE_ERROR;

	if (ValueSize != 1 && ValueSize != 2 && ValueSize != 4)
		return WRITE_ERROR;

	/* Fill the buffer */
	for (i = 0; i < BufferCount; i++)
	{
		if (ValueSize == 1)
			buf[i] = value & 0xff;
		else if (ValueSize == 2)
		{
			buf[i * 2] = value & 0xff;
			buf[i * 2 + 1] = (value >> 8) & 0xff;
		}
		else /* ValueSize == 4 */
		{
			buf[i * 4] = value & 0xff;
			buf[i * 4 + 1] = (value >> 8) & 0xff;
			buf[i * 4 + 2] = (value >> 16) & 0xff;
			buf[i * 4 + 3] = (value >> 24) & 0xff;
		}
	}

	while (lCount > 0)
	{
		long c = lCount > BufferCount ? BufferCount : lCount;
		Result = WriteData(addr, c, lStride, (int *) buf, ValueSize);
		if (Result != NO_ERR) return Result;
		lCount -= c;
		addr += c * lStride;
	}
	return NO_ERR;
}


/**
 *****************************************************************************
 * Write a buffer to flash device.
 *
 * @param	ulStart		Address in flash to start the writes at
 * @param	lCount		Number of elements to write, in this case bytes
 * @param	lStride		Number of locations to skip between writes
 * @param	*pnData		Pointer to data buffer(written to by the programmer)
 * @param	ValueSize	Size of the value in bytes (1, 2, or 4)
 *
 * @return				value if any error occurs during write
 */
static ERROR_CODE WriteData(unsigned long ulStart, long lCount, long lStride, int *pnData, int ValueSize)
{
	int result;
	uint32_t addr;
	uint8_t *buf = (uint8_t *) pnData;

	/* See the comment for AFP_ValueSize.  */
	//cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "ulStart : %d, lCount : %d, lStride : %d, ValueSize : %d\n\r", ulStart, lCount, lStride, ValueSize);
	if (lStride == 1)
		ValueSize = 1;

	if (lStride < ValueSize)
		return WRITE_ERROR;

	if (ValueSize != 1 && ValueSize != 2 && ValueSize != 4)
		return WRITE_ERROR;

	/* Make sure the buffer is large enough.  */
	if (ValueSize * lCount > BUFFER_SIZE)
		return WRITE_ERROR;

	addr = ulStart;

	if (lStride == 1)
	{
		/* Valid modes: STANDARD or QUAD_INPUT */
		if (flash_info->modes & QUAD_INPUT)
			flash_set_mode(flash_info, QUAD_INPUT);

		result = flash_program(flash_info, addr, buf, lCount);
	}
	else
	{
		long i;

		/* Valid modes: STANDARD or QUAD_INPUT */
		flash_set_mode(flash_info, STANDARD);

		for (i = 0; i < lCount; i++)
		{
			result = flash_program(flash_info, addr, buf + i * ValueSize, ValueSize);
			if (result) break;
			addr += lStride;
		}
	}
	//cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "\n\r RESULT: %d, AFP_Verify:%d\n\r",result,AFP_Verify);

	if (result == 0 && AFP_Verify == TRUE)
	{
		/* Use a small buffer to reduce memory usage.  */

		uint8_t buf2[4];
		ERROR_CODE ErrorCode;
		long i;

		addr = ulStart;
		for (i = 0; i < lCount; i++)
		{
			result = flash_read(flash_info, addr, buf2, ValueSize);
			//cld_console(CLD_CONSOLE_BLACK, CLD_CONSOLE_GREEN, "FLASH READ");
			if (result)
				return NOT_READ_ERROR;
			if (memcmp (buf, buf2, ValueSize))
				return VERIFY_WRITE;
			ulStart += lStride;
			buf += ValueSize;
		}
	}

	return (result ? WRITE_ERROR : NO_ERR);
}


/**
 *****************************************************************************
 * Read a buffer from flash device.
 *
 * @param	ulStart		Address in flash to start the reads at
 * @param	lCount		Number of elements to read, in this case bytes
 * @param	lStride		Number of locations to skip between reads
 * @param	*pnData		Pointer to data buffer to fill
 * @param	ValueSize	Size of the value in bytes (1, 2, or 4)
 *
 * @return				value if any error occurs during reading
 */
static ERROR_CODE ReadData(unsigned long ulStart, long lCount, long lStride, int *pnData, int ValueSize)
{
	int result;
	uint8_t *buf = (uint8_t *)pnData;

	/* See the comment for AFP_ValueSize.  */
	if (lStride == 1)
		ValueSize = 1;

	if (lStride < ValueSize)
		return NOT_READ_ERROR;

	if (ValueSize != 1 && ValueSize != 2 && ValueSize != 4)
		return NOT_READ_ERROR;

	/* Make sure the buffer is large enough.  */
	if (ValueSize * lCount > BUFFER_SIZE)
		return NOT_READ_ERROR;

	if (lStride == 1)
	{
		/* Valid modes: STANDARD, DUAL_OUTPUT, DUAL_IO, QUAD_OUTPUT, QUAD_IO */
		if (flash_info->modes & QUAD_OUTPUT)
			flash_set_mode(flash_info, QUAD_OUTPUT);
		result = flash_read(flash_info, ulStart, buf, lCount);
	}
	else
	{
		long i;

		flash_set_mode(flash_info, STANDARD);
		for (i = 0; i < lCount; i++)
		{
			result = flash_read(flash_info, ulStart, buf, ValueSize);
			if (result)
				break;
			ulStart += lStride;
			buf += ValueSize;
		}
	}

	return (result ? NOT_READ_ERROR : NO_ERR);
}

