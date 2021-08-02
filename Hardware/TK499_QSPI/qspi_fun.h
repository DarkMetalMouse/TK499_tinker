#ifndef __QSPI_FUN_H__
#define __QSPI_FUN_H__
#include "HAL_device.h"
#include "QSPI_REG.h"

void QSPI_FunCnofig(u8 mode);
void QSPI_FunSendbyte(u8 data);
void QSPI_FunSendGroup(u8* pData,u32 len);
void QSPI_FunNSS(u8 cs);
void QSPI_FunDmaRxConfig(u32 adress,u32 len);
void QSPI_FunDmaTxConfig(u32 adress,u32 len);
void QSPI_FunRW(u8 rw);
void QFLASH_FunReadGroup(u32 add,u8 *pBuf,u32 len);
void QFLASH_FunWriteCmd(u8 cmd);
void QFLASH_FunCheckStatus(void);
void QFLASH_FunProgram(u32 add,u8*pBuf,u32 len);
void QFLASH_FunErase(u32 add,u8 eType);
void QFLASH_FunEraseAll(void);

int get_file_address_NOR_FLASH(char *s);
#endif

