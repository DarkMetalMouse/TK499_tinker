#include "HAL_conf.h"
#include "TK499_timer.h"
#include "TK499_GPIO.h"
#include "stdio.h"

 void TIM3_Config(u16 arr,u16 psc)
{
   TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
   NVIC_InitTypeDef NVIC_InitStruct;        //�жϲ����ṹ��
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE); //TIM3ʱ��ʹ��
   TIM_TimeBaseStructure.TIM_Period = arr; //��װ��ֵ        
   TIM_TimeBaseStructure.TIM_Prescaler =psc; //Ԥ��Ƶֵ
   TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //���������ù�
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //���ϼ���
   TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //��ʼ��
        //�ж����ȼ�NVIC���� 
   TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //ʹ��TIM3�жϣ���������ж�
   NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
   NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;  //ռ�����ȼ�1
   NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3
   NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
   NVIC_Init(&NVIC_InitStruct);  //��ʼ��NVIC�Ĵ���
        
   TIM_Cmd(TIM3, ENABLE);  //ʹ��TIM8 
}
//void TIM3_IRQHandler(void)   //TIM3�ж�
//{
//	static int i=1;
//	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
//		{
//				if(i)
//				{
//					GPIO_SetBits(GPIOD, GPIO_Pin_8); //PD8����ߵ�ƽ������LED
//					i=0;
//				}
//				else 
//				{
//					GPIO_ResetBits(GPIOD, GPIO_Pin_8);//PD8����͵�ƽ��Ϩ��LED
//					i=1;
//				}
//				
//		}
//			TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //���TIMx�����жϱ�־ 
//}
//ͨ�ö�ʱ��8�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��8!
void TIM8_Config(u32 psc,u32 arr)
{
   TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
   NVIC_InitTypeDef NVIC_InitStruct;        //�жϲ����ṹ��
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM8,ENABLE); //TIM8ʱ��ʹ��
   TIM_TimeBaseStructure.TIM_Period = arr; //��װ��ֵ        
   TIM_TimeBaseStructure.TIM_Prescaler =psc; //Ԥ��Ƶֵ
   TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //���������ù�
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //���ϼ���
   TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure); //��ʼ��
        //�ж����ȼ�NVIC���� 
   TIM_ITConfig(TIM8,TIM_IT_Update,ENABLE ); //ʹ��TIM8�жϣ���������ж�
   NVIC_InitStruct.NVIC_IRQChannel = TIM8_IRQn;  //TIM6�ж�
   NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;  //ռ�����ȼ�0
   NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;  //�����ȼ�0
   NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
   NVIC_Init(&NVIC_InitStruct);  //��ʼ��NVIC�Ĵ���
}

void TIM8_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM8, TIM_IT_Update) != RESET)  //���TIM8�����жϷ������
		{
				TIM_Cmd(TIM8, DISABLE);  //ʧ��TIM8 		
		}
			TIM_ClearITPendingBit(TIM8, TIM_IT_Update  );  //���TIMx�����жϱ�־ 
}

void TIM10_Config(u16 arr,u16 psc)
{
   TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
   NVIC_InitTypeDef NVIC_InitStruct;        //�жϲ����ṹ��
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM10,ENABLE); //TIM8ʱ��ʹ��
   TIM_TimeBaseStructure.TIM_Period = arr; //��װ��ֵ        
   TIM_TimeBaseStructure.TIM_Prescaler =psc; //Ԥ��Ƶֵ
   TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //���������ù�
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //���ϼ���
   TIM_TimeBaseInit(TIM10, &TIM_TimeBaseStructure); //��ʼ��
        //�ж����ȼ�NVIC���� 
   TIM_ITConfig(TIM10,TIM_IT_Update,ENABLE ); //ʹ��TIM10�жϣ���������ж�
   NVIC_InitStruct.NVIC_IRQChannel = TIM10_IRQn;  //TIM10�ж�
   NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;  //ռ�����ȼ�0
   NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;  //�����ȼ�0
   NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
   NVIC_Init(&NVIC_InitStruct);  //��ʼ��NVIC�Ĵ���
}

void TIM10_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM10, TIM_IT_Update) != RESET)  //���TIM10�����жϷ������
		{	
//			if((Normal_flag_out==2)&&(Hight_flag==0))
//			{	
//				Flag_Hight_OFF;Flag_normal_ON;
//			}
//			else 
//			{
//				TIM_Cmd(TIM10, DISABLE);  //ʧ��TIM10 
//				Flag_normal_OFF;
//			}
//			Normal_flag_out=1;
		}
	TIM_ClearITPendingBit(TIM10, TIM_IT_Update  );  //���TIMx�����жϱ�־
}









