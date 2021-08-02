#ifndef __Tiky_LCD_APP_H__
#define __Tiky_LCD_APP_H__

#include "tk499.h"
#include "sys.h"
#include "Tiky_LCD_APP.h"
#include "GUI.h"
#define BACK_COLOR   0x0ff000
			
/*定义常见颜色*/
//*************  16位色定义 *************//
//#define White          0xFFFF
//#define Black          0x0000
//#define Blue           0x001F
//#define Blue2          0x051F
//#define Red            0xF800
//#define Magenta        0xF81F
//#define Green          0x07E0
//#define Cyan           0x7FFF
//#define Yellow         0xFFE0

//*************  24位色（1600万色）定义 D00~D23排序是RGB *************//
#define White          0xFFFFFF
#define Black          0x000000
#define Blue           0xFF0000
#define Blue2          0xFF3F3F
#define Red            0x0000FF
#define Magenta        0xFF00FF
#define Green          0x00FF00
#define Cyan           0xFFFF00
#define Yellow         0x00FFFF

void Touch_Pad_Config(void);

void User_RGB_LCD_Initial(void);	//RGB模式液晶屏初始化入口
void LCD_Reset(void);
void LTDC_Clock_Set(void);	//设置LTDC时钟
void GPIO_RGB_INIT(void);
void SPI_WriteComm(u16 CMD);
void SPI_WriteData(u16 tem_data);
void LCD_WR_REG(unsigned short cmd,unsigned short data);
void Set_LTDC_REG(LCD_FORM_TypeDef* LCD_FORM);
void Set_LCD_Timing_to_LTDC(void);


void User_MCU_LCD_Initial(void);	//MCU模式液晶屏初始化入口
volatile void LCD_delay(int j);
void GPIO_TK80_init(void);
void LCD_TK80_init(void);
void WriteComm(unsigned short cmd);
void WriteData(unsigned int data);
void TK80_DMA_Init(u32 srcAdd ,u32 len);
void Lcd_ColorBox(u16 xStart,u16 yStart,u16 xLong,u16 yLong,u32 Color);
void LCD_Fill_Pic(u16 x, u16 y,u16 pic_H, u16 pic_V, u32* pic);
void LCD_Picture_Move(u16 n, u16 x, u16 y,u16 pic_H, u16 pic_V, u32* pic,unsigned long Back_color);
char display_picture(char *filename);
void scan_TFCard_CTP_key(void);
#endif
