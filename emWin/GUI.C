#include "sys.h"
#include "LCDCONF.H"
#include "TK499_timer.h"
#include "TK499_I2C.h"
#if USE_RTP
void GUI_TOUCH_X_ActivateX(void) {}
void GUI_TOUCH_X_ActivateY(void) {}
int ADC_Value_X,ADC_Value_Y;
static u16 Temp_Value_X[10],Temp_Value_Y[10],flag_ADC=0,LED_flag=0;
static u32	temp_led=0;
void TOUCHPAD_IRQHandler()
{
	
	if(TOUCHPAD->TPCR & (0x1<<16))
	{
//		TOUCHPAD->TPCR &= (~0x2);// 禁用A/D触摸屏中断
//		TOUCHPAD->TPCR |= 0x1<<16;// 比较标志位,写1清零
		


//			ADC_Value_X = TOUCHPAD->TPYDR;
//			ADC_Value_Y = TOUCHPAD->TPXDR;
		Temp_Value_X[flag_ADC] = TOUCHPAD->TPYDR;
		Temp_Value_Y[flag_ADC] = TOUCHPAD->TPXDR;
		if(flag_ADC==9) 
		{
			u16 k,cou,temp;
			flag_ADC = 0;
			//将数据升序排列
			for(k=0;k<8;k++)
			{	  
				for(cou=0;cou<8-k;cou++)
				{
					if(Temp_Value_Y[cou]>Temp_Value_Y[cou+1])
					{
						temp=Temp_Value_Y[cou+1];
						Temp_Value_Y[cou+1]=Temp_Value_Y[cou];
						Temp_Value_Y[cou]=temp;
					}  
				}
			}
//			if((Temp_Value_Y[8]>TOUCH_Y_MAX)||(Temp_Value_Y[0]<TOUCH_Y_MIN)) {ADC_Value_Y=8000;goto restart_ADC;}
			if(Temp_Value_Y[7]-Temp_Value_Y[2]>200) goto restart_ADC; 
			for(k=0;k<8;k++)
			{	  
				for(cou=0;cou<8-k;cou++)
				{
					if(Temp_Value_X[cou]>Temp_Value_X[cou+1])
					{
						temp=Temp_Value_X[cou+1];
						Temp_Value_X[cou+1]=Temp_Value_X[cou];
						Temp_Value_X[cou]=temp;
					}  
				}
			}
//			if((Temp_Value_X[8]>TOUCH_X_MIN)||(Temp_Value_X[0]<TOUCH_X_MAX)) {ADC_Value_X=8000;goto restart_ADC;}
			if(Temp_Value_X[7]-Temp_Value_X[2]>200) goto restart_ADC; 
			
			ADC_Value_X = (Temp_Value_X[3] + Temp_Value_X[4] + Temp_Value_X[5] +Temp_Value_X[6])>>2;
			ADC_Value_Y = (Temp_Value_Y[3] + Temp_Value_Y[4] + Temp_Value_Y[5] +Temp_Value_Y[6])>>2;

			
//			if(ADC_Value_X>2100) ADC_Value_X = ADC_Value_X-200*(ADC_Value_X-2100)/((3700-500)/2);
//			if(ADC_Value_X<2100) ADC_Value_X = ADC_Value_X-(50*(2100-ADC_Value_X)/((3700-500)/2));
//			
//			if(ADC_Value_Y>2100) ADC_Value_Y = ADC_Value_Y+100*(ADC_Value_Y-2100)/((3900-280)/2);
//			if(ADC_Value_Y<2100)  ADC_Value_Y = ADC_Value_Y-(100*(2100-ADC_Value_Y)/((3900-280)/2));

			
			restart_ADC:
			flag_ADC = 0;
//			if(temp_led%100==0)
//			{
//				if(LED_flag){GPIOD->BSRR = 0x100;LED_flag=0;}
//				else {GPIOD->BRR = 0x100;LED_flag=1;}
//			}
//		temp_led++;
			 
		}
		else flag_ADC++;
		
		
		TOUCHPAD->TPCR |= 0x2;//重新使能A/D触摸屏中断
//		TOUCHPAD->ADCR |= 0x1<<8;  //A/D转换开始 (ADC start)
		
//		if(ADC_Value_X&(0x1<<21))
//		{
//			flag_x = 1;
//		}
//		ADC_Value_Y = TOUCHPAD->TPYDR;
//		if(ADC_Value_Y&(0x1<<21))
//		{
//			flag_y = 1;
//		}
		
	}

}
int  GUI_TOUCH_X_MeasureX(void)
{
	return ADC_Value_X;
}
int  GUI_TOUCH_X_MeasureY(void)
{
	return ADC_Value_Y;
}
#endif

//#define SystemCoreClock         168000000  
void GUI_X_Init(void)
{
//	unsigned long  cnts;
//    RCC_ClocksTypeDef  rcc_clocks;

//    RCC_GetClocksFreq(&rcc_clocks);
//    cnts=(unsigned long)rcc_clocks.HCLK_Frequency/1000;
//    SysTick_Config(cnts);
	
//   if (SysTick_Config(SystemCoreClock / 1000)) //SysTick配置函数
//   { 
//     while (1);
//   }  
//   NVIC_SetPriority(SysTick_IRQn, 5);//SysTick中断优先级设置
}

volatile uint32_t OS_TimeMS=0;
unsigned char MXT224_CTP_ready=0;
void TIM3_IRQHandler(void)
{
if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
{
	OS_TimeMS++;
#if GUI_SUPPORT_TOUCH&&USE_MXT224_CTP
	
	if(OS_TimeMS%15==0){if ((GPIOD->IDR & GPIO_Pin_7) == 0)MXT224_CTP_ready=1;I2C1->IC_DATA_CMD = 242;}//
	
	if(OS_TimeMS%15==5)I2C1->IC_DATA_CMD = 0 ;
	if(OS_TimeMS%15==10)
		{
				GUI_TOUCH_Exec();
				MXT224_CTP_ready=0;
		}
	
//	if(OS_TimeMS%15==0)I2C1->IC_DATA_CMD = 242;
//	if(OS_TimeMS%15==5)I2C1->IC_DATA_CMD = 0 ;
//	if(OS_TimeMS%15==10){GUI_TOUCH_Exec();}
#elif GUI_SUPPORT_TOUCH
	GUI_TOUCH_Exec();
#endif
}
TIM_ClearITPendingBit(TIM3, TIM_IT_Update );  //清除TIMx更新中断标志 
}


int GUI_X_GetTime(void)
{
	return OS_TimeMS;
}
void GUI_X_Delay(int Period)
{
		int tEnd = OS_TimeMS + Period;
//#if USE_RTP
//		while ((tEnd - OS_TimeMS) > 0) GUI_TOUCH_Exec();
//#else	
		while ((tEnd - OS_TimeMS) > 0);
//#endif
}
void GUI_X_ExecIdle(void) {}
void GUI_X_InitOS(void)    {}
void GUI_X_Unlock(void)    {}
void GUI_X_Lock(void)      {}
unsigned long  GUI_X_GetTaskId(void) { return 1; }

void GUI_X_WaitEvent(void)            {}
void GUI_X_WaitEventTimed(int Period) {}
void GUI_X_SignalEvent(void)          {}

void GUI_X_ErrorOut(const char *s) {  }