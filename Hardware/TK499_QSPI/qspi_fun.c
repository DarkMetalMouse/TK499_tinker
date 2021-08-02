#include "qspi_fun.h"


void QSPI_FunCnofig(u8 mode)//config
{
  RCC->APB2ENR |= 0x1<<24;//QSPI enable

  RCC->AHB1ENR |= 0x8;//GPIOD enable;

  GPIOD->AFRH &= 0xf000000f;
  GPIOD->AFRH |= 0x06666660;//AF6

  GPIOD->CRH &= 0xf000000f;
  GPIOD->CRH |= 0x0bbbbbb0;
  
	QSPI->GCTL = 0x1C;
	QSPI->CCTL = 0x19;//mode0
	QSPI->SPBRG = 3;  //速度
	QSPI->SCSR = 0;
  QSPI->RXDNR = 1;
	if(mode == 2)
	{
		QSPI->MODE = 0xD;//
	}
	else if(mode == 4)
	{
		QSPI->MODE = 0xE;//
	}
	else
	{
		QSPI->MODE = 0xC;//
	}
  QSPI->GCTL |= 1;
  QSPI_FunRW(0);
}

void QSPI_FunLine(u8 mode)//config
{
	if(mode == 2)
	{
		QSPI->MODE = 0xD;//
	}
	else if(mode == 4)
	{
		QSPI->MODE = 0xE;//
	}
	else
	{
		QSPI->MODE = 0xC;//
	}
}


void QSPI_FunRW(u8 rw)//send 8bit clk at switch to received
{
  QSPI->GCTL &= ~0x18;
  if(rw == 0)
  { 
    QSPI->GCTL |= 1<<3;
  }
  else
  {
    QSPI->GCTL |= 1<<4;//switch to received
  }
}

void QSPI_FunSendbyte(u8 data)//byte send
{
	QSPI->TXREG = data;
  while(!(QSPI->CSTAT & 0x1));
}

u8 QSPI_FunRecbyte(void)//byte send
{
  QSPI->RXDNR = 1;
	QSPI_FunRW(1);
  while(!(QSPI->CSTAT & 0x2));
  return QSPI->RXREG ;
}

void QSPI_FunSendGroup(u8* pData,u32 len)//group send
{
  #if 0
	while(len--)
  {
    QSPI_FunSendbyte(*pData++);
  }
  #else
  while(len --)
  {
    QSPI->TXREG = *pData++;
    while(!(QSPI->CSTAT & 0x1));
  }
  
  #endif
}

void QSPI_FunNSS(u8 cs)//qspi chip select
{
  if(cs == 0)
  {
    QSPI->SCSR = 0;
  }
  else
  {
    QSPI->SCSR = 1;
  }
}

void QSPI_FunDmaRxConfig(u32 adress,u32 len)//qspi rx dma
{

	QSPI_FunRW(1);//send 8bit clk at switch to received
	DMA2_Channel4->CCR = 0;//RX
	DMA2_Channel4->CNDTR = 0;
	DMA2_Channel4->CCR = 0x280;
	DMA2_Channel4->CPAR = (u32)&(QSPI->RXREG);
	DMA2_Channel4->CMAR = adress;

	QSPI->RXDNR = len;
	DMA2_Channel4->CNDTR = len;  
	DMA2_Channel4->CCR |= 1;  
	QSPI->GCTL |= 1<<9;

	while((DMA2->ISR & 1<<13) == 0 );
	DMA2->IFCR = 1<<13;
	QSPI->GCTL &= ~(1<<9);

	QSPI_FunRW(0);
}

void QSPI_FunDmaTxConfig(u32 adress,u32 len)//qspi tx dma
{
  QSPI_FunRW(0);
  DMA2_Channel5->CCR = 0;//TX
	DMA2_Channel5->CNDTR = 0;
	DMA2_Channel5->CCR = 0x290;//mem add disable
	DMA2_Channel5->CPAR = (u32)&(QSPI->TXREG);
	DMA2_Channel5->CMAR = adress;
	DMA2_Channel5->CNDTR = len;
	DMA2_Channel5->CCR |= 1;
  
	QSPI->GCTL |= 1<<9;
  #if 1
	while((DMA2->ISR & 1<<17) == 0 );
	DMA2->IFCR = 1<<17;
  QSPI->GCTL &= ~(1<<9);
  while(!(QSPI->CSTAT & 0x1));
  #endif
}

