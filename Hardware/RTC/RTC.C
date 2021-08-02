#include "tk499.h"
#include "main.h"

#include "usart.h"
void rtc_work_cfg()
{
    uint32_t    scaler;
    uint32_t    cnt;

    RCC->APB1ENR |= 1<<29;   //Enable the PWREN clock.
    PWR->CR |= 1<<8;         //Enable access to the RTC and backup registers.

    RCC->BDCR |= 1<<16;    //Force the Backup domain reset.
    RCC->BDCR &= ~(1<<16); //Release the Backup domain reset.
    RCC->BDCR |= 1<<15;    //Enable RTC clock.
    RCC->BDCR |= 1<<8;     //select LES as RTC clock source.
    RCC->BDCR |= 1<<0;     //External low-speed oscillar enable.

    while(!(RCC->BDCR & 0x1<<1));  //External low-speed clock ready flag.
    while(!(RTC->CRL & 1<<5)); //Wait until last write operation on RTC registers has finished.
    
    RTC->CRL |= 1<<4;      //Enter the RTC configuration mode.
    RTC->ALRH = 0x0;      //Set the RTC alarm value.
    RTC->ALRL = 0x300;    
    RTC->PRLH = 0x0;      //Set the RTC prescaler value.
    RTC->PRLL = 0x10;
    RTC->CNTH = 0x0;      //Set the RTC counter value.
    RTC->CNTL = 0x50;
    RTC->CRL &= ~(1<<4);  //Exit from the RTC configuration mode.
    
    while(!(RTC->CRL & 1<<5));  //Wait until last write operation on RTC registers has finished.
    while(!(RTC->CRL & 1<<3));  //wait until the RTC registers (RTC_CNT, RTC_ALR and RTC_PRL) are synchronized with RTC APB clock.

while(1)
{
    cnt  = RTC->CNTL;
    cnt |= RTC->CNTH << 16;
    scaler  = RTC->PRLL;
    scaler |= RTC->PRLH << 16;
    printf("Prescaler = %x,cnt = %x\n",scaler,cnt);
		GUI_Delay(500);
}
}