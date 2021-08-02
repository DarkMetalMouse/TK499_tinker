#include "QSPI_REG.h"
#include "stdio.h"
#include "UART.h"


extern void WriteQEDisable(void);
extern void WriteQEEnable(void);
extern void QSPI_ChipErase(void);
extern void QSPI_RecData_Standard(int rxnum);
extern unsigned long QSPI_SendNbytesData_Quad(unsigned char *puCSendData,uint32_t datanum,uint32_t timeout);


u8 W25QXX_SPI_QPI_MODE=0;		
/* 
QSPI模式标志:
   0. SPI模式_standard; 
   1. SPI模式_Dual; 
   2. SPI模式_Quad; 
   3. QPI模式. 
*/
u8 W25QXX_WRITE_MODE=0;	
/* 
QSPI Write模式标志:
   0. Disable; 
   1. Enable; 
*/

u8 W25QXX_QE_MODE=0;	
/* 
QSPI QE模式标志:
   0. Disable; 
   1. Enable; 
*/




#define BURST     0x00   
#define SIZE      0x1

#define RXDATAMAXLENGTH  256//512

uint32_t u32data[RXDATAMAXLENGTH+1];// __attributer__ at 0x00003400;
//int32_t code[25]={0xa8,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
//                  0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,
//                  0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
//                  0x88};
void CheckStatus(void);
void delay(unsigned long num)
{
    unsigned long i;
    for(i=0;i<num;i++);
}

void QSPI_SFRReset()
{
    RCC->APB2RSTR |= 0X1 << 24;
    delay(50);
    RCC->APB2RSTR &= 0X0 << 24;
}

//GPIO,Standard
void QSPI_GpioInit_Standard()
{
    RCC->AHB1ENR |= 0x1f;//GPIO enable;
    GPIOD->AFRH = 0x06666660;//AF6
    GPIOD->CRH   |= 0x09899990;//PD[11],io3,output
}

//GPIO,Quad
void QSPI_GpioInit_Quad_Output()
{
    RCC->AHB1ENR |= 0x1f;//GPIO enable;
    GPIOD->AFRH = 0x06666660;//AF6
    GPIOD->CRH   |= 0x09999990;//PD[11],io3,output
}

void QSPI_GpioInit_Quad_Iutput()
{
    RCC->AHB1ENR |= 0x1f;//GPIO enable;
    GPIOD->AFRH = 0x06666660;//AF6
    GPIOD->CRH   |= 0x08898980;//PD[11],io3,input
}

//GPIO,Dual
void QSPI_GpioInit_Dual_Output()
{
    RCC->AHB1ENR |= 0x1f;//GPIO enable;
    GPIOD->AFRH = 0x06666660;//AF6
    GPIOD->CRH   |= 0x09999990;
}

void QSPI_GpioInit_Dual_Iutput()
{
    RCC->AHB1ENR |= 0x1f;//GPIO enable;
    GPIOD->AFRH = 0x06666660;//AF6
    GPIOD->CRH    = 0x09899980;
}


void ClockInit()
{
    RCC->AHB1ENR |= 0x3<<21;//DMA  enable
    RCC->APB1ENR |= 0x1<<29;//PWR  enable
    RCC->APB1ENR |= 0x1<<9; //BKP  enable
    RCC->APB2ENR |= 0x1<<24;//QSPI enable
    
    RCC->AHB1ENR |= 0x1f;//GPIO enable;
    
    GPIOD->AFRH &= 0xf000000f;
    GPIOD->AFRH |= 0x06666660;//AF6
    
    GPIOD->CRH &= 0xf000000f;
    GPIOD->CRH |= 0x0bbbbbb0;
}

int rxcnt;
void WaitQSPI_TranDataComple()
{
    while(1)
    {
        if((QSPI_CSTAT & 0x1) ==0x1) //txept=1
        {
            break;
        }
    }
}

#if 1
void  WaitQSPI_ReceiveDataCompleOver8bytes(int rxnum)
{
    rxcnt=0;
	memset(u32data,0,256);
    while(1)
    {
        if((QSPI_INTSTAT & 0x020)==0x020)//rxavl=1
        {
            u32data[rxcnt]=QSPI_RXREG;
            rxcnt++;
        }
        if(rxcnt==rxnum)
        {
             break;
        }
    }
}
#endif
#if 1
void  WaitQSPI_ReceiveDataComple(int rxnum)
{
    rxcnt=0;
    while(1)
    {
        if((QSPI_CSTAT & 0x02)==0x02)//rxavl=1
        {
            u32data[rxcnt] = QSPI_RXREG;
            rxcnt++;
        }
        if(rxcnt==rxnum)
        {
             break;
        }
    }
}
#endif
#if 0
void  WaitQSPI_ReceiveDataComple(int rxnum)
{
    rxcnt=0;
	memset(u32data,0,256);
    while(1)
    {
        if((QSPI_INTSTAT & 0x02))//rxavl=1 //if((QSPI_INTSTAT & 0x20)==0x20)//rxavl=1
        {
            u32data[rxcnt]=QSPI_RXREG;
            rxcnt++;
        }
        if(rxcnt==rxnum)
        {
             break;
        }
    }
}
#endif
#if 1
void QSPI_Init()
{

    //QSPI_GCTL|=CS_High;
    QSPI_SCSR|=CS_High;
    QSPI_GCTL|=MM|QSPIEN;//|CSC_SEL;
    QSPI_SPBRG=0x34;
    QSPI_CCTL=(Len_8bit| CKPL_Low | CKPH_Sam1Tran2_Edge); //  ok
    //QSPI_CCTL=(Len_8bit| CKPL_High | CKPH_Sam1Tran2_Edge);// lose  1 clk
   // QSPI_CCTL=(Len_8bit| CKPL_High | CKPH_Sam2Tran1_Edge); // 
    //QSPI_CCTL=(Len_8bit| CKPL_Low | CKPH_Sam2Tran1_Edge); 
}

/*
#define   CKPL_Low    0x0
#define   CKPL_High   0x2
#define   CKPH_Sam1Tran2_Edge 0x1
#define   CKPH_Sam2Tran1_Edge  0x0

SPI_CCTL寄存器的CPOL和CPHA位，能够组合成四种可能的时序关系。
CPOL(时钟极性)位控制 在没有数据传输时时钟的空闲状态电平，此位对主模式和从模式下的设备都有效。
如果CPOL被清‘0’，SCK引脚在空闲状态保持低电平；如果CPOL被置‘1’，SCK引脚在空闲状态保持高电平。

如果CPHA(时钟相位)位被置‘1’，SCK时钟的第二个边沿
(CPOL位为0时就是下降沿，CPOL位为1时就是上升沿)
进行数据位的采样，数据在第二个时钟边沿被锁存。

如果CPHA位被清‘0’，SCK 时钟的第一边沿
(CPOL位为0时就是下降沿，CPOL位为1时就是上升沿)
进行数据位采样，数据在第一个时钟边沿被锁存。


*/

void QSPI_Initwithmode(uint32_t mode)
{
    uint32_t temp=0;
    //    uint32_t mode=0;
    //QSPI_GCTL|=CS_High;
    QSPI_SCSR|=CS_High;
	  #if QSPI_PSR_DEBUG == 0
    QSPI_SPBRG = 0x06;// 0x34;// OK Quad   0x4;//0x34// 
	  #else
		QSPI_SPBRG = QSPI_PSR_VALUE;// High speed
		#endif
    temp = QSPI_CCTL;
    if(mode==0)
    {
        temp |= Len_8bit|CKPH_Sam1Tran2_Edge;
        temp &= (~CKPL_High); //CKPL_Low
    }
    else if(mode==1)
    {
        temp |= Len_8bit;
        temp &= (~(CKPL_High|CKPH_Sam1Tran2_Edge)); //CKPL_Low
    }
    else if(mode==2)
    {
        temp |= Len_8bit|CKPL_High|CKPH_Sam1Tran2_Edge;//CKPL_High;  when /CS=High CLK = 0
    }

    else if(mode==3)
    {
        temp |= Len_8bit|CKPL_High;//CKPL_High; when /CS=High CLK = 0
        temp &= (~CKPH_Sam1Tran2_Edge); //
    }
    QSPI_CCTL=temp;
    // temp=(Len_8bit| CKPL_Low | CKPH_Sam1Tran2_Edge); //  ok
    //QSPI_CCTL=(Len_8bit| CKPL_High | CKPH_Sam1Tran2_Edge);// lose  1 clk
    // QSPI_CCTL=(Len_8bit| CKPL_High | CKPH_Sam2Tran1_Edge); // 
    //QSPI_CCTL=(Len_8bit| CKPL_Low | CKPH_Sam2Tran1_Edge); 
    QSPI_GCTL|=MM|QSPIEN;//|CSC_SEL;
}
#endif
//Send Data 3 Ways
void QSPI_SendData_Standard(unsigned char SendData)
{
    //      uint32_t temp;
    //    temp = QSPI_MODE;
    //    temp = temp&(~0x3);
    //    temp = temp|0x0C;
    //    QSPI_MODE = temp; //      QSPI_MODE|=0xc;//SINGLE
    //    temp = QSPI_MODE;
    //    QSPI_TXREG =temp;
    QSPI_MODE=0xc;//SINGLE
    QSPI_GCTL|=TXEN;
    //  QSPI_GCTL&=(~RXEN);


    QSPI_TXREG=SendData;
  #if 1
    while((QSPI_CSTAT & 0x1) == 0x0); 
  #else
    while((QSPI_CSTAT & 0x4) == 0x4); 
  #endif
    #if 0
    WaitQSPI_TranDataComple();
    #endif
}

//Send Data 3 Ways
unsigned long QSPI_SendNbytesData_Standard(unsigned char *puCSendData,uint32_t datanum,uint32_t timeout)
{

    unsigned long ulresult=0;
    unsigned long ulI;
    if(datanum==0)
        return 0;
//      uint32_t temp;
//    temp = QSPI_MODE;
//    temp = temp&(~0x3);
//    temp = temp|0x0C;
//    QSPI_MODE = temp; //      QSPI_MODE|=0xc;//SINGLE
//    temp = QSPI_MODE;
//    QSPI_TXREG =temp;
    QSPI_MODE=0xc;//SINGLE
    QSPI_GCTL|=TXEN;
//  QSPI_GCTL&=(~RXEN);
    for(ulI=0;ulI<datanum;ulI++)
    {
    
      QSPI_TXREG=*puCSendData++;
      while(1)
      {
          if((QSPI_CSTAT & 0x1) ==0x1) //txept=1
          {
              break;
          }
          else
          {
            delay(100000);
            if(timeout==0)
              break;
            timeout--;
            ulresult++;
          }
      }
    }
    if(ulresult==0)
      return 1;
    else
      return 0;
   //   WaitQSPI_TranDataComple();
}

void QSPI_SendData_Dual(unsigned char SendData)
{
//      unsigned char temp;
//      uint32_t temp;
//    temp = QSPI_MODE;
//    temp = temp&(~0x3);
//    temp = temp|0x0D; //0x01 or 0xd
//    QSPI_MODE = temp; //  
  QSPI_MODE=0xd;//dual
      QSPI_GCTL|=TXEN;
      QSPI_TXREG=SendData;
      WaitQSPI_TranDataComple();
}

