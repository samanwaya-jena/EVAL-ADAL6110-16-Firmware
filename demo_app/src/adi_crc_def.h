/*********************************************************************************

Copyright(c) 2016 Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/

/*!
 * @file:    adi_crc_def.h
 * @brief:   Private header file for for CRC driver.
 * @version: $Revision: 25455 $
 * @date:    $Date: 2016-03-04 10:51:13 -0500 (Fri, 04 Mar 2016) $
 *
 * @details
 *           This is a private header file for the CRC driver,
 *           which contains the API declarations, data and
 *           constant definitions used in driver implementation
 */

#ifndef __ADI_CRC_DEF_H__
#define __ADI_CRC_DEF_H__

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_5_1:"Allow identifiers to be more than 31 characters")
#pragma diag(suppress:misra_rule_5_3:" ")
#pragma diag(suppress:misra_rule_5_4:" ")
#pragma diag(suppress:misra_rule_5_7:"Allow identifier names to be reused")
#pragma diag(suppress:misra_rule_18_4:" ")
#pragma diag(suppress:misra_rule_19_14:"Allow #if")
#endif /* _MISRA_RULES */

/*=============  I N C L U D E S   =============*/

/* CRC Driver includes */
#include <drivers/crc/adi_crc.h>
/* memcpy includes */
#include <string.h>
/* Memory size check */
#include <assert.h>

/*==============  D E F I N E S  ===============*/

/* ---------------
       CRC_CTL
   --------------- */

/* Bit Mask for data mirror configuration fields */
#define BITM_CRC_CTL_MIRR_DATA              (BITM_CRC_CTL_W16SWP | BITM_CRC_CTL_BYTMIRR | BITM_CRC_CTL_BITMIRR)
/* Bit Mask for register mirror configuration fields */
#define BITM_CRC_CTL_MIRR_REG               (BITM_CRC_CTL_FDSEL | BITM_CRC_CTL_RSLTMIRR | BITM_CRC_CTL_POLYMIRR | BITM_CRC_CTL_CMPMIRR)

/* Enumeration of CRC Control register field configuration values */
#define ENUM_CRC_CTL_CMPMIRR_EN             0x00480000  /* CMPMIRR: Enable COMPARE Register mirroring */
#define ENUM_CRC_CTL_POLYMIRR_EN            0x00280000  /* POLYMIRR: Enable Polynomial Register mirroring */
#define ENUM_CRC_CTL_RSLTMIRR_EN            0x00180000  /* RSLTMIRR: Enable Result Register mirroring */
#define ENUM_CRC_CTL_FDSEL_EN               0x00080000  /* FDSEL: Write modified data to FIFO */
#define ENUM_CRC_CTL_W16SWP_EN              0x00040000  /* W16SWP: Enable word16 swapping */
#define ENUM_CRC_CTL_BYTMIRR_EN             0x00020000  /* BYTMIRR: Enable Byte Mirroring */
#define ENUM_CRC_CTL_BITMIRR_EN             0x00010000  /* BITMIRR: Enable Bit Mirroring */
#define ENUM_CRC_CTL_IRRSTALL_EN            0x00002000  /* IRRSTALL: Enable Stall on IRR */
#define ENUM_CRC_CTL_OBRSTALL_EN            0x00001000  /* OBRSTALL: Enable Stall on OBR */
#define ENUM_CRC_CTL_AUTOCLRF_EN            0x00000200  /* AUTOCLRF: Enable Auto Clear to One */
#define ENUM_CRC_CTL_AUTOCLRZ_EN            0x00000100  /* AUTOCLRZ: Enable Auto Clear to Zero */

