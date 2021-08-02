#include "HAL_conf.h"
#include "touch_CTP.h"
#include "TK499_I2C.h"
volatile unsigned char rx_flag = 0;
volatile unsigned char tx_flag = 0;
extern uint16_t I2C_DMA_DIR;
unsigned char tx_buffer0[16] = {0x55,1,2,3,4,5,6,7,8,9,0xa,0xb,0xc,0xd,0xe,0xf};
unsigned char rx_buffer0[16] ;

volatile unsigned short txData_Number = 0;
volatile unsigned short txData_count = 0;

#define FLASH_DEVICE_ADDR 0xa8
//#define FLASH_DEVICE_ADDR 0xa0

/********************************************************************************************************
**函数信息 ：I2CTXEmptyCheck(I2C_TypeDef *I2Cx)                 
**功能描述 ：检查发送中断标志位
**输入参数 ：I2C_TypeDef *I2Cx，选择I2C1,I2C2
**输出参数 ：无
********************************************************************************************************/
void I2CTXEmptyCheck(I2C_TypeDef *I2Cx)
{
	while(1)
	{
		if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_TX_EMPTY))
		{
			break;
		}
	}
}
/********************************************************************************************************
**函数信息 ：I2CTXEmptyCheck(I2C_TypeDef *I2Cx)                 
**功能描述 ：检查接收中断标志位
**输入参数 ：I2C_TypeDef *I2Cx，选择I2C1,I2C2
**输出参数 ：无
********************************************************************************************************/
void I2CRXFullCheck(I2C_TypeDef *I2Cx)
{
	int i=0;
	while(1)
	{
		if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_RX_FULL))
		{

			break;
		}	
		i++;
			if(i>400)//超时退出
			break;
	}
}
/********************************************************************************************************
**函数信息 ：I2CTXByte(I2C_TypeDef *I2Cx,unsigned short cmd,unsigned char temp)              
**功能描述 ：I2C发送数据
**输入参数 ：I2C_TypeDef *I2Cx，选择I2C1,I2C2；cmd；temp
**输出参数 ：无
********************************************************************************************************/
void I2CTXByte(I2C_TypeDef *I2Cx,unsigned short cmd,unsigned char temp)
{
	I2C_SendData(I2Cx, temp);
	I2CTXEmptyCheck(I2Cx);

}
/********************************************************************************************************
**函数信息 ：I2CRXByte(I2C_TypeDef *I2Cx)             
**功能描述 ：I2C接收数据
**输入参数 ：I2C_TypeDef *I2Cx，选择I2C1,I2C2
**输出参数 ：temp
********************************************************************************************************/
unsigned char I2CRXByte(I2C_TypeDef *I2Cx)
{
	unsigned char temp;

	I2CRXFullCheck(I2Cx);
	
	temp = I2C_ReceiveData(I2Cx);
	return (unsigned char)temp;
}

/********************************************************************************************************
**函数信息 ：I2CMasterWrite(I2C_TypeDef *I2Cx,unsigned char device_id, unsigned short mem_byte_addr, unsigned short tx_count, unsigned char *tx_data )             
**功能描述 ：I2C发送数据
**输入参数 ：I2C_TypeDef *I2Cx，选择I2C1,I2C2；cmd；temp
**输出参数 ：无
********************************************************************************************************/
void I2CMasterWrite(I2C_TypeDef *I2Cx,unsigned char device_id, unsigned short mem_byte_addr, unsigned short tx_count, unsigned char *tx_data )
{
	unsigned short temp;
	unsigned short i;
	unsigned char *p;
	uint32_t j;
	p = tx_data;
	
//	txData_count = tx_count;
	I2C_Cmd(I2Cx, DISABLE);
	I2C_Send7bitAddress(I2Cx, FLASH_DEVICE_ADDR, I2C_Direction_Transmitter);
	I2C_Cmd(I2Cx, ENABLE);
	temp = ((mem_byte_addr) & 0xff);
	I2CTXByte(I2Cx,CMD_WRITE,temp);  //tx memory addr

	for(i=0;i<tx_count;i++)
	{
		temp = *p;
		p++;
//		txData_Number ++;
		if(i==(tx_count-1))
			I2Cx->IC_DATA_CMD =temp| 0x200; // //muaul set stop flag
		else
		I2CTXByte(I2Cx,CMD_WRITE,temp); //tx data
		
		
	}	

for(j=0;j<0x5000;j++); 

}

/********************************************************************************************************
**函数信息 ：I2CMasterRead(I2C_TypeDef *I2Cx,unsigned char device_id, unsigned short mem_byte_addr, unsigned short rx_count, unsigned char *rx_data )           
**功能描述 ：I2C接收数据
**输入参数 ：I2C_TypeDef *I2Cx，选择I2C1,I2C2；device_id；mem_byte_addr；rx_count；rx_data
**输出参数 ：无
********************************************************************************************************/
void I2CMasterRead(I2C_TypeDef *I2Cx,unsigned char device_id, unsigned short mem_byte_addr, unsigned short rx_count, unsigned char *rx_data )
{
	unsigned char temp=0;
	unsigned short i;

	I2C_Send7bitAddress(I2Cx, FLASH_DEVICE_ADDR, I2C_Direction_Transmitter);
	
	I2C_Cmd(I2Cx, ENABLE);
	
	temp = ((mem_byte_addr) & 0xff);
	
	I2CTXByte(I2Cx,CMD_WRITE,temp);  //tx memory addr
	
	for(i=0;i<rx_count;i++)
	{
		rx_data[i] = I2CRXByte(I2Cx);
		printf("RX data%d is  : %x \r\n",i,rx_data[i]);
	}
}

/********************************************************************************************************
**函数信息 ：I2CMasterTest(I2C_TypeDef *I2Cx)          
**功能描述 ：I2C接收发送数据测试程序
**输入参数 ：I2C_TypeDef *I2Cx，选择I2C1,I2C2
**输出参数 ：无
********************************************************************************************************/
unsigned char rxBuffer[16] = {0};

void I2CMasterTest(I2C_TypeDef *I2Cx)
{
	unsigned char i;
//	I2CInitMasterMode(I2C1);
	
	I2CMasterWrite(I2C1,FLASH_DEVICE_ADDR, 16*1, 16, tx_buffer0 );
	

	for(i = 0; i < 16 ;i ++)
	{
		printf("TX data is: %x \r\n", tx_buffer0[i]);
	}

	I2CMasterRead(I2C1,FLASH_DEVICE_ADDR, 16*1, 16, rx_buffer0 );

}