void QSPI_SendData_Quad(unsigned char SendData)
{
//      unsigned char temp;
//      uint32_t temp;
//    temp = QSPI_MODE;
//    temp = temp&(~0x3);
//    temp = temp|0x0D; //0x01 or 0xd
//    QSPI_MODE = temp; //  
	QSPI_MODE=0xe;//dual
	QSPI_GCTL|=TXEN;
	QSPI_TXREG=SendData;
	WaitQSPI_TranDataComple();
}
//Send Data 3 Ways
unsigned long QSPI_SendNbytesData_Quad(unsigned char *puCSendData,uint32_t datanum,uint32_t timeout)
{

    unsigned long ulresult=0;
  unsigned long ulI;
    if(datanum==0)
      return 0;
//      uint32_t temp;
//    temp = QSPI_MODE;
//    temp = temp&(~0x3);
//    temp = temp|0x0C;
//    QSPI_MODE = temp; //      QSPI_MODE|=0xc;//SINGLE
//    temp = QSPI_MODE;
//    QSPI_TXREG =temp;
      QSPI_MODE=0xe;//SINGLE
    QSPI_GCTL|=TXEN;
//  QSPI_GCTL&=(~RXEN);
    for(ulI=0;ulI<datanum;ulI++)
    {
    
      QSPI_TXREG=*puCSendData++;
      while(1)
      {
          if((QSPI_CSTAT & 0x1) ==0x1) //txept=1
          {
              break;
          }
          else
          {
            delay(10);
            if(timeout==0)
              break;
            timeout--;
            ulresult++;
          }
      }
    }
    if(ulresult==0)
      return 1;
    else
      return 0;
   //   WaitQSPI_TranDataComple();  
}

//Recieve Data 3 Ways
void QSPI_RecData_Standard(int rxnum)
{
//    uint32_t RecData;
//    int i;
//    uint32_t temp;
//    temp = QSPI_MODE;
//    temp = temp&(~0x3);
//    temp = temp|0xC;
//    QSPI_MODE = temp; //
  QSPI_MODE=0xc;//SINGLE
    QSPI_GCTL|=RXEN;
    WaitQSPI_ReceiveDataComple(rxnum);
}

void QSPI_RecData_Dual(int rxnum)
{
//    uint32_t RecData;
//    int i;
//      uint32_t temp;
//    temp = QSPI_MODE;
//    temp = temp&(~0x3);
//    temp = temp|0x0D; //0x0d 0x01
//    QSPI_MODE = temp; 
    QSPI_MODE=0xd;//dual
    QSPI_GCTL|=RXEN;
    WaitQSPI_ReceiveDataComple(rxnum);
}

void QSPI_RecData_Quad(int rxnum)
{
//    uint32_t RecData;
//    int i;
//      uint32_t temp;
//    temp = QSPI_MODE;
//    temp = temp&(~0x3);
//    temp = temp|0x2;//0X0E
//    QSPI_MODE = temp; //    
    QSPI_MODE=0xe;//quad  // QSPI_MODE=0x2;//quad //
    QSPI_GCTL|=RXEN;
    WaitQSPI_ReceiveDataComple(rxnum);
}
void QSPI_RecData_Quadover8Bytes(int rxnum)
{
//    uint32_t RecData;
//    int i;
//      uint32_t temp;
//    temp = QSPI_MODE;
//    temp = temp&(~0x3);
//    temp = temp|0x2;//0X0E
//    QSPI_MODE = temp; //    
    QSPI_MODE=0xe;//quad  // QSPI_MODE=0x2;//quad //
    QSPI_GCTL|=RXEN;
    WaitQSPI_ReceiveDataCompleOver8bytes(rxnum);
}
void QSPI_ReadID(void)
{
    QSPI_SCSR&=CS_Low;//CS=Low
    QSPI_SendData_Standard(RDID_Com);
    QSPI_GCTL&=TXDIS;
    QSPI_GCTL&=Data_8bit;
    QSPI_RXDNR=0x3; 
    QSPI_RecData_Standard(3);
    QSPI_GCTL&=RXDIS;
    QSPI_SCSR|=CS_High;
    if((u32data[0]!=ManuID) | (u32data[1]!=MemType) |(u32data[2]!=MemCap))
    {
        printf("Read ID fail..\r\n");
        //while(1);
    }
    else
    {
       printf("Read ID PASS..\r\n");
    }
}
void QSPI_ReadManufacturer_Device_IDbyStandard(void)
{
//    unsigned char i;
    QSPI_SCSR&=CS_Low;//CS=Low
    QSPI_SendData_Standard(0x90);
    QSPI_SendData_Standard(0x00); //dummy
    QSPI_SendData_Standard(0x00);
    QSPI_SendData_Standard(0x00);  //0x00
    QSPI_GCTL&=TXDIS;
    QSPI_GCTL&=Data_8bit;
    QSPI_RXDNR=0x2; 
    QSPI_RecData_Standard(2);
    QSPI_GCTL&=RXDIS;
    QSPI_SCSR|=CS_High;
    if((u32data[0]!=ManuID) | (u32data[1]!=W25Q32JVID) )
    {
        printf("QSPI_ReadManufacturer_Device_IDbyStandard fail..\r\n");
//        while(1);
    }
    else
    {
        printf("QSPI_ReadManufacturer_Device_IDbyStandard PASS..\r\n");
    }
}

void QSPI_ReadManufacturer_Device_IDby_Dual_IO(unsigned long address)
{
    unsigned char addr0,addr1,addr2;

    address = address&0x00ffffff; 
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
    addr2 = ((unsigned char)address)&0xff;
    QSPI_GCTL&=Data_8bit;

    QSPI_SCSR&= CS_Low; 
    QSPI_SendData_Standard(0x92);
    QSPI_SendData_Dual(addr0);
    QSPI_SendData_Dual(addr1);
    QSPI_SendData_Dual(addr2);
    QSPI_SendData_Dual(0x00);// Dummy

    //QSPI_GpioInit_Quad_Iutput();
    QSPI_GCTL&=TXDIS;
    QSPI_RXDNR=2; 
    QSPI_RecData_Dual(2);
    QSPI_GCTL&=RXDIS;
    QSPI_SCSR|= CS_High;  
    if((u32data[0]!=ManuID) | (u32data[1]!=W25Q32JVID) )
    {
         printf("QSPI_ReadManufacturer_Device_IDby_Dual_I_O fail..\r\n");
    }
    else
    {
       printf("QSPI_ReadManufacturer_Device_IDby_Dual_I_O PASS..\r\n");
    }
}

//QSPI_SendNbytesData_Quad
#if 0
//ReadManufacturer/Device ID Quad I/O (94h)
void QSPI_ReadManufacturer_Device_IDbyQuad_I_O(unsigned long address)
{

//    unsigned int j;
    unsigned char addr0,addr1,addr2;
    unsigned char sendbuf[10];
  
  
 //   WriteQEEnable();
    CheckStatus();
    address = address&0x00ffffff; //page address
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
    addr2 = ((unsigned char)address)&0xff;
    QSPI_GCTL&=Data_8bit;

    QSPI_SCSR&= CS_Low; 
    QSPI_SendData_Standard(RDMANUIDQIO_Com);
  
//    QSPI_GpioInit_Quad_Output();
//    QSPI_SendData_Quad(addr0);
//    QSPI_SendData_Quad(addr1);
//    QSPI_SendData_Quad(0x00); //A7~0 = 0X00
//    QSPI_SendData_Quad(0xf0); //M7~0 must to be set to 0xFx
//    QSPI_SendData_Quad(0x00); //Dummy
//    QSPI_SendData_Quad(0x00); //Dummy
    sendbuf[0]=addr0;
    sendbuf[1]=addr1;
    sendbuf[2]=0x00;
    sendbuf[3]=0xf0;
    sendbuf[4]=0x00;
    sendbuf[5]=0x00;
    QSPI_SendNbytesData_Quad(sendbuf,6,100000); //6
    QSPI_GCTL&=TXDIS;
//    QSPI_GCTL|=RXEN;
    QSPI_RXDNR=2; 
//  QSPI_GpioInit_Quad_Iutput();
    QSPI_RecData_Quad(2);
    QSPI_GCTL&=RXDIS;
    QSPI_SCSR|= CS_High;  
    if((u32data[0]!=ManuID) | (u32data[1]!=W25Q32JVID) )
    {
         printf("QSPI_ReadManufacturer_Device_IDbyQuad_I_O fail..\r\n");
        //while(1);
    }
    else
    {
       printf("QSPI_ReadManufacturer_Device_IDbyQuad_I_O PASS..\r\n");
    }
}
#endif
#if 0
//ReadManufacturer/Device ID Quad I/O (94h)
void QSPI_ReadManufacturer_Device_IDbyQuad_I_O(unsigned long address)
{
//    unsigned int j;
    unsigned char addr0,addr1,addr2;
    CheckStatus();
    address = address&0x00ffffff; //page address
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
    addr2 = ((unsigned char)address)&0xff;
    QSPI_GCTL&=Data_8bit;

    QSPI_SCSR&= CS_Low; 
    QSPI_SendData_Standard(RDMANUIDQIO_Com);
//    QSPI_GpioInit_Quad_Output();
    QSPI_SendData_Quad(addr0);
    QSPI_SendData_Quad(addr1);
    QSPI_SendData_Quad(0x00); //A7~0 = 0X00
    QSPI_SendData_Quad(0xf0); //M7~0 must to be set to 0xFx
    QSPI_SendData_Quad(0x00); //Dummy
    QSPI_SendData_Quad(0x00); //Dummy
    QSPI_GCTL&=TXDIS;
//    QSPI_GCTL|=RXEN;
    delay(100);
    QSPI_RXDNR=2; 
//  QSPI_GpioInit_Quad_Iutput();
    QSPI_RecData_Quad(2);
    QSPI_GCTL&=RXDIS;
    QSPI_SCSR|= CS_High;  
    if((u32data[0]!=ManuID) | (u32data[1]!=W25Q32JVID) )
    {
         printf("QSPI_ReadManufacturer_Device_IDbyQuad_I_O fail..\r\n");
        //while(1);
    }
    else
    {
       printf("QSPI_ReadManufacturer_Device_IDbyQuad_I_O PASS..\r\n");
    }
}
#endif
#if 1
//ReadManufacturer/Device ID Quad I/O (94h)
void QSPI_ReadManufacturer_Device_IDbyQuad_I_O(unsigned long address)
{
//    unsigned int j;
    unsigned char addr0,addr1;//,addr2=0;
    CheckStatus();
    address = address&0x00ffffff; //page address
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
//    addr2 = ((unsigned char)address)&0xff;
    QSPI_GCTL&=Data_8bit;

    QSPI_SCSR&= CS_Low; 
    QSPI_SendData_Standard(RDMANUIDQIO_Com);
//    QSPI_GpioInit_Quad_Output();
    QSPI_SendData_Quad(addr0);
    QSPI_SendData_Quad(addr1);
    QSPI_SendData_Quad(0x00); //A7~0 = 0X00
    QSPI_SendData_Quad(0x00); //M7~0 must to be set to 0xFx
    QSPI_SendData_Quad(0x00); //Dummy
   // QSPI_SendData_Quad(0x00); //Dummy
    QSPI_GCTL&=TXDIS;
//    QSPI_GCTL|=RXEN;
    delay(100);
    
    QSPI_RXDNR=3; 
//  QSPI_GpioInit_Quad_Iutput();
    QSPI_RecData_Quad(3);
    QSPI_GCTL&=RXDIS;
    QSPI_SCSR|= CS_High;  
    if((u32data[1]!=ManuID) | (u32data[2]!=W25Q32JVID) )
    {
        printf("QSPI_ReadManufacturer_Device_IDbyQuad_I_O fail..\r\n");
        //while(1);
    }
    else
    {
       printf("QSPI_ReadManufacturer_Device_IDbyQuad_I_O PASS..\r\n");
    }
}
#endif
void QSPI_WriteEnable()
{
    QSPI_SCSR&=CS_Low;
    QSPI_GCTL&=Data_8bit;
    QSPI_SendData_Standard(WREN_Com);
    QSPI_SCSR|=CS_High;
}

