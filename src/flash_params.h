#ifndef _FLASH_PARAMS_H_
#define _FLASH_PARAMS_H_

int Flash_Init(void);
int Flash_ResetToFactoryDefault(void);
int Flash_SaveConfig(uint16_t * pParams, int num);
int Flash_LoadConfig(uint16_t * pParams, int * pNum);

#endif //_FLASH_PARAMS_H_
