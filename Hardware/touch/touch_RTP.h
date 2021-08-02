
#ifndef  __TOUCH_RTP_H__
#define  __TOUCH_RTP_H__

//#ifdef  use_XPT2046_resistance_touch_panel
#include "tk499.h"


// A/D ͨ��ѡ�������ֺ͹����Ĵ���
 #define	CHX 	0xd0 	//ͨ��Y+��ѡ�������	
 #define	CHY 	0x90	//ͨ��X+��ѡ�������
//#define	CHX 	0x90 	//ͨ��Y+��ѡ�������	
//#define	CHY 	0xd0	//ͨ��X+��ѡ�������


#define TP_CS(a)	\
						if (a)	\
						GPIOB->BSRR = GPIO_Pin_3;	\
						else		\
						GPIOB->BRR = GPIO_Pin_3
#define TP_DCLK(a)	\
						if (a)	\
						GPIOB->BSRR = GPIO_Pin_2;	\
						else		\
						GPIOB->BRR = GPIO_Pin_2
#define TP_DIN(a)	\
						if (a)	\
						GPIOB->BSRR = GPIO_Pin_0;	\
						else		\
						GPIOB->BRR = GPIO_Pin_0

#define TP_DOUT		GPIOB->IDR & GPIO_Pin_1//	//��������

//void Touch_GPIO_Config(void);

int GUI_TOUCH_X_MeasureX(void); 
int GUI_TOUCH_X_MeasureY(void);
void TP_GetAdXY(unsigned int *x,unsigned int *y);

#endif                                     
//#endif
