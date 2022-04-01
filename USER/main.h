#ifndef MAIN_H
#define MAIN_H

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int  uint32;

#include "tk499.h"
#include "HAL_conf.h"
#include "GUI.H"
#include "GUICONF.H"
#include "LCDCONF.H"
#include "WM.H"
#include "sys.h"
#include "Tiky_LCD_APP.h"
#include "TK499_GPIO.h"
#include "TK499_timer.h"
#include "touch_CTP.h"
#include "SDIO.h"
#include "UART.h" 
#include "lcd.h" 
#include "QSPI_REG.h"
#include "qspi_fun.h"
#include "xbf_font.h"


extern void COLOR_ShowColorBar(void);
extern void WIDGET_NumPad(void);
extern void WIDGET_Effect(void);
extern void CreateDeej(void);
extern void DeejSendSliderValues(void);
extern void DeejUpdateLevels(int levels[]);

#endif
