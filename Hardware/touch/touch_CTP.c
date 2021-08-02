#include "gui.h"
#include  "touch_CTP.h"
#include "sys.h"
#include "tk499.h"
#include "HAL_conf.h"
#if USE_CTP | New_CTP
void GUI_TOUCH_X_ActivateX(void) {}
void GUI_TOUCH_X_ActivateY(void) {}

#define  IIC_delay  2
/****************************************************************************************
																	电容屏 I2C 底层驱动

SDA一般只有在SCL位低电平的时候才能变化，因为起始，停止信号是在SCL为高的时候SDA发生变化，为了区别，所以一般在写入/读出数据时只能在SCL为低时SDA发生变化。这点在编程的时候要特别注意
	1.IIC总线空闲的时候，SDA和SCL都为高电平，所以硬件电路中一般在SDA信号线上接一个电阻后再接VCC，注意，这里由于只工作在主机模式，没涉及切换，所以SCK直是输出模式，不用串电阻。
	
	   应答信号分为主机应答 和从机应答 ，这两者有着很大的区别。主机应答是在主机从从机中读取数据时每次读取完一个字节的数据后主机给从机的一个应答信号，表示主机已收到数据了。
	而从机应答是指主机给从机发送数据时从机给主机的应答，给一个应答就代表从机已经收到了数据，为主机接下来的工作做个判断。主机在核查从机的应答信号的时候，必须先将SDA总线拉高，释放总线。
     非应答信号是主机给从机的，当读取完一字节数据以后，主机不再去读取数据就给从机一个非应答信号，接着一个停止信号，直接给停止信号也是可以结束此次读操作，但是会对后面的操作带来影响。
	   
	所以不能图省事，养成好的习惯，非应答信号在需要的时候必须给。
	
	低电平变化，高电平稳定，所以SCL为高电平时读取SDA的数据
***************************************************************************************/
	
//*************************************************
//函数名称 : void Touch_GPIO_Config(void)  
//功能描述 : 设置触屏的IIC引脚,用软件模拟的方法实现IIC功能

//*************************************************
void Touch_GPIO_Config(void) 
{
	#if New_CTP	
	GPIOB->CRL&=0XFFFFF0FF;//电容触摸IIC_SCK信号线
  GPIOB->CRL|=0X00000300;
	GPIOB->ODR|=GPIO_Pin_0;//电容触摸IIC_SDA信号线
	#else
	GPIOB->CRH&=0XFFF0FFFF;
  GPIOB->CRH|=0X00030000;
	GPIOB->ODR|=GPIO_Pin_3;
	#endif
	I2C1_SDA_set_out();	
	I2C1_SCL(1);
	I2C1_SDA(1);
	I2C1_Stop();
}
void I2C1_Delay_us(u16 cnt)
{
volatile	u16 i;
	for(i=0;i<cnt;i++);
}

//void EXTI4_IRQHandler(void)
//{
//	if(EXTI_GetITStatus(EXTI_Line4) != RESET)
//	{
//		EXTI->PR = EXTI_Line4;  				// 清除中断标示
//    keyId = 1;
//	}
//}
int SDA_read_Bit(void)
{
	int a;
	
	#if New_CTP	//电容触摸IIC_SDA信号线
	a=GPIOB->IDR & GPIO_Pin_0;
	#else
	a=GPIOB->IDR & GPIO_Pin_3;
	#endif
	
	return a;
}
/************************************************
*	函 数 名: I2C_Start
*	功能说明: CPU发起I2C总线启动信号
在SCK高电平期间，SDA一个下降沿，即构成了IIC的起始信号
//START:when CLK is high,DATA change form high to low
**************************************************/ 
void I2C1_Start(void)  
{ 
	I2C1_SDA(1);
	I2C1_SCL(1);
	I2C1_Delay_us(IIC_delay*5);
	I2C1_SDA(0);
	I2C1_Delay_us(IIC_delay);
	I2C1_SCL(0); //钳住I2C总线，准备发送或接收数据
} 