void QSPI_Volatile_SR_WriteEnable()
{
    QSPI_SCSR&=CS_Low;
    QSPI_GCTL&=Data_8bit;
    QSPI_SendData_Standard(0x50);
    QSPI_SCSR|=CS_High;
}

void QSPI_WriteDisable()
{
    QSPI_SCSR&= CS_Low; 
    QSPI_SendData_Standard(WRDI_Com); 
    QSPI_SCSR|= CS_High;  
}

//Read and Write Registers 1/2/3
void QSPI_ReadStatusReg1()
{
    QSPI_SCSR&=CS_Low;
    QSPI_GCTL&=Data_8bit;
    QSPI_RXDNR=1;
    QSPI_SendData_Standard(0x5);
    QSPI_GCTL&=TXDIS;
    QSPI_RecData_Standard(1);
    delay(5);
    QSPI_SCSR|=CS_High;
}

void QSPI_WriteStatusReg1(uint8_t u32data)
{
    //QSPI_Volatile_SR_WriteEnable();
    QSPI_SCSR&=CS_Low;
    QSPI_GCTL&=Data_8bit;
    QSPI_SendData_Standard(0x01);
    QSPI_SendData_Standard(u32data);
    QSPI_SCSR|=CS_High;
}

void QSPI_ReadStatusReg2()
{
    QSPI_SCSR&=CS_Low;
    QSPI_GCTL&=Data_8bit;
    QSPI_RXDNR=1;
    QSPI_SendData_Standard(0x35);
    QSPI_GCTL&=TXDIS;
    QSPI_RecData_Standard(1);
    delay(5);
    QSPI_SCSR|=CS_High;
}

void QSPI_WriteStatusReg2(uint8_t u32data)
{
    QSPI_SCSR&=CS_Low;
    QSPI_GCTL&=Data_8bit;
    QSPI_SendData_Standard(0x31);
    QSPI_SendData_Standard(u32data);
    QSPI_SCSR|=CS_High;
}

void writeOutputDriverStrength(void)
{
  unsigned char temp=0;
  QSPI_ReadStatusReg2();
  temp=(unsigned char)(u32data[0]);
  temp = temp&0x9F;
  temp |= 0x00;//100%
//  temp |= 0x20;//75%
//  temp |= 0x40;//50%
//  temp |= 0x60;//25%
  QSPI_WriteStatusReg2(temp);
  
}

//  while(1){
//    if(W25x32_ReadStatus() & 0x1C){  
//    W25x32_WriteStatus(0x00);  
//    }
void DisableProtect(void)
{
  unsigned char temp=0;
  while(1){
    QSPI_ReadStatusReg1();
    temp=(unsigned char)(u32data[0]);
    if(temp&0x1C)
    QSPI_WriteStatusReg1(temp);
    else
      break;
  }
}

void QSPI_ReadStatusReg3()
{
    QSPI_SCSR&=CS_Low;
    QSPI_GCTL&=Data_8bit;
    QSPI_RXDNR=1;
    QSPI_SendData_Standard(0x15);
    QSPI_GCTL&=TXDIS;
    QSPI_RecData_Standard(1);
    delay(5);
    QSPI_SCSR|=CS_High;
}

void QSPI_WriteStatusReg3(uint8_t u32data)
{
    QSPI_SCSR&=CS_Low;
    QSPI_GCTL&=Data_8bit;
    QSPI_SendData_Standard(0x11);
    QSPI_SendData_Standard(u32data);
    QSPI_SCSR|=CS_High;
}

void QSPI_Reset()
{
    QSPI_SCSR&=CS_Low;
    QSPI_GCTL&=Data_8bit;
    QSPI_SendData_Standard(0x66);
    QSPI_SCSR|=CS_High;

    QSPI_SCSR&=CS_Low;
    QSPI_SendData_Standard(0x99);
    QSPI_SCSR|=CS_High;
}
#if 0
void CheckStatus()
{
    QSPI_SCSR&= CS_Low; 
    QSPI_GCTL&=Data_8bit;
    QSPI_RXDNR=0x1; 
    QSPI_SendData_Standard(RDSR_Com);
    QSPI_GCTL&=TXDIS;
    while(1)
    {
        QSPI_RecData_Standard(1);
        QSPI_GCTL&=RXDIS;
        if((u32data[0] & 0x01)==0)
        {
            break;
        }
    }
    QSPI_SCSR|= CS_High;  
}
#else
void CheckStatus(void)
{
	#if 1
    while(1)    
    {
        QSPI_SCSR&= CS_Low; 
        QSPI_GCTL&=Data_8bit;
        QSPI_RXDNR=0x1; 
        QSPI_SendData_Standard(RDSR_Com);
        QSPI_GCTL&=TXDIS;


        QSPI_RecData_Standard(1);
        QSPI_GCTL&=RXDIS;
        QSPI_SCSR|= CS_High;  
        if((u32data[0] & 0x01)==0)
        {
            break;
        }
        #if 0
        else
        {
            delay(10);
        }
        #endif
    }
		#endif

}
#endif
void QSPI_PageProgram_Standard(unsigned long address,uint32_t datanum)
{
    unsigned int j;
    unsigned char addr0,addr1,addr2;
	
    address = address&0x00ffffff; //page address
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
    addr2 = ((unsigned char)address)&0xff;
    QSPI_WriteEnable();
    QSPI_GCTL&=Data_8bit;
    //QSPI_RXDNR=0x1; 
    QSPI_SCSR&= CS_Low; 
    QSPI_SendData_Standard(PP_Com);
    QSPI_SendData_Standard(addr0);
    QSPI_SendData_Standard(addr1);
    QSPI_SendData_Standard(addr2);
    for(j=0;j<datanum;j++)
    {
       //QSPI_SendData_Standard(j+0x55);//code[j]);//code[j]);
        //QSPI_SendData_Standard(0xcc);//code[j]);//code[j]);
        QSPI_SendData_Standard(j);//code[j]);//code[j]);
    }
    QSPI_SCSR|= CS_High;  //Spi cs assign to this pin,select
    QSPI_WriteDisable();
}



void QSPI_PageProgram_Quad(unsigned long address,uint32_t datanum)
{
    unsigned int j;
    unsigned char addr0,addr1,addr2;
	
    address = address&0x00ffffff; //page address
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
    addr2 = ((unsigned char)address)&0xff;
    QSPI_WriteEnable();
    QSPI_GCTL&=Data_8bit;
  //  QSPI_RXDNR=0x1; 
    QSPI_SCSR&= CS_Low; 
    QSPI_SendData_Standard(PP_Com_Quad);
    QSPI_SendData_Standard(addr0);
    QSPI_SendData_Standard(addr1);
    QSPI_SendData_Standard(addr2);
    QSPI_GpioInit_Quad_Output();
    for(j=0;j<datanum;j++)
    {
       QSPI_SendData_Quad(j);//code[j])
    }
    QSPI_SCSR|= CS_High;  //Spi cs assign to this pin,select
}
/*
Data Input Output         Byte 1 | Byte 2 | Byte 3 | Byte 4 | Byte 5 | Byte 6 | Byte 7
Number of Clock(1-1-1)    8      | 8      | 8      | 8      | 8      | 8      | 8
Read Data                 03h    | A23-A16|	A15-A8 | A7-A0  | (D7-D0)| 
RD_Com 0x03
*/
void QSPIRead(unsigned long address,uint32_t datanum)
{

    unsigned char addr0,addr1,addr2;

    
    address = address&0x00ffffff; 
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
    addr2 = ((unsigned char)address)&0xff;
    QSPI_GCTL&=Data_8bit;

    QSPI_SCSR&= CS_Low; 
    QSPI_SendData_Standard(RD_Com); //SPDAT = PP;
    QSPI_SendData_Standard(addr0);
    QSPI_SendData_Standard(addr1);
    QSPI_SendData_Standard(addr2);
    QSPI_GCTL&=TXDIS;
    QSPI_RXDNR=datanum;
    QSPI_RecData_Standard(datanum);
    QSPI_GCTL&=RXDIS;
    QSPI_SCSR|= CS_High;  
}
#define QSPI_READ_MAX 0x8000
void QSPIReadGroup(unsigned long address,u8* buf,uint32_t datanum)
{
		unsigned char addr0,addr1,addr2;
    u32 rxcnt = 0,rxTemp = 0;
    address = address&0x00ffffff; 
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
    addr2 = ((unsigned char)address)&0xff;
    QSPI_GCTL&=Data_8bit;
    QSPI_SCSR&= CS_Low; 
    QSPI_SendData_Standard(RD_Com); //SPDAT = PP;
    QSPI_SendData_Standard(addr0);
    QSPI_SendData_Standard(addr1);
    QSPI_SendData_Standard(addr2);
    QSPI_GCTL&=TXDIS;
		#if  QSPI_QUAL_DEBUG == 0
	  QSPI_MODE = 0xc;//SINGLE
	  #else
	  QSPI_MODE = 0x2;//SINGLE
    
		#endif
    #if 1
    for(;datanum > QSPI_READ_MAX;datanum -= QSPI_READ_MAX)
    {
      QSPI_GCTL&=TXDIS;
      QSPI_RXDNR = QSPI_READ_MAX;
      
      rxTemp = 0;
      
			
			#if QSPI_DMA_DEBUG == 1
			qspiRecDMA_Config((u32)buf,QSPI_READ_MAX);
			buf +=QSPI_READ_MAX;
			#else
			QSPI_GCTL|=RXEN;
      while(1)
      {
          #if 1
          while((QSPI_CSTAT & 0x02)==0x00);//rxavl=1
          buf[rxcnt] = QSPI_RXREG;
          rxcnt ++;
          rxTemp ++;
          if(rxTemp == QSPI_READ_MAX)
          {
               break;
          }
          #else
          while((QSPI_CSTAT & 0x02)==0x00);//rxavl=1
          buf[rxcnt] = QSPI_RXREG;
          rxcnt++;
          rxTemp ++;
          if(rxTemp == QSPI_READ_MAX)
          {
               break;
          }
          #endif
      }
			#endif
      
      QSPI_GCTL&=RXDIS;
    }
      QSPI_GCTL&=TXDIS;
      QSPI_RXDNR = datanum;
//      QSPI_MODE = 0xc;//SINGLE
      QSPI_GCTL|=RXEN;
      rxTemp = 0;
      while(1)
      {
          while((QSPI_CSTAT & 0x02)==0x00);//rxavl=1
          buf[rxcnt] = QSPI_RXREG;
          rxcnt++;
          rxTemp++;
          if(rxTemp == datanum)
          {
               break;
          } 
      }
      QSPI_GCTL&=RXDIS;
    #else
    QSPI_RecData_Standard(datanum);
    #endif
    QSPI_SCSR|= CS_High;  
}
/*
Data Input Output         Byte 1 | Byte 2 | Byte 3 | Byte 4 | Byte 5 | Byte 6 | Byte 7
Number of Clock(1-1-1)    8      | 8      | 8      | 8      | 8      | 8      | 8
Fast Read                 0Bh    | A23-A16|	A15-A8 | A7-A0  | Dummy  | (D7-D0)| 
FastRD_Com 0x0B
*/
void QSPIFastRead(unsigned long address,uint32_t datanum)
{
    unsigned char addr0,addr1,addr2;
    address = address&0x00ffffff; 
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
    addr2 = ((unsigned char)address)&0xff;
    QSPI_GCTL&=Data_8bit;

    QSPI_SCSR&= CS_Low; 
    QSPI_SendData_Standard(FastRD_Com); //SPDAT = PP;
    QSPI_SendData_Standard(addr0);
    QSPI_SendData_Standard(addr1);
    QSPI_SendData_Standard(addr2);
    QSPI_SendData_Standard(0xff);//dummy
    QSPI_GCTL&=TXDIS;
    QSPI_RXDNR=datanum;
    QSPI_RecData_Standard(datanum);
    QSPI_GCTL&=RXDIS;
    QSPI_SCSR|= CS_High;  
}

