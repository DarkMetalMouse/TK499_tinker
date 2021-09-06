#include "Tiky_LCD_APP.h"
#include "LCDCONF.H"
#include "stdio.h"
#include <string.h>

#include "sdio.h"
#include "sdio_sdcard.h" 
#include "diskio.h"	
#include "ff.h"	
#include "TK499_I2C.h"
#include "TK499_GPIO.h"
#include "main.h"

#define ALPHA_COMPOSITE(bg,fg,a) ( (((fg) * (a)) + ((bg) * (255-(a)))) >> 8 )

#if USE_GT911_CTP

void GUI_TOUCH_X_ActivateX(void) {}
void GUI_TOUCH_X_ActivateY(void) {}
//GT911读写命令	
#define GT_CMD_WR 		0XBA     	//写命令
#define GT_CMD_RD 		0XBB		//读命令
/********************************************************************************************************
**函数信息 ：I2CInitMasterMode(I2C_TypeDef *I2Cx,unsigned long apb_mhz,unsigned long i2c_baud_rate) //unit is Khz                    
**功能描述 ：初始化I2C
**输入参数 ：I2C_TypeDef *I2Cx，选择I2C1,I2C2
**输出参数 ：无
********************************************************************************************************/
void I2CInitMasterMode(I2C_TypeDef *I2Cx) 
{
	I2C_InitTypeDef I2C_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);  //i2c1 clk enable
		
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	
	GPIO_PinAFConfig(GPIOB, GPIO_Pin_0 | GPIO_Pin_2, GPIO_AF_I2C); //PA9、PA10复用为串口1
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0 | GPIO_Pin_2 ;   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; // 复用开漏输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	
	
  I2C_InitStructure.I2C_Mode = I2C_Mode_MASTER;//主模式
  I2C_InitStructure.I2C_OwnAddress = GT_CMD_WR;
  I2C_InitStructure.I2C_Speed = I2C_Speed_FAST;
  I2C_InitStructure.I2C_ClockSpeed = 400000;   //速度设置为400K
	I2C_Init(I2C1, &I2C_InitStructure);
	
	I2C_Send7bitAddress(I2C1, GT_CMD_WR, 0);
	I2C_Cmd(I2C1, ENABLE);
}
/****************************************************************************************
																	电容屏 GT911芯片驱动

***************************************************************************************/


//GT9147 部分寄存器定义 
#define GT_CTRL_REG 	0X8040   	//GT9147控制寄存器
#define GT_CFGS_REG 	0X8047   	//GT9147配置起始地址寄存器
#define GT_CHECK_REG 	0X80FF   	//GT9147校验和寄存器
#define GT_PID_REG 		0X8140   	//GT9147产品ID寄存器

#define GT_GSTID_REG 	0X814E   	//GT9147当前检测到的触摸情况
#define GT_TP1_REG 		0X8150  	//第一个触摸点数据地址
#define GT_TP2_REG 		0X8158		//第二个触摸点数据地址
#define GT_TP3_REG 		0X8160		//第三个触摸点数据地址
#define GT_TP4_REG 		0X8168		//第四个触摸点数据地址
#define GT_TP5_REG 		0X8170		//第五个触摸点数据地址  
//向GT9147写入一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:写数据长度
//返回值:0,成功;1,失败.
void GT9147_WR_Reg(u16 reg,u8 *buf,u8 len)
{
	u8 i;
	I2C1->IC_DATA_CMD =GT_CMD_WR | 0x200;//写入从机地址，开启起始信号

	I2CTXByte(I2C1,CMD_WRITE,reg>>8);   	//发送高8位地址
	 										  		   
	I2CTXByte(I2C1,CMD_WRITE,reg&0XFF);   	//发送低8位地址
  
	for(i=0;i<len;i++)
	{	   
    	I2CTXByte(I2C1,CMD_WRITE,buf[i]);  	//发数据
	}
}
//从GT9147读出一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:读数据长度			  
void GT9147_RD_Reg(u16 reg,u8 *buf,u8 len)
{
	u8 i; 
 	I2C1->IC_DATA_CMD =GT_CMD_WR | 0x200;//写入从机地址，开启起始信号
	
// 	I2CTXByte(I2C1,CMD_WRITE,GT_CMD_WR);   //发送写命令 	 

 	I2CTXByte(I2C1,CMD_WRITE,reg>>8);   	//发送高8位地址
	 										  		   
 	I2CTXByte(I2C1,CMD_WRITE,reg&0XFF);   	//发送低8位地址
	 	   
//	I2CTXByte(I2C1,CMD_WRITE,GT_CMD_RD);   //发送读命令		   
  
	for(i=0;i<len;i++)
	{	   
    	buf[i]=I2CRXByte(I2C1); //发数据	  
	} 
  
} 
u8 GT9147_Init(void)
{
	u8 temp[5];  
	GT9147_RD_Reg(GT_PID_REG,temp,4);	//读取产品ID
	temp[4]=0;
	printf("CTP ID:%s\r\n",temp);		//打印ID

	return 1;
}
#define TP_PRES_DOWN 0x80  		//触屏被按下	  
#define TP_CATH_PRES 0x40  		//有按键按下了 
#define CT_MAX_TOUCH  5    		//电容屏支持的点数,固定为5点
struct 
{ 

	u16 x[CT_MAX_TOUCH]; 		//当前坐标
	u16 y[CT_MAX_TOUCH];		//电容屏有最多5组坐标,电阻屏则用x[0],y[0]代表:此次扫描时,触屏的坐标,用
								//x[4],y[4]存储第一次按下时的坐标. 
	u8  sta;					//笔的状态 
								//b7:按下1/松开0; 
	                            //b6:0,没有按键按下;1,有按键按下. 
								//b5:保留
								//b4~b0:电容触摸屏按下的点数(0,表示未按下,1表示按下)
   
}tp_dev;
const u16 GT9147_TPX_TBL[5]={GT_TP1_REG,GT_TP2_REG,GT_TP3_REG,GT_TP4_REG,GT_TP5_REG};
int touch_n=0;
int  GUI_TOUCH_X_MeasureX(void) 
{
	u8 buf[4];
	u8 i=0;
	u8 temp;
	u8 mode;

		GT9147_RD_Reg(GT_GSTID_REG,&mode,1);	//读取触摸点的状态  
	
 		if(mode&0X80&&((mode&0XF)<6))
		{
			temp=0;
				
			GT9147_WR_Reg(GT_GSTID_REG,&temp,1);//清标志 
	
		}		
		
		if((mode&0XF)&&((mode&0XF)<6))
		{
			temp=0XFF<<(mode&0XF);		//将点的个数转换为1的位数,匹配tp_dev.sta定义 

			tp_dev.sta=(~temp)|TP_PRES_DOWN|TP_CATH_PRES; 

				if(tp_dev.sta&(1<<i))	//触摸有效?
				{
					GT9147_RD_Reg(GT9147_TPX_TBL[i],buf,4);	//读取XY坐标值
					tp_dev.x[i]=((u16)buf[1]<<8)+buf[0];
					tp_dev.y[i]=((u16)buf[3]<<8)+buf[2];
					
//					printf("%d,%d\r\n",tp_dev.x[i],tp_dev.y[i]);
//					Lcd_ColorBox(1024-tp_dev.x[i],600-tp_dev.y[i],2,2,Yellow);
					touch_n=0;
					return tp_dev.x[0];
				}  
//				
		}
		else 
		{
			touch_n++;
			if(touch_n>2)
			{
				touch_n=0;
			tp_dev.x[0]=4000;
			tp_dev.y[0]=4000;
			return 4000;
			}
			return tp_dev.x[0];
		}

	}
