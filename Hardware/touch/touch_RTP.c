#include "LCDCONF.H"
#include  "touch_RTP.h"
#include "sys.h"

#if use_XPT2046

extern void GUI_TOUCH_X_ActivateX(void) {}
extern void GUI_TOUCH_X_ActivateY(void) {}

//*************************************************
//函数名称 : void Touch_GPIO_Config(void)  
//功能描述 : 设置触屏的SPI引脚,用软件模拟的方法实现SPI功能
//输入参数 : 
//输出参数 : 
//返回值   : 
//*************************************************

void Touch_GPIO_Config(void) 
{
	GPIOB->CRL&=0XFFFF0000;
  GPIOB->CRL|=0X00003383;
}
void Delayus(int i)
{
	while(--i);
}
//====================================================================================
static void WR_CMD (unsigned char cmd) 
{
    unsigned char buf;
    unsigned char i;
//     TP_CS(0);
    TP_DIN(0);
    TP_DCLK(0);
//     TP_CS(0);
    for(i=0;i<8;i++) 
    {
        buf=(cmd>>(7-i))&0x1;
        TP_DIN(buf);
        Delayus(5);
        TP_DCLK(1);
        Delayus(5);
        TP_DCLK(0);

    }
}
//====================================================================================
static unsigned short RD_AD(void) 
{
    unsigned short buf=0,temp;
    unsigned char i;
    TP_DIN(0);
    TP_DCLK(1);
    for(i=0;i<12;i++) 
    {
        TP_DCLK(0);    
				Delayus(5);      
        temp= (TP_DOUT) ? 1:0;
        buf|=(temp<<(11-i));
        Delayus(5);
        TP_DCLK(1);
				Delayus(5);
    }
//     TP_CS(1);
    buf&=0x0fff;
    return(buf);
}
//====================================================================================
 int GUI_TOUCH_X_MeasureX(void) 
{ 
	int i[6],temp;
	u8 cou=0,k=0;
	WR_CMD(CHX);
	RD_AD();
	while(cou<5)	//循环读数5次
	{	
		WR_CMD(CHX); 
		cou++;	
		i[cou]=RD_AD();		  
	}
        //将数据升序排列
	for(k=0;k<4;k++)
	{	  
		for(cou=1;cou<5-k;cou++)
		{
			if(i[cou]>i[cou+1])
			{
				temp=i[cou+1];
				i[cou+1]=i[cou];
				i[cou]=temp;
			}  
		}
	}
	return (i[1]+i[2]+i[3])/3;   
}
//====================================================================================
 int GUI_TOUCH_X_MeasureY(void) 
{ 
int i[6],temp;
	u8 cou=0,k=0;
	WR_CMD(CHY);
	RD_AD();
	while(cou<5)	//循环读数5次
	{	
		WR_CMD(CHY); 
		cou++;	
		i[cou]=RD_AD();		  
	}
        //将数据升序排列
	for(k=0;k<4;k++)
	{	  
		for(cou=1;cou<5-k;cou++)
		{
			if(i[cou]>i[cou+1])
			{
				temp=i[cou+1];
				i[cou+1]=i[cou];
				i[cou]=temp;
			}  
		}
	}
	return (i[1]+i[2]+i[3])/3;
}

#endif
//while(1)
//{
//	GUI_DispDecAt( GUI_TOUCH_X_MeasureX() ,120,210,4);
//	GUI_DispDecAt( GUI_TOUCH_X_MeasureY() ,230,210,4);
//	i=200000;while(--i);
//}	