/*
Data Input Output        | Byte 1 | Byte 2 | Byte 3 | Byte 4 | Byte 5 | Byte 6 | Byte 7 | Byte 8 | Byte 9 |
Number of Clock(1-1-2)   |  8     |  8     |  8     |  8     |  4     |  4     |  4     |  4     |  4     |
Fast Read Dual Output    |  3Bh   | A23-A16| A15-A8 | A7-A0  | Dummy  |  Dummy | (D7-D0)| (7)	   | …     |

FastRD_Dual_Com 0x3B
*/

void QSPIFastRead_Dual(unsigned long address,uint32_t datanum)
{

    unsigned char addr0,addr1,addr2;

    
    address = address&0x00ffffff; 
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
    addr2 = ((unsigned char)address)&0xff;
    QSPI_GCTL&=Data_8bit;

    QSPI_SCSR&= CS_Low; 
    QSPI_SendData_Standard(FastRD_Dual_Com);
    QSPI_SendData_Standard(addr0);
    QSPI_SendData_Standard(addr1);
    QSPI_SendData_Standard(addr2);
//    QSPI_SendData_Dual(0xff);
//    QSPI_SendData_Dual(0xff);
    QSPI_SendData_Standard(0xff);
    //QSPI_GpioInit_Quad_Iutput();
    QSPI_GCTL&=TXDIS;
    QSPI_RXDNR=datanum; 
    QSPI_RecData_Dual(datanum);
    QSPI_GCTL&=RXDIS;
    QSPI_SCSR|= CS_High;  
}

/*
Data Input Output        | Byte 1 | Byte 2 | Byte 3 | Byte 4 | Byte 5 | Byte 6 | Byte 7 | Byte 8 | Byte 9 |
Number of Clock(1-1-2)   |  8     |  4     |  4     |  4     |  4     |  4     |  4     |  4     |  4     |
Fast Read Dual Output    |  BBh   | A23-A16| A15-A8 | A7-A0  | M7-M0  | (D7-D0)|  ...   |

Number of Clock(1-2-2)		8	4	4	4	4	4	4	4	4
Fast Read Dual I/O		BBh	A23-A16(6)	A15-A8(6)	A7-A0(6)	M7-M0	(D7-D0)(7)	…


FastRD_DualIo_Com 0xBB
*/

void QSPIFastRead_Dual_IO(unsigned long address,uint32_t datanum)
{

    unsigned char addr0,addr1,addr2;

    
    address = address&0x00ffffff; 
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
    addr2 = ((unsigned char)address)&0xff;
    QSPI_GCTL&=Data_8bit;

    QSPI_SCSR&= CS_Low; 
    QSPI_SendData_Standard(FastRD_DualIo_Com);
    QSPI_SendData_Dual(addr0);
    QSPI_SendData_Dual(addr1);
    QSPI_SendData_Dual(addr2);
    QSPI_SendData_Dual(0x00);//M7~0

    //QSPI_GpioInit_Quad_Iutput();
    QSPI_GCTL&=TXDIS;
    QSPI_RXDNR=datanum; 
    QSPI_RecData_Dual(datanum);
    QSPI_GCTL&=RXDIS;
    QSPI_SCSR|= CS_High;  
}

#if 0
/*
Data Input Output        | Byte 1 | Byte 2 | Byte 3 | Byte 4 | Byte 5 | Byte 6 | Byte 7 | Byte 8 | Byte 9 |
Number of Clock(1-1-4)   |  8     |  8     |  8     |  8     |  2     |  2     |  2     |  2     |  2     |
Fast Read Quad Output    |  6Bh   | A23-A16| A15-A8 | A7-A0  | Dummy  |  Dummy |  Dummy |  Dummy | (D7-D0)| 

FastRD_Quad_Com 0x6B


*/
void QSPIFastRead_Quad(unsigned long address,uint32_t datanum)
{
//    unsigned int j;
    unsigned char addr0,addr1,addr2;
    
    address = address&0x00ffffff; 
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
    addr2 = ((unsigned char)address)&0xff;
    QSPI_GCTL&=Data_8bit;
    QSPI_SCSR&= CS_Low; 
    QSPI_SendData_Standard(FastRD_Quad_Com);
    QSPI_SendData_Standard(addr0);
    QSPI_SendData_Standard(addr1);
    QSPI_SendData_Standard(addr2);
    QSPI_SendData_Quad(0x00);
    QSPI_SendData_Quad(0x00);
    QSPI_SendData_Quad(0x00);
    QSPI_SendData_Quad(0x00);
//    QSPI_SendData_Standard(0xff);
    QSPI_GCTL&=TXDIS;
    delay(100);
    QSPI_RXDNR=datanum; 
//    QSPI_GpioInit_Quad_Iutput();
    QSPI_RecData_Quad(datanum);
    QSPI_GCTL&=RXDIS;
    QSPI_SCSR|= CS_High;  
}
/*
Data Input Output        | Byte 1 | Byte 2 | Byte 3 | Byte 4 | Byte 5 | Byte 6 | Byte 7 | Byte 8 | Byte 9 |
Number of Clock(1-4-4)   |  8     |  2(8)  |  2(8)  | 2(8)   |  2     |  2     |  2     |  2     |  2     |
Fast Read Quad I/O       |  EBh   | A23-A16| A15-A8 | A7-A0  | M7-M0  |  Dummy |  Dummy | (D7-D0)| ...

FastRD_QuadIo_Com	0xeb


*/
void QSPIFastRead_Quad_IO(unsigned long address,uint32_t datanum)
{
//    unsigned int j;
    unsigned char addr0,addr1,addr2;
    
    address = address&0x00ffffff; 
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
    addr2 = ((unsigned char)address)&0xff;
    QSPI_GCTL&=Data_8bit;
    QSPI_SCSR&= CS_Low; 
    QSPI_SendData_Standard(FastRD_QuadIo_Com);
    QSPI_SendData_Quad(addr0);
    QSPI_SendData_Quad(addr1);
    QSPI_SendData_Quad(addr2);
    QSPI_SendData_Quad(0xf0);//M7~0
    QSPI_SendData_Quad(0x00);//dummy1
    QSPI_SendData_Quad(0x00);//dummy2

    QSPI_GCTL&=TXDIS;
      delay(100);
    QSPI_RXDNR=datanum; 
//    QSPI_GpioInit_Quad_Iutput();
    QSPI_RecData_Quad(datanum);
    QSPI_GCTL&=RXDIS;
    QSPI_SCSR|= CS_High;  
}
#endif



#if 1
/*
Data Input Output        | Byte 1 | Byte 2 | Byte 3 | Byte 4 | Byte 5 | Byte 6 | Byte 7 | Byte 8 | Byte 9 |
Number of Clock(1-1-4)   |  8     |  8     |  8     |  8     |  2     |  2     |  2     |  2     |  2     |
Fast Read Quad Output    |  6Bh   | A23-A16| A15-A8 | A7-A0  | Dummy  |  Dummy |  Dummy |  Dummy | (D7-D0)| 

FastRD_Quad_Com 0x6B


*/
void QSPIFastRead_Quad(unsigned long address,uint32_t datanum)
{
//    unsigned int j;
    unsigned char addr0,addr1,addr2;
    
    address = address&0x00ffffff; 
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
    addr2 = ((unsigned char)address)&0xff;
    QSPI_GCTL&=Data_8bit;
    QSPI_SCSR&= CS_Low; 
    QSPI_SendData_Standard(FastRD_Quad_Com);
    QSPI_SendData_Standard(addr0);
    QSPI_SendData_Standard(addr1);
    QSPI_SendData_Standard(addr2);
    QSPI_SendData_Quad(0x00);
    QSPI_SendData_Quad(0x00);
    QSPI_SendData_Quad(0x00);
//    QSPI_SendData_Quad(0x00);
//    QSPI_SendData_Standard(0xff);
    QSPI_GCTL&=TXDIS;
    delay(100);

    QSPI_RXDNR=datanum+1; 
//    QSPI_GpioInit_Quad_Iutput();
//   QSPI_RecData_Quadover8Bytes(datanum+1);//
   QSPI_RecData_Quad(datanum+1);
   ///////////////////test on 0607 
    QSPI_GCTL&=RXDIS;
    QSPI_SCSR|= CS_High;  
}
/*
Data Input Output        | Byte 1 | Byte 2 | Byte 3 | Byte 4 | Byte 5 | Byte 6 | Byte 7 | Byte 8 | Byte 9 |
Number of Clock(1-4-4)   |  8     |  2(8)  |  2(8)  | 2(8)   |  2     |  2     |  2     |  2     |  2     |
Fast Read Quad I/O       |  EBh   | A23-A16| A15-A8 | A7-A0  | M7-M0  |  Dummy |  Dummy | (D7-D0)| ...

FastRD_QuadIo_Com	0xeb


*/
void QSPIFastRead_Quad_IO(unsigned long address,uint32_t datanum)
{
//    unsigned int j;
    unsigned char addr0,addr1,addr2;
    
    address = address&0x00ffffff; 
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
    addr2 = ((unsigned char)address)&0xff;
    QSPI_GCTL&=Data_8bit;
    QSPI_SCSR&= CS_Low; 
    QSPI_SendData_Standard(FastRD_QuadIo_Com);
    QSPI_SendData_Quad(addr0);
    QSPI_SendData_Quad(addr1);
    QSPI_SendData_Quad(addr2);
    QSPI_SendData_Quad(0x00);//M7~0
    QSPI_SendData_Quad(0x00);//dummy1
//    QSPI_SendData_Quad(0x00);//dummy2

    QSPI_GCTL&=TXDIS;
      delay(100);

    QSPI_RXDNR=datanum+1; 
//    QSPI_GpioInit_Quad_Iutput();
    QSPI_RecData_Quad(datanum+1);
    QSPI_GCTL&=RXDIS;
    QSPI_SCSR|= CS_High;  
}
#endif
void QSPI_SectorErase(unsigned long address)
{
//    unsigned int j;
    unsigned char addr0,addr1,addr2;
    address = address&0x00ffffff; 
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
    addr2 = ((unsigned char)address)&0xff;
    QSPI_WriteEnable();
    QSPI_SCSR&= CS_Low; 
    QSPI_GCTL&=Data_8bit;
    QSPI_SendData_Standard(SE_Com);
    QSPI_SendData_Standard(addr0);
    QSPI_SendData_Standard(addr1);
    QSPI_SendData_Standard(addr2);
    QSPI_SCSR|= CS_High; 
    QSPI_WriteDisable();
}
void QSPI_Block64Erase(unsigned long address)
{
//    unsigned int j;
    unsigned char addr0,addr1,addr2;
    address = address&0x00ffffff; 
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
    addr2 = ((unsigned char)address)&0xff;
    QSPI_WriteEnable();
    QSPI_SCSR&= CS_Low; 
    QSPI_GCTL&=Data_8bit;
    QSPI_SendData_Standard(BE64K_Com);
    QSPI_SendData_Standard(addr0);
    QSPI_SendData_Standard(addr1);
    QSPI_SendData_Standard(addr2);
    QSPI_SCSR|= CS_High; 
    QSPI_WriteDisable();
}

