#include "sdio_sdcard.h"
#include "string.h"  
 
#include "sdio.h"
extern char prinfBuf[];
extern char printBuf[];
extern void UartSendGroup(u8* buf,u16 len);
 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//SDIO ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/1/6
//�汾��V1.1
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved		 
//********************************************************************************
//����˵��
//V1.1 20160528
//�޸�SD_WriteDisk��SD_ReadDiskһ�ζ�д����������255�����ִ����bug.
////////////////////////////////////////////////////////////////////////////////// 	 
 
static u8 CardType=SDIO_STD_CAPACITY_SD_CARD_V1_1;		//SD�����ͣ�Ĭ��Ϊ1.x����
static u32 CSD_Tab[4],CID_Tab[4],RCA=0;					//SD��CSD,CID�Լ���Ե�ַ(RCA)����
static u8 DeviceMode=SD_DMA_MODE;		   				//����ģʽ,ע��,����ģʽ����ͨ��SD_SetDeviceMode,�������.����ֻ�Ƕ���һ��Ĭ�ϵ�ģʽ(SD_DMA_MODE)
static u8 StopCondition=0; 								//�Ƿ���ֹͣ�����־λ,DMA����д��ʱ���õ�  
volatile SD_Error TransferError=SD_OK;					//���ݴ�������־,DMA��дʱʹ��	    
volatile u8 TransferEnd=0;								//���������־,DMA��дʱʹ��
SD_CardInfo SDCardInfo;									//SD����Ϣ

//SD_ReadDisk/SD_WriteDisk����ר��buf,�����������������ݻ�������ַ����4�ֽڶ����ʱ��,
//��Ҫ�õ�������,ȷ�����ݻ�������ַ��4�ֽڶ����.
__align(4) u8 SDIO_DATA_BUFFER[512];						  
 

////�ر������ж�(���ǲ�����fault��NMI�ж�)
//void INTX_DISABLE(void)
//{
//	__ASM volatile("cpsid i");
//}
////���������ж�
//void INTX_ENABLE(void)
//{
//	__ASM volatile("cpsie i");		  
//}

//��ʼ��SD��
//����ֵ:�������;(0,�޴���)
SD_Error SD_Init(void)
{
	SD_Error errorstatus=SD_OK;	  
	u8 clkdiv=0;
	//SDIO IO�ڳ�ʼ��   	 
	  
// 	MY_NVIC_Init(0,0,SDIO_IRQn,2);		//SDIO�ж�����
	errorstatus = SD_PowerON();			//SD���ϵ�
	errorstatus = SD_PowerON();			//SD���ϵ�
	if(errorstatus==SD_OK)
		errorstatus=SD_InitializeCards();			//��ʼ��SD��				
	if(errorstatus==SD_OK)
		errorstatus=SD_GetCardInfo(&SDCardInfo);	//��ȡ����Ϣ		
	if(errorstatus==SD_OK)
		errorstatus=SD_SelectDeselect((u32)(SDCardInfo.RCA<<16));//ѡ��SD��  }
//	if(errorstatus==SD_OK)
//		errorstatus=SD_EnableWideBusOperation(1);	//����Ϊ1��4λ���,����Ϊ0��Ϊ1λ��ȣ������MMC��,������4λģʽ 
	
	SDIO_Clock_Set(0);	//����ʱ��Ƶ��,SDIOʱ�Ӽ��㹫ʽ:SDIO_CKʱ��=SDIOCLK/[clkdiv+2];����,SDIOCLK�̶�Ϊ48Mhz
	if((errorstatus==SD_OK)||(SDIO_MULTIMEDIA_CARD==CardType))
	{  		    
		if(SDCardInfo.CardType==SDIO_STD_CAPACITY_SD_CARD_V1_1||SDCardInfo.CardType==SDIO_STD_CAPACITY_SD_CARD_V2_0)
		{
//			clkdiv=SDIO_TRANSFER_CLK_DIV+2;	//V1.1/V2.0�����������48/4=12Mhz
		}
    else 
		{
//			clkdiv=SDIO_TRANSFER_CLK_DIV;	//SDHC�����������������48/2=24Mhz
			TEST_SDIOx->MMC_IO_MBCTL |= 0x8;//ʱ���½���push��������pull
			TEST_SDIOx->MMC_CTRL |= 0x1<<6;
		}
		//errorstatus=SD_SetDeviceMode(SD_DMA_MODE);	//����ΪDMAģʽ
		errorstatus=SD_SetDeviceMode(SD_POLLING_MODE);//����Ϊ��ѯģʽ
 	}
	return errorstatus;		 
}
//SDIOʱ�ӳ�ʼ������
//clkdiv:ʱ�ӷ�Ƶϵ��
//CKʱ��=SDIOCLK/[clkdiv+2];(SDIOCLKʱ�ӹ̶�Ϊ48Mhz)
void SDIO_Clock_Set(u8 clkdiv)
{
	u32 tmpreg=TEST_SDIOx->MMC_CTRL; 
  tmpreg&=0XFFFFFFC7; 
 	tmpreg|=clkdiv<<3; 
//	tmpreg|=3<<6;  
	TEST_SDIOx->MMC_CTRL=tmpreg;
//  TEST_SDIOx->MMC_CTRL |= 0x1<<6;//20191107�Ķ������� 
} 
//SDIO���������
//cmdindex:��������,����λ��Ч
//waitrsp:�ڴ�����Ӧ.00/10,����Ӧ;01,����Ӧ;11,����Ӧ
//arg:����

void SDIO_Send_Cmd(u8 cmdindex,u8 waitrsp,u32 arg)
{			
  int i;
//  int n;
//	u32 tmpreg;
//	TEST_SDIOx->ARG=arg;
//	tmpreg=TEST_SDIOx->CMD; 
//	tmpreg&=0XFFFFF800;		//���index��waitrsp
//	tmpreg|=cmdindex&0X3F;	//�����µ�index			 
//	tmpreg|=waitrsp<<6;		//�����µ�wait rsp 
//	tmpreg|=0<<8;			//�޵ȴ�
//  	tmpreg|=1<<10;			//����ͨ��״̬��ʹ��
//	TEST_SDIOx->CMD=tmpreg;

  TEST_SDIOx->CMD_BUF[4] = 0x40 | cmdindex;  
  TEST_SDIOx->CMD_BUF[3] = ((arg&0xff000000)>>24);
  TEST_SDIOx->CMD_BUF[2] = ((arg&0xff0000)>>16);
  TEST_SDIOx->CMD_BUF[1] = ((arg&0xff00)>>8);
  TEST_SDIOx->CMD_BUF[0] = (arg&0xff);
	TEST_SDIOx->CLR_MMC_INT |= 0;
  TEST_SDIOx->MMC_IO = 0x04;  //2019 auto only command transfer TEST_SDIOx->MMC_IO |= 0x04;
  while(!(TEST_SDIOx->CLR_MMC_INT & 0x1));

	if(TEST_SDIOx->CLR_MMC_INT & 0x1)
		TEST_SDIOx->CLR_MMC_INT |= 0x1;   //write 1 clear interrup
  
  if(waitrsp == 0x1)
  {
//    n = 1000;while(n--);
    TEST_SDIOx->MMC_IO = 0x8c;//2019 TEST_SDIOx->MMC_IO = 0x0c;
//    while(!(TEST_SDIOx->CLR_MMC_INT & 0x1));
//    TEST_SDIOx->CLR_MMC_INT = 0x1;   //write 1 clear interrup
  }
  else if(waitrsp == 0x3)
  {
    TEST_SDIOx->MMC_IO = 0x9c;//2019 TEST_SDIOx->MMC_IO = 0x1c;
//    while(!(TEST_SDIOx->CLR_MMC_INT & 0x1));
//    TEST_SDIOx->CLR_MMC_INT = 0x1;   //write 1 clear interrup
  }

  i = 100;while(i--);
//  TEST_SDIOx->MMC_IO = 0x20;
//  while(!(TEST_SDIOx->CLR_MMC_INT & 0x1));
//  TEST_SDIOx->CLR_MMC_INT = 0x1;   //write 1 clear interrup

}
//SDIO�����������ú���
//datatimeout:��ʱʱ������
//datalen:�������ݳ���,��25λ��Ч,����Ϊ���С��������
//blksize:���С.ʵ�ʴ�СΪ:2^blksize�ֽ�
//dir:���ݴ��䷽��:0,����������;1,����������;
void SDIO_Send_Data_Cfg(u32 datatimeout,u32 datalen,u8 blksize,u8 dir)
{
	u32 tmpreg,tmpreg1,tmpreg2;
//	TEST_SDIOx->DTIMER=datatimeout;
//  TEST_SDIOx->DLEN=datalen&0X1FFFFFF;	//��25λ��Ч
//	tmpreg=TEST_SDIOx->DCTRL; 
//	tmpreg&=0xFFFFFF08;		//���֮ǰ������.
//	tmpreg|=blksize<<4;		//���ÿ��С
//	tmpreg|=0<<2;			//�����ݴ���
//	tmpreg|=(dir&0X01)<<1;	//�������
//	tmpreg|=1<<0;			//���ݴ���ʹ��,DPSM״̬��
//	TEST_SDIOx->DCTRL=tmpreg;		
  tmpreg = TEST_SDIOx->MMC_IO_MBCTL;
  tmpreg1 = TEST_SDIOx->MMC_IO;
  tmpreg &= ~((0x3<<4)|(03<<0));
  if(datatimeout<100)
  {
    TEST_SDIOx->MMC_TIMEOUTCNT = datatimeout;
    tmpreg |= 0x0<<4;
  }
  else if(datatimeout<10000)
  {
    TEST_SDIOx->MMC_TIMEOUTCNT = datatimeout/100;
    tmpreg |= 0x1<<4;
  }
  else if(datatimeout<1000000)
  {
    TEST_SDIOx->MMC_TIMEOUTCNT = datatimeout/10000;
    tmpreg |= 0x2<<4;
  }
  else
  {
    TEST_SDIOx->MMC_TIMEOUTCNT = datatimeout/1000000;
    tmpreg |= 0x3<<4;
  }
  TEST_SDIOx->MMC_BYTECNTL = datalen&0x1FFFFFF;	;
  TEST_SDIOx->MMC_BLOCKCNT = blksize;
  if(dir == 0)
  {
    tmpreg |= 1<<1;
    tmpreg1 |= 1<<1;
    tmpreg2 |= 1<<11;
  }
  else
  { 
    tmpreg &= ~(1<<1);
    tmpreg1 &= ~(1<<1);
    tmpreg2 &= ~(1<<11);
  }

  TEST_SDIOx->MMC_IO_MBCTL = tmpreg;
  
  TEST_SDIOx->MMC_IO = tmpreg1;  
  TEST_SDIOx->BUF_CTL = tmpreg2;
//  TEST_SDIOx->BUF_CTL |= 0x20<<2;
//  TEST_SDIOx->MMC_IO          |= 0x1;
}  

