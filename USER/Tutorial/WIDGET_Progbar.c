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
File        : WIDGET_Progbar.c
Purpose     : Simple demo shows the use of the PROGBAR widget
Requirements: WindowManager - (x)
              MemoryDevices - ( )
              AntiAliasing  - ( )
              VNC-Server    - ( )
              PNG-Library   - ( )
              TrueTypeFonts - ( )
----------------------------------------------------------------------
*/

#include "GUI.h"
#include "PROGBAR.h"
#include <stddef.h>

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
//
// Recommended memory to run the sample with adequate performance
//
#define RECOMMENDED_MEMORY (1024L * 5)

/*******************************************************************
*
*       static code
*
********************************************************************
*/
/*******************************************************************
*
*       _DemoProgBar
*/
static void _DemoProgBar(void) {
  int i;
  PROGBAR_Handle ahProgBar[2];

  GUI_SetBkColor(GUI_BLACK);
  GUI_Clear();
  GUI_SetColor(GUI_WHITE);
  GUI_SetFont(&GUI_Font24_ASCII);
  GUI_DispStringHCenterAt("WIDGET_Progbar - Sample", 160, 5);
  GUI_SetFont(&GUI_Font8x16);
  GUI_DispStringAt("Progress bar", 100,80);
  //
  // Create the progbars
  //
  ahProgBar[0] = PROGBAR_Create(100,100,100,20, WM_CF_SHOW);
  ahProgBar[1] = PROGBAR_Create( 80,150,140,10, WM_CF_SHOW);
  //
  // Use memory device (optional, for better looks)
  //
  WM_EnableMemdev(ahProgBar[0]);
  WM_EnableMemdev(ahProgBar[1]);
  PROGBAR_SetMinMax(ahProgBar[1], 0, 500);
  PROGBAR_SetFont(ahProgBar[0], &GUI_Font8x16);
  GUI_Delay(500);
  while(1) {
    PROGBAR_SetFont(ahProgBar[0], &GUI_Font8x16);
    if (LCD_GetDevCap(LCD_DEVCAP_BITSPERPIXEL) <= 4) {
      PROGBAR_SetBarColor(ahProgBar[0], 0, GUI_DARKGRAY);
      PROGBAR_SetBarColor(ahProgBar[0], 1, GUI_LIGHTGRAY);
    } else {
      PROGBAR_SetBarColor(ahProgBar[0], 0, GUI_GREEN);
      PROGBAR_SetBarColor(ahProgBar[0], 1, GUI_RED);
    }
    PROGBAR_SetTextAlign(ahProgBar[0], GUI_TA_HCENTER);
    PROGBAR_SetText(ahProgBar[0], NULL);
    for (i=0; i<=100; i++) {
      PROGBAR_SetValue(ahProgBar[0], i);
      PROGBAR_SetValue(ahProgBar[1], i);
      GUI_Delay(5);
    }
    PROGBAR_SetText(ahProgBar[0], "Tank empty");
    for (; i>=0; i--) {
      PROGBAR_SetValue(ahProgBar[0], i);
      PROGBAR_SetValue(ahProgBar[1], 200-i);
      GUI_Delay(5);
    }
    PROGBAR_SetText(ahProgBar[0], "Any text...");
    PROGBAR_SetTextAlign(ahProgBar[0], GUI_TA_LEFT);
    for (; i<=100; i++) {
      PROGBAR_SetValue(ahProgBar[0], i);
      PROGBAR_SetValue(ahProgBar[1], 200+i);
      GUI_Delay(5);
    }
    PROGBAR_SetTextAlign(ahProgBar[0], GUI_TA_RIGHT);
    for (; i>=0; i--) {
      PROGBAR_SetValue(ahProgBar[0], i);
      PROGBAR_SetValue(ahProgBar[1], 400-i);
      GUI_Delay(5);
    }
    PROGBAR_SetFont(ahProgBar[0], &GUI_FontComic18B_1);
    PROGBAR_SetText(ahProgBar[0], "Any font...");
    for (; i<=100; i++) {
      PROGBAR_SetValue(ahProgBar[0], i);
      PROGBAR_SetValue(ahProgBar[1], 400+i);
      GUI_Delay(5);
    }
    GUI_Delay(500);
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       MainTask
*/
void MainTask(void) {
  GUI_Init();
  //
  // Check if recommended memory for the sample is available
  //
  if (GUI_ALLOC_GetNumFreeBytes() < RECOMMENDED_MEMORY) {
    GUI_ErrorOut("Not enough memory available."); 
    return;
  }
  while (1) {
    _DemoProgBar();
  }
}

/*************************** End of file ****************************/

