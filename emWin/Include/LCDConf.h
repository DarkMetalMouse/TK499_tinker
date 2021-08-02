/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2011  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.12 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The software has been licensed to  ARM LIMITED whose registered office
is situated at  110 Fulbourn Road,  Cambridge CB1 9NJ,  England solely
for  the  purposes  of  creating  libraries  for  ARM7, ARM9, Cortex-M
series,  and   Cortex-R4   processor-based  devices,  sublicensed  and
distributed as part of the  MDK-ARM  Professional  under the terms and
conditions  of  the   End  User  License  supplied  with  the  MDK-ARM
Professional. 
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : LCDConf.h
Purpose     : Display driver configuration file
----------------------------------------------------------------------
*/

#ifndef LCDCONF_H
#define LCDCONF_H
#include "GUI.h"
#include "sys.h"
#include "Tiky_LCD_APP.h"


void LCD_init_code(void);//Òº¾§ÆÁ³õÊ¼»¯´úÂë
void BlockWrite(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend);
void LCD_DrawLineH(int x0, int x1,  int y , int LCD_COLORINDEX);
void LCD_DrawLineV(int x , int y0,  int y1, int LCD_COLORINDEX);
#if TK020F9168 | TFT1P1061 | USE_16bit_LCD
		void LCD_DrawHColorLine(U16 x1 , U16 y1, U16 Width, U16 *_pColor);
#else
		void LCD_DrawHColorLine(U16 x1 , U16 y1, U16 Width, U32 *_pColor);
#endif
void LCD_PutPixel(U16 x,U16 y,int PixelIndex);
int LCD_GetPixel(U16 x,U16 y);


#endif /* LCDCONF_H */

/*************************** End of file ****************************/