//���ϵ�
//��ѯ����SDIO�ӿ��ϵĿ��豸,����ѯ���ѹ������ʱ��
//����ֵ:�������;(0,�޴���)
SD_Error SD_PowerON(void)
{
 	u8 i=0;
	SD_Error errorstatus=SD_OK;
	u32 response=0,count=0,validvoltage=0;
	u32 SDType=SD_STD_CAPACITY;
	//����CLKCR�Ĵ��� 
//	TEST_SDIOx->CLKCR=0;				//���CLKCR֮ǰ������
//	TEST_SDIOx->CLKCR|=0<<9;			//��ʡ��ģʽ
//	TEST_SDIOx->CLKCR|=0<<10;			//�ر���·,CK���ݷ�Ƶ�������
//	TEST_SDIOx->CLKCR|=0<<11;			//1λ���ݿ��
//	TEST_SDIOx->CLKCR|=0<<13;			//SDIOCLK�����ز���SDIOCK
//	TEST_SDIOx->CLKCR|=0<<14;			//�ر�Ӳ��������    
//	SDIO_Clock_Set(SDIO_INIT_CLK_DIV);//����ʱ��Ƶ��(��ʼ����ʱ��,���ܳ���400Khz)			 
// 	TEST_SDIOx->POWER=0X03;			//�ϵ�״̬,������ʱ��    
//  	TEST_SDIOx->CLKCR|=1<<8;			//SDIOCKʹ�� 
//SDIO_Clock_Set(7); 
  for(i=0;i<74;i++)
	{
		SDIO_Send_Cmd(SD_CMD_GO_IDLE_STATE,0,0);//����CMD0����IDLE STAGEģʽ����.												  
		errorstatus=CmdError();
		if(errorstatus==SD_OK)break;
 	}
 	if(errorstatus)return errorstatus;//���ش���״̬
//	i = 1000;while(i--);
	SDIO_Send_Cmd(SDIO_SEND_IF_COND,1,SD_CHECK_PATTERN);//����CMD8,����Ӧ,���SD���ӿ�����.
 														//arg[11:8]:01,֧�ֵ�ѹ��Χ,2.7~3.6V
														//arg[7:0]:Ĭ��0XAA
														//������Ӧ7
  	errorstatus=CmdResp7Error();						//�ȴ�R7��Ӧ
 	if(errorstatus==SD_OK) 								//R7��Ӧ����
	{
		CardType=SDIO_STD_CAPACITY_SD_CARD_V2_0;		//SD 2.0��
		SDType=SD_HIGH_CAPACITY;			   			//��������
	}
	SDIO_Send_Cmd(SD_CMD_APP_CMD,1,0);					//����CMD55,����Ӧ	 
	errorstatus=CmdResp1Error(SD_CMD_APP_CMD); 		 	//�ȴ�R1��Ӧ
	SDIO_Send_Cmd(SD_CMD_APP_CMD,1,0);					//����CMD55,����Ӧ	 
	errorstatus=CmdResp1Error(SD_CMD_APP_CMD); 		 	//�ȴ�R1��Ӧ
	if(errorstatus==SD_OK)//SD2.0/SD 1.1,����ΪMMC��
	{																  
		//SD��,����ACMD41 SD_APP_OP_COND,����Ϊ:0x8010000SD_MAX_VOLT_TRIAL0 
		while((!validvoltage)&&(count<3000))   /*****�������� count<SD_MAX_VOLT_TRIAL*****/
		{							
			SDIO_Send_Cmd(SD_CMD_APP_CMD,1,0);				//����CMD55,����Ӧ	 
			errorstatus=CmdResp1Error(SD_CMD_APP_CMD); 	 	//�ȴ�R1��Ӧ   
 			if(errorstatus!=SD_OK)return errorstatus;   	//��Ӧ����
			SDIO_Send_Cmd(SD_CMD_SD_APP_OP_COND,1,SD_VOLTAGE_WINDOW_SD|SDType);//����ACMD41,����Ӧ	 
			errorstatus=CmdResp3Error(); 					//�ȴ�R3��Ӧ   
 			if(errorstatus!=SD_OK)return errorstatus;   	//��Ӧ����  
//			response=TEST_SDIOx->RESP1;			 //�õ���Ӧ
      response=TEST_SDIOx->CMD_BUF[3]<<24  |TEST_SDIOx->CMD_BUF[2]<<16  |TEST_SDIOx->CMD_BUF[1]<<8  | TEST_SDIOx->CMD_BUF[0];	
			validvoltage=(((response>>31)==1)?1:0);			//�ж�SD���ϵ��Ƿ����
			
//			response=TEST_SDIOx->CMD_BUF[3];
//			validvoltage=(((response&0x80)>0)?1:0);
			count++;
		}
		if(count>=SD_MAX_VOLT_TRIAL)
		{
			errorstatus=SD_INVALID_VOLTRANGE;
			return errorstatus;
		}
		if(response&=SD_HIGH_CAPACITY)
		{
			CardType=SDIO_HIGH_CAPACITY_SD_CARD;
		}
 	}
	else//MMC��
	{
		//MMC��,����CMD1 SDIO_SEND_OP_COND,����Ϊ:0x80FF8000 
		while((!validvoltage)&&(count<3000))//SD_MAX_VOLT_TRIAL
		{	   										   				   
			SDIO_Send_Cmd(SD_CMD_SEND_OP_COND,1,SD_VOLTAGE_WINDOW_MMC);//����CMD1,����Ӧ	 
			errorstatus=CmdResp3Error(); 					//�ȴ�R3��Ӧ   
 			if(errorstatus!=SD_OK)return errorstatus;   	//��Ӧ����  
			response=TEST_SDIOx->CMD_BUF[3]<<24  |TEST_SDIOx->CMD_BUF[2]<<16  |TEST_SDIOx->CMD_BUF[1]<<8  | TEST_SDIOx->CMD_BUF[0];			   				//�õ���Ӧ
			validvoltage=(((response>>31)==1)?1:0);
			count++;
		}
		if(count>=SD_MAX_VOLT_TRIAL)
		{
			errorstatus=SD_INVALID_VOLTRANGE;
			return errorstatus;
		}	 			    
		CardType=SDIO_MULTIMEDIA_CARD;	  
  	}  
	
  	return(errorstatus);		
}
//SD�� Power OFF
//����ֵ:�������;(0,�޴���)
SD_Error SD_PowerOFF(void)
{
//  	TEST_SDIOx->POWER&=~(3<<0);//SDIO��Դ�ر�,ʱ��ֹͣ	
  TEST_SDIOx->MMC_CARDSEL &= ~(0x3<<6);//SDIO��Դ�ر�,ʱ��ֹͣ	
	return SD_OK;		  
}   
//��ʼ�����еĿ�,���ÿ��������״̬
//����ֵ:�������
SD_Error SD_InitializeCards(void)
{
  int i;
 	SD_Error errorstatus=SD_OK;
	u16 rca = 0x01;
// 	if((TEST_SDIOx->POWER&0X03)==0)return SD_REQUEST_NOT_APPLICABLE;//����Դ״̬,ȷ��Ϊ�ϵ�״̬
  if(((TEST_SDIOx->MMC_CARDSEL>>6)&0X03)==0) return SD_REQUEST_NOT_APPLICABLE;//����Դ״̬,ȷ��Ϊ�ϵ�״̬
	
 	if(SDIO_SECURE_DIGITAL_IO_CARD!=CardType)			//��SECURE_DIGITAL_IO_CARD
	{
		SDIO_Send_Cmd(SD_CMD_ALL_SEND_CID,3,0);			//����CMD2,ȡ��CID,����Ӧ	 
		i=100;while(i--);
		errorstatus=CmdResp2Error(); 					//�ȴ�R2��Ӧ   
		if(errorstatus!=SD_OK)return errorstatus;   	//��Ӧ����
//    CID_Tab[0]=TEST_SDIOx->RESP1;    
//    CID_Tab[1]=TEST_SDIOx->RESP2;
//		CID_Tab[2]=TEST_SDIOx->RESP3;
//		CID_Tab[3]=TEST_SDIOx->RESP4;
    
    for(i = 0;i<4;i++)
    {
      if(i == 0)
      {
        CID_Tab[3 -i ]= TEST_SDIOx->CMD_BUF[2+4*i]<<24  |TEST_SDIOx->CMD_BUF[1+4*i]<<16  |TEST_SDIOx->CMD_BUF[0+4*i]<<8  | TEST_SDIOx->CMD_BUF[0+4*i];
      }
      else
      {
        CID_Tab[3 -i ]= TEST_SDIOx->CMD_BUF[2+4*i]<<24  |TEST_SDIOx->CMD_BUF[1+4*i]<<16  |TEST_SDIOx->CMD_BUF[0+4*i]<<8  | TEST_SDIOx->CMD_BUF[4*i-1];
      }
    }
	}
	if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType)||(SDIO_STD_CAPACITY_SD_CARD_V2_0==CardType)||(SDIO_SECURE_DIGITAL_IO_COMBO_CARD==CardType)||(SDIO_HIGH_CAPACITY_SD_CARD==CardType))//�жϿ�����
	{
		SDIO_Send_Cmd(SD_CMD_SET_REL_ADDR,1,0);			//����CMD3,����Ӧ 
		errorstatus=CmdResp6Error(SD_CMD_SET_REL_ADDR,&rca);//�ȴ�R6��Ӧ 
		if(errorstatus!=SD_OK)return errorstatus;   	//��Ӧ����		    
	}
  if (SDIO_MULTIMEDIA_CARD==CardType)
  {
    SDIO_Send_Cmd(SD_CMD_SET_REL_ADDR,1,(u32)(rca<<16));//����CMD3,����Ӧ 	   
    errorstatus=CmdResp2Error(); 					        //�ȴ�R2��Ӧ
    if(errorstatus!=SD_OK)return errorstatus;   	//��Ӧ����
  }
	if (SDIO_SECURE_DIGITAL_IO_CARD!=CardType)			//��SECURE_DIGITAL_IO_CARD
	{
		RCA = rca;
		SDIO_Send_Cmd(SD_CMD_SEND_CSD,3,(u32)(rca<<16));//����CMD9+��RCA,ȡ��CSD,����Ӧ 	   
		errorstatus=CmdResp2Error(); 					//�ȴ�R2��Ӧ   
		if(errorstatus!=SD_OK)return errorstatus;   	//��Ӧ����		    
    
    for(i = 0;i<4;i++)
    {
      if(i == 0)
      {
        CSD_Tab[3 -i ]= TEST_SDIOx->CMD_BUF[2+4*i]<<24  |TEST_SDIOx->CMD_BUF[1+4*i]<<16  |TEST_SDIOx->CMD_BUF[0+4*i]<<8  | TEST_SDIOx->CMD_BUF[0+4*i];
      }
      else
      {
        CSD_Tab[3 -i ]= TEST_SDIOx->CMD_BUF[2+4*i]<<24  |TEST_SDIOx->CMD_BUF[1+4*i]<<16  |TEST_SDIOx->CMD_BUF[0+4*i]<<8  | TEST_SDIOx->CMD_BUF[4*i-1];
      }
    }   

		CSD_Tab[0]= TEST_SDIOx->CMD_BUF[0]<<24  |TEST_SDIOx->CMD_BUF[1]<<16  |TEST_SDIOx->CMD_BUF[2]<<8  | TEST_SDIOx->CMD_BUF[3];
		CSD_Tab[1]= TEST_SDIOx->CMD_BUF[4]<<24  |TEST_SDIOx->CMD_BUF[5]<<16  |TEST_SDIOx->CMD_BUF[6]<<8  | TEST_SDIOx->CMD_BUF[7];
		CSD_Tab[2]= TEST_SDIOx->CMD_BUF[8]<<24 |TEST_SDIOx->CMD_BUF[9]<<16 |TEST_SDIOx->CMD_BUF[10]<<8  | TEST_SDIOx->CMD_BUF[11];
		CSD_Tab[3]= TEST_SDIOx->CMD_BUF[12]<<24 |TEST_SDIOx->CMD_BUF[13]<<16 |TEST_SDIOx->CMD_BUF[14]<<8 | TEST_SDIOx->CMD_BUF[15];			    
	}
	return SD_OK;//����ʼ���ɹ�
} 
//�õ�����Ϣ
//cardinfo:����Ϣ�洢��
//����ֵ:����״̬
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo)
{
 	SD_Error errorstatus=SD_OK;
	u8 tmp=0;	   
	cardinfo->CardType=(u8)CardType; 				//������
	cardinfo->RCA=(u16)RCA;							//��RCAֵ
	tmp=(u8)((CSD_Tab[0]&0xFF000000)>>24);
	cardinfo->SD_csd.CSDStruct=(tmp&0xC0)>>6;		//CSD�ṹ
	cardinfo->SD_csd.SysSpecVersion=(tmp&0x3C)>>2;	//2.0Э�黹û�����ⲿ��(Ϊ����),Ӧ���Ǻ���Э�鶨���
	cardinfo->SD_csd.Reserved1=tmp&0x03;			//2������λ  
	tmp=(u8)((CSD_Tab[0]&0x00FF0000)>>16);			//��1���ֽ�
	cardinfo->SD_csd.TAAC=tmp;				   		//���ݶ�ʱ��1
	tmp=(u8)((CSD_Tab[0]&0x0000FF00)>>8);	  		//��2���ֽ�
	cardinfo->SD_csd.NSAC=tmp;		  				//���ݶ�ʱ��2
	tmp=(u8)(CSD_Tab[0]&0x000000FF);				//��3���ֽ�
	cardinfo->SD_csd.MaxBusClkFrec=tmp;		  		//�����ٶ�	   
	tmp=(u8)((CSD_Tab[1]&0xFF000000)>>24);			//��4���ֽ�
	cardinfo->SD_csd.CardComdClasses=tmp<<4;    	//��ָ�������λ
	tmp=(u8)((CSD_Tab[1]&0x00FF0000)>>16);	 		//��5���ֽ�
	cardinfo->SD_csd.CardComdClasses|=(tmp&0xF0)>>4;//��ָ�������λ
	cardinfo->SD_csd.RdBlockLen=tmp&0x0F;	    	//����ȡ���ݳ���
	tmp=(u8)((CSD_Tab[1]&0x0000FF00)>>8);			//��6���ֽ�
	cardinfo->SD_csd.PartBlockRead=(tmp&0x80)>>7;	//����ֿ��
	cardinfo->SD_csd.WrBlockMisalign=(tmp&0x40)>>6;	//д���λ
	cardinfo->SD_csd.RdBlockMisalign=(tmp&0x20)>>5;	//�����λ
	cardinfo->SD_csd.DSRImpl=(tmp&0x10)>>4;
	cardinfo->SD_csd.Reserved2=0; 					//����
 	if((CardType==SDIO_STD_CAPACITY_SD_CARD_V1_1)||(CardType==SDIO_STD_CAPACITY_SD_CARD_V2_0)||(SDIO_MULTIMEDIA_CARD==CardType))//��׼1.1/2.0��/MMC��
	{
		cardinfo->SD_csd.DeviceSize=(tmp&0x03)<<10;	//C_SIZE(12λ)
	 	tmp=(u8)(CSD_Tab[1]&0x000000FF); 			//��7���ֽ�	
		cardinfo->SD_csd.DeviceSize|=(tmp)<<2;
 		tmp=(u8)((CSD_Tab[2]&0xFF000000)>>24);		//��8���ֽ�	
		cardinfo->SD_csd.DeviceSize|=(tmp&0xC0)>>6;
 		cardinfo->SD_csd.MaxRdCurrentVDDMin=(tmp&0x38)>>3;
		cardinfo->SD_csd.MaxRdCurrentVDDMax=(tmp&0x07);
 		tmp=(u8)((CSD_Tab[2]&0x00FF0000)>>16);		//��9���ֽ�	
		cardinfo->SD_csd.MaxWrCurrentVDDMin=(tmp&0xE0)>>5;
		cardinfo->SD_csd.MaxWrCurrentVDDMax=(tmp&0x1C)>>2;
		cardinfo->SD_csd.DeviceSizeMul=(tmp&0x03)<<1;//C_SIZE_MULT
 		tmp=(u8)((CSD_Tab[2]&0x0000FF00)>>8);	  	//��10���ֽ�	
		cardinfo->SD_csd.DeviceSizeMul|=(tmp&0x80)>>7;
 		cardinfo->CardCapacity=(cardinfo->SD_csd.DeviceSize+1);//���㿨����
		cardinfo->CardCapacity*=(1<<(cardinfo->SD_csd.DeviceSizeMul+2));
		cardinfo->CardBlockSize=1<<(cardinfo->SD_csd.RdBlockLen);//���С
		cardinfo->CardCapacity*=cardinfo->CardBlockSize;
	}else if(CardType==SDIO_HIGH_CAPACITY_SD_CARD)	//��������
	{
 		tmp=(u8)(CSD_Tab[1]&0x000000FF); 		//��7���ֽ�	
		cardinfo->SD_csd.DeviceSize=(tmp&0x3F)<<16;//C_SIZE
 		tmp=(u8)((CSD_Tab[2]&0xFF000000)>>24); 	//��8���ֽ�	
 		cardinfo->SD_csd.DeviceSize|=(tmp<<8);
 		tmp=(u8)((CSD_Tab[2]&0x00FF0000)>>16);	//��9���ֽ�	
 		cardinfo->SD_csd.DeviceSize|=(tmp);
 		tmp=(u8)((CSD_Tab[2]&0x0000FF00)>>8); 	//��10���ֽ�	
 		cardinfo->CardCapacity=(long long)(cardinfo->SD_csd.DeviceSize+1)*512*1024;//���㿨����
		cardinfo->CardBlockSize=512; 			//���С�̶�Ϊ512�ֽ�
	}	  
	cardinfo->SD_csd.EraseGrSize=(tmp&0x40)>>6;
	cardinfo->SD_csd.EraseGrMul=(tmp&0x3F)<<1;	   
	tmp=(u8)(CSD_Tab[2]&0x000000FF);			//��11���ֽ�	
	cardinfo->SD_csd.EraseGrMul|=(tmp&0x80)>>7;
	cardinfo->SD_csd.WrProtectGrSize=(tmp&0x7F);
 	tmp=(u8)((CSD_Tab[3]&0xFF000000)>>24);		//��12���ֽ�	
	cardinfo->SD_csd.WrProtectGrEnable=(tmp&0x80)>>7;
	cardinfo->SD_csd.ManDeflECC=(tmp&0x60)>>5;
	cardinfo->SD_csd.WrSpeedFact=(tmp&0x1C)>>2;
	cardinfo->SD_csd.MaxWrBlockLen=(tmp&0x03)<<2;	 
	tmp=(u8)((CSD_Tab[3]&0x00FF0000)>>16);		//��13���ֽ�
	cardinfo->SD_csd.MaxWrBlockLen|=(tmp&0xC0)>>6;
	cardinfo->SD_csd.WriteBlockPaPartial=(tmp&0x20)>>5;
	cardinfo->SD_csd.Reserved3=0;
	cardinfo->SD_csd.ContentProtectAppli=(tmp&0x01);  
	tmp=(u8)((CSD_Tab[3]&0x0000FF00)>>8);		//��14���ֽ�
	cardinfo->SD_csd.FileFormatGrouop=(tmp&0x80)>>7;
	cardinfo->SD_csd.CopyFlag=(tmp&0x40)>>6;
	cardinfo->SD_csd.PermWrProtect=(tmp&0x20)>>5;
	cardinfo->SD_csd.TempWrProtect=(tmp&0x10)>>4;
	cardinfo->SD_csd.FileFormat=(tmp&0x0C)>>2;
	cardinfo->SD_csd.ECC=(tmp&0x03);  
	tmp=(u8)(CSD_Tab[3]&0x000000FF);			//��15���ֽ�
	cardinfo->SD_csd.CSD_CRC=(tmp&0xFE)>>1;
	cardinfo->SD_csd.Reserved4=1;		 
	tmp=(u8)((CID_Tab[0]&0xFF000000)>>24);		//��0���ֽ�
	cardinfo->SD_cid.ManufacturerID=tmp;		    
	tmp=(u8)((CID_Tab[0]&0x00FF0000)>>16);		//��1���ֽ�
	cardinfo->SD_cid.OEM_AppliID=tmp<<8;	  
	tmp=(u8)((CID_Tab[0]&0x000000FF00)>>8);		//��2���ֽ�
	cardinfo->SD_cid.OEM_AppliID|=tmp;	    
	tmp=(u8)(CID_Tab[0]&0x000000FF);			//��3���ֽ�	
	cardinfo->SD_cid.ProdName1=tmp<<24;				  
	tmp=(u8)((CID_Tab[1]&0xFF000000)>>24); 		//��4���ֽ�
	cardinfo->SD_cid.ProdName1|=tmp<<16;	  
	tmp=(u8)((CID_Tab[1]&0x00FF0000)>>16);	   	//��5���ֽ�
	cardinfo->SD_cid.ProdName1|=tmp<<8;		 
	tmp=(u8)((CID_Tab[1]&0x0000FF00)>>8);		//��6���ֽ�
	cardinfo->SD_cid.ProdName1|=tmp;		   
	tmp=(u8)(CID_Tab[1]&0x000000FF);	  		//��7���ֽ�
	cardinfo->SD_cid.ProdName2=tmp;			  
	tmp=(u8)((CID_Tab[2]&0xFF000000)>>24); 		//��8���ֽ�
	cardinfo->SD_cid.ProdRev=tmp;		 
	tmp=(u8)((CID_Tab[2]&0x00FF0000)>>16);		//��9���ֽ�
	cardinfo->SD_cid.ProdSN=tmp<<24;	   
	tmp=(u8)((CID_Tab[2]&0x0000FF00)>>8); 		//��10���ֽ�
	cardinfo->SD_cid.ProdSN|=tmp<<16;	   
	tmp=(u8)(CID_Tab[2]&0x000000FF);   			//��11���ֽ�
	cardinfo->SD_cid.ProdSN|=tmp<<8;		   
	tmp=(u8)((CID_Tab[3]&0xFF000000)>>24); 		//��12���ֽ�
	cardinfo->SD_cid.ProdSN|=tmp;			     
	tmp=(u8)((CID_Tab[3]&0x00FF0000)>>16);	 	//��13���ֽ�
	cardinfo->SD_cid.Reserved1|=(tmp&0xF0)>>4;
	cardinfo->SD_cid.ManufactDate=(tmp&0x0F)<<8;    
	tmp=(u8)((CID_Tab[3]&0x0000FF00)>>8);		//��14���ֽ�
	cardinfo->SD_cid.ManufactDate|=tmp;		 	  
	tmp=(u8)(CID_Tab[3]&0x000000FF);			//��15���ֽ�
	cardinfo->SD_cid.CID_CRC=(tmp&0xFE)>>1;
	cardinfo->SD_cid.Reserved2=1;	 
	return errorstatus;
}
//����SDIO���߿��(MMC����֧��4bitģʽ)
//wmode:λ��ģʽ=0,1λ���ݿ��;=1,4λ���ݿ��;=2,8λ���ݿ��
//����ֵ:SD������״̬
SD_Error SD_EnableWideBusOperation(u32 wmode)
{
  	SD_Error errorstatus=SD_OK;
	u16 clkcr=0;
  	if(SDIO_MULTIMEDIA_CARD==CardType)return SD_UNSUPPORTED_FEATURE;//MMC����֧��
 	else if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType)||(SDIO_STD_CAPACITY_SD_CARD_V2_0==CardType)||(SDIO_HIGH_CAPACITY_SD_CARD==CardType))
	{
		if(wmode>=2)return SD_UNSUPPORTED_FEATURE;//��֧��8λģʽ
 		else   
		{
			errorstatus=SDEnWideBus(wmode);
 			if(SD_OK==errorstatus)
			{
//				clkcr=TEST_SDIOx->CLKCR;		//��ȡCLKCR��ֵ
//				clkcr&=~(3<<11);		//���֮ǰ��λ������    
//				clkcr|=(u16)wmode<<11;	//1λ/4λ���߿�� 
//				clkcr|=0<<14;			//������Ӳ��������
//				TEST_SDIOx->CLKCR=clkcr;		//��������CLKCRֵ 
        clkcr = TEST_SDIOx->MMC_CTRL;
        clkcr &= ~(1<<7);
        clkcr|=(u16)wmode<<7;
        clkcr|=(u16)1<<8;
        TEST_SDIOx->MMC_CTRL = clkcr;
			}
		}  
	} 
	return errorstatus; 
}
//����SD������ģʽ
//Mode:
//����ֵ:����״̬
SD_Error SD_SetDeviceMode(u32 Mode)
{
	SD_Error errorstatus = SD_OK;
 	if((Mode==SD_DMA_MODE)||(Mode==SD_POLLING_MODE))DeviceMode=Mode;
	else errorstatus=SD_INVALID_PARAMETER;
	return errorstatus;	    
}
//ѡ��
//����CMD7,ѡ����Ե�ַ(rca)Ϊaddr�Ŀ�,ȡ��������.���Ϊ0,�򶼲�ѡ��.
//addr:����RCA��ַ
SD_Error SD_SelectDeselect(u32 addr)
{
 	SDIO_Send_Cmd(SD_CMD_SEL_DESEL_CARD,1,addr);	//����CMD7,ѡ��,����Ӧ	 	   
   	return CmdResp1Error(SD_CMD_SEL_DESEL_CARD);	  
}  
//SD����ȡ����/����� 
//buf:�����ݻ�����
//addr:��ȡ��ַ
//blksize:���С
//nblks:Ҫ��ȡ�Ŀ���,1,��ʾ��ȡ������
//����ֵ:����״̬ 
int ob_cnt = 0;
SD_Error SD_ReadBlocks(u8 *buf,long long addr,u16 blksize,u32 nblks)
{
  SD_Error errorstatus=SD_OK; 
  u32 tempNblk;
	u32 timeout=SDIO_DATATIMEOUT;  
	u32 *tempbuff=(u32*)buf;	//ת��Ϊu32ָ�� 
//    TEST_SDIOx->DCTRL=0x0;			//���ݿ��ƼĴ�������(��DMA)   
  ob_cnt = 0;
  

//  TEST_SDIOx->BUF_CTL |= 1<<15;
//  TEST_SDIOx->BUF_CTL &= ~(1<<15);
  
  
  TEST_SDIOx->BUF_CTL &= ~(1<<14);
	if(CardType==SDIO_HIGH_CAPACITY_SD_CARD)//��������
	{
		blksize=512;
		addr>>=9;
	}    

	SDIO_Send_Cmd(SD_CMD_SET_BLOCKLEN,1,blksize);			//����CMD16+�������ݳ���Ϊblksize,����Ӧ 	   
	errorstatus=CmdResp1Error(SD_CMD_SET_BLOCKLEN);			//�ȴ�R1��Ӧ   
	if(errorstatus!=SD_OK)return errorstatus;   			//��Ӧ����
	SDIO_Send_Data_Cfg(SD_DATATIMEOUT,nblks*blksize,blksize,1);	//nblks*blksize,���С��Ϊ512,����������	 
 
  
	if(nblks>1)												//����  
	{									    
		SDIO_Send_Cmd(SD_CMD_READ_MULT_BLOCK,1,addr);		//����CMD18+��addr��ַ����ȡ����,����Ӧ 	   
		errorstatus=CmdResp1Error(SD_CMD_READ_MULT_BLOCK);	//�ȴ�R1��Ӧ   
		if(errorstatus!=SD_OK)return errorstatus;   		//��Ӧ����	 
	}else													//�����
	{ 

		SDIO_Send_Cmd(SD_CMD_READ_SINGLE_BLOCK,1,addr);		//����CMD17+��addr��ַ����ȡ����,����Ӧ 	   
		errorstatus=CmdResp1Error(SD_CMD_READ_SINGLE_BLOCK);//�ȴ�R1��Ӧ   
		if(errorstatus!=SD_OK)return errorstatus;   		//��Ӧ����	 
	}
  
  
	if(DeviceMode==SD_POLLING_MODE)//
	{ 
		INTX_DISABLE();//�ر����ж�(POLLINGģʽ,�Ͻ��жϴ��SDIO��д����!!!)
		TEST_SDIOx->MMC_BLOCKCNT = nblks;
    TEST_SDIOx->MMC_IO          = 0x02;  //2019 receive TEST_SDIOx->MMC_IO          = 0x2;
		ob_cnt = 0;
    tempNblk = nblks;
		while(tempNblk--)
		{
//			TEST_SDIOx->BUF_CTL |= 1<<15;

			TEST_SDIOx->MMC_IO |= 0x1;//2019 TEST_SDIOx->MMC_IO          |= 0x1

			while(!(TEST_SDIOx->CLR_MMC_INT&((1<<2)|(1<<6)|(1<<1))))//CRC/��ʱ/���(��־)
			{
				if(!(TEST_SDIOx->BUF_CTL&(1<<1)))	//FIFO����,�����ڿ�������
				{
					*(u32*)tempbuff=TEST_SDIOx->DATA_BUF0;	//ѭ����ȡ����
					tempbuff++;
					ob_cnt++;
					timeout=0X7FFFFF;
				} 
				if(timeout == 0)
				{
					return SD_DATA_TIMEOUT;
				}
				timeout--;
			}
			
			if(TEST_SDIOx->CLR_MMC_INT&(1<<6))		//���ݳ�ʱ����
			{					
				TEST_SDIOx->CLR_MMC_INT = 1<<6; 		//������־
				return SD_DATA_TIMEOUT;
			}else if(TEST_SDIOx->CLR_MMC_INT&(1<<2))	//���ݿ�CRC����
			{
				TEST_SDIOx->CLR_MMC_INT = 1<<2; 		//������־
				return SD_DATA_CRC_FAIL;		   
			}
			
			while(!(TEST_SDIOx->BUF_CTL&(1<<1)))	//FIFO����,�����ڿ�������
			{
				*(u32*)tempbuff=TEST_SDIOx->DATA_BUF0;	//ѭ����ȡ����
				tempbuff++;
				ob_cnt++;
			}  
	  }

    if(nblks>1)
    {
      if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType)||(SDIO_STD_CAPACITY_SD_CARD_V2_0==CardType)||(SDIO_HIGH_CAPACITY_SD_CARD==CardType))
      {
        SDIO_Send_Cmd(SD_CMD_STOP_TRANSMISSION,1,0);		//����CMD12+�������� 	   
        errorstatus=CmdResp1Error(SD_CMD_STOP_TRANSMISSION);//�ȴ�R1��Ӧ   
        if(errorstatus!=SD_OK)return errorstatus;	 
      }
    }
		INTX_ENABLE();//�������ж�
