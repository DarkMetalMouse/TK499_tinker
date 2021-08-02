#include "SDIO.h"
#include "sdio_sdcard.h"

//按照TK499配置
void SDIO1_GPIOInitRemap(void)
{
	
		#if (0)
    GPIOC->CRL  = 0x0;   
    GPIOC->AFRL = 0x0;
	
    GPIOC->AFRL = 0xCCCCCCCC;
    GPIOC->CRL = 0xAAAAAAAA;   //GPIOC_Pin_6 clear

 
	#else
    GPIOD->AFRL &= 0xFF000000;
		GPIOD->AFRL |= 0x00CCCCCC;
		GPIOD->CRL  &= 0xFF000000;   
		GPIOD->CRL  |= 0x00aaaaaa;   
	#endif
}
//按照TK499配置
void SDIO_Init(void) 
{
  SDIO1_GPIOInitRemap();
	
//	RCC->APB2ENR |= 0x1<<11; //open sdio1 clkgate
	RCC->APB2ENR |= 0x1<<12; //open sdio2 clkgate
	TEST_SDIOx->MMC_CARDSEL = 0xC0;   //enable module, enable mmcclk
	TEST_SDIOx->MMC_CTRL    = 0x03|((0x7)<<3)|(1<<8);//4bit,low speed,1/16 divider,初始化时必须用低速		
//	TEST_SDIOx->MMC_INT_MASK = 0x01;  //unmask interrupt
	TEST_SDIOx->MMC_CRCCTL   = 0xC0; 
	
//	TEST_SDIOx->MMC_IO_MBCTL |= 0x8;//2019
	SD_Init();
//	while()
//	{
//	   printf("检查不到卡的信息!\n");
//	}
	show_sdcard_info();
//	
}

//======================================================
// set up
// Test:  Init sequence, With response check  
// CMD 0. Reset Card
// CMD 8. Get voltage (Only 2.0 Card response to this)            ////
// CMD55. Indicate Next Command are Application specific
// ACMD44. Get Voltage windows
// CMD2. CID reg
// CMD3. Get RCA.
//======================================================