int GUI_TOUCH_X_MeasureY(void)
{
	return tp_dev.y[0];
}
#endif
//================================================GT911 Touch CTP end=====================================================//

//================================================MXT224 Touch CTP start=====================================================//
#if USE_MXT224_CTP

void GUI_TOUCH_X_ActivateX(void) {}
void GUI_TOUCH_X_ActivateY(void) {}
//GT911读写命令	
#define MXT224_ADDR		0x94		//地址为 0x4a<<1, 要移一位
/********************************************************************************************************
**函数信息 ：I2CInitMasterMode(I2C_TypeDef *I2Cx,unsigned long apb_mhz,unsigned long i2c_baud_rate) //unit is Khz                    
**功能描述 ：初始化I2C
**输入参数 ：I2C_TypeDef *I2Cx，选择I2C1,I2C2
**输出参数 ：无
********************************************************************************************************/
void I2CInitMasterMode(I2C_TypeDef *I2Cx) 
{
	I2C_InitTypeDef I2C_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);  //i2c1 clk enable
		
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	
	GPIO_PinAFConfig(GPIOB, GPIO_Pin_0 | GPIO_Pin_2, GPIO_AF_I2C); //PB0、PB2复用为IIC
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0 | GPIO_Pin_2 ;   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; // 复用开漏输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
//	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0  ;   
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; // 复用开漏输出
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
//	
//	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2 ;   
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//========== PD7 初始化为浮空输入
	GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
  I2C_InitStructure.I2C_Mode = I2C_Mode_MASTER;//主模式
  I2C_InitStructure.I2C_OwnAddress = MXT224_ADDR;
  I2C_InitStructure.I2C_Speed = I2C_Speed_FAST;
  I2C_InitStructure.I2C_ClockSpeed = 800000;   //速度设置为800K
	I2C_Init(I2C1, &I2C_InitStructure);
	
	I2C_Send7bitAddress(I2C1, MXT224_ADDR, 0);
	I2C_Cmd(I2C1, ENABLE);
}

static int touch_n=0,touchX=0,touchY=0;
static int16_t buf[5];
extern  unsigned char MXT224_CTP_ready;
int  GUI_TOUCH_X_MeasureX(void) 
{
	volatile int i,j,tempx,tempy;
	
//	if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_7))
//	{
//		touch_n++;
//			if(touch_n>2)
//			{
//				touch_n=0;
//				touchX=4095;
//				touchY=4095;
//				return 4095;
//			}
//		return touchX;
//	}
//if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_7))return 4095;
//	I2CTXByte(I2C1,CMD_WRITE,242);
//	I2CTXByte(I2C1,CMD_WRITE,242>>8);

	if(MXT224_CTP_ready==0)//读寄存器，清标志
	{
		i=I2CRXByte(I2C1);
		touchX=8000;
		touchY=8000;
		return touchX;
	}
	for(i=0;i<5;i++) 
		{
			*(buf+i)=I2CRXByte(I2C1);//库函数法读取IIC数据
		}
	
//	if( buf[0] <=11)//<=11 &2      //解释命令 
	{
		tempy=(buf[2]<<4)+(buf[4]>>4) ;    
		tempx=(buf[3]<<4)+(buf[4] & 0x0f); 
		if((tempx==1799)&&(tempy==676)) 
		{
				touchX=8000;
				touchY=8000;
				return touchX;
		}
		if((tempx>10)&&(tempx<3200)&&(tempy>10)&&(tempy<1900))
			{
			touchX = tempx>>2;
			touchY = tempy>>2;

			return touchX;
			}
		else
			{
				touchX=8000;
				touchY=8000;
				return touchX;
		}
	}
//	else 
//		{
//				touchX=8000;
//				touchY=8000;
//				return touchX;	
//		}
}
void Touch_Test(void)//异常点(1799,676)
{ 
	volatile int i,j;
	u8 buf[32];
	
	I2C1->IC_DATA_CMD =FT6206_ADDR | 0x200;//写入从机地址，开启起始信号
//	i=1000;while(i--);
	I2CTXByte(I2C1,CMD_WRITE,242);
//	i=100;while(i--);
	I2CTXByte(I2C1,CMD_WRITE,242>>8);
//	i=1000;while(i--);
//	printf("torfgbhX=  \r\n");

	for(i=0;i<5;i++) 
		{
			*(buf+i)=I2CRXByte(I2C1);//库函数法读取IIC数据
//			j=1000;while(j--);
		}

	if( buf[0] <=11 )//&2         
	{
		
		touchY=(((int16_t)buf[2]<<4))+(buf[4]>>4) ;    
		touchX=((int16_t)buf[3]<<4)+(buf[4] & 0x0f); 
		if((touchX==1799)&&(touchY==676))return;
		if((touchX>10)&&(touchX<3200)&&(touchY>10)&&(touchY<1900))
		{
		printf("touchX= %d \r\n",touchX);
		printf("touchY= %d \r\n",touchY);
		Lcd_ColorBox(touchX>>2,480-(touchY>>2),2,2,Yellow);
		}

	}

}
int GUI_TOUCH_X_MeasureY(void)
{
	return touchY;
}
#endif

//================================================MXT224 Touch CTP end=====================================================//
volatile void LCD_delay( int j)
{
volatile U32 i;	
	while(j--)
for(i=500;i>0;i--);
}
void LCD_Reset(void)
{
	LCD_RST(0);
	LCD_delay(100);					   
	LCD_RST(1);		 	 
	LCD_delay(100);
}
/*
TK80接口初始化
*/
void LCD_TK80_init(void)
{
		RCC->AHB2ENR |= 1<<31;    //打开 TK80 clock
	
		//DMA2 准备成32位
	#if USE_16bit_LCD
		DMA2_Channel2->CCR = 0x3590;//  16位
	#else
		DMA2_Channel2->CCR = 0xa90;//  32位
	#endif
		DMA2_Channel2->CPAR = (u32)&(TK80->DINR);
	
		#if TK020F9168 | TFT1P1061
    TK80->CFGR1 = 0x05050503;
    TK80->CFGR2 = 0x0503;
		#elif TK022RB417 
		TK80->CFGR1 = 0x05020202;
    TK80->CFGR2 = 0x0501;
		#else
		TK80->CFGR1 = 0x00050202;
    TK80->CFGR2 = 0x0502;
		#endif
	
    #if (TK80_INT_USED != 0)
    TK80->SR |= 0xC<<8; 
    TK499_NVIC_Init(0,0,TK80_IRQn,2);	 
    #endif
}
#if TK020F9168
void WriteComm(unsigned short cmd) //发送指令
{
		TK80->CR = 0x000c0b04;//软件产生CS，拉低CS
    TK80->CMDIR = cmd>>8;while(TK80->SR & 0x10000);
		TK80->CMDIR = cmd;while(TK80->SR & 0x10000);
		TK80->CR = 0x000c0b06;//软件产生CS，拉高CS
	
}

void WriteData(unsigned short data)	//发送数据
{
		TK80->CR = 0x000c0b04;//软件产生CS，拉低CS
    TK80->DINR = data>>8;while(TK80->SR & 0x10000);
		TK80->DINR = data;while(TK80->SR & 0x10000);
		TK80->CR = 0x000c0b06;//软件产生CS，拉高CS
}
#elif TK020RB424||TK022RB417 
void WriteComm(unsigned short cmd)
{
	TK80->CMDIR = ((cmd&0xff00)<<2)|((cmd&0x00ff)<<1);
}
void WriteData(unsigned int data)
{
	TK80->DINR = ((data&0xff00)<<2)|((data&0x00ff)<<1);
}
#else
void WriteComm(unsigned short cmd) //发送指令
{
    TK80->CMDIR = cmd;
}

