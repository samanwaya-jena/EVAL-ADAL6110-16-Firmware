#ifndef _FLASH_PARAMS_H_
#define _FLASH_PARAMS_H_

#include <stdint.h>

int Flash_Init(void);
int Flash_ResetToFactoryDefault(int idx);
int Flash_SaveConfig(int idx, uint16_t * pParams, int num);
int Flash_LoadConfig(int idx, uint16_t * pParams, int * pNum);

void LoadDefaultConfig(int idx);

#endif //_FLASH_PARAMS_H_
