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
																	������ I2C �ײ�����

SDAһ��ֻ����SCLλ�͵�ƽ��ʱ����ܱ仯����Ϊ��ʼ��ֹͣ�ź�����SCLΪ�ߵ�ʱ��SDA�����仯��Ϊ����������һ����д��/��������ʱֻ����SCLΪ��ʱSDA�����仯������ڱ�̵�ʱ��Ҫ�ر�ע��
	1.IIC���߿��е�ʱ��SDA��SCL��Ϊ�ߵ�ƽ������Ӳ����·��һ����SDA�ź����Ͻ�һ��������ٽ�VCC��ע�⣬��������ֻ����������ģʽ��û�漰�л�������SCKֱ�����ģʽ�����ô����衣
	
	   Ӧ���źŷ�Ϊ����Ӧ�� �ʹӻ�Ӧ�� �����������źܴ����������Ӧ�����������Ӵӻ��ж�ȡ����ʱÿ�ζ�ȡ��һ���ֽڵ����ݺ��������ӻ���һ��Ӧ���źţ���ʾ�������յ������ˡ�
	���ӻ�Ӧ����ָ�������ӻ���������ʱ�ӻ���������Ӧ�𣬸�һ��Ӧ��ʹ���ӻ��Ѿ��յ������ݣ�Ϊ�����������Ĺ��������жϡ������ں˲�ӻ���Ӧ���źŵ�ʱ�򣬱����Ƚ�SDA�������ߣ��ͷ����ߡ�
     ��Ӧ���ź����������ӻ��ģ�����ȡ��һ�ֽ������Ժ���������ȥ��ȡ���ݾ͸��ӻ�һ����Ӧ���źţ�����һ��ֹͣ�źţ�ֱ�Ӹ�ֹͣ�ź�Ҳ�ǿ��Խ����˴ζ����������ǻ�Ժ���Ĳ�������Ӱ�졣
	   
	���Բ���ͼʡ�£����ɺõ�ϰ�ߣ���Ӧ���ź�����Ҫ��ʱ��������
	
	�͵�ƽ�仯���ߵ�ƽ�ȶ�������SCLΪ�ߵ�ƽʱ��ȡSDA������
***************************************************************************************/
	
//*************************************************
//�������� : void Touch_GPIO_Config(void)  
//�������� : ���ô�����IIC����,�����ģ��ķ���ʵ��IIC����

//*************************************************
void Touch_GPIO_Config(void) 
{
	#if New_CTP	
	GPIOB->CRL&=0XFFFFF0FF;//���ݴ���IIC_SCK�ź���
  GPIOB->CRL|=0X00000300;
	GPIOB->ODR|=GPIO_Pin_0;//���ݴ���IIC_SDA�ź���
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
//		EXTI->PR = EXTI_Line4;  				// ����жϱ�ʾ
//    keyId = 1;
//	}
//}
int SDA_read_Bit(void)
{
	int a;
	
	#if New_CTP	//���ݴ���IIC_SDA�ź���
	a=GPIOB->IDR & GPIO_Pin_0;
	#else
	a=GPIOB->IDR & GPIO_Pin_3;
	#endif
	
	return a;
}
/************************************************
*	�� �� ��: I2C_Start
*	����˵��: CPU����I2C���������ź�
��SCK�ߵ�ƽ�ڼ䣬SDAһ���½��أ���������IIC����ʼ�ź�
//START:when CLK is high,DATA change form high to low
**************************************************/ 
void I2C1_Start(void)  
{ 
	I2C1_SDA(1);
	I2C1_SCL(1);
	I2C1_Delay_us(IIC_delay*5);
	I2C1_SDA(0);
	I2C1_Delay_us(IIC_delay);
	I2C1_SCL(0); //ǯסI2C���ߣ�׼�����ͻ��������
} 

/****************************************************
*	�� �� ��: I2C_Stop
*	����˵��: CPU����I2C����ֹͣ�ź�
��SCK�ߵ�ƽ�ڼ䣬SDAһ�������أ���������IIC����ֹ�ź�
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
*	�� �� ��: I2C_Ack
*	����˵��: CPU����һ��ACK�ź�
*	��    �Σ���
*	�� �� ֵ: �� 
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
*	�� �� ��: I2C_NoAck
*	����˵��: CPU����1��NACK�ź�
*	��    �Σ���
*	�� �� ֵ: ��
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
*	�� �� ��: I2C_WaitAck
*	����˵��: CPU����һ��ʱ�ӣ�����ȡ������ACKӦ���ź�
*	��    �Σ���
*	�� �� ֵ: ����0��ʾ��ȷӦ��1��ʾ��������Ӧ
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
*	�� �� ��: I2C_Read_Byte
*	����˵��: CPU��I2C�����豸��ȡ8bit����
*	��    �Σ���
*	�� �� ֵ: ����������
*************************************************/
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
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

   if (!ack)  I2C1_NoAck();//����nACK
   else       I2C1_Ack(); //����ACK   
    
	 return receive;
}




/****************************************************************************************
																	������ FT6336оƬ����

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
				//����������λ480*800
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
	GUI_SetFont(&GUI_Font32_ASCII);//��������
	GUI_SetBkColor(GUI_LIGHTBLUE);//���ñ���ɫΪ����ɫ
	GUI_SetColor(GUI_BLACK);//����ǰ��ɫΪ��ɫ
	while(1)
		{
							GUI_DispDecAt( GUI_TOUCH_X_MeasureX() ,120,210,4);
							GUI_DispDecAt( GUI_TOUCH_X_MeasureY() ,230,210,4);
							i=100000;while(--i);
		}

}

#endif
