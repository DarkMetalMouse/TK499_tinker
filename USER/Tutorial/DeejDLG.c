/*********************************************************************
*                                                                    *
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
*                                                                    *
**********************************************************************
*                                                                    *
* C-file generated by:                                               *
*                                                                    *
*        GUI_Builder for emWin version 5.44                          *
*        Compiled Nov 10 2017, 08:53:57                              *
*        (c) 2017 Segger Microcontroller GmbH & Co. KG               *
*                                                                    *
**********************************************************************
*                                                                    *
*        Internet: www.segger.com  Support: support@segger.com       *
*                                                                    *
**********************************************************************
*/

// USER START (Optionally insert additional includes)
#include <stdio.h>
// USER END

#include "DIALOG.h" 

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ID_FRAMEWIN_0         (GUI_ID_USER + 0x01)
#define ID_SLIDER_0         (GUI_ID_USER + 0x02)
#define ID_SLIDER_1         (GUI_ID_USER + 0x03)
#define ID_SLIDER_2         (GUI_ID_USER + 0x04)
#define ID_SLIDER_3         (GUI_ID_USER + 0x05)
#define ID_SLIDER_4         (GUI_ID_USER + 0x06)

// USER START (Optionally insert additional defines)
// USER END

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

// USER START (Optionally insert additional static data)
// USER END

/*********************************************************************
*
*       _aDialogCreate
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "Deej", ID_FRAMEWIN_0, 10, 10, 835, 470, 0, 0x0, 0 },
  // USER START (Optionally insert additional widgets)
	{ SLIDER_CreateIndirect, "Slider", ID_SLIDER_0, 20, 20, 50, 400, 8, 0x0, 0 },
  { SLIDER_CreateIndirect, "Slider", ID_SLIDER_1, 204, 20, 50, 400, 8, 0x0, 0 },
	{ SLIDER_CreateIndirect, "Slider", ID_SLIDER_2, 388, 20, 50, 400, 8, 0x0, 0 },
  { SLIDER_CreateIndirect, "Slider", ID_SLIDER_3, 571, 20, 50, 400, 8, 0x0, 0 },
	{ SLIDER_CreateIndirect, "Slider", ID_SLIDER_4, 755, 20, 50, 400, 8, 0x0, 0 },

  // USER END
};



/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

// USER START (Optionally insert additional static code)
static WM_HWIN _hSliders[5];
static int _sliderValues[5];
// USER END

/*********************************************************************
*
*       _cbDialog
*/
static void _cbDialog(WM_MESSAGE * pMsg) {
  int NCode;
  int Id;
  // USER START (Optionally insert additional variables)
  // USER END

  switch (pMsg->MsgId) {
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch(Id) {
    case ID_SLIDER_0: // Notifications sent by 'Slider'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    case ID_SLIDER_1: // Notifications sent by 'Slider'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    // USER START (Optionally insert additional code for further Ids)
    // USER END
    }
    break;
  // USER START (Optionally insert additional message handling)
  // USER END
  default:
    WM_DefaultProc(pMsg);
    break;
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
*       CreateDeej
*/
WM_HWIN CreateDeej(void);
WM_HWIN CreateDeej(void) {
  WM_HWIN hWin;
	unsigned int i;
	
  hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
	
	for(i = 1; i <= 5; i++){
		_hSliders[i-1] = WM_GetDialogItem(hWin, _aDialogCreate[i].Id);
		SLIDER_SetRange(_hSliders[i-1],0,1023);
	}
  return hWin;
}

// USER START (Optionally insert additional public code)
void UpdateDeej(void) {
	unsigned int i;
	for(i = 0; i < 5; i++)
		_sliderValues[i] = 1023-SLIDER_GetValue(_hSliders[i]);
	for(i = 0; i < 5-1; i++){
		printf("%d|",_sliderValues[i]);
	}
	printf("%d\r\n",_sliderValues[i]);
}
// USER END

/*************************** End of file ****************************/