//		TEST_SDIOx->ICR = 0X5FF;	 		//������б�� 
    TEST_SDIOx->CLR_MMC_INT = 0xFF;
	}
  else if(DeviceMode==SD_DMA_MODE)
	{
		TransferError=SD_OK;
		if(nblks>1)StopCondition=1;	//����,��Ҫ����ֹͣ����ָ�� 
		else StopCondition=0;		//�����,����Ҫ����ֹͣ����ָ�� 
		TransferEnd=0;				//�����������λ�����жϷ�����1
    
//		TEST_SDIOx->MASK|=(1<<1)|(1<<3)|(1<<8)|(1<<5)|(1<<9);	//������Ҫ���ж� 
//		TEST_SDIOx->DCTRL|=1<<3;		 						//SDIO DMAʹ�� 
    TEST_SDIOx->BUF_CTL |= (1<<14);
		SD_DMA_Config((u32*)buf,nblks*blksize,0); 
    
//		while(((DMA2->LISR&(1<<27))==RESET)&&timeout)timeout--;//�ȴ�������� 
    
		if(timeout==0)return SD_DATA_TIMEOUT;//��ʱ
		while((TransferEnd==0)&&(TransferError==SD_OK)); 
		if(TransferError!=SD_OK)errorstatus=TransferError;  	 
	}
	return errorstatus;
}	
 		    																  