#define ENUM_CRC_CTL_OPMODE_RESERVED        0x00000010  /* OPMODEP: CRC compute/compare memory transfer */
#define ENUM_CRC_CTL_OPMODE_XFER            0x00000010  /* OPMODEP: CRC compute/compare memory transfer */
#define ENUM_CRC_CTL_OPMODE_FILL            0x00000020  /* OPMODEP: Data fill memory transfer */
#define ENUM_CRC_CTL_OPMODE_SCAN            0x00000030  /* OPMODEP: CRC compute/compare memory scan */
#define ENUM_CRC_CTL_OPMODE_VERIFY          0x00000040  /* OPMODEP: Data verify memory scan */

#define ENUM_CRC_CTL_BLKEN_EN               0x00000001  /* BLKEN: Block Enable */

/* ---------------
    CRC_INEN_SET
   --------------- */
/* Enumeration of CRC Interrupt Enable set register field configuration values */
#define ENUM_CRC_INEN_SET_DCNTEXP_SET       0x00000010  /* DCNTEXP: Data Count Expired (Status) Interrupt Enable Set */
#define ENUM_CRC_INEN_SET_CMPERR_SET        0x00000002  /* CMPERR: Compare Error Interrupt Enable Set */

/* ---------------
    CRC_INEN_CLR
   --------------- */
/* Enumeration of CRC Interrupt Enable clear register field configuration values */
#define ENUM_CRC_INEN_CLR_DCNTEXP_CLR       0x00000010  /* DCNTEXP: Data Count Expired (Status) Interrupt Enable Clear */
#define ENUM_CRC_INEN_CLR_CMPERR_CLR        0x00000002  /* CMPERR: Compare Error Interrupt Enable Clear */

/* ---------------
       CRC_STAT
   --------------- */
#define ENUM_CRC_STAT_FSTAT                 0x00700000  /* FSTAT: FIFO Status */
#define ENUM_CRC_STAT_LUTDONE               0x00080000  /* LUTDONE: Look Up Table Done */
#define ENUM_CRC_STAT_IRR                   0x00040000  /* IRR: Intermediate Result Ready */
#define ENUM_CRC_STAT_OBR                   0x00020000  /* OBR: Output Buffer Ready */
#define ENUM_CRC_STAT_IBR                   0x00010000  /* IBR: Input Buffer Ready */
#define ENUM_CRC_STAT_DCNTEXP               0x00000010  /* DCNTEXP: Data Count Expired */
#define ENUM_CRC_STAT_CMPERR                0x00000002  /* CMPERR: Compare Error */

/* Clear CRC interrupts */
#define ADI_CRC_CLEAR_INTERRUPTS           (ENUM_CRC_INEN_CLR_DCNTEXP_CLR | ENUM_CRC_INEN_CLR_CMPERR_CLR)

#if defined (WA_19000009) && WA_19000009
#define CRC_LUT_REG_OFFSET  (0x0080)
#define pINT_REG            ((volatile int*)(0x20005010))
#endif
/* Enumeration of CRC operation status */
typedef enum
{

    ADI_CRC_OP_IDLE                = 0u,        /* CRC idle */
    ADI_CRC_OP_IN_PROGRESS         = 0x01u,     /* CRC operation in progress */
    ADI_CRC_OP_DATA_COUNT_EXPIRED  = 0x02u,     /* CRC Data count expired */
    ADI_CRC_OP_DMA_PROCESSED       = 0x04u,     /* DMA Data buffer processed */
    ADI_CRC_OP_COMPLETE            = 0x07u,     /* CRC operation complete */
    ADI_CRC_OP_DMA_ERROR           = 0x08u,     /* DMA Error */
    ADI_CRC_OP_CRC_COMPARE_ERROR   = 0x10u      /* CRC Data compare error */

} ADI_CRC_OP_STATUS;

/*
 * A section of the structure used to handle MDMA stream
 * This structure will be used to update the MDMA stream 'bIsCrcMode' field
 * The field is used to indicate if the MDMA stream is in CRC mode or in MDMA mode.
 */