//写SPI FLASH  
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//WriteAddr:开始写入的地址(最大32bit)						
//NumByteToWrite:要写入的字节数(最大65535)   

void QSPI_SectorSafeErase(uint32_t WriteAddr, uint32_t NumByteToWrite)   
{ 
	u32 secpos;
	u32 lastsecpos;
	unsigned long i=100000;//00;
 
    
 	secpos=WriteAddr/4096;//扇区地址  	
	lastsecpos=(WriteAddr+NumByteToWrite)/4096;//扇区地址  


	
	while(1) 
	{
			if(secpos<=lastsecpos){
				QSPI_SectorErase(secpos*4096);//0xc0000);//W25QXX_Erase_Sector(secpos);//擦除这个扇区
       CheckStatus();
				while(i--);
				secpos++;				
			}
			else{
				break;
			}
	}
}


void QSPI_ChipErase(void)
{
    QSPI_WriteEnable();
    QSPI_SCSR&= CS_Low; 
    QSPI_GCTL&=Data_8bit;
    QSPI_SendData_Standard(CE_Com);
    QSPI_SCSR|= CS_High; 
}

void QSPI_PAGE_Quad(uint32_t address,uint32_t datanum)
{
	    unsigned int i;
	unsigned int j=0;
      printf("Quad SPI  QSPI_PAGE_Quad test start!\r\n");
	j=100;
	while(j--){
    QSPI_SectorSafeErase(address, datanum); //QSPI_SectorErase(address);//0xc0000);
 //   CheckStatus();
		CheckStatus();
    QSPIRead(address,datanum);//0xc0000
		    for(i=0;i<datanum;i++)
    {
       if(u32data[i]!=0xff)//code[i])
       {
         break;//printf("Right u32data is %x,Wrong u32data is %x\n",i,u32data[i]);
       }
    }
    //QSPI_PageProgram_Standard(address,datanum);//0xc0000);
        QSPI_WriteEnable();
    QSPI_WriteStatusReg2(0x02);
    CheckStatus();
    QSPI_PageProgram_Quad(address,datanum);//0xc0000);
    CheckStatus();
//    QSPIRead(address,datanum);//0xc0000
//    QSPIFastRead_Dual(address,datanum);//0xc0000
    QSPI_WriteEnable();
    QSPI_WriteStatusReg2(0x02);
    CheckStatus();
		QSPIFastRead_Quad(address,datanum);//0xc0000
				    for(i=0;i<datanum;i++)
    {
       if(u32data[i]!=i)//code[i])
       {
         break;//printf("Right u32data is %x,Wrong u32data is %x\n",i,u32data[i]);
       }
    }
      printf("Quad SPI  QSPI_PAGE_Quad test finish!\r\n");

//    
	}
	return;
}
void QSPI_Run_QuadWriteDualIOread(uint32_t address,uint32_t datanum)
{
	    unsigned int i;
	unsigned int j=0;
  printf("Quad SPI  QSPI_Run_QuadWriteDualIOread test start!\r\n");
	j=100;
	while(j--){
    QSPI_SectorSafeErase(address, datanum); //QSPI_SectorErase(address);//0xc0000);
   // CheckStatus();
		CheckStatus();
//		QSPI_WriteEnable();
//    WriteQEDisable();//QSPI_WriteStatusReg2(0x02);
//    CheckStatus();
    QSPIRead(address,datanum);//0xc0000
		    for(i=0;i<datanum;i++)
    {
       if(u32data[i]!=0xff)//code[i])
       {
         printf("Quad SPI  sector Erase test fail!\r\n");
         break;//printf("Right u32data is %x,Wrong u32data is %x\n",i,u32data[i]);
       }
    }
//		QSPI_WriteEnable();
//    WriteQEEnable();//QSPI_WriteStatusReg2(0x02);
//    CheckStatus();
    //QSPI_PageProgram_Standard(address,datanum);//0xc0000);
    QSPI_PageProgram_Quad(address,datanum);//0xc0000);
    CheckStatus();
//    QSPIRead(address,datanum);//0xc0000
//		QSPI_WriteEnable();
//    WriteQEDisable();//QSPI_WriteStatusReg2(0x02);
//    CheckStatus();
    QSPIFastRead_Dual(address,datanum);//0xc0000
//    QSPI_WriteEnable();
//    QSPI_WriteStatusReg2(0x02);
//    CheckStatus();
//		QSPIFastRead_Quad(address,datanum);//0xc0000
				    for(i=0;i<datanum;i++)
    {
       if(u32data[i]!=i)//code[i])
       {
         printf("Quad SPI  QSPIFastRead_Quad test fail!\r\n");
         break;//printf("Right u32data is %x,Wrong u32data is %x\n",i,u32data[i]);
       }
    }

  printf("Quad SPI  QSPI_Run_QuadWriteDualIOread test finish!\r\n");
//    
	}
	return;
}
void QSPI_Run_Standard(uint32_t address,uint32_t datanum)
{
	    unsigned int i;
	unsigned int j=0;
      printf("Quad SPI  QSPI_Run_Standard test start!\r\n");
	j=100;
	while(j--){
    //QSPI_SectorErase(address);//0xc0000);
		QSPI_SectorSafeErase(address, datanum);
  //  CheckStatus();
		CheckStatus();
    QSPIRead(address,datanum);//0xc0000
		    for(i=0;i<datanum;i++)
    {
       if(u32data[i]!=0xff)//code[i])
       {
         break;//printf("Right u32data is %x,Wrong u32data is %x\n",i,u32data[i]);
       }
    }
    QSPI_PageProgram_Standard(address,datanum);//0xc0000);
 //   QSPI_PageProgram_Quad(address,datanum);//0xc0000);
    CheckStatus();
    QSPIRead(address,datanum);//0xc0000
				    for(i=0;i<datanum;i++)
    {
       if(u32data[i]!=i)//code[i])
       {
         break;//printf("Right u32data is %x,Wrong u32data is %x\n",i,u32data[i]);
       }
    }
//    QSPIFastRead_Dual(address,datanum);//0xc0000
//    QSPI_WriteEnable();
//    QSPI_WriteStatusReg2(0x02);
//    CheckStatus();

//    QSPIFastRead_Quad(address,datanum);//0xc0000
	}
    printf("Quad SPI  QSPI_Run_Standard test finish No.%03d!\r\n",100-j);
	return;
}






//void QSPI_ReadStatusReg2()
//{
//    QSPI_SCSR&=CS_Low;
//    QSPI_GCTL&=Data_8bit;
//    QSPI_RXDNR=1;
//    QSPI_SendData_Standard(0x35);
//    QSPI_GCTL&=TXDIS;
//    QSPI_RecData_Standard(1);
//    delay(5);
//    QSPI_SCSR|=CS_High;
//}

//void QSPI_WriteStatusReg2(uint8_t u32data)

void WriteQEEnable(void)
{
	uint8_t ucdata=0;
	QSPI_ReadStatusReg2();
	ucdata=u32data[0];
	ucdata = ucdata|0x02;
	QSPI_WriteStatusReg2(ucdata);
}
	
void WriteQEDisable(void)
{
		uint8_t ucdata=0;
	QSPI_ReadStatusReg2();
	ucdata=u32data[0];
	ucdata = ucdata&(~0x02);
	QSPI_WriteStatusReg2(ucdata);	
}
//void testSectorErasefunction(void)
//{
//	    unsigned int i;
//	unsigned int j=0;
//	j=100;
//	while(j--){
//    //QSPI_SectorErase(address);//0xc0000);
//		QSPI_SectorSafeErase(address, datanum);
//  //  CheckStatus();
//		CheckStatus();
//    QSPIRead(address,datanum);//0xc0000
//		    for(i=0;i<datanum;i++)
//    {
//       if(u32data[i]!=0xff)//code[i])
//       {
//         break;//printf("Right u32data is %x,Wrong u32data is %x\n",i,u32data[i]);
//       }
//    }  
//  
//  QSPI_ChipErase();
//  
//  
//}
#define  W25Q32JVSECTORNUMBER  1024  // 4194304 bytes /4096 = 1024
#define  W25Q32JVPAGENUMBER  16384  // 4194304 bytes /256 = 16384
#define  W25Q32JVSIZE  4194304UL   //33554432 bits /8 = 4194304 bytes
#define  SECTORSIZE 4096
unsigned long  testSectorErasefunction(void)
{
	unsigned int i=0;
	unsigned int j=0;

  unsigned long address=0;
  unsigned long errorcount=0;
//  uint32_t datanum;
  //QSPI_ChipErase();
    QSPI_WriteEnable();
	//for(j=0;j<(4*SECTORSIZE/SECTORSIZE); j++) //
  for(j=0;j<(W25Q32JVSIZE/SECTORSIZE); j++)
  {
      QSPI_SectorErase(j*SECTORSIZE);
    		CheckStatus();
  }
  
	for(j=0;j<(4*SECTORSIZE/RXDATAMAXLENGTH); j++)//for(j=0;j<(W25Q32JVSIZE/RXDATAMAXLENGTH); j++)
  {
    address = i*RXDATAMAXLENGTH;
    QSPIRead(address,RXDATAMAXLENGTH);//0xc0000
		for(i=0;i<RXDATAMAXLENGTH;i++)
    {
       if(u32data[i]!=0xff)//code[i])
       {
         errorcount++;
         //break;//printf("Right u32data is %x,Wrong u32data is %x\n",i,u32data[i]);
       }
    }
  }
  return errorcount;
}

unsigned long  testchipEraseFunction(void)
{


  unsigned long errorcount=0;
  QSPI_WriteEnable();
//  uint32_t datanum;
  QSPI_ChipErase();
  CheckStatus();
    /*
	for(j=0;j<(32); j++)
  //for(j=0;j<(W25Q32JVSIZE/RXDATAMAXLENGTH); j++)
  {
    address = i*RXDATAMAXLENGTH;
    QSPIRead(address,RXDATAMAXLENGTH);//0xc0000
		for(i=0;i<RXDATAMAXLENGTH;i++)
    {
       if(u32data[i]!=0xff)//code[i])
       {
         errorcount++;
         //break;//printf("Right u32data is %x,Wrong u32data is %x\n",i,u32data[i]);
       }
    }
  }*/
  return errorcount;
}

