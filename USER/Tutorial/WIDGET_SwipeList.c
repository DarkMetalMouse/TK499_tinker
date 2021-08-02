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
File        : WIDGET_Spinbox.c
Purpose     : Shows how to use the SPINBOX widget.
Requirements: WindowManager - (x)
              MemoryDevices - ( )
              AntiAliasing  - ( )
              VNC-Server    - ( )
              PNG-Library   - ( )
              TrueTypeFonts - ( )
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"
#include "DIALOG.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
//
// Recommended memory to run the sample with adequate performance
//
#define RECOMMENDED_MEMORY (1024L * 10)

#define COLOR_GREEN     GUI_MAKE_COLOR(0x14FF39)  // Define a color
#define NUM_SEPARATORS  3                         // Number of available separators
#define NUM_ITEMS       5                         // Number of available items

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
//
// This array contains information about each item
//
static const char * paItemText[3][5] = {
  {
  "Wifi",
  "Mobile Data",
  "Bluetooth",
  "Hot Spot",
  "Mobil",
  },
  {
  "Brightness",
  "Background",
  "Idle",
  "Night Mode",
  "Font Size",
  },
  {
  "Account",
  "Sound",
  "Memory",
  "Alert",
  "Clock",
  },
};

//
// Information about the separators
//
static const char * paSeperatorText[] = {
  "Network",
  "Display",
  "Settings"
};

/*********************************************************************
*
*       MainTask
*/
void MainTask(void) {
  WM_HWIN hSwipelist;
  WM_HWIN hItem;
  int     i;
  int     j;

#ifdef WIN32
  WM_SetCreateFlags(WM_CF_MEMDEV);  // Automatic use of memory devices in simulation
#else
  WM_MULTIBUF_Enable(1);            // Automatic use of multi buffering on hardware
#endif
  GUI_Init();                       // Initialize the GUI
  //
  // Check if recommended memory for the sample is available
  //
  if (GUI_ALLOC_GetNumFreeBytes() < RECOMMENDED_MEMORY) {
    GUI_ErrorOut("Not enough memory available."); 
    return;
  }
  WM_MOTION_Enable(1);              // Enable motion support
  WM_SetBkWindowColor(GUI_BLACK);   // Set a default background color
  //
  // Set default color for selected items and separators
  //
  SWIPELIST_SetDefaultBkColor(SWIPELIST_CI_BK_ITEM_SEL, COLOR_GREEN);
  SWIPELIST_SetDefaultBkColor(SWIPELIST_CI_BK_SEP_ITEM, GUI_DARKGRAY);
  //
  // Create a swipelist
  //
  hSwipelist = SWIPELIST_CreateEx(10, 10, 200, 220, WM_HBKWIN, WM_CF_SHOW, 0, 0);
  WM_MOTION_SetMoveable(hSwipelist, WM_CF_MOTION_Y, 1);
  //
  // Add separators and five items under each
  //
  for (j = 0; j < NUM_SEPARATORS; j++) {
    SWIPELIST_AddSepItem(hSwipelist, paSeperatorText[j], 35);
    for (i = 0; i < NUM_ITEMS; i++) {
      SWIPELIST_AddItem(hSwipelist, paItemText[j][i], 35);
    }
  }
  //
  // Attaching a checkbox to each of the first five items
  //
  for (i = 0; i < 5; i++) {
    hItem = CHECKBOX_Create(0, 0, 15, 15, WM_HBKWIN, 0, WM_CF_SHOW);
    SWIPELIST_ItemAttachWindow(hSwipelist, 1 + i, hItem, 180, 10);
  }
  while (1) {
    GUI_Delay(100);
  }
}

/*************************** End of file ****************************/

