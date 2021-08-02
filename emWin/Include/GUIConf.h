/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2017  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.46 - Graphical user interface for embedded applications **
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
Licensing information
Licensor:                 SEGGER Software GmbH
Licensed to:              ARM Ltd, 110 Fulbourn Road, CB1 9NJ Cambridge, UK
Licensed SEGGER software: emWin
License number:           GUI-00181
License model:            LES-SLA-20007, Agreement, effective since October 1st 2011 
Licensed product:         MDK-ARM Professional
Licensed platform:        ARM7/9, Cortex-M/R4
Licensed number of seats: -
----------------------------------------------------------------------
File        : GUIConf.h
Purpose     : Configures emWins abilities, fonts etc.
----------------------------------------------------------------------
*/

#ifndef GUICONF_H
#define GUICONF_H

/*********************************************************************
*
*       Multi layer/display support
*/
#define GUI_NUM_LAYERS            1    // Maximum number of available layers

/*********************************************************************
*
*       Multi tasking support
*/
#define GUI_OS                    (0)  // Compile with multitasking support

/*********************************************************************
*
*       Configuration of touch support
*/

#define GUI_SUPPORT_TOUCH         (1)  // Support a touch screen 
#define USE_GT911_CTP 	 0
#define USE_MXT224_CTP 	 0
#define USE_RTP          0
#define USE_CTP          1
#define New_CTP 				 1
#define use_XPT2046      0


#define TK020F9168       0
#define TFT1P1061        0
#define TK043F1508       0
#define TK022RB417       0

#define USE_16bit_LCD    0
#define LCD_MODE_MCU_or_RGB      1  //MCU mode=0 ;RGB mode=1;
#define LCD_RGB_ORIENTATION      1  //ORIENTATION=1旋转90度   or normal=0 


/*********************************************************************
*
*       Default font
*/
#define GUI_DEFAULT_FONT          &GUI_Font6x8
#define GUI_USE_ARGB         0    // Swaps the meaning of a logical color from  ABGR to ARGB
#define GUI_SUPPORT_BIDI     0
/*********************************************************************
*
*         Configuration of available packages
*/
#define GUI_SUPPORT_MOUSE    0    // Mouse support
#define GUI_WINSUPPORT       1    // Use Window Manager
#define GUI_SUPPORT_MEMDEV   1    // Use Memory Devices
#define GUI_SUPPORT_DEVICES  1    // Enable use of device pointers
#define GUI_ALLOC_SIZE            1024*10
#endif  // Avoid multiple inclusion

// 物理显示尺寸定义

#define XSIZE_PHYS 480
#define YSIZE_PHYS 854
#if GUI_SUPPORT_TOUCH
		// Touch screen
		//触摸屏校验值
		//==============RTP 电阻触摸==============//
//		#if USE_RTP 
//		#define TOUCH_X_MIN 460
//		#define TOUCH_X_MAX 3900
//		#define TOUCH_Y_MIN 500
//		#define TOUCH_Y_MAX 3750
//		#endif

		#if USE_RTP 
		#define TOUCH_X_MIN 300
		#define TOUCH_X_MAX 3900
		#define TOUCH_Y_MIN 500
		#define TOUCH_Y_MAX 3800
		#endif

		#if use_XPT2046 
		#define TOUCH_X_MIN 150
		#define TOUCH_X_MAX 3900
		#define TOUCH_Y_MIN 230
		#define TOUCH_Y_MAX 3850
		#endif


		//==============CTP 电容触摸==============//
		#if USE_GT911_CTP
		#define TOUCH_X_MIN 1
		#define TOUCH_X_MAX 1024
		#define TOUCH_Y_MIN 600
		#define TOUCH_Y_MAX 1
		#endif
		
		#if USE_MXT224_CTP | New_CTP | USE_CTP
			#define TOUCH_X_MIN 1
			#define TOUCH_X_MAX 854
			#define TOUCH_Y_MIN 1
			#define TOUCH_Y_MAX 480
		#endif
#endif
/*************************** End of file ****************************/