/****************************************************
*	函 数 名: I2C_Stop
*	功能说明: CPU发起I2C总线停止信号
在SCK高电平期间，SDA一个上升沿，即构成了IIC的终止信号
//STOP:when CLK is high DATA change form low to high
****************************************************/
void I2C1_Stop(void)  
{ 
	I2C1_SDA_set_out();
	I2C1_SDA(0);
	I2C1_SCL(0);
	I2C1_Delay_us(IIC_delay);
	I2C1_SDA(1);
	I2C1_SCL(1);
	I2C1_Delay_us(IIC_delay);
}

/************************************** 
*	函 数 名: I2C_Ack
*	功能说明: CPU产生一个ACK信号
*	形    参：无
*	返 回 值: 无 
**************************************/
void I2C1_Ack(void) 
{ 
	I2C1_SCL(0);
	I2C1_SDA_set_out();
	I2C1_SDA(0);
	I2C1_Delay_us(IIC_delay);
	I2C1_SCL(1);
	I2C1_Delay_us(IIC_delay);
	I2C1_SCL(0);
} 
/*
*************************************************
*	函 数 名: I2C_NoAck
*	功能说明: CPU产生1个NACK信号
*	形    参：无
*	返 回 值: 无
*************************************************
*/
void I2C1_NoAck(void)
{
	I2C1_SCL(0);
	I2C1_SDA(1);
	I2C1_Delay_us(IIC_delay);
	I2C1_SCL(1);
	I2C1_Delay_us(IIC_delay);
	I2C1_SCL(0);	
}
/*************************************************************
*	函 数 名: I2C_WaitAck
*	功能说明: CPU产生一个时钟，并读取器件的ACK应答信号
*	形    参：无
*	返 回 值: 返回0表示正确应答，1表示无器件响应
*************************************************************/
uint8_t I2C1_WaitAck(void)
{ 
	__IO uint16_t t = 0;
	I2C1_SDA_set_in();
	I2C1_SDA(1);
	I2C1_Delay_us(IIC_delay);	
	I2C1_SCL(1);	
	I2C1_Delay_us(IIC_delay);	
	while(I2C1_SDA_Read())
	{
		t++;
		if(t>50)
		{
			I2C1_Stop();
			return 1;
		}	
	}
	I2C1_SCL(0);
	I2C1_SDA_set_out();
	return 0; 
}

void I2C1_Send_Byte(uint8_t dat)
{
	__IO uint8_t i;
	I2C1_SCL(0);
	for(i=0; i<8; i++)
	{		
		if(dat & 0x80)
		{
			I2C1_SDA(1);
		}
		else
		{
			I2C1_SDA(0);
		}
		I2C1_Delay_us(IIC_delay);
		I2C1_SCL(1);	
		I2C1_Delay_us(IIC_delay);
		I2C1_SCL(0);
		I2C1_Delay_us(IIC_delay);
		dat <<= 1;	
	}
}

/************************************************
*	函 数 名: I2C_Read_Byte
*	功能说明: CPU从I2C总线设备读取8bit数据
*	形    参：无
*	返 回 值: 读到的数据
*************************************************/
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
uint8_t I2C1_Read_Byte(uint8_t ack)
{
	
	unsigned char i,receive=0;
	I2C1_SDA_set_in();
	I2C1_SDA(1);
  for(i=0;i<8;i++ )
	{
		I2C1_SCL(0); 
    I2C1_Delay_us(IIC_delay*6);
		I2C1_SCL(1);
    receive<<=1;
    if(I2C1_SDA_Read())receive++;   
		I2C1_Delay_us(IIC_delay*6); 
   }		

   if (!ack)  I2C1_NoAck();//发送nACK
   else       I2C1_Ack(); //发送ACK   
    
	 return receive;
}




