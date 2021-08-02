/*********************************************************************

----------------------------------------------------------------------
File        : LCDConf.c
Purpose     : 显示驱动配置

---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"
#include "LCDCONF.H"
#include "GUIDRV_Template.h"
#include "GUIDRV_Lin.h"

/*********************************************************************
*
*       Layer configuration
*
**********************************************************************
*/
//static int                        _aPendingBuffer[GUI_NUM_LAYERS];




//
// 色彩转换
//
#define COLOR_CONVERSION GUICC_M888

//
// 显示驱动
//
#if LCD_MODE_MCU_or_RGB
	#define DISPLAY_DRIVER      &GUIDRV_Lin_OS_32_API//   GUIDRV_Lin_32_API
#else
	#define DISPLAY_DRIVER &GUIDRV_Template_API  //GUIDRV_Lin_32_API  GUIDRV_Lin_OSX_32_API 
#endif

//
// 缓存 / 虚拟屏幕
//
#define NUM_BUFFERS   1
#define NUM_VSCREENS  1

//
// 显示方向
//
// #define DISPLAY_ORIENTATION  GUI_MIRROR_X
// #define DISPLAY_ORIENTATION               (GUI_MIRROR_X | GUI_MIRROR_Y)
// #define DISPLAY_ORIENTATION (GUI_SWAP_XY | GUI_MIRROR_Y)
// #define DISPLAY_ORIENTATION (GUI_SWAP_XY | GUI_MIRROR_X)
// #define DISPLAY_ORIENTATION GUI_SWAP_XY



/*********************************************************************
*
*       Configuration checking
*
**********************************************************************
*/
#ifndef   XSIZE_PHYS
  #error Physical X size of display is not defined!
#endif
#ifndef   YSIZE_PHYS
  #error Physical Y size of display is not defined!
#endif
#ifndef   COLOR_CONVERSION
  #error Color conversion not defined!
#endif
#ifndef   DISPLAY_DRIVER
  #error No display driver defined!
#endif
#ifndef   NUM_VSCREENS
  #define NUM_VSCREENS 1
#else
  #if (NUM_VSCREENS <= 0)
    #error At least one screeen needs to be defined!
  #endif
#endif
#if (NUM_VSCREENS > 1) && (NUM_BUFFERS > 1)
  #error Virtual screens and multiple buffers are not allowed!
#endif
#ifndef   DISPLAY_ORIENTATION
  #define DISPLAY_ORIENTATION  0
#endif

#if ((DISPLAY_ORIENTATION & GUI_SWAP_XY) != 0)
#define LANDSCAPE   1
#else
#define LANDSCAPE   0
#endif

#if (LANDSCAPE == 1)
#define WIDTH       YSIZE_PHYS  /* Screen Width (in pixels)         */
#define HEIGHT      XSIZE_PHYS  /* Screen Hight (in pixels)         */
#else
#define WIDTH       XSIZE_PHYS  /* Screen Width (in pixels)         */
#define HEIGHT      YSIZE_PHYS  /* Screen Hight (in pixels)         */
#endif

#if ((DISPLAY_ORIENTATION & GUI_SWAP_XY) != 0)
  #if ((DISPLAY_ORIENTATION & GUI_MIRROR_X) != 0)
    #define TOUCH_TOP    TOUCH_X_MAX
    #define TOUCH_BOTTOM TOUCH_X_MIN
  #else
    #define TOUCH_TOP    TOUCH_X_MIN
    #define TOUCH_BOTTOM TOUCH_X_MAX
  #endif
  #if ((DISPLAY_ORIENTATION & GUI_MIRROR_Y) != 0)
    #define TOUCH_LEFT   TOUCH_Y_MAX
    #define TOUCH_RIGHT  TOUCH_Y_MIN
  #else
    #define TOUCH_LEFT   TOUCH_Y_MIN
    #define TOUCH_RIGHT  TOUCH_Y_MAX
  #endif
