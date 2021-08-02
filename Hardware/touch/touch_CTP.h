//#ifdef  use_capacitive_touch_panel

#ifndef  __TOUCH_CTP_H__
#define  __TOUCH_CTP_H__

#include "sys.h"
#include "GUICONF.H"
#if New_CTP			
#define I2C1_SCL(x) 	\
						if (x)	\
						GPIOB->BSRR = GPIO_Pin_2;	\
						else		\
						GPIOB->BRR  = GPIO_Pin_2;
						
#define I2C1_SDA(x)	\
						if (x)	\
						GPIOB->BSRR = GPIO_Pin_0;	\
						else		\
						GPIOB->BRR  = GPIO_Pin_0;


#define	I2C1_SDA_Read()  	  	SDA_read_Bit()   
#define	I2C1_SDA_set_out();  	GPIOB->CRL = (GPIOB->CRL&0xfffffff0)|0x00000003;
#define	I2C1_SDA_set_in();  	GPIOB->CRL = (GPIOB->CRL&0xfffffff0)|0x00000008;	
#else
#define I2C1_SCL(x) 	\
						if (x)	\
						GPIOB->BSRR = GPIO_Pin_12;	\
						else		\
						GPIOB->BRR  = GPIO_Pin_12;
						
#define I2C1_SDA(x)	\
						if (x)	\
						GPIOB->BSRR = GPIO_Pin_3;	\
						else		\
						GPIOB->BRR  = GPIO_Pin_3;


#define	I2C1_SDA_Read()  	  	SDA_read_Bit()   
#define	I2C1_SDA_set_out();  	GPIOB->CRL = (GPIOB->CRL&0xffff0fff)|0x00003000;
#define	I2C1_SDA_set_in();  	GPIOB->CRL = (GPIOB->CRL&0xffff0fff)|0x00008000;
#endif

#define FT6206_ADDR		0x70		//地址为0x38要移一位 
	
void Touch_GPIO_Config(void);
						
int SDA_read_Bit(void);
void I2C1_Start(void);
void I2C1_Stop(void);
void I2C1_Ack(void);
void I2C1_NoAck(void);

void I2C1_Send_Byte(uint8_t dat);
uint8_t I2C1_Read_Byte(uint8_t ack);
uint8_t I2C1_WaitAck(void);


static uint8_t FT6206_Write_Reg(uint8_t startaddr,uint8_t *pbuf,uint32_t len);
uint8_t FT6206_Read_Reg(uint8_t *pbuf,uint32_t len);

int GUI_TOUCH_X_MeasureX(void); 
int GUI_TOUCH_X_MeasureY(void);

void TP_GetAdXY(unsigned int *x,unsigned int *y);
void Touch_CTP_Test(void);



#endif                                     
//#endif
