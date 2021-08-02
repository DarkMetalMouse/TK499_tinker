#include "TK499_GPIO.h"
#include "LCDCONF.H"
#include "Tiky_LCD_APP.h"
void GPIO_TK80_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;//����GPIO��ʼ���ṹ�����

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOD | RCC_AHBPeriph_GPIOE, ENABLE);
	
	GPIO_PinAFConfig(GPIOA, GPIO_Pin_14 | GPIO_Pin_15, GPIO_AF_GPIO); //PA14��PA15������ͨGPIO
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// CS=PB11, RS=PB10, WR=PB9, RD=PB8
	GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//PE0~PE23����24λ1600��ɫʱ��ȫ����ΪҺ����������;�������18λ����ֻ��Ҫ��ʼ����18λ;�������16λ����ֻ��ʼ��16λ���Դ�����
  GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11, GPIO_AF_TK80_SDIO); //PB8~11����ΪTK80�Ŀ����ź���
	GPIO_PinAFConfig(GPIOE, GPIO_Pin_All, GPIO_AF_TK80_SDIO); //GPIOE���е�IOȫ������ΪTK80��������

	//lcd_reset:PD6      LCD_Black_Light On Off  :PD8
	GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_6 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
}
void GPIO_RGB_INIT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;//����GPIO��ʼ���ṹ�����

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOD | RCC_AHBPeriph_GPIOE, ENABLE);
	
	//DE=PB4, PCLK=PB5, HSYNC=PB6, VSYNC=PB7
	GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//                              SPI_DCLK     LCD_SPI_CS
	GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_9 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_0 ;  // SPI_SDA 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	//lcd_reset:PD6      LCD_Black_Light On Off  :PD8
	GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_6 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, GPIO_AF_LTDC); //PB4~7����ΪLTDC��ͬ���ź���
//	LCD_Reset(); 
	LCD_init_code();

  GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	
	GPIO_PinAFConfig(GPIOE, GPIO_Pin_All, GPIO_AF_LTDC); //GPIOE���е�IOȫ������ΪLTDC��������
}
void Touch_Pad_Config(void)
{
	TK499_NVIC_Init(3,3,TOUCHPAD_IRQn,3);//��ռ 1�������ȼ� 3���� 2	
	
	RCC->APB2ENR |= 0x1<<8;
	RCC->APB2ENR |= 0x1<<25;
	
	GPIOB->CRL &= 0xffff0000;                
	GPIOB->CRL |= 0xbbbb;	    
	GPIOB->AFRL &= ~0xffff; 
	GPIOB->AFRL |= 0xdddd;
	 
	
	TOUCHPAD->ADCFG =0x3E1E70;      //16��Ƶ   AD�Ƚ�������
	TOUCHPAD->ADCR = 0x400;      //0x200������ģʽ;0x400����ɨ��ģʽ
	TOUCHPAD->ADCHS = 0xc;      //ͨ��ʹ�� 6,7,8,9
//	TOUCHPAD->TPCSR = 0x10;   //X\Yͨ��ѡ�� 1��0
	TOUCHPAD->TPFR = 0x0ffffff;    //����1�� ��Ч��ֵΪ0xfff

	TOUCHPAD->TPCR = 0x1;      //ʹ�ܴ�����ģʽ
	TOUCHPAD->TPCR |= 0x2;       //�������ж�ʹ��
	
//	TOUCHPAD->ADCR |=0x1;        //ʹ��ADC�ж�
	
	TOUCHPAD->ADCFG |= 0x1;      //ADCʹ��
	TOUCHPAD->ADCR |= 0x1<<8;  //A/Dת����ʼ (ADC start)
}
