/*
  Copyright 2018 Phantom Intelligence Inc.
*/

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "../adal6110_16.h"
#include "cld_bf70x_bulk_lib.h"

#include "../parameters.h"
#include "../demo_app.h"



static uint16_t buf[FRAME_NUM_PTS]; // raw data communication buffer



void Lidar_PrintInfo(void)
{
	uint16_t data;

	ADAL_ReadParamFromSPI(DeviceIDAddress, &data);

	uint8_t mid = (data >> 8) & 0xFF;
	uint8_t pid = (data >> 4) & 0x0F;
	uint8_t rid = (data & 0x0F);
	cld_console(CLD_CONSOLE_CYAN,  CLD_CONSOLE_BLACK, "ADAL6110-16 Eval Kit\r\n");
	cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Device ID: 0x%04X\r\n", LiDARParameters[param_deviceID]);
	cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Manufacturing date: 2%03d/%d\r\n", LiDARParameters[param_manufDate]>>8,LiDARParameters[param_manufDate]&0x0F);
	cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Serial Number: %04d\r\n", LiDARParameters[param_serialNumber]);
	cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Firmware version: %02d.%03d\r\n",FIRMWARE_MAJOR_REV,FIRMWARE_MINOR_REV);

	cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Lidar 0x%02x/0x%02x mid:%d pid:%d rid:%d\r\n", data & 0xFF, (data >> 8) & 0xFF, mid, pid, rid);
}


const char * hex = "0123456789ABCDEF";

void Lidar_DumpRegs(void)
{
	uint8_t regs[] = { DeviceIDAddress,Control0Address,Control1Address,DataControlAddress,DelayBetweenFlashesAddress,TriggerOutAddress,FRAMEDELAY};
	int i, ch;

	int num = sizeof(regs) / sizeof(regs[0]);
	for (i=0; i<num; i++)
	{
		uint16_t reg = regs[i];
		uint16_t data = 0xFFFF;

		ADAL_ReadParamFromSPI(reg, &data);
		cld_console(CLD_CONSOLE_GREEN,CLD_CONSOLE_BLACK,"%02d: 0x%04X\r\n",reg,data);
	}

	cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Ch: En\tTIA\tcpd\tTIA fb\tBAL\r\n");
	for (ch=0; ch<16; ch++)
	{
		uint16_t en, tia, bal;
		uint16_t data = 0xFFFF;
		uint8_t tia_feedback,chx_cpd_select;

		ADAL_ReadParamFromSPI(ChannelEnableAddress, &data);
		en = (data & (1 << ch) );
		ADAL_ReadParamFromSPI(CH0ControlReg0Address + 4 * ch, &data);
		tia = data & 0x1F;
		ADAL_ReadParamFromSPI(CH0ControlReg1Address + 4 * ch, &data);
		chx_cpd_select =  (uint8_t) ((data >> 8) & 0x7);
		tia_feedback = (uint8_t) (data & 0xFF);
		ADAL_ReadParamFromSPI(CH0ControlReg2Address + 4 * ch, &data);
		bal = (data & 0x01FF);
		cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "%02d: %d\t%02X\t%02X\t%02X\t%03X\r\n", ch, (en) ? 1 : 0, tia,chx_cpd_select , tia_feedback, bal);
	}

}
/*
void Lidar_GetData(void)
{
	uint16_t banknum = 0;
	int i,j,ch;
	CLD_RV ret;

	memset(buf, 0, sizeof(buf));

	ADAL_GetADIData(&banknum, buf);

	uint16_t * pData = buf;
	uint16_t data_to_send[25]; //send 100 data on 5 lines

	if (banknum)
	{
		cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "bank %d\r\n", banknum);

		for(ch=0; ch<DEVICE_NUM_CHANNEL; ch++)
		{
			while(CLD_SUCCESS != cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Ch %d\r\n", LiDARParameters[param_channel_map_offset+ch]));

			for(j=0; j<4; j++)
			{
				for(i=0; i<25; i++)
				{
					data_to_send[i] = pData[ch*DEVICE_SAMPLING_LENGTH + j*25 + i];
					//uint16_t data = pData[ch*DEVICE_SAMPLING_LENGTH + j*20 + i];
					//cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, " %c%c%c%c ",
					//	hex[(data>>12)&0xF], hex[(data>>8)&0xF], hex[(data>>4)&0xF], hex[(data>>0)&0xF]);
				}
				do
				{
					ret = cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK,"  %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X\r\n",
							data_to_send[0],data_to_send[1],data_to_send[2],data_to_send[3],data_to_send[4],
							data_to_send[5],data_to_send[6],data_to_send[7],data_to_send[8],data_to_send[9],
							data_to_send[10],data_to_send[11],data_to_send[12],data_to_send[13],data_to_send[14],
							data_to_send[15],data_to_send[16],data_to_send[17],data_to_send[18],data_to_send[19],
							data_to_send[20],data_to_send[21],data_to_send[22],data_to_send[23],data_to_send[24]);
				}while(ret != CLD_SUCCESS);
				//cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "\r\n");
			}
		}
	}
	else
		cld_console(CLD_CONSOLE_RED, CLD_CONSOLE_BLACK, "No data!\r\n");
}
*/
/*
void Lidar_GetDataCSV(void)
{
	uint16_t banknum = 0;
	int ch,sample;
	int16_t data_to_send[DEVICE_NUM_CHANNEL];
	CLD_RV ret;

	memset(buf, 0, sizeof(buf));

	ADAL_GetADIData(&banknum, buf);

	uint16_t * pData = buf;

	if (banknum)
	{
		cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "##,\tch00,\tch01,\tch02,\tch03,\tch04,\tch05,\tch06,\tch07,\tch08,\tch09,\tch10,\tch11,\tch12,\tch13,\tch14,\tch15\r\n", banknum);
		for(sample=0; sample<DEVICE_SAMPLING_LENGTH; sample++)
		{
			for(ch=0; ch<DEVICE_NUM_CHANNEL; ch++)
				data_to_send[LiDARParameters[param_channel_map_offset+ch]] = (int16_t)pData[DEVICE_SAMPLING_LENGTH*ch + sample];
			do
			{
				ret = cld_console(CLD_CONSOLE_GREEN,CLD_CONSOLE_BLACK,"%03d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d\r\n",sample,
					data_to_send[0],data_to_send[1],data_to_send[2],data_to_send[3],data_to_send[4],data_to_send[5],data_to_send[6],data_to_send[7],
					data_to_send[8],data_to_send[9],data_to_send[10],data_to_send[11],data_to_send[12],data_to_send[13],data_to_send[14],data_to_send[15]);
			}while(ret != CLD_SUCCESS);
		}
	}
	else
		cld_console(CLD_CONSOLE_RED, CLD_CONSOLE_BLACK, "No data!\r\n");
}
*/

