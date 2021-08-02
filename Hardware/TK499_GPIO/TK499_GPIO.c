#include "TK499_GPIO.h"
#include "LCDCONF.H"
#include "Tiky_LCD_APP.h"
void GPIO_TK80_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;//定义GPIO初始化结构体变量

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOD | RCC_AHBPeriph_GPIOE, ENABLE);
	
	GPIO_PinAFConfig(GPIOA, GPIO_Pin_14 | GPIO_Pin_15, GPIO_AF_GPIO); //PA14、PA15复用普通GPIO
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// CS=PB11, RS=PB10, WR=PB9, RD=PB8
	GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//PE0~PE23在用24位1600万色时，全部作为液晶屏数据线;如果是用18位，则只需要初始化低18位;如果是用16位，则只初始化16位，以此类推
  GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11, GPIO_AF_TK80_SDIO); //PB8~11复用为TK80的控制信号线
	GPIO_PinAFConfig(GPIOE, GPIO_Pin_All, GPIO_AF_TK80_SDIO); //GPIOE所有的IO全部复用为TK80的数据线

	//lcd_reset:PD6      LCD_Black_Light On Off  :PD8
	GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_6 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
}
void GPIO_RGB_INIT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;//定义GPIO初始化结构体变量

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
	GPIO_PinAFConfig(GPIOB, GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, GPIO_AF_LTDC); //PB4~7复用为LTDC的同步信号线
//	LCD_Reset(); 
	LCD_init_code();

  GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	
	GPIO_PinAFConfig(GPIOE, GPIO_Pin_All, GPIO_AF_LTDC); //GPIOE所有的IO全部复用为LTDC的数据线
}
void Touch_Pad_Config(void)
{
	TK499_NVIC_Init(3,3,TOUCHPAD_IRQn,3);//抢占 1，子优先级 3，组 2	
	
	RCC->APB2ENR |= 0x1<<8;
	RCC->APB2ENR |= 0x1<<25;
	
	GPIOB->CRL &= 0xffff0000;                
	GPIOB->CRL |= 0xbbbb;	    
	GPIOB->AFRL &= ~0xffff; 
	GPIOB->AFRL |= 0xdddd;
	 
	
	TOUCHPAD->ADCFG =0x3E1E70;      //16分频   AD比较器禁用
	TOUCHPAD->ADCR = 0x400;      //0x200单周期模式;0x400连续扫描模式
	TOUCHPAD->ADCHS = 0xc;      //通道使能 6,7,8,9
//	TOUCHPAD->TPCSR = 0x10;   //X\Y通道选择 1，0
	TOUCHPAD->TPFR = 0x0ffffff;    //采样1次 有效阈值为0xfff

	TOUCHPAD->TPCR = 0x1;      //使能触摸屏模式
	TOUCHPAD->TPCR |= 0x2;       //触摸屏中断使能
	
//	TOUCHPAD->ADCR |=0x1;        //使能ADC中断
	
	TOUCHPAD->ADCFG |= 0x1;      //ADC使能
	TOUCHPAD->ADCR |= 0x1<<8;  //A/D转换开始 (ADC start)
}
