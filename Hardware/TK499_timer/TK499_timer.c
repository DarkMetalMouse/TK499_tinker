#include "HAL_conf.h"
#include "TK499_timer.h"
#include "TK499_GPIO.h"
#include "stdio.h"

 void TIM3_Config(u16 arr,u16 psc)
{
   TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
   NVIC_InitTypeDef NVIC_InitStruct;        //中断参数结构体
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE); //TIM3时钟使能
   TIM_TimeBaseStructure.TIM_Period = arr; //重装载值        
   TIM_TimeBaseStructure.TIM_Prescaler =psc; //预分频值
   TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //不懂，不用管
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //向上计数
   TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //初始化
        //中断优先级NVIC设置 
   TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能TIM3中断，允许更新中断
   NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
   NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;  //占先优先级1
   NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;  //次优先级3
   NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
   NVIC_Init(&NVIC_InitStruct);  //初始化NVIC寄存器
        
   TIM_Cmd(TIM3, ENABLE);  //使能TIM8 
}
//void TIM3_IRQHandler(void)   //TIM3中断
//{
//	static int i=1;
//	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
//		{
//				if(i)
//				{
//					GPIO_SetBits(GPIOD, GPIO_Pin_8); //PD8输出高电平，点亮LED
//					i=0;
//				}
//				else 
//				{
//					GPIO_ResetBits(GPIOD, GPIO_Pin_8);//PD8输出低电平，熄灭LED
//					i=1;
//				}
//				
//		}
//			TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx更新中断标志 
//}
//通用定时器8中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器8!
void TIM8_Config(u32 psc,u32 arr)
{
   TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
   NVIC_InitTypeDef NVIC_InitStruct;        //中断参数结构体
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM8,ENABLE); //TIM8时钟使能
   TIM_TimeBaseStructure.TIM_Period = arr; //重装载值        
   TIM_TimeBaseStructure.TIM_Prescaler =psc; //预分频值
   TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //不懂，不用管
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //向上计数
   TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure); //初始化
        //中断优先级NVIC设置 
   TIM_ITConfig(TIM8,TIM_IT_Update,ENABLE ); //使能TIM8中断，允许更新中断
   NVIC_InitStruct.NVIC_IRQChannel = TIM8_IRQn;  //TIM6中断
   NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;  //占先优先级0
   NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;  //次优先级0
   NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
   NVIC_Init(&NVIC_InitStruct);  //初始化NVIC寄存器
}

void TIM8_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM8, TIM_IT_Update) != RESET)  //检查TIM8更新中断发生与否
		{
				TIM_Cmd(TIM8, DISABLE);  //失能TIM8 		
		}
			TIM_ClearITPendingBit(TIM8, TIM_IT_Update  );  //清除TIMx更新中断标志 
}

void TIM10_Config(u16 arr,u16 psc)
{
   TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
   NVIC_InitTypeDef NVIC_InitStruct;        //中断参数结构体
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM10,ENABLE); //TIM8时钟使能
   TIM_TimeBaseStructure.TIM_Period = arr; //重装载值        
   TIM_TimeBaseStructure.TIM_Prescaler =psc; //预分频值
   TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //不懂，不用管
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //向上计数
   TIM_TimeBaseInit(TIM10, &TIM_TimeBaseStructure); //初始化
        //中断优先级NVIC设置 
   TIM_ITConfig(TIM10,TIM_IT_Update,ENABLE ); //使能TIM10中断，允许更新中断
   NVIC_InitStruct.NVIC_IRQChannel = TIM10_IRQn;  //TIM10中断
   NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;  //占先优先级0
   NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;  //次优先级0
   NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
   NVIC_Init(&NVIC_InitStruct);  //初始化NVIC寄存器
}

void TIM10_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM10, TIM_IT_Update) != RESET)  //检查TIM10更新中断发生与否
		{	
//			if((Normal_flag_out==2)&&(Hight_flag==0))
//			{	
//				Flag_Hight_OFF;Flag_normal_ON;
//			}
//			else 
//			{
//				TIM_Cmd(TIM10, DISABLE);  //失能TIM10 
//				Flag_normal_OFF;
//			}
//			Normal_flag_out=1;
		}
	TIM_ClearITPendingBit(TIM10, TIM_IT_Update  );  //清除TIMx更新中断标志
}









