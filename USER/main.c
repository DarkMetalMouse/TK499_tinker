#include "main.h"
#include "UART.h"
//#include "MM1_240.h"
//#include "MM_T035.h"
//#include "set.h"
//#include "tell.h"
//#include "World.h"
//#include "timer_pic.h"
//#include "temper_pic.h"
#include "deej_logo.h"
//#include "part.h"
#include "Tiky_LCD_APP.h"

void speed_test(void);

#define MAX_LEN 80
#define READ_OK 0
#define READ_TOO_LONG 1

int send_slider_values = 0;
char command_buffer[MAX_LEN + 1]; // +1 for null char

int read_command_until(char end)
{
	char c = getchar();
	unsigned int i = 0;
	while (c != end && i < MAX_LEN)
	{
		command_buffer[i++] = c;
		c = getchar();
	}
	command_buffer[i] = '\0';

	return i;
}

int main(void)
{

	RemapVtorTable();
	SystemClk_HSEInit(RCC_PLLMul_20); //启动PLL时钟

	RCC->AHB1ENR |= 3 << 21; // DMA1,DMA2 Clock

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 2：2，全局性函数，仅需设置一次
	User_RGB_LCD_Initial();							//初始化液晶屏入口
	UartInit(UART1, 115200);						//配置串口1，波特率为460800
	printf("\r\n  Hello!welcome to use TK499!  \r\n");

	GUI_Init();					   // emWin 初始化
	GUI_SetTextMode(GUI_TM_TRANS); //透明方式显示字体，即无背景
								   //		TIM8_Config(1000,12000);   //配置定时器8，0.5秒中断一次

	TIM3_Config(200, 2400); // 2mS

	// LCD_Fill_Pic(0,0,800,480,(u32*)gImage_part);//显示一张图片
	LCD_Fill_Pic((LCD_GetXSize() - 384) / 2, (LCD_GetYSize() - 384) / 2, 384, 384, (u32 *)gImage_deej_logo); //显示一张图片

	GUI_SetFont(&GUI_FontComic24B_ASCII);
	GUI_SetColor(GUI_WHITE);
	GUI_DispStringHCenterAt("Hello, welcome to TK499!", LCD_GetXSize() / 2, (LCD_GetYSize() + 384) / 2 + 10);

	GUI_Delay(100);

	//		SDIO_Init();//SD卡的功能已经就绪，但没插卡时一般不开启
	//		scan_TFCard_CTP_key();//按键或者触摸屏上下翻，按中键退出，设置上限是200张图片，支持16、24、32位的位图，支持奇数分辨率
	//		display_picture("8.bmp");
	//		Test_SDcard_read_TXT();

	//		Lcd_ColorBox(0,0,XSIZE_PHYS,YSIZE_PHYS,White);//用矩形填充函数清屏，填充的大小与屏分辨率一样就行
	//		speed_test();//速度测试

	//		GUI_DrawGradientV(0, 0, LCD_GetXSize(), LCD_GetYSize(), 0xFF8080, 0x8080FF);//全屏显示渐变色

	//		//============= 外置XBF字体测试 包含全字库 (注意；测试全字库，要把字库下载进去才行，下载字库与下载程序方式一样，参考评估板下载说明)=============//
	//		XBF_Disp();//宋体16*16

	//		XKF24_Disp();//经典行楷繁
	//		XKJ32_Disp();//迷你简行楷32
	//		XKF_NumAllDisp();//数码管字体
	//		GUI_Delay(500);//延时一会儿看看效果
	//
	//		//============= 用取模到内部数组方式显示少量汉字 =============//
	//		innner_HZ();

	//
	//		COLOR_ShowColorBar();//显示一个彩条
	//		GUI_Delay(100);
	CreateDeej();
	// printf("CreateDeej\r\n");
	while (1) //几个子例程，可以逐个释放出来用
	{

		DeejSendSliderValues();

		GUI_Delay(10);

		//				WIDGET_ButtonRound();
		//				WIDGET_NumPad();
		//				VGA_Demonstration();
		//				WIDGET_Effect();
		//				SKINNING_NestedModal();
	}
}