//SD��д����/����� 
//buf:���ݻ�����
//addr:д��ַ
//blksize:���С
//nblks:Ҫ��ȡ�Ŀ���,1,��ʾ��ȡ������
//����ֵ:����״̬			
extern u32 regTemp[] ;
SD_Error SD_WriteBlocks(u8 *buf,long long addr,u16 blksize,u32 nblks)
{
	SD_Error errorstatus = SD_OK;
	u8  cardstate=0;
	u32 timeout=0,bytestransferred=0;
	u32 cardstatus=0;
	u32 tlen=nblks*blksize;						//�ܳ���(�ֽ�)
	u32 *tempbuff=(u32*)buf;					
  u32 i;
 	if(buf==NULL)return SD_INVALID_PARAMETER;	//��������   
//  	TEST_SDIOx->DCTRL=0x0;							//���ݿ��ƼĴ�������(��DMA)  
  TEST_SDIOx->BUF_CTL &= ~(1<<14); 

 	if(CardType==SDIO_HIGH_CAPACITY_SD_CARD)	//��������
	{
		blksize=512;
		addr>>=9;
	}    
    
	SDIO_Send_Cmd(SD_CMD_SET_BLOCKLEN,1,blksize);			//����CMD16+�������ݳ���Ϊblksize,����Ӧ 	   
	errorstatus=CmdResp1Error(SD_CMD_SET_BLOCKLEN);			//�ȴ�R1��Ӧ   
	if(errorstatus!=SD_OK)return errorstatus;   			//��Ӧ����  
	if(nblks>1)												//���д
	{									     
		if(nblks*blksize>SD_MAX_DATA_LENGTH)return SD_INVALID_PARAMETER;   
     	if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType)||(SDIO_STD_CAPACITY_SD_CARD_V2_0==CardType)||(SDIO_HIGH_CAPACITY_SD_CARD==CardType))
    	{
			//�������
	 	   	SDIO_Send_Cmd(SD_CMD_APP_CMD,1,(u32)RCA<<16);	//����ACMD55,����Ӧ 	   
			errorstatus=CmdResp1Error(SD_CMD_APP_CMD);		//�ȴ�R1��Ӧ   		   
			if(errorstatus!=SD_OK)return errorstatus;				    
	 	   	SDIO_Send_Cmd(SD_CMD_SET_BLOCK_COUNT,1,nblks);	//����CMD23,���ÿ�����,����Ӧ 	   
			errorstatus=CmdResp1Error(SD_CMD_SET_BLOCK_COUNT);//�ȴ�R1��Ӧ   		   
			if(errorstatus!=SD_OK)return errorstatus;				    
		} 
		SDIO_Send_Cmd(SD_CMD_WRITE_MULT_BLOCK,1,addr);		//����CMD25,���дָ��,����Ӧ 	   
		errorstatus=CmdResp1Error(SD_CMD_WRITE_MULT_BLOCK);	//�ȴ�R1��Ӧ    
	}else													//����д		
	{ 
		SDIO_Send_Cmd(SD_CMD_SEND_STATUS,1,(u32)RCA<<16);	//����CMD13,��ѯ����״̬,����Ӧ 	   
		errorstatus=CmdResp1Error(SD_CMD_SEND_STATUS);		//�ȴ�R1��Ӧ   		   
		if(errorstatus!=SD_OK)return errorstatus;
		cardstatus=TEST_SDIOx->CMD_BUF[3]<<24  |TEST_SDIOx->CMD_BUF[2]<<16  |TEST_SDIOx->CMD_BUF[1]<<8  | TEST_SDIOx->CMD_BUF[0];													  
		timeout=SD_DATATIMEOUT;
		while(((cardstatus&0x00000100)==0)&&(timeout>0)) 	//���READY_FOR_DATAλ�Ƿ���λ
		{
			timeout--;
			SDIO_Send_Cmd(SD_CMD_SEND_STATUS,1,(u32)RCA<<16);//����CMD13,��ѯ����״̬,����Ӧ 	   
			errorstatus=CmdResp1Error(SD_CMD_SEND_STATUS);	//�ȴ�R1��Ӧ   		   
			if(errorstatus!=SD_OK)return errorstatus;				    
			cardstatus=TEST_SDIOx->CMD_BUF[3]<<24  |TEST_SDIOx->CMD_BUF[2]<<16  |TEST_SDIOx->CMD_BUF[1]<<8  | TEST_SDIOx->CMD_BUF[0];												  
		}
		if(timeout==0)return SD_ERROR;  
    
		SDIO_Send_Cmd(SD_CMD_WRITE_SINGLE_BLOCK,1,addr);	//����CMD24,д����ָ��,����Ӧ 	   
		errorstatus=CmdResp1Error(SD_CMD_WRITE_SINGLE_BLOCK);//�ȴ�R1��Ӧ   	
	}	   
	if(errorstatus!=SD_OK)return errorstatus;   	   
 	SDIO_Send_Data_Cfg(SD_DATATIMEOUT,nblks*blksize,9,0);	//blksize,���С��Ϊ512�ֽ�,����������	  
	timeout=SDIO_DATATIMEOUT; 
	if(DeviceMode==SD_POLLING_MODE)
	{
    
		INTX_DISABLE();//�ر����ж�(POLLINGģʽ,�Ͻ��жϴ��SDIO��д����!!!)

    
    timeout=0X7FFFFF;
    
    TEST_SDIOx->MMC_IO          = 0x00;//2019 TEST_SDIOx->MMC_IO          = 0x0;
    TEST_SDIOx->BUF_CTL         = 0x8800;    //disable dma, write sd card
		TEST_SDIOx->MMC_BLOCKCNT = nblks;
		bytestransferred = 0;
		
	#if 1
		#if 1
    while(!(TEST_SDIOx->CLR_MMC_INT&((1<<2)|(1<<6)|(1<<1))))//CRC/��ʱ/���(��־)
    {
      if(!(TEST_SDIOx->BUF_CTL&(1<<0)))							//send buffer full
			{
				if((tlen-bytestransferred) > 0)
				{
          TEST_SDIOx->DATA_BUF0 = *(u32*)tempbuff++;
          bytestransferred+=4;

				}
        if(tlen == bytestransferred)
        {
					TEST_SDIOx->MMC_IO   = 0x1; //2019  TEST_SDIOx->MMC_IO   = 0x1;
          break;
        }

				timeout=0X3FFFFFFF;	//д�������ʱ��
			}
			else
			{
				if(timeout==0)return SD_DATA_TIMEOUT; 
				timeout--;
			}
    }
		#endif
		
      
//    TEST_SDIOx->MMC_IO          = 0x1;      //write data, auto transfer   
    
		if(TEST_SDIOx->CLR_MMC_INT&(1<<6))		//���ݳ�ʱ����
		{										   
			TEST_SDIOx->CLR_MMC_INT = 1<<6; 		//������־
			return SD_DATA_TIMEOUT;
		}else if(TEST_SDIOx->CLR_MMC_INT&(1<<2))	//���ݿ�CRC����
		{
			TEST_SDIOx->CLR_MMC_INT = 1<<2; 		//������־
			return SD_DATA_CRC_FAIL;		   
		}
		if(TEST_SDIOx->CLR_MMC_INT&(1<<6))		//���ݳ�ʱ����
		{										   
			TEST_SDIOx->CLR_MMC_INT = 1<<6; 		//������־
			return SD_DATA_TIMEOUT;
		}else if(TEST_SDIOx->CLR_MMC_INT&(1<<2))	//���ݿ�CRC����
		{
			TEST_SDIOx->CLR_MMC_INT = 1<<2; 		//������־
			return SD_DATA_CRC_FAIL;		   
		}
		
		while(!(TEST_SDIOx->BUF_CTL&(1<<1)));
		TEST_SDIOx->CLR_MMC_INT |= 0;
		i = 100;while(i--);
//		TEST_SDIOx->MMC_IO          = 0x0;
		TEST_SDIOx->BUF_CTL         = 0x8800;    //disable dma, write sd card
    #else
//		while(1)//CRC/��ʱ/���(��־)
    {
      if(TEST_SDIOx->BUF_CTL&(1<<1))							//send buffer empty
			{			
				for(i = 0;i < tlen; )
				{

					while(!(TEST_SDIOx->BUF_CTL&(1<<0)))
					{

						TEST_SDIOx->DATA_BUF0 = *(u32*)tempbuff++;
						i += 4;
						if(i == tlen)break;
					}
					TEST_SDIOx->MMC_IO   = 0x1; 
				}
//				TEST_SDIOx->MMC_IO_MBCTL &= ~0x1;
				if(TEST_SDIOx->CLR_MMC_INT&(1<<6))		//���ݳ�ʱ����
				{										   
					TEST_SDIOx->CLR_MMC_INT = 1<<6; 		//������־
					return SD_DATA_TIMEOUT;
				}else if(TEST_SDIOx->CLR_MMC_INT&(1<<2))	//���ݿ�CRC����
				{
					TEST_SDIOx->CLR_MMC_INT = 1<<2; 		//������־
					return SD_DATA_CRC_FAIL;		   
				}
				
				while(!(TEST_SDIOx->BUF_CTL&(1<<1)));
				TEST_SDIOx->CLR_MMC_INT |= 0;
				i = 1000;while(i--);
				TEST_SDIOx->MMC_IO          = 0x0;
				TEST_SDIOx->BUF_CTL         = 0x8800;    //disable dma, write sd card
				timeout=0X3FFFFFFF;	//д�������ʱ��
			}
    }
		
    #endif
    
       
    if(nblks>1)
    {
      if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType)||(SDIO_STD_CAPACITY_SD_CARD_V2_0==CardType)||(SDIO_HIGH_CAPACITY_SD_CARD==CardType))
      {
        SDIO_Send_Cmd(SD_CMD_STOP_TRANSMISSION,1,0);		//����CMD12+�������� 	   
        errorstatus=CmdResp1Error(SD_CMD_STOP_TRANSMISSION);//�ȴ�R1��Ӧ   
        if(errorstatus!=SD_OK)return errorstatus;	 
      }
    }

		INTX_ENABLE();				//�������ж�
//		TEST_SDIOx->ICR=0X5FF;	 		//������б��	  
    TEST_SDIOx->CLR_MMC_INT = 0xFF;
	}else if(DeviceMode==SD_DMA_MODE)
	{
//   		TransferError=SD_OK;
//		if(nblks>1)StopCondition=1;	//���д,��Ҫ����ֹͣ����ָ�� 
//		else StopCondition=0;		//����д,����Ҫ����ֹͣ����ָ��  
//		TransferEnd=0;				//�����������λ�����жϷ�����1
//		TEST_SDIOx->MASK|=(1<<1)|(1<<3)|(1<<8)|(1<<4)|(1<<9);	//���ò������ݽ�������ж�
//		SD_DMA_Config((u32*)buf,nblks*blksize,1);		//SDIO DMA����
// 	 	TEST_SDIOx->DCTRL|=1<<3;								//SDIO DMAʹ��.  
//    TEST_SDIOx->BUF_CTL |= (1<<14);
// 		while(((DMA2->LISR&(1<<27))==RESET)&&timeout)timeout--;//�ȴ�������� 
//		if(timeout==0)
//		{
//  			SD_Init();	 					//���³�ʼ��SD��,���Խ��д������������
//			return SD_DATA_TIMEOUT;			//��ʱ	 
// 		}
//		timeout=SDIO_DATATIMEOUT;
//		while((TransferEnd==0)&&(TransferError==SD_OK)&&timeout)timeout--;
// 		if(timeout==0)return SD_DATA_TIMEOUT;			//��ʱ	 
//  		if(TransferError!=SD_OK)return TransferError;
  }  