unsigned long  ReadanyAddrunderRXDATAMAXLENGTHwithmode(unsigned long startaddress,uint32_t datanum,unsigned char readmode)
{
  if(datanum>RXDATAMAXLENGTH)
    return 0;
  if(readmode==0)
  {
    QSPIRead(startaddress,datanum); 
  }
  else if(readmode==1)
  {
    QSPIFastRead_Dual(startaddress,datanum);
  }
  else
  {
    QSPIFastRead_Quad(startaddress,datanum);
  }
  return 1;
}
unsigned long  ReadanyAddrunderRXDATAMAXLENGTHwithbuffermode(unsigned long startaddress,unsigned char *pbuffer, uint32_t datanum,unsigned char readmode)
{
  uint32_t i;
  if(datanum>RXDATAMAXLENGTH)
    return 0;
  if(readmode==0)
  {
    QSPIRead(startaddress,datanum); 
  }
  else if(readmode==1)
  {
    QSPIFastRead_Dual(startaddress,datanum);
  }
  else if(readmode == 2)
  {
    QSPIFastRead_Quad(startaddress,datanum);
  }
  else if(readmode==3)
  {
    QSPIFastRead(startaddress,datanum); 
  }
  else if(readmode==4)
  {
    QSPIFastRead_Dual_IO(startaddress,datanum);
  }
  else
  {
    QSPIFastRead_Quad_IO(startaddress,datanum);
  }
  if((readmode == 2)||(readmode >4)){
      for(i=0;i<datanum;i++) 
      {
        
        *pbuffer= (unsigned char) (u32data[i+1]&0xff);
        pbuffer++;
      }
    }
  else
  {
    for(i=0;i<datanum;i++) 
    {    
      *pbuffer= (unsigned char) (u32data[i]&0xff);
      pbuffer++;
    }
    
  }
  //memcpy( pbuffer, &u32data[0], datanum);
  return 1;
}
unsigned long  testanyAddrAnyLengthByteswithbytesRead(unsigned long startaddress,uint32_t datanum,unsigned char readmode)
{
	unsigned int i=0;


  unsigned long address=0;
  unsigned long errorcount=0;
 
  uint32_t readbytes,lastbytes=0;

  address   = startaddress;
  lastbytes = datanum;
  while(1)
  {
    if(lastbytes>=RXDATAMAXLENGTH)
    {
      readbytes = RXDATAMAXLENGTH;
      lastbytes = lastbytes - RXDATAMAXLENGTH;
    }
    else
    {
      readbytes = datanum%RXDATAMAXLENGTH;
    }
    ReadanyAddrunderRXDATAMAXLENGTHwithmode(address,readbytes,0); //QSPIRead(address,readbytes);//0xc0000  break out
    for(i=0;i<readbytes;i++)
    {
      printf("read address %08x = %02x \r\n",(address+i),u32data[i]);
//       if(u32data[i]!=0xff)//code[i] check code
//       {
//         errorcount++;
//         //break;//printf("Right u32data is %x,Wrong u32data is %x\n",i,u32data[i]);
//       }
    }
    if(lastbytes>0)
    {      
      address = address + RXDATAMAXLENGTH;
    }
    else
    {
      break;
    }

  }
  return errorcount;
}
#define PAGELENGTH  256

//only can be writen on one page
void QSPI_PageXbytesProgram_Standard_NoCheck(unsigned long address,unsigned char *ucbuffer,uint32_t datanum)
{
    unsigned int j;
    unsigned char addr0,addr1,addr2;
//    unsigned long address;

    if((address%256+datanum)>256)
      return;
    address = address&0x00ffffff; //page address
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
    addr2 = ((unsigned char)address)&0xff;
    QSPI_WriteEnable();
    QSPI_GCTL&=Data_8bit;
    //QSPI_RXDNR=0x1; 
    QSPI_SCSR&= CS_Low; 
    QSPI_SendData_Standard(PP_Com);
    QSPI_SendData_Standard(addr0);
    QSPI_SendData_Standard(addr1);
    QSPI_SendData_Standard(addr2);
    
    #if 1//for test
    QSPI_MODE=0xc;//SINGLE
    QSPI_GCTL|=TXEN;
    //  QSPI_GCTL&=(~RXEN);
    #endif
    
//    QSPI_FunDmaTxConfig(u32 adress,u32 len)
    for(j=0;j<datanum;j++)
    {
      #if 1//for test
//        while((QSPI_CSTAT & 0x1) == 0x0); 
        QSPI_TXREG = *ucbuffer++;
        
        while((QSPI_CSTAT & 0x4) == 0x4);  
      #else
       //QSPI_SendData_Standard(j+0x55);//code[j]);//code[j]);
        //QSPI_SendData_Standard(0xcc);//code[j]);//code[j]);
        QSPI_SendData_Standard(*ucbuffer);//code[j]);//code[j]);
        ucbuffer++;
      #endif
    }
    while((QSPI_CSTAT & 0x1) == 0x0);  //wait last data 
    
    QSPI_SCSR|= CS_High;  //Spi cs assign to this pin,select
    QSPI_WriteDisable();
    CheckStatus();
}


//only can be writen on one page
void QSPI_PageProgram(unsigned long address,unsigned char *ucbuffer,uint32_t datanum)
{
    unsigned int j;
    unsigned char addr0,addr1,addr2;
//    unsigned long address;

    if((address%256+datanum)>256)
      return;
    address = address&0x00ffffff; //page address
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
    addr2 = ((unsigned char)address)&0xff;
    QSPI_WriteEnable();
    QSPI_GCTL&=Data_8bit;
    //QSPI_RXDNR=0x1; 
    QSPI_SCSR&= CS_Low; 
    QSPI_SendData_Standard(PP_Com);
    QSPI_SendData_Standard(addr0);
    QSPI_SendData_Standard(addr1);
    QSPI_SendData_Standard(addr2);
    for(j=0;j<datanum;j++)
    {
       //QSPI_SendData_Standard(j+0x55);//code[j]);//code[j]);
        //QSPI_SendData_Standard(0xcc);//code[j]);//code[j]);
        QSPI_SendData_Standard(*ucbuffer);//code[j]);//code[j]);
        ucbuffer++;
    }
    QSPI_SCSR|= CS_High;  //Spi cs assign to this pin,select
    QSPI_WriteDisable();
    CheckStatus();
}

/*
Number of Clock(1-1-4)		8	8	8	8	2	2	2	2	2
Quad Input Page Program		32h	A23-A16	A15-A8	A7-A0	(D7-D0)(9)	(D7-D0)(3)	…
*/
void QSPI_PageXbytesProgram_Quad_NoCheck(unsigned long address,unsigned char *ucbuffer,uint32_t datanum)
{
//    unsigned int j;
    unsigned char addr0,addr1,addr2;
    if((address%256+datanum)>256)
      return;	
    address = address&0x00ffffff; //page address
    addr0 = ((unsigned char)(address>>16))&0xff;
    addr1 = ((unsigned char)(address>>8))&0xff;
    addr2 = ((unsigned char)address)&0xff;
    QSPI_WriteEnable();
    QSPI_GCTL&=Data_8bit;
  //  QSPI_RXDNR=0x1; 
    QSPI_SCSR&= CS_Low; 
    QSPI_SendData_Standard(PP_Com_Quad);
    QSPI_SendData_Standard(addr0);
    QSPI_SendData_Standard(addr1);
    QSPI_SendData_Standard(addr2);
    QSPI_GpioInit_Quad_Output();
    QSPI_SendNbytesData_Quad(ucbuffer, datanum , 100000);
//    for(j=0;j<datanum;j++)
//    {
//       QSPI_SendData_Quad(*ucbuffer);//code[j])
//       ucbuffer++;
//    }
    QSPI_SCSR|= CS_High;  //Spi cs assign to this pin,select
    CheckStatus();
}


typedef struct {
//  uint32_t step;
  uint32_t pgtimes;
  unsigned long pgstartaddress;
  uint32_t pgdatanum;
}programblock;


unsigned long  testanyAddrAnyLengthByteswithbytesWrite(unsigned long startaddress,unsigned char *ucbuffer,uint32_t datanum,unsigned char writemode)
{
  uint32_t writebytes;
  
  if((datanum==0)||(datanum>256))
    return 0;
  
  if((startaddress%PAGELENGTH+datanum)<=PAGELENGTH){
//    if(writemode==0){//2017.6.23 
      QSPI_PageXbytesProgram_Standard_NoCheck( startaddress, ucbuffer, datanum );
//    }//2017.6.23 
//    else{
//      QSPI_PageXbytesProgram_Quad_NoCheck( startaddress, ucbuffer, datanum );
//    }
  }
  else if((startaddress%PAGELENGTH+datanum)<=(PAGELENGTH*2))
  {
    writebytes = PAGELENGTH - (startaddress%PAGELENGTH);
//    if(writemode==0){//2017.6.23 
      QSPI_PageXbytesProgram_Standard_NoCheck( startaddress, ucbuffer, datanum );
//    }//2017.6.23 
//    else{
//      QSPI_PageXbytesProgram_Quad_NoCheck( startaddress, ucbuffer, datanum );
//    }
    startaddress = startaddress + writebytes;
    ucbuffer = ucbuffer +writebytes;
    writebytes = datanum-writebytes;
//    if(writemode==0){//2017.6.23 
      QSPI_PageXbytesProgram_Standard_NoCheck( startaddress, ucbuffer, datanum );
//    }//2017.6.23 
//    else{
//      QSPI_PageXbytesProgram_Quad_NoCheck( startaddress, ucbuffer, datanum );
//    }
  }
  else
  {
      
  }
  return 0;
}