void WriteData(unsigned int data)	//发送数据
{
    TK80->DINR = data;
}
#endif
void LCD_WR_REG(unsigned short cmd,unsigned short data)
{
	TK80->CMDIR = cmd;
	while(TK80->SR & 0x10000);
	TK80->DINR = data;
	while(TK80->SR & 0x10000);
}
void User_MCU_LCD_Initial(void)	//MCU模式液晶屏初始化入口
{
	GPIO_TK80_init();
	LCD_TK80_init();
	#if GUI_SUPPORT_TOUCH
		#if USE_CTP|use_XPT2046|New_CTP
			Touch_GPIO_Config();
		#elif USE_RTP
			Touch_Pad_Config();
		#endif
	#endif
	#if USE_GT911_CTP
	 I2CInitMasterMode(I2C1);
	#endif
	LCD_Reset();
	LCD_init_code();//液晶屏初始化代码
//	Lcd_ColorBox(0,0,XSIZE_PHYS,YSIZE_PHYS,White);//5寸ILI9806G要两次清屏方可使用，否则在电压低时有乱点
	init_all_Font_addr();
	Lcd_Light_ON;
}
void TK80_DMA_Init(u32 srcAdd ,u32 len)	 
{   
    DMA2_Channel2->CNDTR = len;
    DMA2_Channel2->CMAR = srcAdd;
    DMA2_Channel2->CCR |= 1;
		TK80->CR |= 1;
} 

//============================================  RGB方式代码段开始  ============================================//
#if LCD_MODE_MCU_or_RGB
__align(256) U32 LTDC_emWin[XSIZE_PHYS*YSIZE_PHYS];
#endif
void User_RGB_LCD_Initial(void)	//RGB模式液晶屏初始化入口
{
	LTDC_Clock_Set();
	Set_LCD_Timing_to_LTDC();
	GPIO_RGB_INIT();
//	LCD_Reset();
//	LCD_init_code();//液晶屏初始化代码
	
	
	#if GUI_SUPPORT_TOUCH
		#if USE_CTP|use_XPT2046
			Touch_GPIO_Config();
		#elif USE_RTP
			Touch_Pad_Config();
		#endif
	#endif
	#if USE_GT911_CTP | USE_MXT224_CTP
	 I2CInitMasterMode(I2C1);
//		GT9147_Init();
	#endif
	init_all_Font_addr();
	Lcd_Light_ON;
}
void LTDC_Clock_Set(void)	//设置LTDC时钟
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_LTDC, ENABLE);
	RCC->CR |= 1<<28;
	RCC->PLLDCKCFGR = 0x1<<16;  //分频系数 0~3 --> 2,4,6,8
	RCC->PLLLCDCFGR = 7<<6;   	//倍频系数
}


void Set_LTDC_REG(LCD_FORM_TypeDef* LCD_FORM)
{
    u32 aHorStart;
    u32 aHorEnd;
    u32 aVerStart;
    u32 aVerEnd;

    aHorStart = LCD_FORM->blkHorEnd + 1;
    aHorEnd = aHorStart + LCD_FORM->aHorLen;  
    aVerStart = LCD_FORM->blkVerEnd + 1 ;
    aVerEnd = aVerStart + LCD_FORM->aVerLen;

		LTDC->P_HOR = aHorEnd;//总宽度
    LTDC->HSYNC = (LCD_FORM->sHsyncStart <<16 )|LCD_FORM->sHsyncEnd;//水平同步信号起始和结束，位于背景色中间
    LTDC->A_HOR = (aHorStart<<16)|aHorEnd;//水平激活起始和结束
    LTDC->A_HOR_LEN = LCD_FORM->aHorLen ;//水平激活域宽度
    LTDC->BLK_HOR = (0<<16)|LCD_FORM->blkHorEnd;//背景开始和结束宽度0~激活地址	
    LTDC->P_VER =  aVerEnd;
    LTDC->VSYNC = (LCD_FORM->sVsyncStart<<16)|LCD_FORM->sVsyncEnd;
    LTDC->A_VER = (aVerStart<<16)|aVerEnd;
    LTDC->A_VER_LEN = LCD_FORM->aVerLen ;
		LTDC->BLK_VER = (0<<16)|LCD_FORM->blkVerEnd;
}
LCD_FORM_TypeDef LCD_FORM;

//HBP=sHsyncStart=0X20;
//VBP=sVsyncStart=0X05;

//HSW=sHsyncEnd-sHsyncStart+1=0X23-0X20+1;
//VSW=sVsyncEnd-sVsyncStart+1=0X08-0X05+1;