#else
  #if ((DISPLAY_ORIENTATION & GUI_MIRROR_X) != 0)
    #define TOUCH_LEFT   TOUCH_X_MAX
    #define TOUCH_RIGHT  TOUCH_X_MIN
  #else
    #define TOUCH_LEFT   TOUCH_X_MIN
    #define TOUCH_RIGHT  TOUCH_X_MAX
  #endif
  #if ((DISPLAY_ORIENTATION & GUI_MIRROR_Y) != 0)
    #define TOUCH_TOP    TOUCH_Y_MAX
    #define TOUCH_BOTTOM TOUCH_Y_MIN
  #else
    #define TOUCH_TOP    TOUCH_Y_MIN
    #define TOUCH_BOTTOM TOUCH_Y_MAX
  #endif
#endif

/*********************************************************************
*
*       Driver Port functions
*
**********************************************************************
*/
void LCD_init_code(void)//液晶屏初始化代码
{
//****************************************************************************//
//****************************** Page 1 Command ******************************//
//****************************************************************************//
SPI_WriteComm(0xFF); // Change to Page 1
SPI_WriteData(0xFF); 
SPI_WriteData(0x98); 
SPI_WriteData(0x06);
SPI_WriteData(0x04); 
SPI_WriteData(0x01);

SPI_WriteComm(0X08);SPI_WriteData(0X10);                 // output SDA

SPI_WriteComm(0X21);SPI_WriteData(0X01);                 // DE = 1 Active

SPI_WriteComm(0X30);SPI_WriteData(0X01);                 // 480 X 854

SPI_WriteComm(0X31);SPI_WriteData(0X02);                 // 2-dot Inversion

SPI_WriteComm(0X40);SPI_WriteData(0X18);                // DDVDH/L BT 17 -10 5.94 -5.47 -2.3 1.6 43.4 21.88 0.17

SPI_WriteComm(0X41);SPI_WriteData(0X66);              // DDVDH/L CLAMP 

SPI_WriteComm(0X42);SPI_WriteData(0X02);                 // VGH/VGL 

SPI_WriteComm(0X43);SPI_WriteData(0X0A);                 // VGH/VGL 

SPI_WriteComm(0X50);SPI_WriteData(0X78);                 // VGMP 4.6 

SPI_WriteComm(0X51);SPI_WriteData(0X78);                 // VGMN 4.6 

SPI_WriteComm(0X52);SPI_WriteData(0X00);                 // Flicker 

SPI_WriteComm(0X53);SPI_WriteData(0X1F);                 // Flicker 

SPI_WriteComm(0X57);SPI_WriteData(0X50);

SPI_WriteComm(0X60);SPI_WriteData(0X07);                 // SDTI

SPI_WriteComm(0X61);SPI_WriteData(0X01);                 // CRTI 00

SPI_WriteComm(0X62);SPI_WriteData(0X07);                 // EQTI 07

SPI_WriteComm(0X63);SPI_WriteData(0X00);                 // PCTI  00


////++++++++++++++++++ Gamma Setting ++++++++++++++++++//
//SPI_WriteComm(0xFF); // Change to Page 1
//SPI_WriteData(0xFF); 
//SPI_WriteData(0x98); 
//SPI_WriteData(0x06);
//SPI_WriteData(0x04); 
//SPI_WriteData(0x01);

SPI_WriteComm(0XA0);SPI_WriteData(0X00);  // Gamma 0 

SPI_WriteComm(0XA1);SPI_WriteData(0X0A);  // Gamma 4 

SPI_WriteComm(0XA2);SPI_WriteData(0X12);  // Gamma 8

SPI_WriteComm(0XA3);SPI_WriteData(0X0D);  // Gamma 16

SPI_WriteComm(0XA4);SPI_WriteData(0X07);  // Gamma 24

SPI_WriteComm(0XA5);SPI_WriteData(0X09);  // Gamma 52

SPI_WriteComm(0XA6);SPI_WriteData(0X06); // Gamma 80

SPI_WriteComm(0XA7);SPI_WriteData(0X03);  // Gamma 108

SPI_WriteComm(0XA8);SPI_WriteData(0X09);  // Gamma 147

SPI_WriteComm(0XA9);SPI_WriteData(0X0D);  // Gamma 175

SPI_WriteComm(0XAA);SPI_WriteData(0X15);  // Gamma 203

SPI_WriteComm(0XAB);SPI_WriteData(0X07);  // Gamma 231

SPI_WriteComm(0XAC);SPI_WriteData(0X0D);  // Gamma 239

SPI_WriteComm(0XAD);SPI_WriteData(0X0D);  // Gamma 247

SPI_WriteComm(0XAE);SPI_WriteData(0X06);  // Gamma 251

SPI_WriteComm(0XAF);SPI_WriteData(0X00);  // Gamma 255

///==============Nagitive
SPI_WriteComm(0XC0);SPI_WriteData(0X00);  // Gamma 0 

SPI_WriteComm(0XC1);SPI_WriteData(0X0A);  // Gamma 4

SPI_WriteComm(0XC2);SPI_WriteData(0X12);  // Gamma 8

SPI_WriteComm(0XC3);SPI_WriteData(0X0E);  // Gamma 16

SPI_WriteComm(0XC4);SPI_WriteData(0X07);  // Gamma 24

SPI_WriteComm(0XC5);SPI_WriteData(0X09);  // Gamma 52

SPI_WriteComm(0XC6);SPI_WriteData(0X07);  // Gamma 80

SPI_WriteComm(0XC7);SPI_WriteData(0X03);  // Gamma 108

SPI_WriteComm(0XC8);SPI_WriteData(0X09);  // Gamma 147

SPI_WriteComm(0XC9);SPI_WriteData(0X0C);  // Gamma 175

SPI_WriteComm(0XCA);SPI_WriteData(0X15);  // Gamma 203

SPI_WriteComm(0XCB);SPI_WriteData(0X07);  // Gamma 231

SPI_WriteComm(0XCC);SPI_WriteData(0X0C);  // Gamma 239

SPI_WriteComm(0XCD);SPI_WriteData(0X0D); // Gamma 247

SPI_WriteComm(0XCE);SPI_WriteData(0X06); // Gamma 251

SPI_WriteComm(0XCF);SPI_WriteData(0X00);  // Gamma 255


//****************************************************************************//
//****************************** Page 6 Command ******************************//
//****************************************************************************//
SPI_WriteComm(0xFF); // Change to Page 6
SPI_WriteData(0xFF); 
SPI_WriteData(0x98); 
SPI_WriteData(0x06);
SPI_WriteData(0x04); 
SPI_WriteData(0x06);

SPI_WriteComm(0X00);SPI_WriteData(0X21);

SPI_WriteComm(0X01);SPI_WriteData(0X06);

SPI_WriteComm(0X02);SPI_WriteData(0XA0 );   

SPI_WriteComm(0X03);SPI_WriteData(0X02);

SPI_WriteComm(0X04);SPI_WriteData(0X01);

SPI_WriteComm(0X05);SPI_WriteData(0X01);

SPI_WriteComm(0X06);SPI_WriteData(0X80);    

SPI_WriteComm(0X07);SPI_WriteData(0X04);

SPI_WriteComm(0X08);SPI_WriteData(0X00);

SPI_WriteComm(0X09);SPI_WriteData(0X80);  

SPI_WriteComm(0X0A);SPI_WriteData(0X00);   

SPI_WriteComm(0X0B);SPI_WriteData(0X00);    

SPI_WriteComm(0X0C);SPI_WriteData(0X2c);

SPI_WriteComm(0X0D);SPI_WriteData(0X2c);

SPI_WriteComm(0X0E);SPI_WriteData(0X1c);

SPI_WriteComm(0X0F);SPI_WriteData(0X00);

SPI_WriteComm(0X10);SPI_WriteData(0XFF);

SPI_WriteComm(0X11);SPI_WriteData(0XF0);

SPI_WriteComm(0X12);SPI_WriteData(0X00);

SPI_WriteComm(0X13);SPI_WriteData(0X00);

SPI_WriteComm(0X14);SPI_WriteData(0X00);

SPI_WriteComm(0X15);SPI_WriteData(0XC0);

SPI_WriteComm(0X16);SPI_WriteData(0X08);

SPI_WriteComm(0X17);SPI_WriteData(0X00);

SPI_WriteComm(0X18);SPI_WriteData(0X00);

SPI_WriteComm(0X19);SPI_WriteData(0X00);

SPI_WriteComm(0X1A);SPI_WriteData(0X00);

SPI_WriteComm(0X1B);SPI_WriteData(0X00);

SPI_WriteComm(0X1C);SPI_WriteData(0X00);

SPI_WriteComm(0X1D);SPI_WriteData(0X00);


SPI_WriteComm(0X20);SPI_WriteData(0X01);

SPI_WriteComm(0X21);SPI_WriteData(0X23);

SPI_WriteComm(0X22);SPI_WriteData(0X45);

SPI_WriteComm(0X23);SPI_WriteData(0X67);

SPI_WriteComm(0X24);SPI_WriteData(0X01);

SPI_WriteComm(0X25);SPI_WriteData(0X23);

SPI_WriteComm(0X26);SPI_WriteData(0X45);

SPI_WriteComm(0X27);SPI_WriteData(0X67);


SPI_WriteComm(0X30);SPI_WriteData(0X12);

SPI_WriteComm(0X31);SPI_WriteData(0X22);

SPI_WriteComm(0X32);SPI_WriteData(0X22);

SPI_WriteComm(0X33);SPI_WriteData(0X22);

SPI_WriteComm(0X34);SPI_WriteData(0X87);

SPI_WriteComm(0X35);SPI_WriteData(0X96);

SPI_WriteComm(0X36);SPI_WriteData(0XAA);

SPI_WriteComm(0X37);SPI_WriteData(0XDB);

SPI_WriteComm(0X38);SPI_WriteData(0XCC);

SPI_WriteComm(0X39);SPI_WriteData(0XBD);

SPI_WriteComm(0X3A);SPI_WriteData(0X78);

SPI_WriteComm(0X3B);SPI_WriteData(0X69);

SPI_WriteComm(0X3C);SPI_WriteData(0X22);

SPI_WriteComm(0X3D);SPI_WriteData(0X22);

SPI_WriteComm(0X3E);SPI_WriteData(0X22);

SPI_WriteComm(0X3F);SPI_WriteData(0X22);

SPI_WriteComm(0X40);SPI_WriteData(0X22);

SPI_WriteComm(0X52);SPI_WriteData(0X10);

SPI_WriteComm(0X53);SPI_WriteData(0X10);


//SPI_WriteComm(0xFF); // Change to Page 7
//SPI_WriteData(0xFF); 
//SPI_WriteData(0x98); 
//SPI_WriteData(0x06);
//SPI_WriteData(0x04); 
//SPI_WriteData(0x07);

////SPI_WriteComm(0X17);SPI_WriteData(0X22);

//SPI_WriteComm(0X18);SPI_WriteData(0X1D);

////SPI_WriteComm(0X02);SPI_WriteData(0X77);

////SPI_WriteComm(0XE1);SPI_WriteData(0X79);

////SPI_WriteComm(0X06);SPI_WriteData(0X01);


//****************************************************************************//
SPI_WriteComm(0xFF); // Change to Page 0
SPI_WriteData(0xFF); 
SPI_WriteData(0x98); 
SPI_WriteData(0x06);
SPI_WriteData(0x04); 
SPI_WriteData(0x00);

SPI_WriteComm(0X36);SPI_WriteData(0X02);
SPI_WriteComm(0X3A);SPI_WriteData(0X77);


SPI_WriteComm(0X11);              // Sleep-Out
LCD_delay(120);
SPI_WriteComm(0X29);               // Display On

}
#if TK020F9168
void BlockWrite(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend) 
{

	WriteComm(0x406);   
	WriteData(Ystart);
	WriteComm(0x407);   
	WriteData(Yend);
	WriteComm(0x408);   
	WriteData(Xstart);
	WriteComm(0x409);   
	WriteData(Xend);
	
	WriteComm(0x200);  //特别说明，从 Xstart开始，Yend结束，而不是全部从x与y的开头起始
	WriteData(Yend);
	WriteComm(0x201);   
	WriteData(Xstart);

	WriteComm(0x202);
}
#elif TFT1P1061
void BlockWrite(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend) 
{
	LCD_WR_REG(0x80,Ystart>>8); // Set CAC=0x0000 
	LCD_WR_REG(0x81,Ystart);   
	LCD_WR_REG(0x82,Xstart>>8); // Set RAC=0x0000 
	LCD_WR_REG(0x83,Xstart);
	
	LCD_WR_REG(0x02,Xstart>>8); 
	LCD_WR_REG(0x03,Xstart);     //Column Start 
	LCD_WR_REG(0x04,Xend>>8); 
	LCD_WR_REG(0x05,Xend);     //Column End 
	 
	LCD_WR_REG(0x06,Ystart>>8); 
	LCD_WR_REG(0x07,Ystart);     //Row Start 
	LCD_WR_REG(0x08,Yend>>8); 
	LCD_WR_REG(0x09,Yend);     //Row End 

	WriteComm(0x22);
}
#else
void BlockWrite(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend) 
{
#if TK043F1508	
	WriteComm(0x2a00);   
	WriteData(Xstart>>8);
	WriteComm(0x2a01); 
	WriteData(Xstart);
	WriteComm(0x2a02); 
	WriteData(Xend>>8);
	WriteComm(0x2a03); 
	WriteData(Xend);

	WriteComm(0x2b00);   
	WriteData(Ystart>>8);
	WriteComm(0x2b01); 
	WriteData(Ystart);
	WriteComm(0x2b02); 
	WriteData(Yend>>8);
	WriteComm(0x2b03); 
	WriteData(Yend);

	WriteComm(0x2c00);
#elif TK022RB417
	WriteComm(0x0020);WriteData(Ystart);//H Start
	WriteComm(0x0021);WriteData(Xstart);//V Start

	WriteComm(0x0050);WriteData(Ystart);
	WriteComm(0x0051);WriteData(Yend);
	WriteComm(0x0052);WriteData(Xstart);
	WriteComm(0x0053);WriteData(Xend);
	WriteComm(0x0022);
#else
	WriteComm(0x2a);
	WriteData(Xstart>>8);
	WriteData(Xstart);
	WriteData(Xend>>8);
	WriteData(Xend);

	WriteComm(0x2b);   
	WriteData(Ystart>>8);
	WriteData(Ystart);
	WriteData(Yend>>8);
	WriteData(Yend);
	
	WriteComm(0x2c);
#endif
}
#endif
void LCD_DrawLineH(int x0, int x1,  int y, int LCD_COLORINDEX)
{
#if TK020F9168
	u32 i,j;
	BlockWrite(x0,x1,y,y);
	TK80->CR = 0x000c0b04;//软件产生CS，拉低CS
	j=x1-x0+1;
	for(i=0;i<j;i++)
	{
		WriteData(LCD_COLORINDEX);
	}
	TK80->CR = 0x000c0b06;//软件产生CS，拉高CS
#else	
	BlockWrite(x0,x1,y,y);
	TK80->CFGR3=x1-x0+1;
		TK80->DINR = LCD_COLORINDEX;
	while(TK80->SR & 0x10000);
#endif
}
void LCD_DrawLineV(int x, int y0,  int y1, int LCD_COLORINDEX)
{
#if TK020F9168
	u32 i,j;
	BlockWrite(x,x,y0,y1);
	TK80->CR = 0x000c0b04;//软件产生CS，拉低CS
	j=y1-y0+1;
	for(i=0;i<j;i++)
	{
		WriteData(LCD_COLORINDEX);
	}
	TK80->CR = 0x000c0b06;//软件产生CS，拉高CS
#else	
	BlockWrite(x,x,y0,y1);
	TK80->CFGR3=y1-y0+1;
		TK80->DINR = LCD_COLORINDEX;
	while(TK80->SR & 0x10000);
#endif
}
/*
*********************************************************************************************************
*	函 数 名: DrawHColorLine
*	功能说明: 绘制一条彩色水平线 （主要用于UCGUI的接口函数）
*	形    参：_usX1    ：起始点X坐标
*			  _usY1    ：水平线的Y坐标
*			  _usWidth ：直线的宽度
*			  _pColor : 颜色缓冲区
*	返 回 值: 无
*********************************************************************************************************
*/
#if TK020F9168
void LCD_DrawHColorLine(U16 x1 , U16 y1, U16 Width, U16 *_pColor)
{
	BlockWrite(x1,x1+Width-1,y1,y1);
	TK80->CR = 0x000c0b04;//软件产生CS，拉低CS
	for (x1 = 0; x1 < Width; x1++)
	{
		WriteData(*_pColor++);
	}
	TK80->CR = 0x000c0b06;//软件产生CS，拉高CS
}
#elif TFT1P1061 | USE_16bit_LCD
void LCD_DrawHColorLine(U16 x1 , U16 y1, U16 Width, U16 *_pColor)
{
	BlockWrite(x1,x1+Width-1,y1,y1);
	for (x1 = 0; x1 < Width; x1++)
	{
		WriteData(*_pColor++);
	}
}
#else
void LCD_DrawHColorLine(U16 x1 , U16 y1, U16 Width, U32 *_pColor)
{
	BlockWrite(x1,x1+Width-1,y1,y1);
	TK80_DMA_Init((u32)_pColor,Width);
//	y1=x1+Width;
//	for (x1 = 0; x1 < y1; x1++)
//	{
//		TK80->DINR =  *_pColor++;
//	}
}
#endif
#if TK020F9168|TFT1P1061
void LCD_PutPixel(U16 x,U16 y,int PixelIndex)
{
	BlockWrite(x,x,y,y);
	WriteData(PixelIndex);
}
#else
void LCD_PutPixel(U16 x,U16 y,int PixelIndex)
{
#if TK043F1508	
	WriteComm(0x2a00);   
	WriteData(x>>8);
	WriteComm(0x2a01); 
	WriteData(x);

	WriteComm(0x2b00);   
	WriteData(y>>8);
	WriteComm(0x2b01); 
	WriteData(y);

	WriteComm(0x2c00);while(TK80->SR & 0x10000);
#else
	BlockWrite(x,x,y,y);
#endif
	TK80->DINR = PixelIndex;
}
#endif
 int  LCD_GetPixel(U16 x,U16 y)
{
int i;
#if TFT1P1061
	u16 dat;
 	LCD_WR_REG(0x80,x>>8); // Set CAC=0x0000 
	LCD_WR_REG(0x81,x);   
	LCD_WR_REG(0x82,y>>8); // Set RAC=0x0000 
	LCD_WR_REG(0x83,y);
	WriteComm(0x22);

	dat = TK80->DOUTR;
	dat = TK80->DOUTR;
	dat = (dat&0xf800)|((dat&0x00fc)<<3)|(TK80->DOUTR>>11);
	return dat; 
#elif TK043F1508
	WriteComm(0x2a00);   
	WriteData(x>>8);
	WriteComm(0x2a01); 
	WriteData(x);

	WriteComm(0x2b00);   
	WriteData(y>>8);
	WriteComm(0x2b01); 
	WriteData(y);

	WriteComm(0x2E00);while(TK80->SR & 0x10000);
	TK80->CFGR1 = 0x05180505;
  TK80->CFGR2 = 0x0505;
	i = TK80->BRDR;
	TK80->CFGR1 = 0x05050102;
  TK80->CFGR2 = 0x0501;
	return(i);
#elif TK022RB417
	WriteComm(0x0020);WriteData(y);//H Start
	WriteComm(0x0021);WriteData(x);//V Start

	WriteComm(0x0050);WriteData(y);
	WriteComm(0x0051);WriteData(y);
	WriteComm(0x0052);WriteData(x);
	WriteComm(0x0053);WriteData(x);
	WriteComm(0x0022);while(TK80->SR & 0x10000);
//	TK80->CFGR1 = 0x05050505;
//  TK80->CFGR2 = 0x0505;
	i = TK80->DOUTR;while(TK80->SR & 0x10000);
	i = TK80->DOUTR;while(TK80->SR & 0x10000);
//	TK80->CFGR1 = 0x05050102;
//  TK80->CFGR2 = 0x0501;
	return(i);
#else
	WriteComm(0x2a);   
	WriteData(x>>8);
	WriteData(x&0xff);
	WriteData(x>>8);
	WriteData(x&0xff);

	WriteComm(0x2b);   
	WriteData(y>>8);
	WriteData(y&0xff);
	WriteData(y>>8);
	WriteData(y&0xff);
//BlockWrite(x,x,y,y);
	WriteComm(0x2E);while(TK80->SR & 0x10000);
	
//	x = TK80->DOUTR;while(TK80->SR & 0x10000);
//	x = TK80->DOUTR;while(TK80->SR & 0x10000);
//	x = (x&0xf800)|((x&0x00fc)<<3);
//	y = TK80->DOUTR;while(TK80->SR & 0x10000);

//	return x|(y>>11);
	TK80->CFGR1 = 0x05080505;//把速度调低
  TK80->CFGR2 = 0x0506;
	x = TK80->DOUTR;while(TK80->SR & 0x10000);
	i = TK80->DOUTR;while(TK80->SR & 0x10000);
	TK80->CFGR1 = 0x00050202;//恢复高速写入速度
  TK80->CFGR2 = 0x0502;
	return(i);
#endif
}


