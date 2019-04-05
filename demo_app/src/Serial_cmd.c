/*
  Copyright 2018 Phantom Intelligence Inc.
*/

#include <stdint.h>
#include <string.h>
#include "cld_bf70x_bulk_lib.h"
#include "Guardian_ADI.h"



int gLogData = 1;
static uint16_t buf[FRAME_NUM_PTS];

void ReadParamFromSPI(uint16_t _startAddress, uint16_t *_data);
void WriteParamToSPI(uint16_t _startAddress, uint16_t _data);


void Lidar_PrintInfo(void)
{
	uint16_t data;

	ReadParamFromSPI(DeviceIDAddress, &data);

	uint8_t mid = (data >> 8) & 0xFF;
	uint8_t pid = (data >> 4) & 0x0F;
	uint8_t rid = (data & 0x0F);

	cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Lidar 0x%02x/0x%02x mid:%d pid:%d rid:%d\r\n", data & 0xFF, (data >> 8) & 0xFF, mid, pid, rid);
}


const char * hex = "0123456789ABCDEF";

void Lidar_DumpRegs(void)
{
	uint8_t regs[] = { DeviceIDAddress,Control0Address,Control1Address,DataControlAddress,DelayBetweenFlashesAddress,TriggerOutAddress};
	int i, ch;

	int num = sizeof(regs) / sizeof(regs[0]);

	for (i=0; i<num; i++)
	{
		uint16_t reg = regs[i];
		uint16_t data = 0xFFFF;

		ReadParamFromSPI(reg, &data);

		cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "%c%c: %c%c%c%c\r\n", hex[(reg>>4)&0xF], hex[(reg>>0)&0xF],
				hex[(data>>12)&0xF], hex[(data>>8)&0xF], hex[(data>>4)&0xF], hex[(data>>0)&0xF]);
	}

	cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Ch: E TIA BAL\r\n");

	for (ch=0; ch<16; ch++)
	{
		uint16_t en, tia, bal;
		uint16_t data = 0xFFFF;
		uint8_t tia_feedback,chx_cpd_select;

		ReadParamFromSPI(ChannelEnableAddress, &data);
		en = (data & (1 << ch) );

		ReadParamFromSPI(CH0ControlReg0Address + 4 * ch, &data);
		tia = data;

		ReadParamFromSPI(CH0ControlReg1Address + 4 * ch, &data);
		chx_cpd_select =  (uint8_t) ((data >> 8) & 0x7);
		tia_feedback = (uint8_t) (data & 0xFF);

		ReadParamFromSPI(CH0ControlReg2Address + 4 * ch, &data);
		bal = (data & 0x01FF);

		cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "%02d: %d %d %d %d %d\r\n", ch, (en) ? 1 : 0, tia,chx_cpd_select , tia_feedback, bal);
	}

}

void Lidar_GetData(void)
{
	uint16_t banknum = 0;
	int i,j,ch;

	memset(buf, 0, sizeof(buf));

	Lidar_GetADIData(&banknum, buf);

	uint16_t * pData = buf;

	if (banknum)
	{
		cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "bank %d\r\n", banknum);

		for(ch=0; ch<3; ch++)
		{
			cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "Ch %d\r\n", ch);

			for(j=0; j<3; j++)
			{
				for(i=0; i<10; i++)
				{
					uint16_t data = pData[ch*100 + j*10 + i];
					cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, " %c%c%c%c ",
						hex[(data>>12)&0xF], hex[(data>>8)&0xF], hex[(data>>4)&0xF], hex[(data>>0)&0xF]);
				}
				cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "\r\n");
			}
		}
	}
	else
		cld_console(CLD_CONSOLE_RED, CLD_CONSOLE_BLACK, "No data!\r\n");
}

int chIdx[16] = {
100 * 0,
100 * 1,
100 * 2,
100 * 3,
100 * 4,
100 * 5,
100 * 6,
100 * 7,
100 * 15,
100 * 14,
100 * 13,
100 * 12,
100 * 11,
100 * 10,
100 * 9,
100 * 8
};

void Lidar_GetDataCSV(void)
{
	uint16_t banknum = 0;
	int ch,sample;

	memset(buf, 0, sizeof(buf));

	Lidar_GetADIData(&banknum, buf);

	uint16_t * pData = buf;

	if (banknum)
	{
		cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "##, ch00 , ch01 , ch02 , ch03 , ch04 , ch05 , ch06 , ch07 , ch08 , ch09 , ch10 , ch11 , ch12 , ch13 , ch14 , ch15\r\n", banknum);

		for(sample=0; sample<100; sample++)
		{
			cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "%2d,", sample);

			for(ch=0; ch<16; ch++)
			{
				int16_t data = pData[chIdx[ch] + sample];

				cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "%6d,", data);
			}
			cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "\r\n");
		}
	}
}




#define MAX_CMD_SIZE  16

static char debugCmdIdx = 0;
static char debugCmd[MAX_CMD_SIZE];

void ProcessChar(char curChar)
{
	cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "%c", curChar);

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
//    case 'I':
//    	PrintFlashInfo();
//    	break;

    case 'l':
   		gLogData = !gLogData;
    	break;

    case 'a':
   		Lidar_GetData();
    	break;

    case 'A':
   		Lidar_GetDataCSV();
    	break;

    case 'i':
   		Lidar_PrintInfo();
    	break;

    case 'I':
    	Lidar_InitADI();
    	break;

    case 'G':
    	Lidar_Trig();
    	break;

    case 's':
      gAcq = 0;
      break;

    case 'q':
      gAcq = 1;
      break;

//    case 'z':
//		cld_console(CLD_CONSOLE_GREEN, CLD_CONSOLE_BLACK, "a %d 1:%d 2:%d\r\n", iAcqNum, iAcqNum1, iAcqNum2);
//		iAcqNum = iAcqNum1 = iAcqNum2 = 0;
//		break;

    case 't':
    	Lidar_SPITriggerMode();
    	break;

    case 'f':
    	Lidar_FreerunMode();
    	break;

    case 'd':
   		Lidar_DumpRegs();
    	break;

    case '1':
   		Lidar_ChannelEnable(0, 1);
    	break;

    case '!':
   		Lidar_ChannelEnable(0, 0);
    	break;

    case '2':
   		Lidar_ChannelEnable(1, 1);
    	break;

    case '@':
   		Lidar_ChannelEnable(1, 0);
    	break;

    case 'g':
    {
    	int i = 1;
    	uint16_t flashGain = 0;

    	while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
    		flashGain = flashGain * 10 + debugCmd[i++] - '0';

    	Lidar_FlashGain(flashGain);
    	break;
    }

    case 'b':
    {
    	int i = 1;
    	uint16_t bal = 0;
    	int ch = 0;

    	while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
    		ch = ch * 10 + debugCmd[i++] - '0';

    	if (debugCmd[i] == ' ' || debugCmd[i] == '=')
    		i++;

    	while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
    		bal = bal * 10 + debugCmd[i++] - '0';

    	Lidar_ChannelDCBal(ch, bal);
    	break;
    }

    case 'F':
    {
    	int i = 1;
    	uint16_t feedback = 0;
    	int ch = 0;

    	while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
    		ch = ch * 10 + debugCmd[i++] - '0';

    	if (debugCmd[i] == ' ' || debugCmd[i] == '=')
    		i++;

    	while (debugCmd[i] >= '0' && debugCmd[i] <= '9')
    		feedback = feedback * 10 + debugCmd[i++] - '0';

    	Lidar_ChannelTIAFeedback(ch, feedback);
    	break;
    }

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