//HFP=blkHorEnd-sHsyncEnd+1=0X2F-0X23+1;
//VFP=blkVerEnd-sVsyncEnd+1=0X18-0X08+1;
void Set_LCD_Timing_to_LTDC(void)//设置LCD的时序到LTDC寄存器中
{
    
    LTDC->OUT_EN = 0;
	#if LCD_MODE_MCU_or_RGB
//    LTDC->DP_ADDR0 = (u32)LTDC_emWin;//第0层地址
	#endif
//    LTDC->DP_ADDR1 = (u32)(SDRAM_RGB_ADD + SDRAM_RGB_OFFSET);//第一层地址

    LTDC->BLK_DATA = 0x00FF;//背景色
	
	//===============2.6寸 240*400===============//
//		LCD_FORM.sHsyncStart = 0x2;  //水平激活起始
//    LCD_FORM.sHsyncEnd = 0x5;    //水平激活结束
//    LCD_FORM.aHorLen = 240 - 1;  //水平分辨率
//    LCD_FORM.blkHorEnd = 0x22;    //水平消隐

//    LCD_FORM.sVsyncStart = 0x2;  //垂直激活起始
//    LCD_FORM.sVsyncEnd = 0x5;    //垂直激活结束
//    LCD_FORM.aVerLen= 400 - 1; 	 //垂直分辨率
//    LCD_FORM.blkVerEnd = 0x3;   //垂直消隐
	
	//===============TK032F8004、TK043F1508、TK032RB431===============//
//		LCD_FORM.sHsyncStart = 0x2;  //水平激活起始
//    LCD_FORM.sHsyncEnd = 0x3;    //水平激活结束
//    LCD_FORM.aHorLen = 480 - 1;  //水平分辨率
//    LCD_FORM.blkHorEnd = 0x15;    //水平消隐

//    LCD_FORM.sVsyncStart = 0x2;  //垂直激活起始
//    LCD_FORM.sVsyncEnd = 0x3;    //垂直激活结束
//    LCD_FORM.aVerLen= 800 - 1; 	 //垂直分辨率
//    LCD_FORM.blkVerEnd = 0xF;    //垂直消隐
	
	
	//BW3532MIB
//		LCD_FORM.sHsyncStart = 0x2;  //水平激活起始
//    LCD_FORM.sHsyncEnd = 0x3;    //水平激活结束
//    LCD_FORM.aHorLen = 320 - 1;  //水平分辨率
//    LCD_FORM.blkHorEnd = 0xf;    //水平消隐

//    LCD_FORM.sVsyncStart = 0x2;  //垂直激活起始
//    LCD_FORM.sVsyncEnd = 0x3;    //垂直激活结束
//    LCD_FORM.aVerLen= 480 - 1; 	 //垂直分辨率
//    LCD_FORM.blkVerEnd = 0xF;   //垂直消隐
	
	//TK032RB422
//		LCD_FORM.sHsyncStart = 0x2;  //水平激活起始
//    LCD_FORM.sHsyncEnd = 0x3;    //水平激活结束
//    LCD_FORM.aHorLen = 480 - 1;  //水平分辨率
//    LCD_FORM.blkHorEnd = 0x14;    //水平消隐

//    LCD_FORM.sVsyncStart = 0x2;  //垂直激活起始
//    LCD_FORM.sVsyncEnd = 0x8;    //垂直激活结束
//    LCD_FORM.aVerLen= 800 - 1; 	 //垂直分辨率
//    LCD_FORM.blkVerEnd = 0xf;   //垂直消隐

//===============TK032LG480===============//
//    LCD_FORM.sHsyncStart = 0x2;  //水平激活起始
//    LCD_FORM.sHsyncEnd = 0x4;    //水平激活结束
//    LCD_FORM.aHorLen = 480 - 1;  //水平分辨率
//    LCD_FORM.blkHorEnd = 0x3D;    //水平消隐

//    LCD_FORM.sVsyncStart = 0x2;  //垂直激活起始
//    LCD_FORM.sVsyncEnd = 0x4;    //垂直激活结束
//    LCD_FORM.aVerLen= 800 - 1; 	 //垂直分辨率
//    LCD_FORM.blkVerEnd = 0xB;   //垂直消隐

//===============TK035F5589-51PIN===============//
//		LCD_FORM.sHsyncStart = 0x20;  //水平激活起始
//    LCD_FORM.sHsyncEnd = 0x23;    //水平激活结束
//    LCD_FORM.aHorLen = 320 - 1;  //水平分辨率
//    LCD_FORM.blkHorEnd = 0x2f;    //水平消隐

//    LCD_FORM.sVsyncStart = 0x5;  //垂直激活起始
//    LCD_FORM.sVsyncEnd = 0x8;    //垂直激活结束
//    LCD_FORM.aVerLen= 480 - 1; 	 //垂直分辨率
//    LCD_FORM.blkVerEnd = 0x18;   //垂直消隐
	
//===============TK043F1569===============//
//    LCD_FORM.sHsyncStart = 0x2;  //水平激活起始
//    LCD_FORM.sHsyncEnd = 0x3;    //水平激活结束
//    LCD_FORM.aHorLen = 480 - 1;  //水平分辨率
//    LCD_FORM.blkHorEnd = 0xf;    //水平消隐

//    LCD_FORM.sVsyncStart = 0x2;  //垂直激活起始
//    LCD_FORM.sVsyncEnd = 0x8;    //垂直激活结束
//    LCD_FORM.aVerLen= 800 - 1; 	 //垂直分辨率
//    LCD_FORM.blkVerEnd = 0xf;   //垂直消隐

//===============TK043F3211===============//
//    LCD_FORM.sHsyncStart = 0x2;  //水平激活起始
//    LCD_FORM.sHsyncEnd = 0x3;    //水平激活结束
//    LCD_FORM.aHorLen = 480 - 1;  //水平分辨率
//    LCD_FORM.blkHorEnd = 0xf;    //水平消隐

//    LCD_FORM.sVsyncStart = 0x2;  //垂直激活起始
//    LCD_FORM.sVsyncEnd = 0x8;    //垂直激活结束
//    LCD_FORM.aVerLen= 800 - 1; 	 //垂直分辨率
//    LCD_FORM.blkVerEnd = 0xf;   //垂直消隐

//===============IPS3P4473===============//
//		LCD_FORM.sHsyncStart = 0x30;  //水平激活起始
//    LCD_FORM.sHsyncEnd = 0x58;    //水平激活结束
//    LCD_FORM.aHorLen = 480 - 1;  //水平分辨率
//    LCD_FORM.blkHorEnd = 0x60;    //水平消隐

//    LCD_FORM.sVsyncStart = 0x2;  //垂直激活起始
//    LCD_FORM.sVsyncEnd = 0x3;    //垂直激活结束
//    LCD_FORM.aVerLen= 800 - 1; 	 //垂直分辨率
//    LCD_FORM.blkVerEnd = 0xf;   //垂直消隐

//===============TK043F1168===============//
//		LCD_FORM.sHsyncStart = 0x2;  //水平激活起始
//    LCD_FORM.sHsyncEnd = 0x3;    //水平激活结束
//    LCD_FORM.aHorLen = 480 - 1;  //水平分辨率
//    LCD_FORM.blkHorEnd = 0x15;    //水平消隐

//    LCD_FORM.sVsyncStart = 0x2;  //垂直激活起始
//    LCD_FORM.sVsyncEnd = 0x3;    //垂直激活结束
//    LCD_FORM.aVerLen= 800 - 1; 	 //垂直分辨率
//    LCD_FORM.blkVerEnd = 0x15;   //垂直消隐

//===============TK050F5590E===============//
		LCD_FORM.sHsyncStart = 0x2;  //水平激活起始
    LCD_FORM.sHsyncEnd = 0x8;    //水平激活结束
    LCD_FORM.aHorLen = 480 - 1;  //水平分辨率
    LCD_FORM.blkHorEnd = 0x3f;    //水平消隐

    LCD_FORM.sVsyncStart = 0x2;  //垂直激活起始
    LCD_FORM.sVsyncEnd = 0x8;    //垂直激活结束
    LCD_FORM.aVerLen= 854 - 1; 	 //垂直分辨率
    LCD_FORM.blkVerEnd = 0xf;   //垂直消隐

//===============7寸 800*480===============//
//		LCD_FORM.sHsyncStart = 0x28; //2  
//    LCD_FORM.sHsyncEnd = 0xa7;  //3
//    LCD_FORM.aHorLen = 800 - 1;  //480 
//    LCD_FORM.blkHorEnd = 0x8f;//0x27

//    LCD_FORM.sVsyncStart = 0x18; //0x28
//    LCD_FORM.sVsyncEnd = 0x20;   //0xa7
//    LCD_FORM.aVerLen= 480 - 1;   //800
//    LCD_FORM.blkVerEnd = 0x15;//0xc7

//===============7寸 1024*600===============//
//		LCD_FORM.sHsyncStart = 0x28; //2  
//    LCD_FORM.sHsyncEnd = 0xa7;  //3
//    LCD_FORM.aHorLen = 1024 - 1;  //480 
//    LCD_FORM.blkHorEnd = 0xff;//0x27

//    LCD_FORM.sVsyncStart = 0x18; //0x28
//    LCD_FORM.sVsyncEnd = 0x20;   //0xa7
//    LCD_FORM.aVerLen= 600 - 1;   //800
//    LCD_FORM.blkVerEnd = 0x15;//0xc7
	
   Set_LTDC_REG(&LCD_FORM);
		LTDC->VI_FORMAT = 0x00;
		
		//R61529=3,TK032F8004=A,
		LTDC->POL_CTL = 0x8+3;
		LTDC->OUT_EN |= 0x107;
}
void LTDC_IRQHandler(void)
{
    LTDC->INTR_CLR = 2;
    LTDC->DP_SWT ^= 1;//连续显示两幅图片
    if(LTDC->DP_SWT !=0 )
    {
//        fun_test(SDRAM_RGB_ADD);
    }
    else
    {
//        fun_test(SDRAM_RGB_ADD+SDRAM_RGB_OFFSET);
    }
//    if(LTDC->INTR_STA & 2)
//    {

//    }
}
void LCD_WriteByteSPI(unsigned char byte)
{
    unsigned char n;
   
    for(n=0; n<8; n++)			
    {  
        if(byte&0x80) SPI_SDA(1)
        else SPI_SDA(0)
        byte<<= 1;

        SPI_DCLK(0);
        SPI_DCLK(1);
    }
}
void SPI_WriteComm(u16 CMD)//3线9bit 串行接口
{			
	LCD_SPI_CS(0);
	SPI_SDA(0);
	SPI_DCLK(0);
	SPI_DCLK(1);
	LCD_WriteByteSPI(CMD);
	LCD_SPI_CS(1);
}
void SPI_WriteData(u16 tem_data)
{			
	LCD_SPI_CS(0);
	SPI_SDA(1);
	SPI_DCLK(0);
	SPI_DCLK(1);
	LCD_WriteByteSPI(tem_data);
	LCD_SPI_CS(1);
}

		//===TK032RB422、TK032RB431、TK043F3211、TK032F8004===//
