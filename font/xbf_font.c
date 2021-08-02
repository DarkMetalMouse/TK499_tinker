#include "main.h"
#include "xbf_font.h"


/*本C文件使用的是UTF_8编码格式*/

/*================================ ST16  宋体16*16 ================================*/
GUI_FONT stFont;
static GUI_XBF_DATA xbfData;
static int stFont_addr;
static int _cbGetData(U32 off,U16 NumBytes, void *pVoid, void *pBuffer)
{
	QSPIReadGroup(stFont_addr + off, pBuffer, NumBytes);
	return 0;
}

void CreateXBF_Font(void)
{
	GUI_XBF_CreateFont(&stFont,&xbfData,GUI_XBF_TYPE_PROP,_cbGetData,0);
}

GUI_FONT * SetFont_Xbf(void)
{
  return ((GUI_FONT *)GUI_SetFont(&stFont));
}



/*================================ 经典行楷繁 24*24 ================================*/
static GUI_FONT xkfFont;
static GUI_XBF_DATA xkfData;
static int XKF24_Font_addr;
static int _cbGetDataXKF(U32 off,U16 NumBytes, void *pVoid, void *pBuffer)
{
	QSPIReadGroup(XKF24_Font_addr + off, pBuffer, NumBytes);
	return 0;
}

void CreateXBF_FontXKF24(void)
{
	GUI_XBF_CreateFont(&xkfFont,&xkfData,GUI_XBF_TYPE_PROP,_cbGetDataXKF,0);
}

GUI_FONT * SetFont_XbfXKF24(void)
{
    return ((GUI_FONT *)GUI_SetFont(&xkfFont));
}

/*================================ xkj32  行楷简32*32 ================================*/
static GUI_FONT xkjFont;
static GUI_XBF_DATA xkjData;
static int xkj32_Font_addr;
static int _cbGetDataXKJ(U32 off,U16 NumBytes, void *pVoid, void *pBuffer)
{
	QSPIReadGroup(xkj32_Font_addr + off, pBuffer, NumBytes);
	return 0;
}

void CreateXBF_FontXKJ32(void)
{
	GUI_XBF_CreateFont(&xkjFont,&xkjData,GUI_XBF_TYPE_PROP,_cbGetDataXKJ,0);
}

GUI_FONT * SetFont_XbfXKJ32(void)
{
    return ((GUI_FONT *)GUI_SetFont(&xkjFont));
}







/*================================ 数码管 24*24 ================================*/
static GUI_FONT smgFont24;
static GUI_XBF_DATA smgData24;
static int SMG24_Font_addr;
static int _cbGetDataSMG24(U32 off,U16 NumBytes, void *pVoid, void *pBuffer)
{
	QSPIReadGroup(SMG24_Font_addr + off, pBuffer, NumBytes);
	return 0;
}

void CreateXBF_FontSMG24(void)
{
	GUI_XBF_CreateFont(&smgFont24,&xkfData,GUI_XBF_TYPE_PROP,_cbGetDataSMG24,0);
}

GUI_FONT * SetFont_XbfSMG24(void)
{
    return ((GUI_FONT *)GUI_SetFont(&smgFont24));
}



/*================================ 数码管 36*36 ================================*/
static GUI_FONT smgFont36;
static GUI_XBF_DATA smgData36;
static int SMG36_Font_addr;
static int _cbGetDataSMG36(U32 off,U16 NumBytes, void *pVoid, void *pBuffer)
{
	QSPIReadGroup(SMG36_Font_addr + off, pBuffer, NumBytes);
	return 0;
}

void CreateXBF_FontSMG36(void)
{
	GUI_XBF_CreateFont(&smgFont36,&xkfData,GUI_XBF_TYPE_PROP,_cbGetDataSMG36,0);
}

GUI_FONT * SetFont_XbfSMG36(void)
{
	return ((GUI_FONT *)GUI_SetFont(&smgFont36));
}



/*================================ 数码管 48*48 ================================*/
static GUI_FONT smgFont48;
static GUI_XBF_DATA smgData48;
static int SMG48_Font_addr;
static int _cbGetDataSMG48(U32 off,U16 NumBytes, void *pVoid, void *pBuffer)
{
	QSPIReadGroup(SMG48_Font_addr + off, pBuffer, NumBytes);
	return 0;
}

void CreateXBF_FontSMG48(void)
{
	GUI_XBF_CreateFont(&smgFont48,&xkfData,GUI_XBF_TYPE_PROP,_cbGetDataSMG48,0);
}

GUI_FONT * SetFont_XbfSMG48(void)
{
  return ((GUI_FONT *)GUI_SetFont(&smgFont48));
}



/*================================ 数码管 64*64 ================================*/
static GUI_FONT smgFont64;
static GUI_XBF_DATA smgData64;
static int SMG64_Font_addr;
static int _cbGetDataSMG64(U32 off,U16 NumBytes, void *pVoid, void *pBuffer)
{
	QSPIReadGroup(SMG64_Font_addr + off, pBuffer, NumBytes);
	return 0;
}