uint8_t c2i(char c)
{
	switch( c)
	{
	case '1':
		return(0x01);
	case '2':
		return(0x02);
	case '3':
		return(0x03);
	case '4':
		return(0x04);
	case '5':
		return(0x05);
	case '6':
		return(0x06);
	case '7':
		return(0x07);
	case '8':
		return(0x08);
	case '9':
		return(0x09);
	case 'A':
	case 'a':
		return(0x0A);
	case 'B':
	case 'b':
		return(0x0B);
	case 'C':
	case 'c':
		return(0x0C);
	case 'D':
	case 'd':
		return(0x0D);
	case 'E':
	case 'e':
		return(0x0E);
	case 'F':
	case 'f':
		return(0x0F);
	default:
		return(0x00);
	}
}


#define MAX_CMD_SIZE  16

static char debugCmdIdx = 0;
static char debugCmd[MAX_CMD_SIZE];

void ProcessChar(char curChar)
{
	cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "%c", curChar);
	int i;

	if (curChar == '\r')
	{
		cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "\n");
	}

  if (debugCmdIdx < MAX_CMD_SIZE)
    debugCmd[debugCmdIdx++] = curChar;

  if (curChar == '\r')
  {
    debugCmd[debugCmdIdx-1] = 0;

    switch (debugCmd[0])
    {
    case 'l':
   		LiDARParameters[param_console_log] ^= CONSOLE_MASK_LOG;
   		if(LiDARParameters[param_console_log] & CONSOLE_MASK_LOG)
   			cld_console(CLD_CONSOLE_GREEN,CLD_CONSOLE_BLACK,"acquisition log set\n\r");
   		else
   			cld_console(CLD_CONSOLE_GREEN,CLD_CONSOLE_BLACK,"acquisition log cleared\n\r");
    	break;

//    case 'a':
//   		Lidar_GetData();
//    	break;

//    case 'A':
//  		Lidar_GetDataCSV();
//    	break;

    case 'i':
   		Lidar_PrintInfo();
    	break;

    case 'I':
    	ADAL_InitADI();
    	break;

    case 'G':
    	ADAL_Trig();
    	break;

    case 's':
    	param_WriteFifoPush(param_acq_enable|0x4000,0);
    	cld_console(CLD_CONSOLE_GREEN,CLD_CONSOLE_BLACK,"Acquisition stopped\n\r");
    	break;

    case 'q':
    	param_WriteFifoPush(param_acq_enable|0x4000,1);
    	cld_console(CLD_CONSOLE_GREEN,CLD_CONSOLE_BLACK,"Acquisition started\n\r");
    	break;

//    case 'z':
//		cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "a %d 1:%d 2:%d\r\n", iAcqNum, iAcqNum1, iAcqNum2);
//		iAcqNum = iAcqNum1 = iAcqNum2 = 0;
//		break;

    case 't':
    	ADAL_SPITriggerMode();
    	cld_console(CLD_CONSOLE_GREEN,CLD_CONSOLE_BLACK,"acquisition set in trigger mode\n\r");
    	break;

    case 'f':
    	ADAL_FreerunMode();
    	cld_console(CLD_CONSOLE_GREEN,CLD_CONSOLE_BLACK,"acquisition set in free run mode\n\r");
    	break;

    case 'd':
   		Lidar_DumpRegs();
    	break;

    case 'g':
    {
    	i = 1;
    	uint16_t flashGain = 0;

    	while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
    		flashGain = flashGain * 10 + debugCmd[i++] - '0';

    	ADAL_FlashGain(flashGain);
    	ADAL_SetFrameRate(LiDARParameters[param_frame_rate]);
    	break;
    }

    case'v':
    {
    	i = 1;
    	uint16_t frame_rate = 0;

		while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
			frame_rate = frame_rate * 10 + debugCmd[i++] - '0';

		param_WriteFifoPush(param_frame_rate|0x4000,frame_rate);
		break;
    }
    case'w':
    {
		i = 1;
		uint16_t width = 0;

		while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
			width = width * 10 + debugCmd[i++] - '0';

		cld_console(CLD_CONSOLE_GREEN,CLD_CONSOLE_BLACK,"ADAL6110-16 pulse width set to %d ns\r\n", ADAL_SetPulseWidth(width));
		break;
    }
    case 'b':
    {
    	i = 1;
    	uint16_t bal = 0;
    	int ch = 0;

    	while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
    		ch = ch * 10 + debugCmd[i++] - '0';

    	if (debugCmd[i] == ' ' || debugCmd[i] == '=')
    		i++;

    	while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
    		bal = bal * 10 + debugCmd[i++] - '0';

    	ADAL_ChannelDCBal(ch, bal);
    	break;
    }

    case 'F':
    {
    	i = 1;
    	uint16_t feedback = 0;
    	int ch = 0;

    	while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
    		ch = ch * 10 + debugCmd[i++] - '0';

    	if (debugCmd[i] == ' ' || debugCmd[i] == '=')
    		i++;

    	while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
    		feedback = feedback * 10 + debugCmd[i++] - '0';

    	ADAL_ChannelTIAFeedback(ch, feedback);
    	break;
    }

    case 'u':
   		LiDARParameters[param_console_log] ^= CONSOLE_MASK_USB;
   		if(LiDARParameters[param_console_log] & CONSOLE_MASK_USB)
   			cld_console(CLD_CONSOLE_GREEN,CLD_CONSOLE_BLACK,"USB log set\n\r");
   		else
   			cld_console(CLD_CONSOLE_GREEN,CLD_CONSOLE_BLACK,"USB log cleared\n\r");
    	break;

    case 'N':
    	i = 1;
    	if (LiDARParameters[param_serialNumber])
    		cld_console(CLD_CONSOLE_RED,CLD_CONSOLE_BLACK,"Serial number already set to: 0x%04X\n\r",LiDARParameters[param_serialNumber]);
    	else
    	{
    		LiDARParameters[param_serialNumber] = atoi(debugCmd+2);
    		cld_console(CLD_CONSOLE_GREEN,CLD_CONSOLE_BLACK,"Serial number set to: 0x%04X\n\r",LiDARParameters[param_serialNumber]);
    	}
    	break;

    case 'D':
       	i = 1;
       	if (LiDARParameters[param_manufDate])
       		cld_console(CLD_CONSOLE_RED,CLD_CONSOLE_BLACK,"Date already set to:  20%02d/%02d\r\n",
       				    LiDARParameters[param_manufDate]>>8,LiDARParameters[param_manufDate]&0x0F);
       	else
       	{
       		LiDARParameters[param_manufDate] = atoi(debugCmd+2);
   			cld_console(CLD_CONSOLE_GREEN,CLD_CONSOLE_BLACK,"Date set to:  20%02d/%02d\r\n",
   				    LiDARParameters[param_manufDate]>>8,LiDARParameters[param_manufDate]&0x0F);
       	}
       	break;

    case 'S':
    	param_SaveConfig();
    	cld_console(CLD_CONSOLE_GREEN,CLD_CONSOLE_BLACK,"Configuration saved to Flash memory\r\n");
    	break;

    case 'R':
    	param_ResetFactoryDefault();
        	cld_console(CLD_CONSOLE_GREEN,CLD_CONSOLE_BLACK,"Configuration reset to factory defaults\r\n");
        	break;

    case 'r':
    	LiDARParameters[param_console_log] ^= CONSOLE_MASK_DIST;
    	if(LiDARParameters[param_console_log] & CONSOLE_MASK_DIST)
			cld_console(CLD_CONSOLE_GREEN,CLD_CONSOLE_BLACK,"Distance log set\n\r");
		else
			cld_console(CLD_CONSOLE_GREEN,CLD_CONSOLE_BLACK,"Distance log cleared\n\r");
    	break;

    case 'p':
        {
        	i = 1;
        	uint16_t addr = 0;
        	uint16_t value = 0;

        	while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
        		addr = addr * 10 + debugCmd[i++] - '0';

        	if (debugCmd[i] == ' ' || debugCmd[i] == '=')
        		i++;

        	if(debugCmd[i] == '?')
        	{
        		param_ReadFifoPush((addr&0x3FFF)|0x4000);
        		break;
        	}
        	while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
        		value = value * 10 + debugCmd[i++] - '0';

        	param_WriteFifoPush((addr&0x3FFF)|0x4000,value);
        	break;
        }

    case 'P':
            {
            	i = 1;
            	uint16_t addr = 0;
            	uint16_t value = 0;

            	while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
            		addr = addr * 10 + debugCmd[i++] - '0';

            	if (debugCmd[i] == ' ' || debugCmd[i] == '=')
            		i++;

            	if(debugCmd[i] == '?')
				{
					param_ReadFifoPush(addr&0x3FFF);
					break;
				}
            	while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
            		value = value * 10 + debugCmd[i++] - '0';

            	param_WriteFifoPush((addr&0x3FFF),value&0xFFFF);
            	break;
            }

    default:
    	cld_console(CLD_CONSOLE_RED,CLD_CONSOLE_BLACK,"Say again...\r\n");
    	break;
    }

    debugCmdIdx = 0;
  }


}



static char rxCharHead = 0;
static char rxCharTail = 0;
static char rxChar[MAX_CMD_SIZE];

void Serial_RxChar(char curChar)
{
	if (((rxCharHead + 1) % MAX_CMD_SIZE) != rxCharTail)
	{
		rxChar[rxCharHead] = curChar;
		rxCharHead = (rxCharHead + 1) % MAX_CMD_SIZE;
	}
}

void Serial_Process(void)
{
	if (rxCharHead != rxCharTail)
	{
		char ch = rxChar[rxCharTail];
		rxCharTail = (rxCharTail + 1) % MAX_CMD_SIZE;
		ProcessChar(ch);
	}
}