#pragma pack(4)
typedef struct
{
    bool                    bIsMemDma;              /* Always FALSE as this is not a Memory DMA channel */
    bool                    bIsXferInProgress;      /* TRUE when a DMA data transfer is currently in progress */
    ADI_CALLBACK            pfCallback;             /* Client supplied callback function */
    void                    *pCBParam;              /* Client supplied callback parameter */
    void                    *pfDataIntHandle;       /* Function to handle DMA data transfer interrupts for channel that allow interrupts */
    bool                    bIsCrcMode;             /* TRUE when the DMA stream is in CRC mode - Field set inside the CRC driver */
} ADI_CRC_MDMA_STREAM;
#pragma pack()

#pragma pack(4)
/* CRC peripheral register structure */
typedef struct
{
    volatile uint32_t   Control;            /* Control Register */
    volatile uint32_t   DataCount;          /* Data Word Count Register */
    volatile uint32_t   ReloadDataCount;    /* Data Word Count Reload Register */
    uint32_t            Padding0[2];        /* Cover the register gap */
    volatile uint32_t   DataCompare;        /* Data Compare Register */
    volatile uint32_t   FillValue;          /* Fill Value Register */
    volatile uint32_t   DataFifo;           /* Data FIFO Register */
    volatile uint32_t   IntEn;              /* Interrupt Enable Register */
    volatile uint32_t   IntEnSet;           /* Interrupt Enable Set Register */
    volatile uint32_t   IntEnClear;         /* Interrupt Enable Clear Register */
    volatile uint32_t   Polynomial;         /* Polynomial Register */
    uint32_t            Padding1[4];        /* Cover the register gap */
    volatile uint32_t   Status;             /* Status Register */
    volatile uint32_t   CountCapture;       /* Data Count Capture Register */
    uint32_t            Padding2;           /* Cover the register gap */
    volatile uint32_t   FinalResult;        /* CRC Final Result Register */
    volatile uint32_t   CurrentResult;      /* CRC Current Result Register */
    uint32_t            Padding3[3];        /* Cover the register gap */
    volatile uint32_t   RevId;              /* Revision ID Register */
} ADI_CRC_REGS;
#pragma pack()

/* Structure to handle CRC Peripheral instance */
typedef struct
{
    bool                    bIsDmaMode;         /* TRUE for DMA driven CRC mode, FALSE for core driven CRC */
    ADI_CRC_OP_STATUS       eCrcOpStatus;       /* Current status of the CRC Operation */
    ADI_CRC_MODE            eCrcMode;           /* CRC operating mode */
    ADI_CALLBACK            pfCallback;         /* Client supplied callback function */
    void                    *pCBParam;          /* Client supplied callback parameter */
    uint32_t                ControlReg;         /* CRC Control register configuration value */
    void                    *pProcessedBuf;     /* Processed data buffer address */
    ADI_CRC_RESULT          eCrcResult;         /* CRC Result to be returned for peek function */
    ADI_DMA_STREAM_HANDLE   hMdmaStream;        /* MDMA Stream handle for CRC operation */
    ADI_DMA_CHANNEL_HANDLE  hSrcChannel;        /* Source channel handle */
    ADI_DMA_CHANNEL_HANDLE  hDestChannel;       /* Destination channel handle */
    void                    *pMdmaStream;       /* Memory to handle a MDMA stream */
} ADI_CRC_DEVICE;

/* Structure to hold CRC device specific information */
typedef struct
{
    const uint32_t              DataCountSID;   /* Datacount expiration SID */
    const ADI_DMA_STREAM_ID     eStreamID;      /* DMA Stream ID linked to this CRC peripheral */
    volatile ADI_CRC_REGS 	    *pReg;          /* CRC peripheral Registers */
    ADI_CRC_DEVICE              *pDevice;       /* Address of the memory allocated to handle the CRC device instance */
} ADI_CRC_INFO;

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */

#endif  /* __ADI_CRC_DEF_H__ */

/*****/
