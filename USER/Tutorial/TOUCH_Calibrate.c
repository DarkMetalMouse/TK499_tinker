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
File        : TOUCH_Calibrate.c
Purpose     : Demonstrates how a touch screen can be calibrated at run time
Requirements: WindowManager - (X)
              MemoryDevices - ( )
              AntiAliasing  - ( )
              VNC-Server    - ( )
              PNG-Library   - ( )
              TrueTypeFonts - ( )
---------------------------END-OF-HEADER------------------------------
*/

#include "DIALOG.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define NUM_CALIB_POINTS  5  // Number of points for calibration

/*********************************************************************
*
*       Static Data
*
**********************************************************************
*/
static int _aRefX[NUM_CALIB_POINTS];
static int _aRefY[NUM_CALIB_POINTS];
static int _aSamX[NUM_CALIB_POINTS];
static int _aSamY[NUM_CALIB_POINTS];

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _Calibrate
*
* Purpose:
*   Calibrates the touch screen. Please note that this sample assumes
*   that the 'Driver API for analog touch screens' is used.
*/
static void _Calibrate(void) {
  GUI_PID_STATE State;
  int i;
  int xSize, ySize;

  xSize = LCD_GetXSize();
  ySize = LCD_GetYSize();
  //
  // Calculate reference points depending on LCD size
  //
  _aRefX[0] = (xSize * 5) / 100;
  _aRefY[0] = (ySize * 5) / 100;
  _aRefX[1] = xSize - (xSize * 5) / 100;
  _aRefY[1] = _aRefY[0];
  _aRefX[2] = _aRefX[1];
  _aRefY[2] = ySize - (ySize * 5) / 100;
  _aRefX[3] = _aRefX[0];
  _aRefY[3] = _aRefY[2];
  _aRefX[4] = xSize / 2;
  _aRefY[4] = ySize / 2;
  //
  // Draw reference points on LCD
  //
  GUI_TOUCH_GetState(&State);
  State.Pressed = 0;
  GUI_SetPenSize(3);
  for (i = 0; i < NUM_CALIB_POINTS; i++) {
    GUI_Clear();
    GUI_DispStringHCenterAt("Please touch the point", LCD_GetXSize() / 2, LCD_GetYSize() / 2 - 60);
    GUI_DrawCircle(_aRefX[i], _aRefY[i], 5);
    while (State.Pressed != 1) {
      GUI_Delay(250);
      GUI_TOUCH_GetState(&State);
    }
    if (State.Pressed == 1) {
      //
      // Store sample points
      //
      _aSamX[i] = GUI_TOUCH_GetxPhys();
      _aSamY[i] = GUI_TOUCH_GetyPhys();
    }
    State.Pressed = 0;
    GUI_Delay(250);
  }
  //
  // Pass measured points to emWin
  //
  GUI_TOUCH_CalcCoefficients(NUM_CALIB_POINTS, _aRefX, _aRefY, _aSamX, _aSamY, xSize, ySize);
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
  // Calibrate touch ...
  //
  _Calibrate();
  //
  // ... and play with it
  //
  GUI_CURSOR_Show();
  BUTTON_Create(100, 100, 200, 50, 1234, WM_CF_SHOW);  // A button to be touched
  while (1) {
    GUI_Delay(100);
  }
}

/*************************** End of file ****************************/
