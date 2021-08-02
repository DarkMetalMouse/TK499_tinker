#ifndef __Tiky_LCD_APP_H
#define __Tiky_LCD_APP_H 
#include "sys.h"
#include "stdio.h"

void UartInit(UART_TypeDef* UARTx,int BaudRate);
void send_data(u8 data);
void send_group(u8*data,u16 len);
void send_str(char *p);

u8 Uart2_Receive(void);
#endif	   
















