#include "sys.h"
#include "uart.h"
#include "main.h"

//加入下列代码，不用勾选USE microlib,使用printf

#pragma import(__use_no_semihosting)                          
struct __FILE 
{ 
int handle; 
}; 
FILE __stdout;       
int _sys_exit(int x) 
{ 
x = x; 
} 
int fputc(int ch, FILE *f)
{      
while((UART1->CSR &0x1) == 0){}
	UART1->TDR = (u8) ch;      
return ch;
}

void UartInit(UART_TypeDef* UARTx,int BaudRate)
{
	UART_InitTypeDef       UART_InitStructure;  
	GPIO_InitTypeDef  GPIO_InitStructure;   
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_9;   //uart1_tx  pa9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 推免复用输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10;  //uart1_rx  pa10
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //上拉输入   

  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOA, GPIO_Pin_9 | GPIO_Pin_10, GPIO_AF_UART_1); //PA9、PA10复用为串口1
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_UART1, ENABLE);
	UART_InitStructure.UART_BaudRate = BaudRate; //波特率
  UART_InitStructure.UART_WordLength = UART_WordLength_8b;//数据位
  UART_InitStructure.UART_StopBits = UART_StopBits_1;//停止位
  UART_InitStructure.UART_Parity = UART_Parity_No ;
  UART_InitStructure.UART_Mode = UART_Mode_Rx | UART_Mode_Tx;//输入输出模式
  UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_None; 	
	UART_Init(UARTx, &UART_InitStructure);
	UART_Cmd(UARTx, ENABLE);  //UART 模块使能
	UART_ITConfig(UARTx, UART_IT_RXIEN, ENABLE);//使能接收中断
  UART_ClearITPendingBit(UARTx, 0xff); 

	NVIC_SetPriority(UART1_IRQn,3);
	NVIC_EnableIRQ(UART1_IRQn);
		
}
u8 Uart1_Receive(void)
{
	//判断数据是否到来
  while((UART1->CSR & (1<<1)) ==0);
	return UART1->RDR;

}
void UART1_IRQHandler(void)
{
   if(UART1->ISR & (1<<1))
	 {
		  UART1->RDR = Uart1_Receive();
	    UART1->ICR |= 1<<1;
		  printf("UART1_RDR = %d\n",UART1->RDR);
	 }
}
void send_data(u8 data)
{
	while((UART1->CSR &0x1) == 0);
	UART1->TDR = data; 
}
void send_group(u8*data,u16 len)
{
	while(len--)
	send_data(*data++);
}
void send_str(char *p)
{
		while(*p != '\0')
		{
			send_data(*p);
			p++;
		}
}






