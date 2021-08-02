#ifndef __SDIO_H
#define __SDIO_H

#include "tk499.h"


void CMD_Send(unsigned int cmd,unsigned int arg);
void SDIO1_GPIOInitRemap(void);
void SDIO1_GPIOInitInput(void);
void SDIO_Init(void) ;




#endif 