//  TEST_SDIOx->ICR=0X5FF;	 		//������б��
  TEST_SDIOx->CLR_MMC_INT = 0xFF;
 	errorstatus=IsCardProgramming(&cardstate);
 	while((errorstatus==SD_OK)&&((cardstate==SD_CARD_PROGRAMMING)||(cardstate==SD_CARD_RECEIVING)))
	{
		errorstatus=IsCardProgramming(&cardstate);
	}   
	return errorstatus;
} 
//SDIO�жϷ�����		  
void SDIO_IRQHandler(void) 
{											
 	SD_ProcessIRQSrc();//��������SDIO����ж�
}	 																    
//SDIO�жϴ�����
//����SDIO��������еĸ����ж�����
//����ֵ:�������
SD_Error SD_ProcessIRQSrc(void)
{
//	if(TEST_SDIOx->STA&(1<<8))//��������ж�
  if(TEST_SDIOx->CLR_MMC_INT & 0x01)
	{	 
		if (StopCondition==1)
		{
			SDIO_Send_Cmd(SD_CMD_STOP_TRANSMISSION,1,0);		//����CMD12,�������� 	   
			TransferError=CmdResp1Error(SD_CMD_STOP_TRANSMISSION);
		}else TransferError = SD_OK;	
// 		TEST_SDIOx->ICR|=1<<8;//�������жϱ��
    TEST_SDIOx->CLR_MMC_INT = 1<<0;
    
//		TEST_SDIOx->MASK&=~((1<<1)|(1<<3)|(1<<8)|(1<<14)|(1<<15)|(1<<4)|(1<<5)|(1<<9));//�ر�����ж�
    TEST_SDIOx->MMC_INT_MASK &= ~((1<<0)|(1<<1)|(1<<6)|(1<<7));
 		TransferEnd = 1;
		return(TransferError);
	}
// 	if(TEST_SDIOx->STA&(1<<1))//����CRC����
  if(TEST_SDIOx->CLR_MMC_INT&(1<<2))
	{
//		TEST_SDIOx->ICR|=1<<1;//����жϱ��
    TEST_SDIOx->CLR_MMC_INT = 1<<2;
//		TEST_SDIOx->MASK&=~((1<<1)|(1<<3)|(1<<8)|(1<<14)|(1<<15)|(1<<4)|(1<<5)|(1<<9));//�ر�����ж�
    TEST_SDIOx->MMC_INT_MASK &= ~((1<<0)|(1<<1)|(1<<6)|(1<<7));
	    TransferError = SD_DATA_CRC_FAIL;
	    return(SD_DATA_CRC_FAIL);
	}
// 	if(TEST_SDIOx->STA&(1<<3))//���ݳ�ʱ����
  if(TEST_SDIOx->CLR_MMC_INT&(1<<6))
	{
//		TEST_SDIOx->ICR|=1<<3;//����жϱ��
    TEST_SDIOx->CLR_MMC_INT = 1<<6;
//		TEST_SDIOx->MASK&=~((1<<1)|(1<<3)|(1<<8)|(1<<14)|(1<<15)|(1<<4)|(1<<5)|(1<<9));//�ر�����ж�
    TEST_SDIOx->MMC_INT_MASK &= ~((1<<0)|(1<<1)|(1<<6)|(1<<7));
	    TransferError = SD_DATA_TIMEOUT;
	    return(SD_DATA_TIMEOUT);
	}
//  	if(TEST_SDIOx->STA&(1<<5))//FIFO�������
//	{
//		TEST_SDIOx->ICR|=1<<5;//����жϱ��
//		TEST_SDIOx->MASK&=~((1<<1)|(1<<3)|(1<<8)|(1<<14)|(1<<15)|(1<<4)|(1<<5)|(1<<9));//�ر�����ж�
//	    TransferError = SD_RX_OVERRUN;
//	    return(SD_RX_OVERRUN);
//	}
//   	if(TEST_SDIOx->STA&(1<<4))//FIFO�������
//	{
//		TEST_SDIOx->ICR|=1<<4;//����жϱ��
//		TEST_SDIOx->MASK&=~((1<<1)|(1<<3)|(1<<8)|(1<<14)|(1<<15)|(1<<4)|(1<<5)|(1<<9));//�ر�����ж�
//	    TransferError = SD_TX_UNDERRUN;
//	    return(SD_TX_UNDERRUN);
//	}
//	if(TEST_SDIOx->STA&(1<<9))//��ʼλ����
//	{
//		TEST_SDIOx->ICR|=1<<9;//����жϱ��
//		TEST_SDIOx->MASK&=~((1<<1)|(1<<3)|(1<<8)|(1<<14)|(1<<15)|(1<<4)|(1<<5)|(1<<9));//�ر�����ж�
//	    TransferError = SD_START_BIT_ERR;
//	    return(SD_START_BIT_ERR);
//	}
	return(SD_OK);
}
  