/*********************************************************************
*
*       LCD_X_Config
*
* Purpose:
*   Called during the initialization process in order to set up the
*   display driver configuration.
*   
*/
/****************************************************************************
* 名    称：LCD_X_Config(void)
* 功    能：显示器的驱动配置
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/ 
#if LCD_MODE_MCU_or_RGB
	extern __align(256) U32 LTDC_emWin[XSIZE_PHYS*YSIZE_PHYS];
#endif
void LCD_X_Config(void) 
{
//#if (NUM_BUFFERS > 1)
//	int i;	
//	for (i = 0; i < GUI_NUM_LAYERS; i++) {
//    GUI_MULTIBUF_ConfigEx(i, NUM_BUFFERS);
//	}
//#endif
	
  /* 设置第一层的显示驱动和颜色转换 */
  GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER, COLOR_CONVERSION, 0, 0);
  /* 公共显示驱动配置 */
  LCD_SetSizeEx(0, YSIZE_PHYS, XSIZE_PHYS);               // 实际显示像素
#if LCD_MODE_MCU_or_RGB
	LCD_SetVRAMAddrEx(0, LTDC_emWin);
	LTDC->DP_ADDR0 = (U32)LTDC_emWin;//第0层地址
#endif
  if (LCD_GetSwapXY()) 
  {
    LCD_SetSizeEx (0, YSIZE_PHYS, XSIZE_PHYS);
    LCD_SetVSizeEx(0, YSIZE_PHYS * NUM_VSCREENS, XSIZE_PHYS);
  } 
  else 
  {
    LCD_SetSizeEx (0, XSIZE_PHYS, YSIZE_PHYS);
    LCD_SetVSizeEx(0, XSIZE_PHYS, YSIZE_PHYS * NUM_VSCREENS);
  }							     

  #if (GUI_SUPPORT_TOUCH == 1)
    /*设置显示方向*/