/****************************************************************************************
																	电容屏 FT6336芯片驱动

***************************************************************************************/


uint8_t FT6206_Read_Reg(uint8_t *pbuf,uint32_t len)
{
	
	int8_t i=0;

	I2C1_Start();
	I2C1_Send_Byte(FT6206_ADDR);
	I2C1_WaitAck();	
	
	I2C1_Send_Byte(0);
	I2C1_WaitAck();	
  I2C1_Stop();
  
	I2C1_Start();
	I2C1_Send_Byte(FT6206_ADDR+1);
	I2C1_WaitAck();	
	
	for(i=0;i<len;i++)
	{
		if(i==(len-1))  *(pbuf+i)=I2C1_Read_Byte(0);
		else            *(pbuf+i)=I2C1_Read_Byte(1);
	}		
	I2C1_Stop();
  
	return 0;
}

u8 a,buf[10];
volatile static u16 touchX=0,touchY=0,lastY=0;

int GUI_TOUCH_X_MeasureX(void)
{
	FT6206_Read_Reg((uint8_t*)&buf, 7);

	if ((buf[2]&0x0f) == 1)
	{
		touchX = (int16_t)(buf[5] & 0x0F)<<8 | (int16_t)buf[6];
		touchY = (int16_t)(buf[3] & 0x0F)<<8 | (int16_t)buf[4];
//		if(touchY==0)touchX=1000;
	}
	else
	{
		touchY =1000;
		touchX =1000;		
	}
	return touchY;
}

int GUI_TOUCH_X_MeasureY(void)
{
	return touchX;
}

void Touch_Test(void)
{
  int16_t x1,y1;

	GUI_SetColor(GUI_BLUE);
	GUI_SetFont(&GUI_Font32B_1);
  GUI_DispStringAt("x =",60,0);
	GUI_DispStringAt("y =",160,0);
	while(1)
	{
//		FT6206_Write_Reg1(0,0);
//		
//		a = FT6206_Read_Reg1(0);
//		GUI_DispDecAt(a, 0, 200, 4);	
//		
//		a = FT6206_Read_Reg1(0xa3);
//		GUI_DispDecAt(a, 0, 0, 4);
//		
//		a = FT6206_Read_Reg1(0xa6);
//		GUI_DispDecAt(a, 100, 0, 4);
//		
//		a = FT6206_Read_Reg1(0xa8);
//		GUI_DispDecAt(a, 200, 0, 4);

//		a = FT6206_Read_Reg1(0xa7);
//		GUI_DispDecAt(a, 300, 0, 4);	

	
		if(1)
		{
			FT6206_Read_Reg((uint8_t*)&buf, 7);
	
			if ((buf[2]&0x0f) == 1)
			{
				//读出的数据位480*800
				x1 = (int16_t)(buf[3] & 0x0F)<<8 | (int16_t)buf[4];
				y1 = (int16_t)(buf[5] & 0x0F)<<8 | (int16_t)buf[6];
			}
			else
			{
				x1 = 0xFFFF;
				y1 = 0xFFFF;
			}
	    if((x1>0)||(y1>0))
			{
				GUI_DispDecAt(x1, 100, 100, 3);
				GUI_DispDecAt(y1, 200, 100, 3);
			}		
		}

	}
}
		//=================touch Test=================//
void Touch_CTP_Test(void)
{
	int i;
	GUI_SetFont(&GUI_Font32_ASCII);//设置字体
	GUI_SetBkColor(GUI_LIGHTBLUE);//设置背景色为淡蓝色
	GUI_SetColor(GUI_BLACK);//设置前景色为黑色
	while(1)
		{
							GUI_DispDecAt( GUI_TOUCH_X_MeasureX() ,120,210,4);
							GUI_DispDecAt( GUI_TOUCH_X_MeasureY() ,230,210,4);
							i=100000;while(--i);
		}

}

#endif