//void SPI_WriteComm(u16 CMD)//3线8bit 串行接口
//{			
//	LCD_SPI_CS(0);
//	LCD_WriteByteSPI(0x70);
//	LCD_WriteByteSPI(CMD);
//	LCD_SPI_CS(1);
//}
//void SPI_WriteData(u16 tem_data)
//{			
//	LCD_SPI_CS(0);
//	LCD_WriteByteSPI(0x72);
//	LCD_WriteByteSPI(tem_data);
//	LCD_SPI_CS(1);
//}

//===TK043F1508==//
//void SPI_WriteComm(u16 CMD)
//{			
//	LCD_SPI_CS(0);
//	LCD_WriteByteSPI(0X20);
//	LCD_WriteByteSPI(CMD>>8);
//	LCD_WriteByteSPI(0X00);
//	LCD_WriteByteSPI(CMD);
//	
//	LCD_SPI_CS(1);
//}
//void SPI_WriteData(u16 tem_data)
//{			
//	LCD_SPI_CS(0);
//	LCD_WriteByteSPI(0x40);
//	LCD_WriteByteSPI(tem_data);
//	LCD_SPI_CS(1);
//}


//     //===TK032LG480===//
//void SPI_WriteComm(u16 CMD)
//{			
//	LCD_SPI_CS(0);
//	LCD_WriteByteSPI(0x7C);
//	LCD_WriteByteSPI(0x00);
//	LCD_WriteByteSPI(CMD);
//	
//	LCD_SPI_CS(1);
//}
//void SPI_WriteData(u16 tem_data)
//{			
//	LCD_SPI_CS(0);
//	LCD_WriteByteSPI(0x7E);
//	LCD_WriteByteSPI(0x00);
//	LCD_WriteByteSPI(tem_data);
//	LCD_SPI_CS(1);
//}
//============================================  RGB方式代码段结束  ============================================//

// ================ 2.6 inch LS026B8PX04==================== //
//void SPI_WriteComm(u16 CMD)//3线8bit 串行接口
//{			
//	LCD_SPI_CS(0);
//	SPI_SDA(0);
//	SPI_DCLK(0);
//	LCD_SPI_CS(1);
//	LCD_WriteByteSPI(0x50);
//	LCD_WriteByteSPI(CMD);
//	
//	LCD_SPI_CS(0);
//}
//void SPI_WriteData(u16 tem_data)
//{		
//	LCD_SPI_CS(0);	
//	SPI_SDA(0);
//	SPI_DCLK(0);
//	LCD_SPI_CS(1);
//	LCD_WriteByteSPI(0x51);
//	LCD_WriteByteSPI(tem_data);
//	LCD_SPI_CS(0);
//}
/**********************************************
函数名：Lcd矩形填充函数

入口参数：xStart x方向的起始点
          ySrart y方向的终止点
          xLong 要选定矩形的x方向长度
          yLong  要选定矩形的y方向长度
返回值：无
***********************************************/
void Lcd_ColorBox(u16 xStart,u16 yStart,u16 xLong,u16 yLong,u32 Color)
{
#if TK020F9168
	u32 i,j;
	BlockWrite(xStart,xStart+xLong-1,yStart,yStart+yLong-1);
	TK80->CR = 0x000c0b04;//软件产生CS，拉低CS
	j=xLong*yLong;
	for(i=0;i<j;i++)
	{
		WriteData(Color);
	}
	TK80->CR = 0x000c0b06;//软件产生CS，拉高CS
#elif	LCD_MODE_MCU_or_RGB	
	#if	LCD_RGB_ORIENTATION
		u16 i,j;
		u32 temp;
		temp = YSIZE_PHYS*xStart;
		for(i=0;i<yLong;i++)
		{
			for(j=0;j<xLong;j++)
			LTDC_emWin[yStart+i+YSIZE_PHYS*j+temp]=Color;
		}
	#else	
		u16 i,j;
		u32 temp;
		temp = XSIZE_PHYS*yStart;
		for(i=0;i<yLong;i++)
		{
			for(j=0;j<xLong;j++)
			LTDC_emWin[xStart+j+XSIZE_PHYS*i+temp]=Color;
		}
	#endif
#else	
	BlockWrite(xStart,xStart+xLong-1,yStart,yStart+yLong-1);
	*((uint32_t *)(0x60000000+0x30))=xLong*yLong;
		TK80->DINR = Color;
	while(TK80->SR & 0x10000);
#endif	
}
/******************************************
x				图片起始x轴坐标
y				图片起始y轴坐标
pic_H		图片水平宽度
pic_V 	图片垂直长度
pic   	图片取模的数组首地址
******************************************/
void LCD_Fill_Pic(u16 x, u16 y,u16 pic_H, u16 pic_V, u32* pic)
{
#if	LCD_MODE_MCU_or_RGB	
	#if	LCD_RGB_ORIENTATION
		u16 i,j;
		u32 Xstart,k=0;
		Xstart = XSIZE_PHYS*x;
		for(i=0;i<pic_V;i++)
		{
		for(j=0;j<pic_H;j++)
		LTDC_emWin[Xstart+i+XSIZE_PHYS*j+y]=pic[k++];
		}
//	 u16 i,j;
//	u32 Xstart,k=0;
//	Xstart = XSIZE_PHYS*x;
//	for(i=pic_V-1;i>0;i--)
//	{
//		for(j=0;j<pic_H;j++)
//		LTDC_emWin[Xstart+i+XSIZE_PHYS*j+y]=pic[k++];
//	}
	#else	
		 u16 i,j;
		u32 Ystart,k=0;
		Ystart = XSIZE_PHYS*y;
		for(i=0;i<pic_V;i++)
		{
			for(j=0;j<pic_H;j++)
			LTDC_emWin[x+j+XSIZE_PHYS*i+Ystart]=pic[k++];
		}
	#endif
#elif TK020F9168
	//直接读写方式
	unsigned long i;
	unsigned long j;
	BlockWrite(x,x+pic_H-1,y,y+pic_V-1);
	TK80->CR = 0x000c0b04;//软件产生CS，拉低CS
	j= pic_H*pic_V*2;
	for (i = 0; i <j; i++)  {TK80->DINR = pic[i];while(TK80->SR & 0x10000);}
	TK80->CR = 0x000c0b06;//软件产生CS，拉高CS
#else	
	//DMA 方式
	BlockWrite(x,x+pic_H-1,y,y+pic_V-1);
	TK80_DMA_Init((u32)pic,pic_H*pic_V);//DMA初始化
	while((DMA2->ISR & 0x20)==0);
	DMA2->IFCR |=1<<5;
#endif
	
}
/******************************************
功能：图片从左往右滚动
n						图片每次移动n个像素
x						图片起始x轴坐标
y						图片起始y轴坐标
pic_H				图片水平宽度
pic_V 			图片垂直长度
pic   			图片取模的数组首地址
Back_color 	背景色
******************************************/
void LCD_Picture_Move(u16 n, u16 x, u16 y,u16 pic_H, u16 pic_V, u32* pic,unsigned long Back_color)
{
	u32 i,j;
	j = pic_H*pic_V;
	for(i=0;i<XSIZE_PHYS-pic_H-x+1;i+=n)      
		{
			BlockWrite(x+i,x+i+pic_H-1,y,y+pic_V-1); 
			TK80_DMA_Init((u32)pic,j);//DMA初始化
			while((DMA2->ISR & 0x20)==0);
			DMA2->IFCR |=1<<5;
			Lcd_ColorBox(x+i,y,n,pic_V,Back_color);
		}
		
	//不用DMA方式
//	for(i=0;i<XSIZE_PHYS-pic_H-x+1;i+=n)      
//		{
//			LCD_Fill_Pic(x+i,y,pic_H,pic_V, pic);
//			Lcd_ColorBox(x+i,y,n,pic_V,Back_color);
//		}
}