//���CMD0��ִ��״̬
//����ֵ:sd��������
SD_Error CmdError(void)
{
	SD_Error errorstatus = SD_OK;
	u32 timeout=SDIO_CMD0TIMEOUT;	   
	while(timeout--)
	{
//		if(TEST_SDIOx->STA&(1<<7))break;	//�����ѷ���(������Ӧ)	 
    if(((TEST_SDIOx->MMC_IO>>2)&0x3) == 0)break;
	}	    
	if(timeout==0)return SD_CMD_RSP_TIMEOUT;  
//	TEST_SDIOx->ICR=0X5FF;				//������
  TEST_SDIOx->CLR_MMC_INT = 0xFF;
	return errorstatus;
}	 
//���R7��Ӧ�Ĵ���״̬
//����ֵ:sd��������
SD_Error CmdResp7Error(void)
{
	SD_Error errorstatus=SD_OK;
	u32 status;
	u32 timeout=SDIO_CMD0TIMEOUT;
 	while(timeout--)
	{
//		status=TEST_SDIOx->STA;
//		if(status&((1<<0)|(1<<2)|(1<<6)))break;//CRC����/������Ӧ��ʱ/�Ѿ��յ���Ӧ(CRCУ��ɹ�)	
    status = TEST_SDIOx->CLR_MMC_INT ;
    if(status&((1<<7)|(1<<6)|(1<<0)))break;   
	}
 	if((timeout==0)||(status&(1<<6)))	//��Ӧ��ʱ
	{																				    
		errorstatus=SD_CMD_RSP_TIMEOUT;	//��ǰ������2.0���ݿ�,���߲�֧���趨�ĵ�ѹ��Χ
//		TEST_SDIOx->ICR|=1<<2;				//���������Ӧ��ʱ��־
    TEST_SDIOx->CLR_MMC_INT = 1<<6;
		return errorstatus;
	}	 
	if(status&1<<0)						//�ɹ����յ���Ӧ
	{								   
		errorstatus=SD_OK;
//		TEST_SDIOx->ICR|=1<<6;				//�����Ӧ��־
    TEST_SDIOx->CLR_MMC_INT = 1<<0;
 	}
	return errorstatus;
}	   
//���R1��Ӧ�Ĵ���״̬
//cmd:��ǰ����
//����ֵ:sd��������
SD_Error CmdResp1Error(u8 cmd)
{	  
  u32 status; 
  u32 response;
	while(1)
	{
//		status=TEST_SDIOx->STA;
//		if(status&((1<<0)|(1<<2)|(1<<6)))break;//CRC����/������Ӧ��ʱ/�Ѿ��յ���Ӧ(CRCУ��ɹ�)
    status = TEST_SDIOx->CLR_MMC_INT ;
    if(status&((1<<7)|(1<<6)|(1<<0)))break;   
	} 

	if(status&(1<<6))					//��Ӧ��ʱ
	{																				    
// 		TEST_SDIOx->ICR=1<<2;					//���������Ӧ��ʱ��־
    TEST_SDIOx->CLR_MMC_INT = 1<<6;
		return SD_CMD_RSP_TIMEOUT;
	}	
 	if(status&(1<<7))					//CRC����
	{																				    
// 		TEST_SDIOx->ICR=1<<0;					//�����־
    TEST_SDIOx->CLR_MMC_INT = 1<<7;
		return SD_CMD_CRC_FAIL;
	}		
  TEST_SDIOx->CLR_MMC_INT = 0xFF;
  
  if((TEST_SDIOx->CMD_BUF[4]&0x3F) != cmd)
  {
    return SD_ILLEGAL_CMD;//���ƥ�� 
  }
//	if(TEST_SDIOx->RESPCMD!=cmd)return SD_ILLEGAL_CMD;//���ƥ�� 
  response=TEST_SDIOx->CMD_BUF[3]<<24  |TEST_SDIOx->CMD_BUF[2]<<16  |TEST_SDIOx->CMD_BUF[1]<<8  | TEST_SDIOx->CMD_BUF[0];	
//  TEST_SDIOx->ICR=0X5FF;	 				//������
	return (SD_Error)(response&SD_OCR_ERRORBITS);//���ؿ���Ӧ
}
//���R3��Ӧ�Ĵ���״̬
//����ֵ:����״̬
SD_Error CmdResp3Error(void)
{
	u32 status;		
  
 	while(1)
	{
//		status=TEST_SDIOx->STA;
//		if(status&((1<<0)|(1<<2)|(1<<6)))break;//CRC����/������Ӧ��ʱ/�Ѿ��յ���Ӧ(CRCУ��ɹ�)	
    status = TEST_SDIOx->CLR_MMC_INT ;
    if(status&((1<<7)|(1<<6)|(1<<0)))break;   
	}

 	if(status&(1<<6))					//��Ӧ��ʱ
	{											 
//		TEST_SDIOx->ICR|=1<<2;				//���������Ӧ��ʱ��־
    TEST_SDIOx->CLR_MMC_INT = 1<<6;
		return SD_CMD_RSP_TIMEOUT;
	}	 
//   	TEST_SDIOx->ICR=0X5FF;	 				//������
  TEST_SDIOx->CLR_MMC_INT = 0xFF;
 	return SD_OK;								  
}
//���R2��Ӧ�Ĵ���״̬
//����ֵ:����״̬
SD_Error CmdResp2Error(void)
{
	SD_Error errorstatus=SD_OK;
	u32 status;
	u32 timeout=SDIO_CMD0TIMEOUT;
   
 	while(timeout--)
	{
//		status=TEST_SDIOx->STA;
//		if(status&((1<<0)|(1<<2)|(1<<6)))break;//CRC����/������Ӧ��ʱ/�Ѿ��յ���Ӧ(CRCУ��ɹ�)	
    status = TEST_SDIOx->CLR_MMC_INT ;
    if(status&((1<<7)|(1<<6)|(1<<0)))break;   
	}

  	if((timeout==0)||(status&(1<<6)))	//��Ӧ��ʱ
	{																				    
		errorstatus=SD_CMD_RSP_TIMEOUT; 
//		TEST_SDIOx->ICR|=1<<2;				//���������Ӧ��ʱ��־
    TEST_SDIOx->CLR_MMC_INT = 1<<6;
		return errorstatus;
	}	 
	if(status&(1<<7))						//CRC����
	{								   
		errorstatus=SD_CMD_CRC_FAIL;
//		TEST_SDIOx->ICR|=1<<0;				//�����Ӧ��־
    TEST_SDIOx->CLR_MMC_INT = 1<<7;
 	}
//	TEST_SDIOx->ICR=0X5FF;	 				//������
  TEST_SDIOx->CLR_MMC_INT = 0xff;
 	return errorstatus;								    		 
} 
//���R6��Ӧ�Ĵ���״̬
//cmd:֮ǰ���͵�����
//prca:�����ص�RCA��ַ
//����ֵ:����״̬
SD_Error CmdResp6Error(u8 cmd,u16*prca)
{
	SD_Error errorstatus=SD_OK;
	u32 status;					    
	u32 rspr1;
 	while(1)
	{
//		status=TEST_SDIOx->STA;
//		if(status&((1<<0)|(1<<2)|(1<<6)))break;//CRC����/������Ӧ��ʱ/�Ѿ��յ���Ӧ(CRCУ��ɹ�)	
    status = TEST_SDIOx->CLR_MMC_INT ;
    if(status&((1<<7)|(1<<6)|(1<<0)))break;   
	}

	if(status&(1<<6))					//��Ӧ��ʱ
	{
      
// 		TEST_SDIOx->ICR|=1<<2;				//���������Ӧ��ʱ��־
    TEST_SDIOx->CLR_MMC_INT = 1<<6;
		return SD_CMD_RSP_TIMEOUT;
	}	 	 
	if(status&(1<<7))						//CRC����
	{								   
//		TEST_SDIOx->ICR|=1<<0;				//�����Ӧ��־
    TEST_SDIOx->CLR_MMC_INT = 1<<7;
 		return SD_CMD_CRC_FAIL;
	}
//if(TEST_SDIOx->RESPCMD!=cmd)					//�ж��Ƿ���Ӧcmd���� TEST_SDIOx->CMD_BUF4&0x3F
  if((TEST_SDIOx->CMD_BUF[4]&0x3F)!=cmd)
	{
 		return SD_ILLEGAL_CMD; 		
	}	    
//	TEST_SDIOx->ICR=0X5FF;	 				//������б��
  TEST_SDIOx->CLR_MMC_INT = 0xFF;
	rspr1=TEST_SDIOx->CMD_BUF[3]<<24  |TEST_SDIOx->CMD_BUF[2]<<16  |TEST_SDIOx->CMD_BUF[1]<<8  | TEST_SDIOx->CMD_BUF[0];						//�õ���Ӧ 	 
	if(SD_ALLZERO==(rspr1&(SD_R6_GENERAL_UNKNOWN_ERROR|SD_R6_ILLEGAL_CMD|SD_R6_COM_CRC_FAILED)))
	{
		*prca=(u16)(rspr1>>16);			//����16λ�õ�,rca
		return errorstatus;
	}
  if(rspr1&SD_R6_GENERAL_UNKNOWN_ERROR)
  {
    return SD_GENERAL_UNKNOWN_ERROR;
  }
  if(rspr1&SD_R6_ILLEGAL_CMD)
  {
    return SD_ILLEGAL_CMD;
  }
  if(rspr1&SD_R6_COM_CRC_FAILED)
  {
    return SD_COM_CRC_FAILED;
  }
	return errorstatus;
}