// void speed_test(void)//特定分辨率简化算法
//{
//	volatile int i,j;
//
//	for(j=0;j<700;j+=120)
//	{
//		LCD_Fill_Pic(j,0,100,100,(u32*)gImage_temper_pic);
//		LCD_Fill_Pic(j,120,100,100,(u32*)gImage_tell);
//
//		LCD_Fill_Pic(j,240,100,100,(u32*)gImage_world);
//		LCD_Fill_Pic(j,360,100,100,(u32*)gImage_timer_pic);
//		i=2000000;while(--i);
//		Lcd_ColorBox(0,0,XSIZE_PHYS,YSIZE_PHYS,White);
//	}
//
//	for(j=0;j<700;j+=120)
//	{
//		LCD_Fill_Pic(j,0,100,100,(u32*)gImage_temper_pic);
//		LCD_Fill_Pic(j,120,100,100,(u32*)gImage_tell);
//
//		LCD_Fill_Pic(j,240,100,100,(u32*)gImage_world);
//		LCD_Fill_Pic(j,360,100,100,(u32*)gImage_timer_pic);
//
//	}
//	i=2000000;while(--i);
//		Lcd_ColorBox(0,0,XSIZE_PHYS,YSIZE_PHYS,White);
//
//	LCD_Fill_Pic(0,0,100,100,(u32*)gImage_temper_pic);
//	LCD_Fill_Pic(0,380,100,100,(u32*)gImage_tell);
//
//	LCD_Fill_Pic(700,0,100,100,(u32*)gImage_world);
//	LCD_Fill_Pic(700,380,100,100,(u32*)gImage_timer_pic);
//
//	LCD_Fill_Pic(350,190,100,100,(u32*)gImage_set);
//
//	i=5000000;while(--i);
//	Lcd_ColorBox(0,0,XSIZE_PHYS,YSIZE_PHYS,White);
//	for(i=0;i<195;i+=5)
//	{
//		LCD_Fill_Pic(i*35/19,i,100,100,(u32*)gImage_set);
//		LCD_Fill_Pic(700-i*35/19,i,100,100,(u32*)gImage_set);
//
//		LCD_Fill_Pic(i*35/19,380-i,100,100,(u32*)gImage_set);
//		LCD_Fill_Pic(700-i*35/19,380-i,100,100,(u32*)gImage_set);
//	}
//	i=245;
//	LCD_Fill_Pic(i*35/19,i,100,100,(u32*)gImage_temper_pic);
//		LCD_Fill_Pic(700-i*35/19,i,100,100,(u32*)gImage_tell);
//
//		LCD_Fill_Pic(i*35/19,380-i,100,100,(u32*)gImage_world);
//		LCD_Fill_Pic(700-i*35/19,380-i,100,100,(u32*)gImage_timer_pic);
//	i=5000000;while(--i);
//
//
//		//下面测试内容要耗时约6秒，需要请开启
////	printf("\r\n  start  \r\n");
////	for(i=0;i<100;i++)
////	{
////		Lcd_ColorBox(0,0,XSIZE_PHYS,YSIZE_PHYS,Red);
////		Lcd_ColorBox(0,0,XSIZE_PHYS,YSIZE_PHYS,Green);
////		Lcd_ColorBox(0,0,XSIZE_PHYS,YSIZE_PHYS,Blue);
////	}
////	printf("\r\n  end  \r\n");
//
//	//下面测试内容要耗时约12秒，需要请开启
//
//	//在函数开头结束都打印字符，利用串口助手显示时间戳就可以知道，移动480帧（800-320=480）耗时约12秒
//	//实际速度可能还快一点点，因为背景擦除去了一点时间
//	//也就是说显示一个320x480的图标一秒约能显示40个，一般图标没那么大，例如100x100的侧可以每秒显示614个，一般应用够了
////	printf("\r\n  start  \r\n");
////	LCD_Picture_Move(1, 0, 0,320, 480, (u32*)gImage_MM_T035,Blue);//显示一幅从左到右移动的图片
////	printf("\r\n  end  \r\n");
//}