void QFLASH_FunReadGroup(u32 add,u8 *pBuf,u32 len)
{
  u32 qFlashMaxLen = 0x0FFFF;
  QSPI_FunRW(0);
	
  while(len)
  {
    QSPI_FunNSS(0); 
    #if (QSPI_QUAL_READ == 0)
    QSPI_FunSendbyte(RD_Com);
    QSPI_FunSendbyte(add>>16);
    QSPI_FunSendbyte(add>>8);
    QSPI_FunSendbyte(add);
    #else
    QSPI_FunSendbyte(FastRD_QuadIo_Com);
    QSPI_FunLine(4);
    QSPI_FunSendbyte(addr0);
    QSPI_FunSendbyte(addr1);
    QSPI_FunSendbyte(addr2);
    QSPI_FunSendbyte(0xF0);//M7-0
    QSPI_FunSendbyte(0);
    QSPI_FunSendbyte(0);
    #endif
    if(len > qFlashMaxLen)
    {
      QSPI_FunDmaRxConfig((u32) pBuf, qFlashMaxLen);
      pBuf += qFlashMaxLen;
      len -= qFlashMaxLen;
    }
    else
    {
      QSPI_FunDmaRxConfig((u32) pBuf, len);
      len = 0;
    }
    
    QSPI_FunNSS(1); 
    #if (QSPI_QUAL_READ != 0)
    QSPI_FunLine(1);
    #endif
    add += qFlashMaxLen;
  }    
  QSPI_FunRW(0);
}
void QFLASH_FunProgram(u32 add,u8*pBuf,u32 len)
{
  u8 addr0,addr1,addr2;
  u32 qFlashPage = 0x0100;
  while(len)
  {
    add = add&0x00ffffff; //page address
    addr0 = (add>>16)&0xff;
    addr1 = (add>>8)&0xff;
    addr2 = (add)&0xff;
    QSPI_FunRW(0);
  
    QFLASH_FunWriteCmd(WREN_Com);
 
    QSPI_FunNSS(0); 
    QSPI_FunSendbyte(PP_Com);
    QSPI_FunSendbyte(addr0);
    QSPI_FunSendbyte(addr1);
    QSPI_FunSendbyte(addr2);
    if(len > qFlashPage)
    {
      QSPI_FunDmaTxConfig((u32) pBuf,qFlashPage);
      pBuf += qFlashPage;
      len -= qFlashPage;
    }
    else
    {
      QSPI_FunDmaTxConfig((u32) pBuf,len);
      len = 0;
    }
    add += qFlashPage;
    QSPI_FunNSS(1); 
    QFLASH_FunWriteCmd(WRDI_Com); 
    QFLASH_FunCheckStatus();
  }
  
}
void QFLASH_FunErase(u32 add,u8 eType)//eType 0:SE_Com (Erase 4KB), 1:BE64K_Com
{
  unsigned char addr0,addr1,addr2;
  add = add&0x00ffffff; 
  addr0 = (add>>16)&0xff;
  addr1 = (add>>8)&0xff;
  addr2 = (add)&0xff;

  QSPI_FunRW(0);

  QFLASH_FunWriteCmd(WREN_Com);// Write Enable

  QSPI_FunNSS(0);
  
  if(eType == 0)
  {
    QSPI_FunSendbyte(SE_Com);//Erase 4KB
  }
  else
  {
    QSPI_FunSendbyte(BE64K_Com);//Erase 64KB
  }
  
  QSPI_FunSendbyte(addr0);
  QSPI_FunSendbyte(addr1);
  QSPI_FunSendbyte(addr2);

  QSPI_FunNSS(1);
  QFLASH_FunWriteCmd(WRDI_Com); 
  QFLASH_FunCheckStatus();
}

void QFLASH_FunEraseAll(void)
{
  QSPI_FunRW(0);
  QFLASH_FunWriteCmd(WREN_Com);
  QFLASH_FunWriteCmd(CE_Com); 
  QFLASH_FunCheckStatus();
}

void QFLASH_FunReadId(u8* id)
{
  
}

void QFLASH_FunWriteCmd(u8 cmd)
{
  QSPI_FunNSS(0);
  QSPI_FunSendbyte( cmd);
  QSPI_FunNSS(1);
}
void QFLASH_FunCheckStatus(void)
{
  u8 tempData;
  while(1)
  {
    QSPI_FunRW(0);
    QSPI_FunNSS(0);
    QSPI_FunSendbyte( RDSR_Com);
    tempData = QSPI_FunRecbyte();
    QSPI_FunNSS(1);
    if((tempData & 0x1) == 0)
    {
      QSPI_FunRW(0);
      break;
    }
  }
}


//获取文件地址函数：输入参数*s为文件名；返回该文件的地址
int get_file_address_NOR_FLASH(char *s)
{
	u8 temp[4096];
	int i,j,k;
	QFLASH_FunReadGroup(0xF000, temp, 4096);//读取文件头
	
	for(i=1;i<=temp[6];i++)
	{
		if((temp[16*i]==*s)||(temp[16*i]==*s-32))//不区分大小写字母
		{
			for(j=0;j<13;j++)
			{
				if(temp[16*i+j]==0)
				{
					k = (temp[16*i+13]<<16)+(temp[16*i+14]<<8)+temp[16*i+15];//拼接出地址放到K中
					return k;//返回该文件地址
				}
				if((temp[16*i+j]!=*(s+j))&&(temp[16*i+j]!=(*(s+j)-32))) break;
			}	
		}	
	}	
	return 0;
}