typedef __packed struct
{
	u8  pic_head[2];				//1
	u16 pic_size_l;			    //2
	u16 pic_size_h;			    //3
	u16 pic_nc1;				    //4
	u16 pic_nc2;				    //5
	u16 pic_data_address_l;	    //6
	u16 pic_data_address_h;		//7	
	u16 pic_message_head_len_l;	//8
	u16 pic_message_head_len_h;	//9
	u16 pic_w_l;					//10
	u16 pic_w_h;				    //11
	u16 pic_h_l;				    //12
	u16 pic_h_h;				    //13	
	u16 pic_bit;				    //14
	u16 pic_dip;				    //15
	u16 pic_zip_l;			    //16
	u16 pic_zip_h;			    //17
	u16 pic_data_size_l;		    //18
	u16 pic_data_size_h;		    //19
	u16 pic_dipx_l;			    //20
	u16 pic_dipx_h;			    //21	
	u16 pic_dipy_l;			    //22
	u16 pic_dipy_h;			    //23
	u16 pic_color_index_l;	    //24
	u16 pic_color_index_h;	    //25
	u16 pic_other_l;			    //26
	u16 pic_other_h;			    //27
	u16 pic_color_p01;		    //28
	u16 pic_color_p02;		    //29
	u16 pic_color_p03;		    //30
	u16 pic_color_p04;		    //31
	u16 pic_color_p05;		    //32
	u16 pic_color_p06;		    //33
	u16 pic_color_p07;		    //34
	u16 pic_color_p08;			//35			
}BMP_HEAD;

BMP_HEAD bmp;

#if LCD_MODE_MCU_or_RGB
char display_picture(char *filename)
{
	u16 ReadValue;
	FATFS fs;            // Work area (file system object) for logical drive
	FIL fsrc;      			// file objects
	u8 buffer[2048]; 		// file copy buffer
	FRESULT res;         // FatFs function common result code
	UINT br;         		// File R/W count
	u16 r_data,g_data,b_data;	
	
	u8 xOfset; //单行像素 4倍数补全
	
	u16 i,j;
	u32 Xstart,Ystart,k=0;
	u32	 tx,ty,temp;
	
	
	f_mount (&fs, "0:", 1);//挂载文件系统
  res = f_open(&fsrc, filename, FA_OPEN_EXISTING | FA_READ);	 //打上图片文件名
  if(res==FR_NO_FILE||res==FR_INVALID_NAME){
		f_mount (NULL, "0:", 1);//注销文件系统
	 return 0;
  }

//  if(res!=FR_OK){
//   f_mount (NULL, "0:", 1);//注销文件系统
//	 SD_Init();//重新初始化SD卡 
//	 
//  }
	if(res!=FR_OK){
   f_mount (NULL, "0:", 1);//注销文件系统
	 SD_Init();//重新初始化SD卡 
	 return 0;
  }

  res = f_read(&fsrc, &bmp, sizeof(bmp), &br);

  if(br!=sizeof(bmp))
	{
		f_close(&fsrc);
		f_mount (NULL, "0:", 1);//注销文件系统
		return 0;
  }
	
  if((bmp.pic_head[0]=='B')&&(bmp.pic_head[1]=='M'))
  {
	res = f_lseek(&fsrc, ((bmp.pic_data_address_h<<16)|bmp.pic_data_address_l));
	if(res!=FR_OK){
     f_close(&fsrc);
     f_mount (NULL, "0:", 1);//注销文件系统
	 return 0;
    }
	if (((bmp.pic_w_l>XSIZE_PHYS)||(bmp.pic_h_l>YSIZE_PHYS))&&((bmp.pic_w_l>YSIZE_PHYS)||(bmp.pic_h_l>XSIZE_PHYS)))//长宽超分辨率了退出
		{
			f_close(&fsrc);
			f_mount (NULL, "0:", 1);//注销文件系统
			return 0;
		}
//	if((bmp.pic_w_l>=bmp.pic_h_l)||(bmp.pic_h_l<=YSIZE_PHYS))//如果图片宽度w大于等于高度，或图片高度h小于等于物理高度，认为是横图
//	{

//	if(bmp.pic_w_l<XSIZE_PHYS||bmp.pic_h_l<YSIZE_PHYS)
//		{
//			
// 			Lcd_ColorBox(0,0,XSIZE_PHYS,YSIZE_PHYS,Black);
// 			BlockWrite((XSIZE_PHYS-bmp.pic_w_l)/2,(XSIZE_PHYS-bmp.pic_w_l)/2+bmp.pic_w_l-1,(YSIZE_PHYS-bmp.pic_h_l)/2,(YSIZE_PHYS-bmp.pic_h_l)/2+bmp.pic_h_l-1);
//		}
//		else 	BlockWrite(0,XSIZE_PHYS-1,0,YSIZE_PHYS-1);
//	}
//	else
//	{
//		WriteComm(0x36); //Set_address_mode
//		WriteData(0x88); //竖屏，从左下角开始，从左到右，从下到上
//		if(bmp.pic_w_l<YSIZE_PHYS||bmp.pic_h_l<XSIZE_PHYS)
//			{
//				Lcd_ColorBox(0,0,YSIZE_PHYS,XSIZE_PHYS,Black);
//				BlockWrite((YSIZE_PHYS-bmp.pic_w_l)/2,(YSIZE_PHYS-bmp.pic_w_l)/2+bmp.pic_w_l-1,(XSIZE_PHYS-bmp.pic_h_l)/2,(XSIZE_PHYS-bmp.pic_h_l)/2+bmp.pic_h_l-1);
//			}
//			else BlockWrite(0,YSIZE_PHYS-1,0,XSIZE_PHYS-1);	
//	}
	//===================================================================================================//
	if(bmp.pic_dip==16) 
	{
		WriteComm(0x3A); //Set_address_mode
		WriteData(0x55);
		TK80->CMDIR =0x2c;
		temp=bmp.pic_w_l*2;  //如果是16位的位图，一行是pic_w_l*2个字节
		xOfset = temp%4;
		if(xOfset !=0 ) xOfset = 4- xOfset;
			for (tx = 0; tx < bmp.pic_h_l; tx++)
				{
					f_read(&fsrc, buffer, (bmp.pic_w_l)*2+xOfset, &br);
					for(ty=0;ty<temp;ty+=2)
						{		
							TK80->DINR = (*(ty +1+buffer)<<8)|(*(ty +0+buffer));
						}
				}
			WriteComm(0x3A); //Set_address_mode
			WriteData(0x77);
	}
	else if(bmp.pic_dip==24)	
		{
			temp=bmp.pic_w_l*3;								 //如果是24位的位图，一行是pic_w_l*3个字节
			xOfset = temp%4;
			if(xOfset !=0 ) xOfset = 4- xOfset;
			
			Xstart = ((XSIZE_PHYS-bmp.pic_w_l)/2);
			Ystart = XSIZE_PHYS*(YSIZE_PHYS-bmp.pic_h_l)/2;
			
//			for(i=0;i<bmp.pic_h_l;i++)
//			{
//				f_read(&fsrc, buffer, (bmp.pic_w_l)*3+xOfset, &br);
//				j=0;
//				for(ty=0;ty<temp;ty+=3)//for(j=0;j<xLong;j++)
//				{	
//					LTDC_emWin[Xstart+j+XSIZE_PHYS*i+Ystart]=((*(ty +0+buffer)<<16))|(*(ty +1+buffer)<<8)|(*(ty +2+buffer));
//					j++;
//				}
//			}

			Xstart = XSIZE_PHYS*((YSIZE_PHYS-bmp.pic_w_l)/2);
			Ystart = (XSIZE_PHYS-bmp.pic_h_l)/2;
			for(i=0;i<bmp.pic_h_l;i++)
			{
				f_read(&fsrc, buffer, (bmp.pic_w_l)*3+xOfset, &br);
				j=0;
				for(ty=0;ty<temp;ty+=3)//for(j=0;j<xLong;j++)
				{	
					LTDC_emWin[Xstart+bmp.pic_h_l-i+XSIZE_PHYS*j+Ystart]=((*(ty +0+buffer)))|(*(ty +1+buffer)<<8)|(*(ty +2+buffer)<<16);
					j++;
				}
			}
//			Xstart = XSIZE_PHYS*x;
//	for(i=pic_V;i>0;i--)
//	{
//		for(j=0;j<pic_H;j++)
//		LTDC_emWin[Xstart+i+XSIZE_PHYS*j+y]=pic[k++];
//	}
			
//			for (tx = 0; tx < bmp.pic_h_l; tx++)
//				{
//					
//					f_read(&fsrc, buffer, (bmp.pic_w_l)*3+xOfset, &br);
//					for(ty=0;ty<temp;ty+=3)
//						{
//							TK80->DINR = ((*(ty +0+buffer)<<16))|(*(ty +1+buffer)<<8)|(*(ty +2+buffer));
//						}
//				}
		}
		else if(bmp.pic_dip==32)	
		{
			temp=bmp.pic_w_l*4;								 //如果是32位的位图，一行是pic_w_l*4个字节
			
			Xstart = XSIZE_PHYS*((YSIZE_PHYS-bmp.pic_w_l)/2);
			Ystart = (XSIZE_PHYS-bmp.pic_h_l)/2;
			for(i=0;i<bmp.pic_h_l;i++)
			{
				f_read(&fsrc, buffer, temp, &br);
				for(ty=0;ty<temp;ty+=4)
				{	
					char alpha = (*(ty +3+buffer));
					u32 old_color = LTDC_emWin[Xstart+bmp.pic_h_l-i+Ystart+XSIZE_PHYS*(ty>>2)];
					
					LTDC_emWin[Xstart+bmp.pic_h_l-i+Ystart+XSIZE_PHYS*(ty>>2)] = (ALPHA_COMPOSITE((u8)old_color,         (*(ty +0+buffer)), alpha))
																																			|((ALPHA_COMPOSITE((u8)(old_color >> 8), (*(ty +1+buffer)), alpha)) << 8)
																																			|((ALPHA_COMPOSITE((u8)(old_color >> 16),(*(ty +2+buffer)), alpha)) << 16);
				}
			}
		}
	else 
		{
			f_mount (NULL, "0:", 1);//注销文件系统
			return 0; //暂时不支持其它格式，返回0
		}
   }
	f_close(&fsrc);
  f_mount (NULL, "0:", 1);//注销文件系统
  return 1;
}
#else