//SDIOʹ�ܿ�����ģʽ
//enx:0,��ʹ��;1,ʹ��;
//����ֵ:����״̬
SD_Error SDEnWideBus(u8 enx)
{
	SD_Error errorstatus = SD_OK;
  u32 response;
 	u32 scr[2]={0,0};
	u8 arg=0X00;
	if(enx)arg=0X02;
	else arg=0X00;
// 	if(TEST_SDIOx->RESP1&SD_CARD_LOCKED)return SD_LOCK_UNLOCK_FAILED;//SD������LOCKED״̬		  
  response = TEST_SDIOx->CMD_BUF[3]<<24  |TEST_SDIOx->CMD_BUF[2]<<16  |TEST_SDIOx->CMD_BUF[1]<<8  | TEST_SDIOx->CMD_BUF[0];	 
  if(response&SD_CARD_LOCKED)return SD_LOCK_UNLOCK_FAILED;//SD������LOCKED״̬	
	
// 	errorstatus=FindSCR(RCA,scr);						//�õ�SCR�Ĵ�������20190504,��ʱע�͵�����Ϊ�����Բ���

 	if(errorstatus!=SD_OK)return errorstatus;
//	if((scr[1]&SD_WIDE_BUS_SUPPORT)!=SD_ALLZERO)		//֧�ֿ����� tao
	{
	 	SDIO_Send_Cmd(SD_CMD_APP_CMD,1,(u32)RCA<<16);	//����CMD55+RCA,����Ӧ			
    
	 	errorstatus=CmdResp1Error(SD_CMD_APP_CMD);
	 	if(errorstatus!=SD_OK)return errorstatus; 
    
	 	SDIO_Send_Cmd(SD_CMD_APP_SD_SET_BUSWIDTH,1,arg);//����ACMD6,����Ӧ,����:10,4λ;00,1λ.											  
		errorstatus=CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);
		return errorstatus;
	}
  //else return SD_REQUEST_NOT_APPLICABLE;				//��֧�ֿ��������� 	 
}												   
//��鿨�Ƿ�����ִ��д����
//pstatus:��ǰ״̬.
//����ֵ:�������
SD_Error IsCardProgramming(u8 *pstatus)
{
 	vu32 respR1 = 0, status = 0; 
  while(!(TEST_SDIOx->BUF_CTL & 0x02));
  SDIO_Send_Cmd(SD_CMD_SEND_STATUS,1,(u32)RCA<<16);		//����CMD13 	   
//  	status=TEST_SDIOx->STA;
//	while(!(status&((1<<0)|(1<<6)|(1<<2))))status=TEST_SDIOx->STA;//�ȴ��������
  
  status = TEST_SDIOx->CLR_MMC_INT ;
  while(!(status&((1<<7)|(1<<6)|(1<<0))))status = TEST_SDIOx->CLR_MMC_INT ;   
  
   	if(status&(1<<7))			//CRC���ʧ��
	{
//		TEST_SDIOx->ICR|=1<<0;		//���������
    TEST_SDIOx->CLR_MMC_INT = 1<<7;
		return SD_CMD_CRC_FAIL;
	}
   	if(status&(1<<6))			//���ʱ 
	{
//		TEST_SDIOx->ICR|=1<<2;		//���������
    TEST_SDIOx->CLR_MMC_INT = 1<<6;
		return SD_CMD_RSP_TIMEOUT;
	}
// 	if(TEST_SDIOx->RESPCMD!=SD_CMD_SEND_STATUS)return SD_ILLEGAL_CMD;
  if((TEST_SDIOx->CMD_BUF[4]&0x3F)!=SD_CMD_SEND_STATUS)return SD_ILLEGAL_CMD;
//	TEST_SDIOx->ICR=0X5FF;	 		//������б��
  TEST_SDIOx->CLR_MMC_INT = 0xff;
	respR1=TEST_SDIOx->CMD_BUF[3]<<24  |TEST_SDIOx->CMD_BUF[2]<<16  |TEST_SDIOx->CMD_BUF[1]<<8  | TEST_SDIOx->CMD_BUF[0];	
	*pstatus=(u8)((respR1>>9)&0x0000000F);
	return SD_OK;
}
//��ȡ��ǰ��״̬
//pcardstatus:��״̬
//����ֵ:�������
SD_Error SD_SendStatus(uint32_t *pcardstatus)
{
	SD_Error errorstatus = SD_OK;
	if(pcardstatus==NULL)
	{
		errorstatus=SD_INVALID_PARAMETER;
		return errorstatus;
	}
 	SDIO_Send_Cmd(SD_CMD_SEND_STATUS,1,RCA<<16);	//����CMD13,����Ӧ		 
	errorstatus=CmdResp1Error(SD_CMD_SEND_STATUS);	//��ѯ��Ӧ״̬ 
	if(errorstatus!=SD_OK)return errorstatus;
	*pcardstatus=TEST_SDIOx->CMD_BUF[3]<<24  |TEST_SDIOx->CMD_BUF[2]<<16  |TEST_SDIOx->CMD_BUF[1]<<8  | TEST_SDIOx->CMD_BUF[0];	//��ȡ��Ӧֵ
	return errorstatus;
} 
//����SD����״̬
//����ֵ:SD��״̬
SDCardState SD_GetState(void)
{
	u32 resp1=0;
	if(SD_SendStatus(&resp1)!=SD_OK)return SD_CARD_ERROR;
	else return (SDCardState)((resp1>>9) & 0x0F);
}
//����SD����SCR�Ĵ���ֵ
//rca:����Ե�ַ
//pscr:���ݻ�����(�洢SCR����)
//����ֵ:����״̬		   
SD_Error FindSCR(u16 rca,u32 *pscr)
{
	u32 index = 0; 
	SD_Error errorstatus = SD_OK;
	u8 tempscr[8]={0,0};  
 	SDIO_Send_Cmd(SD_CMD_SET_BLOCKLEN,1,8);			//����CMD16,����Ӧ,����Block SizeΪ8�ֽ�											  
 	errorstatus=CmdResp1Error(SD_CMD_SET_BLOCKLEN);
 	if(errorstatus!=SD_OK)return errorstatus;	    
  	SDIO_Send_Cmd(SD_CMD_APP_CMD,1,(u32)rca<<16);	//����CMD55,����Ӧ 									  
 	errorstatus=CmdResp1Error(SD_CMD_APP_CMD);
 	if(errorstatus!=SD_OK)return errorstatus;
	SDIO_Send_Data_Cfg(SD_DATATIMEOUT,8,3,1);		//8���ֽڳ���,blockΪ8�ֽ�,SD����SDIO.
   	SDIO_Send_Cmd(SD_CMD_SD_APP_SEND_SCR,1,0);		//����ACMD51,����Ӧ,����Ϊ0											  
 	errorstatus=CmdResp1Error(SD_CMD_SD_APP_SEND_SCR);
 	if(errorstatus!=SD_OK)return errorstatus;							   
// 	while(!(TEST_SDIOx->STA&(SDIO_FLAG_RXOVERR|SDIO_FLAG_DCRCFAIL|SDIO_FLAG_DTIMEOUT|SDIO_FLAG_DBCKEND|SDIO_FLAG_STBITERR)))
  
  for( index = 0; index < 8;index ++ )
  {
    #if 0
			*(tempscr+index)=TEST_SDIOx->CMD_BUF[index];	//��ȡbuf
    #else
      *(tempscr+index)=TEST_SDIOx->DATA_BUF0;	//��ȡbuf
    #endif
			index++;
  }

 	if(TEST_SDIOx->CLR_MMC_INT&(1<<6))		//�������ݳ�ʱ
	{										 
// 		TEST_SDIOx->ICR|=1<<3;		//������
    TEST_SDIOx->CLR_MMC_INT = 1<<6;
		return SD_DATA_TIMEOUT;
	}
	else if(TEST_SDIOx->CLR_MMC_INT&(1<<7))	//�ѷ���/���յ����ݿ�CRCУ�����
	{
// 		TEST_SDIOx->ICR|=1<<1;		//������
    TEST_SDIOx->CLR_MMC_INT = 1<<7;
		return SD_DATA_CRC_FAIL;   
	}
//	else if(TEST_SDIOx->STA&(1<<5))	//����FIFO���
//	{
// 		TEST_SDIOx->ICR|=1<<5;		//������
//		return SD_RX_OVERRUN;   	   
//	}
//	else if(TEST_SDIOx->STA&(1<<9))	//��ʼλ������
//	{
// 		TEST_SDIOx->ICR|=1<<9;		//������
//		return SD_START_BIT_ERR;    
//	}
//   	TEST_SDIOx->ICR=0X5FF;	 		//������	 
  TEST_SDIOx->CLR_MMC_INT = 0xff;
	//������˳��8λΪ��λ������.   	
	*(pscr+1)=((tempscr[0])<<24)|((tempscr[1])<<8)|((tempscr[2])>>8)|((tempscr[3])>>24);
	*(pscr)=((tempscr[7])<<24)|((tempscr[6])<<8)|((tempscr[5])>>8)|((tempscr[4])>>24);
 	return errorstatus;
} 
//����SDIO DMA  
//mbuf:�洢����ַ
//bufsize:����������
//dir:����;1,�洢��-->SDIO(д����);0,SDIO-->�洢��(������);
void SD_DMA_Config(u32*mbuf,u32 bufsize,u8 dir)
{		 
// 	u32 tmpreg=0;//��������
//	while(DMA2_Stream3->CR&0X01);	//�ȴ�DMA������ 
//	DMA2->LIFCR|=0X3D<<22;			//���֮ǰ��stream3�ϵ������жϱ�־
//	
//	
//	DMA2_Stream3->PAR=(u32)&TEST_SDIOx->FIFO;	//DMA2 �����ַ
//	DMA2_Stream3->M0AR=(u32)mbuf; 	//DMA2,�洢��0��ַ;	 
//	DMA2_Stream3->NDTR=0; 			//DMA2,����������0,���������� 
//	tmpreg|=dir<<6;		//���ݴ��䷽�����
//	tmpreg|=0<<8;		//��ѭ��ģʽ(��ʹ����ͨģʽ)
//	tmpreg|=0<<9;		//���������ģʽ
//	tmpreg|=1<<10;		//�洢������ģʽ
//	tmpreg|=2<<11;		//�������ݳ���:32λ
//	tmpreg|=2<<13;		//�洢�����ݳ���:32λ
//	tmpreg|=3<<16;		//������ȼ�
//	tmpreg|=1<<21;		//����ͻ��4�δ���
//	tmpreg|=1<<23;		//�洢��ͻ��4�δ���
//	tmpreg|=(u32)4<<25;	//ͨ��ѡ��
//	DMA2_Stream3->CR=tmpreg; 
//	
//	tmpreg=DMA2_Stream3->FCR;
//	tmpreg&=0XFFFFFFF8;	//���DMDIS��FTH
//	tmpreg|=1<<2;		//FIFOʹ��
//	tmpreg|=3<<0;		//ȫFIFO
//	DMA2_Stream3->FCR=tmpreg;
//	DMA2_Stream3->CR|=1<<5;	//���������� 
//	DMA2_Stream3->CR|=1<<0;	//����DMA���� 
}   
//��SD��
//buf:�����ݻ�����
//sector:������ַ
//cnt:��������	
//����ֵ:����״̬;0,����;����,�������;				

u8 SD_ReadDisk(u8*buf,u32 sector,u32 cnt)
{
	#if 0
	u8 sta=SD_OK;
	long long lsector=sector;
	u32 n;
	if(CardType!=SDIO_STD_CAPACITY_SD_CARD_V1_1)lsector<<=9;
	if((u32)buf%4!=0)
	{
	 	for(n=0;n<cnt;n++)
		{
		 	sta=SD_ReadBlocks(SDIO_DATA_BUFFER,lsector+512*n,512,1);//����sector�Ķ�����
			memcpy(buf,SDIO_DATA_BUFFER,512);
			buf+=512;
		} 
	}else sta=SD_ReadBlocks(buf,lsector,512,cnt);	//����/���sector   
	return sta;
	#endif
	#if 1
	u8 sta=SD_OK;
	long long lsector=sector;
	u32 temp = 1;
	u32 n;
//	if(CardType != SDIO_STD_CAPACITY_SD_CARD_V1_1)
//����֧��SD1.1�Ŀ���һ�������ϻ������֣�������ô���ٵĿ�Ҳû��Ҫ�������ַ�ʽ������
	{
		temp = 1<<9;
		lsector <<= 9;
	}
	for(n= 0;n < cnt ;n++)
	{
    
		sta = SD_ReadBlocks(buf,lsector,512,1);	//����/���sector  
		lsector += temp;
		buf += 512;
	}
  
	return sta;
	#endif
}
//дSD��
//buf:д���ݻ�����
//sector:������ַ
//cnt:��������	
//����ֵ:����״̬;0,����;����,�������;	
u8 SD_WriteDisk(u8*buf,u32 sector,u32 cnt)
{
	#if 0
	u8 sta=SD_OK;
	u32 n;
	long long lsector=sector;
	if(CardType!=SDIO_STD_CAPACITY_SD_CARD_V1_1)lsector<<=9;
	if((u32)buf%4!=0)
	{
	 	for(n=0;n<cnt;n++)
		{
			memcpy(SDIO_DATA_BUFFER,buf,512);
		 	sta=SD_WriteBlocks(SDIO_DATA_BUFFER,lsector+512*n,512,1);//����sector��д����
			buf+=512;
		} 
	}else sta=SD_WriteBlocks(buf,lsector,512,cnt);	//����/���sector   
	return sta;
	#endif
	#if 1
	u8 sta=SD_OK;
	u32 temp = 1;
	u32 n;
	long long lsector=sector;
	if(CardType != SDIO_STD_CAPACITY_SD_CARD_V1_1)
	{	
		temp = 1<<9;
		lsector <<= 9;
	}
	for(n=0;n<cnt;n++)
	{
		memcpy(SDIO_DATA_BUFFER,buf,512);
		sta=SD_WriteBlocks(SDIO_DATA_BUFFER,lsector,512,1);//����sector��д����
		lsector += temp;
		buf+=512;
	} 

	return sta;
	#endif
}

void show_sdcard_info(void)
{
	switch(SDCardInfo.CardType)
	{
		case SDIO_STD_CAPACITY_SD_CARD_V1_1:printf((char *)("Card Type:SDSC V1.1\r\n"));break;
		case SDIO_STD_CAPACITY_SD_CARD_V2_0:printf((char *)("Card Type:SDSC V2.0\r\n"));break;
		case SDIO_HIGH_CAPACITY_SD_CARD:printf((char *)("Card Type:SDHC V2.0\r\n"));break;
		case SDIO_MULTIMEDIA_CARD:printf((char *)("Card Type:MMC Card\r\n"));break;
	}	
    printf((char *)"Card ManufacturerID:%d\r\n",SDCardInfo.SD_cid.ManufacturerID);	//������ID
 	printf((char *)"Card RCA:%d\r\n",SDCardInfo.RCA);								//����Ե�ַ
	printf((char *)"Card Capacity:%d MB\r\n",(u32)(SDCardInfo.CardCapacity>>20));	//��ʾ����
 	printf((char *)"Card BlockSize:%d\r\n\r\n",SDCardInfo.CardBlockSize);			//��ʾ���С
}