unsigned long  runfullchipwritereadtest(void)
{
	unsigned int i=0;
	unsigned int j=0;
  unsigned long ulresult=0;
  unsigned long uladdress=0;
  unsigned long errorcount=0;
    unsigned long totalercount=0;
  unsigned long modeerror[6]={0,0,0,0,0,0};
  char teststring[300];
  //  char teststring[]="UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU\r\n";
    char ateststring[]="MindMotion test Quad SPIkajlkjslkgalkfalksjflkasjflkasjflksafdjlkasdjflkjdsadjglkasdjglkajdsglkasjgdlkasjgdlkajfld;kajflajlfajfalsjfasfkja;lsfdjalkdsfjadslkfjlkasjflkasjflkasjf;lasdkj;fsajkdf;akjfl;dsafj;afja;dsjf;dsajf;ldsajf;adsfjlkdsajfdsalkfjlkdsafj\r\n";
  unsigned char ucRecvbuf[256];
////  uint32_t datanum;
//  //QSPI_ChipErase();
//    QSPI_WriteEnable();
//	//for(j=0;j<(4*SECTORSIZE/SECTORSIZE); j++) //
//  for(j=0;j<(W25Q32JVSIZE/SECTORSIZE); j++)
//  {
//      QSPI_SectorErase(j*SECTORSIZE);
//    		CheckStatus();
//  }
  for(j=0;j<(RXDATAMAXLENGTH); j++)//RXDATAMAXLENGTH:256
  {
    //teststring[j]='U';//0x55
    //teststring[j]=0xAA;//0xAA
    //teststring[j]=0xFA;//0xAA
    teststring[j]=ateststring[j];
    
  }
  ulresult = testchipEraseFunction();
  if(ulresult !=0)
  {
    printf("Chip Erase Error Error %03d!\r\n",ulresult);
  }

  for(j=0;j<(W25Q32JVSIZE/RXDATAMAXLENGTH); j++)//RXDATAMAXLENGTH:256
  {
    uladdress = j*RXDATAMAXLENGTH;
    testanyAddrAnyLengthByteswithbytesWrite(uladdress,(unsigned char *)teststring,RXDATAMAXLENGTH,1);  // 0  Single  //1 Quad Program
       ulresult = ReadanyAddrunderRXDATAMAXLENGTHwithbuffermode(uladdress, &ucRecvbuf[0], RXDATAMAXLENGTH,j%6);
//          ulresult = ReadanyAddrunderRXDATAMAXLENGTHwithbuffermode(uladdress, &ucRecvbuf[0], RXDATAMAXLENGTH,0);  //0   Single Read
//              ulresult = ReadanyAddrunderRXDATAMAXLENGTHwithbuffermode(uladdress, &ucRecvbuf[0], RXDATAMAXLENGTH,3);  //1   Single Fast  Read
//              ulresult = ReadanyAddrunderRXDATAMAXLENGTHwithbuffermode(uladdress, &ucRecvbuf[0], RXDATAMAXLENGTH,1);  //1   Dual Fast  
//              ulresult = ReadanyAddrunderRXDATAMAXLENGTHwithbuffermode(uladdress, &ucRecvbuf[0], RXDATAMAXLENGTH,4);  //4 Dual IO Read
//    ulresult = ReadanyAddrunderRXDATAMAXLENGTHwithbuffermode(uladdress, &ucRecvbuf[0], RXDATAMAXLENGTH,2);  // 2 Quad Read
//        ulresult = ReadanyAddrunderRXDATAMAXLENGTHwithbuffermode(uladdress, &ucRecvbuf[0], RXDATAMAXLENGTH,5);  // 5 Quad IO Read
    if(ulresult == 0){
      totalercount++;
    }
    else{
      errorcount = 0;
      for(i=0;i<RXDATAMAXLENGTH;i++)
      {
         if(ucRecvbuf[i]!=(teststring[i])) //if(ucRecvbuf[i]!=(teststring[i]|0x44)) 
         {
           errorcount++;
         }
      }
      if(errorcount>0){
        totalercount = totalercount + errorcount;
          printf("check page No %08d @mode %d, Error %03d!\r\n",j,j%6, errorcount);
        modeerror[j%6]=modeerror[j%6]+1;
      }
      else{
        printf("check page No %08d  @mode %d Successful!\r\n",j,j%6);
      }
    }
  }
  for(j=0;j<6; j++)
  {
    printf("Error Number = %08d @mode %d!\r\n",modeerror[j],j);
  }
  printf("totalercount = %08d  !\r\n",totalercount);
  return totalercount;
}
#define TESTLENGHT  8
unsigned long  runfullchipwriteread8bytestest(void)
{
	unsigned int i=0;
	unsigned int j=0;
  unsigned long ulresult=0;
  unsigned long uladdress=0;
  unsigned long errorcount=0;
    unsigned long totalercount=0;
   //char teststring[]="UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU\r\n";
     char teststring[]="MindMotion test Quad SPIkajlkjslkgalkfalksjflkasjflkasjflksafdjlkasdjflkjdsadjglkasdjglkajdsglkasjgdlkasjgdlkajfld;kajflajlfajfalsjfasfkja;lsfdjalkdsfjadslkfjlkasjflkasjflkasjf;lasdkj;fsajkdf;akjfl;dsafj;afja;dsjf;dsajf;ldsajf;adsfjlkdsajfdsalkfjlkdsafj\r\n";
  unsigned char ucRecvbuf[256];
////  uint32_t datanum;
//  //QSPI_ChipErase();
//    QSPI_WriteEnable();
//	//for(j=0;j<(4*SECTORSIZE/SECTORSIZE); j++) //
//  for(j=0;j<(W25Q32JVSIZE/SECTORSIZE); j++)
//  {
//      QSPI_SectorErase(j*SECTORSIZE);
//    		CheckStatus();
//  }
  ulresult = testchipEraseFunction();
  if(ulresult !=0)
  {
    printf("Chip Erase Error Error %03d!\r\n",ulresult);
  }

  for(j=0;j<(W25Q32JVSIZE/RXDATAMAXLENGTH); j++)//RXDATAMAXLENGTH:256
  {
    uladdress = j*RXDATAMAXLENGTH;
    testanyAddrAnyLengthByteswithbytesWrite(uladdress,(unsigned char *)teststring,TESTLENGHT,1);  // 0  Single  //1 Quad Program
    ulresult = ReadanyAddrunderRXDATAMAXLENGTHwithbuffermode(uladdress, &ucRecvbuf[0], TESTLENGHT,2);  //0   Single  //2 Quad Read
    if(ulresult == 0){
      totalercount++;
    }
    else{
      errorcount = 0;
      for(i=0;i<TESTLENGHT;i++)
      {
         if(ucRecvbuf[i]!=teststring[i]) 
         {
           errorcount++;
         }
      }
      if(errorcount>0){
        totalercount = totalercount + errorcount;
          printf("check page No %08d Error %03d!\r\n",j,errorcount);
      }
      else{
        printf("check page No %08d Successful!\r\n",j);
      }
    }
  }
  return totalercount;
}
unsigned long  runfullchipwriteread8bytestestmode2(void)
{
	unsigned long i=0;
	unsigned long j=0;
  unsigned long ulresult=0;
  unsigned long uladdress=0;
  unsigned long errorcount=0;
    unsigned long totalercount=0;
   //char teststring[]="UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU\r\n";
     char teststring[]="MindMotion test Quad SPIkajlkjslkgalkfalksjflkasjflkasjflksafdjlkasdjflkjdsadjglkasdjglkajdsglkasjgdlkasjgdlkajfld;kajflajlfajfalsjfasfkja;lsfdjalkdsfjadslkfjlkasjflkasjflkasjf;lasdkj;fsajkdf;akjfl;dsafj;afja;dsjf;dsajf;ldsajf;adsfjlkdsajfdsalkfjlkdsafj\r\n";
  unsigned char ucRecvbuf[256];
////  uint32_t datanum;
//  //QSPI_ChipErase();
//    QSPI_WriteEnable();
//	//for(j=0;j<(4*SECTORSIZE/SECTORSIZE); j++) //
//  for(j=0;j<(W25Q32JVSIZE/SECTORSIZE); j++)
//  {
//      QSPI_SectorErase(j*SECTORSIZE);
//    		CheckStatus();
//  }
  ulresult = testchipEraseFunction();
  if(ulresult !=0)
  {
    printf("Chip Erase Error Error %03d!\r\n",ulresult);
  }
  for(j=0;j<(100*TESTLENGHT/TESTLENGHT);j++)//for(j=0;j<(W25Q32JVSIZE/TESTLENGHT); j++)//RXDATAMAXLENGTH:256
  {
    uladdress = j*TESTLENGHT;
    testanyAddrAnyLengthByteswithbytesWrite(uladdress,(unsigned char *)teststring,TESTLENGHT,1); 
  }
  for(j=0;j<(100*TESTLENGHT/TESTLENGHT);j++)//for(j=0;j<(W25Q32JVSIZE/TESTLENGHT); j++)//RXDATAMAXLENGTH:256
  {
    uladdress = j*TESTLENGHT;
 //   testanyAddrAnyLengthByteswithbytesWrite(uladdress,(unsigned char *)teststring,TESTLENGHT,1);  // 0  Single  //1 Quad Program
    ulresult = ReadanyAddrunderRXDATAMAXLENGTHwithbuffermode(uladdress, &ucRecvbuf[0], TESTLENGHT,2);  //0   Single  //2 Quad Read
    if(ulresult == 0){
      totalercount++;
    }
    else{
      errorcount = 0;
      for(i=0;i<TESTLENGHT;i++)
      {
         if(ucRecvbuf[i]!=teststring[i]) 
         {
           errorcount++;
         }
      }
      if(errorcount>0){
        totalercount = totalercount + errorcount;
          printf("check page No %08d Error %03d!\r\n",j,errorcount);
      }
      else{
        printf("check page No %08d Successful!\r\n",j);
      }
    }
  }
  return totalercount;
}
void QSPI_InitasMode0(void)
{
    ClockInit();
    QSPI_Initwithmode(0);//0 error  1 ID OK //QSPI_Initwithmode(j);//0 error  1 ID OK
 
}
void QSPI_FullChipErase(void){
    QSPI_WriteEnable();
    CheckStatus();
    QSPI_ChipErase();
    CheckStatus();
}
#define BASEADDRESS 0x0000
unsigned long  ProgramBinfromSDRAMtoSPIFlash(unsigned char *puladdress, unsigned long ulLength)
{
    unsigned int i=0;
    unsigned int j=0;
    unsigned long ulresult=0;
    unsigned long uladdress=0;
    unsigned long errorcount=0;
    unsigned long totalercount=0;
    unsigned long modeerror[6]={0,0,0,0,0,0};
    unsigned char ucRecvbuf[256];
    char teststring[300];
    QSPI_GpioInit_Standard();
    QSPI_Reset();
    delay(5000);
    CheckStatus();
    writeOutputDriverStrength();
    delay(5000);
    CheckStatus();
    DisableProtect();
    delay(5000);
    CheckStatus();
    {
        QSPI_ReadID();
        QSPI_ReadManufacturer_Device_IDbyStandard();
    }

    QSPI_WriteEnable();
    QSPI_WriteStatusReg2(0x02);//WriteQEDisable();//QSPI_WriteStatusReg2(0x02);
    CheckStatus();

    QSPI_ReadManufacturer_Device_IDby_Dual_IO(0);
    QSPI_ReadManufacturer_Device_IDbyQuad_I_O(0);

    printf("Program Hex with Quad SPI !\r\n");
    ulresult = testchipEraseFunction();
    if(ulresult !=0)
    {
        printf("Chip Erase Error Error %03d!\r\n",ulresult);
    }
    //QSPI_FullChipErase();
    for(j=0;j<(ulLength/RXDATAMAXLENGTH+1); )//RXDATAMAXLENGTH:256
    {
        uladdress =  BASEADDRESS + j*RXDATAMAXLENGTH;
        for(i=0;i<RXDATAMAXLENGTH;i++)
        {
            teststring[i]=((unsigned char)( *(puladdress+j*RXDATAMAXLENGTH+i)));
        }
        testanyAddrAnyLengthByteswithbytesWrite(uladdress,(unsigned char *)teststring,RXDATAMAXLENGTH,0);  //j%2 0  Single  //1 Quad Program //&teststring[0]
        ulresult = ReadanyAddrunderRXDATAMAXLENGTHwithbuffermode(uladdress, &ucRecvbuf[0], RXDATAMAXLENGTH,0);//j%6
        if(ulresult == 0){
            totalercount++;
        }
        else{
            errorcount = 0;
            for(i=0;i<RXDATAMAXLENGTH;i++)
            {
                if(ucRecvbuf[i]!=teststring[i])//if(ucRecvbuf[i]!=(teststring[i]|0x44)) 
                {
                    errorcount++;
                }
            }
            if(errorcount>0){
            totalercount = totalercount + errorcount;
            printf("check page No %08d @mode %d, Error %03d!\r\n",j,j%6, errorcount);
            modeerror[j%6]=modeerror[j%6]+1;

        }
        else{
            j++;
            printf("check page No %08d  @mode %d Successful!\r\n",j,j%6);
            }
        }
    }
    for(j=0;j<6; j++)
    {
        printf("Error Number = %08d @mode %d!\r\n",modeerror[j],j);
    }
    printf("totalercount = %08d  !\r\n",totalercount);
    return totalercount;

}