//任意屏大小范围内显示图片
char display_picture(char *filename)
{
	FATFS fs;            // Work area (file system object) for logical drive
	FIL fsrc;      			// file objects
	u8 buffer[2048]; 		// file copy buffer
	FRESULT res;         // FatFs function common result code
	UINT br;         		// File R/W count
	u32	 tx,ty,temp;
	u8 xOfset; //单行像素 4倍数补全
	
	f_mount (&fs, "0:", 1);//挂载文件系统
  res = f_open(&fsrc, filename, FA_OPEN_EXISTING | FA_READ);	 //打上图片文件名
  if(res==FR_NO_FILE||res==FR_INVALID_NAME){
		f_mount (NULL, "0:", 1);//注销文件系统
	 return 0;
  }

  if(res!=FR_OK){
   f_mount (NULL, "0:", 1);//注销文件系统
	 SD_Init();//重新初始化SD卡 
	 return 0;
  }

  res = f_read(&fsrc, &bmp, sizeof(bmp), &br);

  if(br!=sizeof(bmp))
	{
		f_close(&fsrc);
		f_mount (NULL, "0:", 1);//注销文件系统
		return 0;
  }
	
  if((bmp.pic_head[0]=='B')&&(bmp.pic_head[1]=='M'))
  {
	res = f_lseek(&fsrc, ((bmp.pic_data_address_h<<16)|bmp.pic_data_address_l));
	if(res!=FR_OK){
     f_close(&fsrc);
     f_mount (NULL, "0:", 1);//注销文件系统
	 return 0;
    }
	if (((bmp.pic_w_l>XSIZE_PHYS)||(bmp.pic_h_l>YSIZE_PHYS))&&((bmp.pic_w_l>YSIZE_PHYS)||(bmp.pic_h_l>XSIZE_PHYS)))//长宽超分辨率了退出
		{
			f_close(&fsrc);
			f_mount (NULL, "0:", 1);//注销文件系统
			return 0;
		}
	if((bmp.pic_w_l>=bmp.pic_h_l)||(bmp.pic_h_l<=YSIZE_PHYS))//如果图片宽度大于等于高度，或图片高度小于等于物理高度，认为是横图
	{
 	WriteComm(0x36); //Set_address_mode
 	WriteData(0x28); //横屏，从左下角开始，从左到右，从下到上  XSIZE_PHYS,YSIZE_PHYS
	if(bmp.pic_w_l<XSIZE_PHYS||bmp.pic_h_l<YSIZE_PHYS)
		{
 			Lcd_ColorBox(0,0,XSIZE_PHYS,YSIZE_PHYS,Black);
 			BlockWrite((XSIZE_PHYS-bmp.pic_w_l)/2,(XSIZE_PHYS-bmp.pic_w_l)/2+bmp.pic_w_l-1,(YSIZE_PHYS-bmp.pic_h_l)/2,(YSIZE_PHYS-bmp.pic_h_l)/2+bmp.pic_h_l-1);
		}
		else 	BlockWrite(0,XSIZE_PHYS-1,0,YSIZE_PHYS-1);
	}
	else
	{
		WriteComm(0x36); //Set_address_mode
		WriteData(0x88); //竖屏，从左下角开始，从左到右，从下到上
		if(bmp.pic_w_l<YSIZE_PHYS||bmp.pic_h_l<XSIZE_PHYS)
			{
				Lcd_ColorBox(0,0,YSIZE_PHYS,XSIZE_PHYS,Black);
				BlockWrite((YSIZE_PHYS-bmp.pic_w_l)/2,(YSIZE_PHYS-bmp.pic_w_l)/2+bmp.pic_w_l-1,(XSIZE_PHYS-bmp.pic_h_l)/2,(XSIZE_PHYS-bmp.pic_h_l)/2+bmp.pic_h_l-1);
			}
			else BlockWrite(0,YSIZE_PHYS-1,0,XSIZE_PHYS-1);	
	}
	//===================================================================================================//
	if(bmp.pic_dip==16) 
	{
		WriteComm(0x3A); //Set_address_mode
		WriteData(0x55);
		TK80->CMDIR =0x2c;
		temp=bmp.pic_w_l*2;  //如果是16位的位图，一行是pic_w_l*2个字节
		xOfset = temp%4;
		if(xOfset !=0 ) xOfset = 4- xOfset;
			for (tx = 0; tx < bmp.pic_h_l; tx++)
				{
					f_read(&fsrc, buffer, (bmp.pic_w_l)*2+xOfset, &br);
					for(ty=0;ty<temp;ty+=2)
						{		
							TK80->DINR = (*(ty +1+buffer)<<8)|(*(ty +0+buffer));
						}
				}
			WriteComm(0x3A); //Set_address_mode
			WriteData(0x77);
	}
	else if(bmp.pic_dip==24)	
		{
			temp=bmp.pic_w_l*3;								 //如果是24位的位图，一行是pic_w_l*3个字节
			xOfset = temp%4;
			if(xOfset !=0 ) xOfset = 4- xOfset;
			for (tx = 0; tx < bmp.pic_h_l; tx++)
				{
					f_read(&fsrc, buffer, (bmp.pic_w_l)*3+xOfset, &br);
					for(ty=0;ty<temp;ty+=3)
						{
							TK80->DINR = ((*(ty +0+buffer)<<16))|(*(ty +1+buffer)<<8)|(*(ty +2+buffer));
						}
				}
		}
		else if(bmp.pic_dip==32)	
		{
			temp=bmp.pic_w_l*4;								 //如果是32位的位图，一行是pic_w_l*4个字节
			for (tx = 0; tx < bmp.pic_h_l; tx++)
				{
					f_read(&fsrc, buffer, (bmp.pic_w_l)*4, &br);
					for(ty=0;ty<temp;ty+=4)
						{
							TK80->DINR = ((*(ty +0+buffer)<<16))|(*(ty +1+buffer)<<8)|(*(ty +2+buffer));
						}
				}
		}
	else 
		{
			f_mount (NULL, "0:", 1);//注销文件系统
			WriteComm(0x36); WriteData(0x68);
			return 0; //暂时不支持其它格式，返回0
		}
   }
	f_close(&fsrc);
  f_mount (NULL, "0:", 1);//注销文件系统
	WriteComm(0x36); WriteData(0x68);
  return 1;
}
#endif
void Test_SDcard_read_TXT()
{
	u8   fs_temp_buf[513];
		FIL  file;
		FATFS ufs;
		u32 fs_cnt;
		FIL *filescr1 = &file;
		FRESULT f_res;
		f_res = f_mount (&ufs, "0:", 1);//挂载文件系统
		f_res = f_open(filescr1, "0:123.TXT",  FA_READ );
		fs_cnt = 512;
		do
  {
    f_res = f_read(filescr1, fs_temp_buf, 512, &fs_cnt);
		printf("%s",fs_temp_buf);
  }
	while(fs_cnt > 0);
}
int bmpCnt=0;
u8 tfBmpName[200][50];
void ReadTF(void)
{
 u16 cnt = 0;
	FRESULT result;
	FATFS fs;
	DIR dirInf;
	FILINFO fileInf;
	
	result = f_mount (&fs, "0:", 1);//挂载文件系统
	if (result != FR_OK) goto MountFalse;//挂载失败,注销文件系统

	/* 打开根文件夹 */
	result = f_opendir(&dirInf, "/"); /* 如果不带参数，则从当前目录开始 */
	
	if(result != FR_OK) goto MountFalse;//如果打开失败，则注销文件系统
	

	/* 读取当前文件夹下的文件和目录 */
	for (cnt = 0; ;cnt++)
	{
		result = f_readdir(&dirInf,&fileInf);   /* 读取目录项，索引会自动下移 */
		if (result != FR_OK || fileInf.fname[0] == 0)
		{
			goto MountFalse;
		}

		if (fileInf.fname[0] == '.')
		{
			continue;
		}
		
		if(strstr(fileInf.fname,"BMP")!=NULL)//识别bmp图片文件
		{
			bmpCnt++;		
			memcpy(tfBmpName[bmpCnt-1],fileInf.fname,sizeof(fileInf.fname));
			printf("\r\n %s",tfBmpName[bmpCnt-1]);
			if(bmpCnt >=1000)  //图片数量超过缓存数
				break;
		}
	}	
	
MountFalse:
	f_mount (NULL, "0:", 1);//注销文件系统
}
void scan_TFCard_CTP_key(void)
{
	int m,j;
	ReadTF();
	if(display_picture((char *)&tfBmpName[0])==0)return;
	j = 1000;while(--j);
	GPIOA->CRL = (GPIOA->CRL&0xfffffff0)|0x00000008;//把PA0设置成输入，作为按键
	GPIOA->CRL = (GPIOA->CRL&0xffffff0f)|0x00000080;//把PA1设置成输入，作为按键
	GPIOB->CRH = (GPIOB->CRH&0xff0fffff)|0x00800000;//把PB13设置成输入，作为按键
//	while(1)//点击上半屏或者按键按向上，图片+1显示，反之退1显示
//	{
//			if(m<bmpCnt)
//			{
//				display_picture((char *)&tfBmpName[m]);
//				j = 100000000;while(--j);
//			}
//			else 
//				{
//					m=0;
//					display_picture((char *)&tfBmpName[m]);
//					j = 100000000;while(--j);
//				}
//			if(GPIOA->IDR & GPIO_Pin_0)break;
//			m++;	
//	 }
	while(1)//点击上半屏或者按键按向上，图片+1显示，反之退1显示
	{
		#if GUI_SUPPORT_TOUCH
		if((GPIOA->IDR & GPIO_Pin_1)||((GUI_TOUCH_X_MeasureY()>10)&&(GUI_TOUCH_X_MeasureY()<(LCD_GetYSize()/2))))
		#else
		if(GPIOA->IDR & GPIO_Pin_1)
		#endif
		{
			for(j=0;j<500;j++);
			#if GUI_SUPPORT_TOUCH
			if((GPIOA->IDR & GPIO_Pin_1)||((GUI_TOUCH_X_MeasureY()>10)&&(GUI_TOUCH_X_MeasureY()<(LCD_GetYSize()/2))))
			#else
			if(GPIOA->IDR & GPIO_Pin_1)
			#endif
			{
				m++;
				if(m<bmpCnt)
					display_picture((char *)&tfBmpName[m]);
				else 
				{
					m=0;
					display_picture((char *)&tfBmpName[m]);
				}
				while(GPIOA->IDR & GPIO_Pin_1);
				for(j=0;j<500;j++);
			}
		}
		#if GUI_SUPPORT_TOUCH
		if((GPIOB->IDR & GPIO_Pin_13)||((GUI_TOUCH_X_MeasureY()>(LCD_GetYSize()/2))&&(GUI_TOUCH_X_MeasureY()<(LCD_GetYSize()-10))))
		#else
		if(GPIOB->IDR & GPIO_Pin_13)
		#endif
		{
			for(j=0;j<500;j++);
			#if GUI_SUPPORT_TOUCH
			if((GPIOB->IDR & GPIO_Pin_13)||((GUI_TOUCH_X_MeasureY()>(LCD_GetYSize()/2))&&(GUI_TOUCH_X_MeasureY()<(LCD_GetYSize()-10))))
			#else
			if(GPIOB->IDR & GPIO_Pin_13)
			#endif
			{
				if(m>0)
				{
					m--;
					display_picture((char *)&tfBmpName[m]);
				}
				else 
				{
					m=bmpCnt-1;
					display_picture((char *)&tfBmpName[m]);
				}
				while(GPIOB->IDR & GPIO_Pin_13);
				for(j=0;j<500;j++);
			}
		}
		if(GPIOA->IDR & GPIO_Pin_0)break;
	}
	WriteComm(0x36); WriteData(0x68);
}