//    GUI_TOUCH_SetOrientation(GUI_SWAP_XY);//GUI_MIRROR_Y DISPLAY_ORIENTATION
    
    /* 校准触摸屏 */  
		#if LCD_MODE_MCU_or_RGB
		GUI_TOUCH_Calibrate(GUI_COORD_X, 0,   HEIGHT - 1,TOUCH_LEFT,  TOUCH_RIGHT);
    GUI_TOUCH_Calibrate(GUI_COORD_Y, 0, WIDTH - 1, TOUCH_TOP, TOUCH_BOTTOM);
		#else
		GUI_TOUCH_Calibrate(GUI_COORD_X, 0,  WIDTH  - 1,TOUCH_LEFT,  TOUCH_RIGHT);
    GUI_TOUCH_Calibrate(GUI_COORD_Y, 0, HEIGHT - 1, TOUCH_TOP, TOUCH_BOTTOM);
		#endif
  #endif
}

/*********************************************************************
*
*       LCD_X_DisplayDriver
*
* Purpose:
*   This function is called by the display driver for several purposes.
*   To support the according task the routine needs to be adapted to
*   the display controller. Please note that the commands marked with
*   'optional' are not cogently required and should only be adapted if 
*   the display controller supports these features.
*
* Parameter:
*   LayerIndex - Index of layer to be configured
*   Cmd        - Please refer to the details in the switch statement below
*   pData      - Pointer to a LCD_X_DATA structure
*
* Return Value:
*   < -1 - Error
*     -1 - Command not handled
*      0 - OK
*/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) {
  int r;

  switch (Cmd) {
  case LCD_X_SHOWBUFFER: { 
    // 
    // Required if multiple buffers are used. The 'Index' element of p contains the buffer index. 
    // 
//    LCD_X_SHOWBUFFER_INFO * p; 

//    p = (LCD_X_SHOWBUFFER_INFO *)pData; 
//    _aPendingBuffer[LayerIndex] = p->Index; 
    break; 
} 
  case LCD_X_INITCONTROLLER: {
    //
    // Called during the initialization process in order to set up the
    // display controller and put it into operation. If the display
    // controller is not initialized by any external routine this needs
    // to be adapted by the customer...
    //
//    _InitController();
    return 0;
  }
  default:
    r = -1;
  }
  return r;
}

/*************************** End of file ****************************/