unsigned long  ReadBinfromSPIFlashtoSDRAM(unsigned char *puladdress, unsigned long ulLength)
{
//  	unsigned int i=0;
	unsigned long i,j=0;
  unsigned long ulresult=0;
  unsigned long uladdress=0;
  unsigned char ucRecvbuf[256];

  for(j=0;j<(ulLength/RXDATAMAXLENGTH+1); j++)//RXDATAMAXLENGTH:256
  {
    uladdress = BASEADDRESS +  j*RXDATAMAXLENGTH;
    memset(ucRecvbuf,0,256);
    ulresult = ReadanyAddrunderRXDATAMAXLENGTHwithbuffermode(uladdress, &ucRecvbuf[0], RXDATAMAXLENGTH,0);
    for(i=0;i<RXDATAMAXLENGTH;i++)
    {
      *puladdress = ucRecvbuf[i];
      puladdress++;
    }
  }
  return ulresult;
}
unsigned long  Compare2blockSDRAMdata(unsigned char *puladdressA, unsigned char *puladdressB, unsigned long ulLength)
{
  unsigned int i=0;

 
  unsigned long errorcount=0;
 
  unsigned char ucA,ucB;
 

      for(i=0;i<ulLength;i++)
      {
        ucA=*puladdressA++;
        ucB=*puladdressB++;
         if(ucA !=ucB)
         {
           errorcount++;
         }
      }
      if(errorcount>0){
        printf("check page Error %03d!\r\n", errorcount);        
      }
      else{
        printf("check page  Successful!\r\n");
      }

  return errorcount;
  
}

int QSPItest(void)
{
  #if 0
    QSPI_ReadID();
  #endif
//    QSPI_ReadManufacturer_Device_IDbyStandard();
  
    /*QSPI_WriteEnable();
    QSPI_WriteStatusReg2(0x02);//WriteQEDisable();//QSPI_WriteStatusReg2(0x02);
    CheckStatus(); taoming 2017.06.29*/

//    QSPI_ReadManufacturer_Device_IDby_Dual_IO(0);
//    QSPI_ReadManufacturer_Device_IDbyQuad_I_O(0);

  return 0;
}
/********************************************************************************************************
**函数信息 ：ProgramQspiFlash(u32 qSpiFlashAdd, u8 *inData, u32 len)               
**功能描述 ：
**输入参数 ：
**输出参数 ：无
********************************************************************************************************/
void QspiFlashProgram(u32 qSpiFlashAdd, u8 *inData, u32 len)
{
    u32 writePage = ((len+255)/RXDATAMAXLENGTH);

    unsigned int j=0;
    unsigned int uladdress=0;
    
    for(j=0;j< writePage ; j++)//RXDATAMAXLENGTH:256
    {
        uladdress = qSpiFlashAdd + j*RXDATAMAXLENGTH;
//        testanyAddrAnyLengthByteswithbytesWrite(uladdress,(unsigned char *)(inData + j*RXDATAMAXLENGTH),RXDATAMAXLENGTH,1);  //j%2 0  Single  //1 Quad Program //&teststring[0]
        QSPI_PageXbytesProgram_Standard_NoCheck(uladdress,(unsigned char *)(inData + j*RXDATAMAXLENGTH),RXDATAMAXLENGTH);
    }
}
/********************************************************************************************************
**函数信息 ：ProgramQspiFlash(u32 qSpiFlashAdd, u8 *inData, u32 len)               
**功能描述 ：
**输入参数 ：
**输出参数 ：无
********************************************************************************************************/
void QspiFlashRead(u32 qspiAdd, u8 *puladdress, u32 len)
{
//  	unsigned int i=0;
    u32 writePage = ((len+255)/RXDATAMAXLENGTH);
    unsigned long j=0;
    unsigned long uladdress=0;

    for(j=0;j<writePage; j++)//RXDATAMAXLENGTH:256
    {
//      printf("uladdress = %8x\r\n",uladdress);
//			printf("writePage = %8x\r\n",writePage);
        uladdress = qspiAdd +  j*RXDATAMAXLENGTH;
//        printf("cnt = %x\r\n",j);
//        printf("uladd = %x\r\n",uladdress);

				#if QSPI_QUAL_DEBUG == 0
        ReadanyAddrunderRXDATAMAXLENGTHwithbuffermode(uladdress, (u8*)(puladdress + j*RXDATAMAXLENGTH), RXDATAMAXLENGTH,0);
				#else
        ReadanyAddrunderRXDATAMAXLENGTHwithbuffermode(uladdress, (u8*)(puladdress + j*RXDATAMAXLENGTH), RXDATAMAXLENGTH,5);
				#endif
    }
}


/********************************************************************************************************
**函数信息 ：void QspiFlashSectorErase(u32 add)            
**功能描述 ：扇区擦除(4K)
**输入参数 ：
**输出参数 ：无
********************************************************************************************************/
void QspiFlashSectorErase(u32 add)
{
    QSPI_WriteEnable();

    QSPI_SectorErase(add);
    CheckStatus();
}

/*
Instruction Set Table 1 (Standard SPI Instructions)
Instruction Set Table 1 (Standard SPI Instructions)

Data Input Output		Byte 1	Byte 2	Byte 3	Byte 4	Byte 5	Byte 6	Byte 7
Number of Clock(1-1-1)		8	8	8	8	8	8	8
Write Enable			06h	
Volatile SR Write Enable	50h	
Write Disable			04h	
Release Power-down / ID		ABh	Dummy	Dummy	Dummy	(ID7-ID0)(2)	
Manufacturer/Device ID		90h	Dummy	Dummy	00h	(MF7-MF0)	(ID7-ID0)	
JEDEC ID			9Fh	(MF7-MF0)	(ID15-ID8)	(ID7-ID0)	
Read Unique ID			4Bh	Dummy	Dummy	Dummy	Dummy	(UID63-0)	
Read Data			03h	A23-A16	A15-A8	A7-A0	(D7-D0)	
Fast Read			0Bh	A23-A16	A15-A8	A7-A0	Dummy	(D7-D0)	
Page Program			02h	A23-A16	A15-A8	A7-A0	D7-D0	D7-D0(3)	
Sector Erase (4KB)		20h	A23-A16	A15-A8	A7-A0	
Block Erase (32KB)		52h	A23-A16	A15-A8	A7-A0	
Block Erase (64KB)		D8h	A23-A16	A15-A8	A7-A0	
Chip Erase			C7h/60h	
Read Status Register-1		05h	(S7-S0)(2)	
Write Status Register-1(4)	01h	(S7-S0)(4)	
Read Status Register-2		35h	(S15-S8)(2)	
Write Status Register-2		31h	(S15-S8)	
Read Status Register-3		15h	(S23-S16)(2)	
Write Status Register-3		11h	(S23-S16)	
Read SFDP Register		5Ah	A23-A16	A15-A8	A7–A0	dummy	(D7-0)	
Erase Security Register(5)	44h	A23-A16	A15-A8	A7-A0	
Program Security Register(5)	42h	A23-A16	A15-A8	A7-A0	D7-D0	D7-D0(3)	
Read Security Register(5)	48h	A23-A16	A15-A8	A7-A0	Dummy	(D7-D0)	
Global Block Lock		7Eh	
Global Block Unlock		98h	
Read Block Lock			3Dh	A23-A16	A15-A8	A7-A0	(L7-L0)	
Individual Block Lock		36h	A23-A16	A15-A8	A7-A0	
Individual Block Unlock		39h	A23-A16	A15-A8	A7-A0	
Erase / Program Suspend		75h	
Erase / Program Resume		7Ah	
Power-down			B9h	
Enter QPI Mode			38h	
Enable Reset			66h	
Reset Device			99h

Instruction Set Table 2 (Dual/Quad SPI Instructions)(1)

Data Input Output		Byte 1	Byte 2	Byte 3	Byte 4	Byte 5	Byte 6	Byte 7	Byte 8	Byte 9
Number of Clock(1-1-2)		8	8	8	8	4	4	4	4	4
Fast Read Dual Output		3Bh	A23-A16	A15-A8	A7-A0	Dummy	Dummy	(D7-D0)(7)	…
Number of Clock(1-2-2)		8	4	4	4	4	4	4	4	4
Fast Read Dual I/O		BBh	A23-A16(6)	A15-A8(6)	A7-A0(6)	M7-M0	(D7-D0)(7)	…
Mftr./Device ID Dual I/O	92h	A23-A16(6)	A15-A8(6)	00(6)	Dummy(14)	(MF7-MF0)	(ID7-ID0)(7)	
Number of Clock(1-1-4)		8	8	8	8	2	2	2	2	2
Quad Input Page Program		32h	A23-A16	A15-A8	A7-A0	(D7-D0)(9)	(D7-D0)(3)	…
Fast Read Quad Output		6Bh	A23-A16	A15-A8	A7-A0	Dummy	Dummy	Dummy	Dummy	(D7-D0)(10)
Number of Clock(1-4-4)		8	2(8)	2(8)	2(8)	2	2	2	2	2
Mftr./Device ID Quad I/O	94h	A23-A16	A15-A8	00	Dummy(14)	Dummy	Dummy	(MF7-MF0)	(ID7-ID0)
Fast Read Quad I/O		EBh	A23-A16	A15-A8	A7-A0	M7-M0	Dummy	Dummy	(D7-D0)	…
Set Burst with Wrap		77h	Dummy	Dummy	Dummy	W7-W0	
*/

void qspiRecDMA_Config(u32 adress,u32 len)
{
	RCC->AHB1ENR |= 1<<22;
	printf("adress = %8x\r\n",adress);
	printf("len = %8x\r\n",len);
	DMA2_Channel4->CCR = 0;
	DMA2_Channel4->CNDTR = 0;
	
	DMA2_Channel4->CCR = 0x280;
	DMA2_Channel4->CPAR = QSPI_BASE+0x4;
	DMA2_Channel4->CMAR = adress;
	DMA2_Channel4->CNDTR = len;
	DMA2_Channel4->CCR |= 1;
	printf("DMA1_Channel4->CCR = %8x\r\n",DMA2_Channel4->CCR);
	printf("DMA1_Channel4->CPAR = %8x\r\n",DMA2_Channel4->CPAR);
	printf("DMA1_Channel4->CMAR = %8x\r\n",DMA2_Channel4->CMAR);
	printf("DMA1_Channel4->CNDTR = %8x\r\n",DMA2_Channel4->CNDTR);
	QSPI->GCTL |= 1<<9;
	
	QSPI_GCTL |= RXEN;
	while((DMA2->ISR & 1<<13) == 0 )
	{
//		printf("DMA1_Channel4->CNDTR = %8x\r\n",DMA2_Channel4->CNDTR);
	}
	while(len--)
	{
		printf("%2x ",*(u8*)adress++);
	}
	printf("dma\r\n");
	DMA2->IFCR = 1<<13;
}