void CreateXBF_FontSMG64(void)
{
	GUI_XBF_CreateFont(&smgFont64,&xkfData,GUI_XBF_TYPE_PROP,_cbGetDataSMG64,0);
}

GUI_FONT * SetFont_XbfSMG64(void)
{
    return ((GUI_FONT *)GUI_SetFont(&smgFont64));
}



void init_all_Font_addr(void)
{
	stFont_addr=get_file_address_NOR_FLASH("ST16.xbf");
	XKF24_Font_addr=get_file_address_NOR_FLASH("JDXKF24.xbf");
	xkj32_Font_addr=get_file_address_NOR_FLASH("JXK32.xbf");

	SMG24_Font_addr=get_file_address_NOR_FLASH("SMG24.XBF");
	SMG36_Font_addr=get_file_address_NOR_FLASH("SMG36.XBF");
	SMG48_Font_addr=get_file_address_NOR_FLASH("SMG48.XBF");
	SMG64_Font_addr=get_file_address_NOR_FLASH("SMG64.XBF");	
}

//========== 存储在外部QSPI FLASH里的全字库 字体 ==========//
void XBF_Disp(void)//宋体16*16
{
	
	CreateXBF_Font();         //create extern flash font lib
	GUI_UC_SetEncodeUTF8();
	SetFont_Xbf();
	GUI_SetColor(GUI_RED);
	GUI_DispStringHCenterAt("欢迎使用好钜润科技产品",240,210);
	GUI_DispStringHCenterAt("当前全字库存储在外部QSPI FLASH中",240,230);
  GUI_DispStringHCenterAt("emWin外部XBF字库演示",240,250);
}
void XKF24_Disp(void)//经典行楷繁
{
	CreateXBF_FontXKF24();         //create extern flash font lib
	GUI_UC_SetEncodeUTF8();
	SetFont_XbfXKF24();
	GUI_SetColor(GUI_BLUE);
  GUI_DispStringHCenterAt("emWin外部XBF字库演示",240,260);
}
void XKJ32_Disp(void)//迷你简行楷32
{
	CreateXBF_FontXKJ32();         //create extern flash font lib
	GUI_UC_SetEncodeUTF8();
	SetFont_XbfXKJ32();
	GUI_SetColor(GUI_GREEN);

  GUI_DispStringHCenterAt("emWin外部XBF字库演示",240,286);
	GUI_DispStringHCenterAt("荡胸生层云，决眦入归鸟",300,360);
	GUI_DispStringHCenterAt("会当凌绝顶，一览众山小",300,400);
}




void XKF_NumAllDisp(void)
{
	CreateXBF_FontSMG24();         //create extern flash font lib
	GUI_UC_SetEncodeUTF8();
	SetFont_XbfSMG24();
	GUI_SetColor(GUI_RED);
	GUI_DispStringHCenterAt("012345678",240,0);
	
	CreateXBF_FontSMG36();         //create extern flash font lib
	GUI_UC_SetEncodeUTF8();
	SetFont_XbfSMG36();
	GUI_SetColor(GUI_GREEN);
	GUI_DispStringHCenterAt("012345678",240,30);
	
	
  CreateXBF_FontSMG48();         //create extern flash font lib
	GUI_UC_SetEncodeUTF8();
	SetFont_XbfSMG48();
	GUI_SetColor(GUI_BLUE);
	GUI_DispStringHCenterAt("012345678",240,80);
	
	
  CreateXBF_FontSMG64();         //create extern flash font lib
	GUI_UC_SetEncodeUTF8();
	SetFont_XbfSMG64();
	GUI_SetColor(GUI_YELLOW);
	GUI_DispStringHCenterAt("012345678",240,132);
}
extern GUI_CONST_STORAGE GUI_FONT GUI_FontHZ_HWXW24;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontFont_Num64;
//内部数组，取模方式字体
void innner_HZ(void)
{
	int i;
		GUI_DrawGradientV(0, 0, LCD_GetXSize(), LCD_GetYSize(), 0xFF8080, 0x8080FF);//全屏显示渐变色
		GUI_UC_SetEncodeUTF8();//显示中文字体前，要先设置编码格式是UTF8
		GUI_SetFont(&GUI_FontHZ_HWXW24);//设置字体是：GUI_FontHZ_HWXW24，这个字体是自己取模的中文字模
		GUI_SetColor(GUI_WHITE);//设置前景色为白色

		for(i=0;i<LCD_GetYSize()-30;i+=30)
		{
			GUI_DispStringAt("深圳市好钜润科技有限公司",10,10+i);
			GUI_DispStringAt("深圳市好钜润科技有限公司",450,10+i);
		}
		
		GUI_Delay(100);//延时一会儿看看效果
}

//		GUI_SetFont(&GUI_Font32_ASCII);//设置字体
//		GUI_SetBkColor(GUI_LIGHTBLUE);//设置背景色为淡蓝色
//		GUI_SetColor(GUI_BLACK);//设置前景色为黑色
//		GUI_DispStringAt("Hello,Welcome to use TK499!",220,50);
//		GUI_Delay(200);//延时一会儿看看效果
//		GUI_SetBkColor(GUI_BLUE);
//    GUI_Clear();