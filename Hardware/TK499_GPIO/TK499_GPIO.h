#ifndef __TK499_GPIO_H__
#define __TK499_GPIO_H__
#include "HAL_conf.h"
#include "sys.h"
//#define LCD_SPI_CS(a)	\
//						if (a)	\
//						GPIOE->BSRR = GPIO_Pin_9;	\
//						else		\
//						GPIOE->BRR  = GPIO_Pin_9;
//						
//#define SPI_DCLK(a)	\
//						if (a)	\
//						GPIOE->BSRR = GPIO_Pin_1;	\
//						else		\
//						GPIOE->BRR  = GPIO_Pin_1;
//		
//#define SPI_SDA(a)	\
//						if (a)	\
//						GPIOE->BSRR = GPIO_Pin_0;	\
//						else		\
//						GPIOE->BRR  = GPIO_Pin_0;

//#define LCD_RST(a)	\
//						if (a)	\
//						GPIO_SetBits(GPIOE, GPIO_Pin_17);	\
//						else		\
//						GPIO_ResetBits(GPIOE, GPIO_Pin_17);

//#define Lcd_Light_ON   GPIO_SetBits(GPIOE, GPIO_Pin_16) //PE16为高电平 背光打开
//#define Lcd_Light_OFF  GPIO_ResetBits(GPIOE, GPIO_Pin_16)  //PE16为低电平 背光关闭

						
//=======================LS026=======================
//#define LCD_SPI_CS(a)	\
//						if (a)	\
//						GPIOE->BSRR = GPIO_Pin_1;	\
//						else		\
//						GPIOE->BRR  = GPIO_Pin_1;					
//#define SPI_DCLK(a)	\
//						if (a)	\
//						GPIOB->BSRR = GPIO_Pin_2;	\
//						else		\
//						GPIOB->BRR  = GPIO_Pin_2;	
//#define SPI_SDA(a)	\
//						if (a)	\
//						GPIOE->BSRR = GPIO_Pin_8;	\
//						else		\
//						GPIOE->BRR  = GPIO_Pin_8;

//#define LCD_RST(a)	\
//						if (a)	\
//						GPIOE->BSRR = GPIO_Pin_0;	\
//						else		\
//						GPIOE->BRR  = GPIO_Pin_0;
//						
//#define Lcd_Light_ON   GPIO_SetBits(GPIOE,GPIO_Pin_17)  //PD8为高电平 背光打开
//#define Lcd_Light_OFF  GPIO_ResetBits(GPIOE,GPIO_Pin_17)  //PD8为低电平 背光关闭

//====================== comm ====================//
#define LCD_SPI_CS(a)	\
						if (a)	\
						GPIOB->BSRR = GPIO_Pin_11;	\
						else		\
						GPIOB->BRR  = GPIO_Pin_11;
//===TK043F1168===//							
//#define SPI_DCLK(a)	\
//						if (a)	\
//						GPIOB->BSRR = GPIO_Pin_10;	\
//						else		\
//						GPIOB->BRR  = GPIO_Pin_10;
//===TK043F1508、TK043F1569、TK043F3211、TK035F65MB、TK050F5590E===//							
#define SPI_DCLK(a)	\
					if (a)	\
					GPIOB->BSRR = GPIO_Pin_9;	\
					else		\
					GPIOB->BRR  = GPIO_Pin_9;
//==============多合一屏、、TK032RB422、TK032F8004、TK032LG480==============//								
//#define SPI_DCLK(a)	\
//						if (a)	\
//						GPIOB->BSRR = GPIO_Pin_2;	\
//						else		\
//						GPIOB->BRR  = GPIO_Pin_2;

						
#define SPI_SDA(a)	\
						if (a)	\
						GPIOE->BSRR = GPIO_Pin_0;	\
						else		\
						GPIOE->BRR  = GPIO_Pin_0;

#define LCD_RST(a)	\
						if (a)	\
						GPIOD->BSRR = GPIO_Pin_6;	\
						else		\
						GPIOD->BRR = GPIO_Pin_6;
		
  //====  4.3smart board  ====//						
//#define LCD_RST(a)	\
//						if (a)	\
//						GPIOA->BSRR = GPIO_Pin_14;	\
//						else		\
//						GPIOA->BRR = GPIO_Pin_14;

#define Lcd_Light_ON   GPIOD->BSRR = GPIO_Pin_8  //PD8为高电平 背光打开
#define Lcd_Light_OFF  GPIOD->BRR  = GPIO_Pin_8  //PD8为低电平 背光关闭

void GPIO_TK80_init(void);
void GPIO_RGB_INIT(void);
void Touch_Pad_Config(void);

#endif

