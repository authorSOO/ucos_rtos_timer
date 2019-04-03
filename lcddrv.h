#ifndef __2440LCD_H__
#define __2440LCD_H__


#define LCDFRAMEBUFFER 	0x33800000 //_NONCACHE_STARTADDRESS 

// 16bit 5-6-5
#define	RGB(R,G,B)      (((R)<<11)|((G)<<5)|(B))

#ifdef _LCD_480x272
#define LCD_XSIZE 	  (480)
#define LCD_YSIZE     (272)
#else
#define LCD_XSIZE 	  (320)
#define LCD_YSIZE     (240)
#endif

#define SCR_XSIZE 	  (LCD_XSIZE*1)   //for virtual screen  
#define SCR_YSIZE	  (LCD_YSIZE*1)

//=========================================================================
// MACRO       : LOW21()
// Description : 하위 21비트만 남기고 나머지는 클리어한다.
// Param       : n - 대상 숫자
//=========================================================================
#define LOW21(n) 		((n) & 0x1fffff)	

//=========================================================================
// Variables
// Frame buffer and Memory buffer
//=========================================================================
typedef unsigned char (*FB_ADDR)[LCD_XSIZE];

// CALC IMAGE data
#define MAGICNUMBER 0x12345678

typedef char* VIMAGE;
typedef struct tagSWAPbuf
{
	int x;	// x position
	int y;	// y position
	int w;	// width
	int h;	// height
	int size;
//	VIMAGE* P;	// swap buffer
	unsigned int P[1024*4];// swap buffer
	int use;	// buffer use
}sSWAPbuf;

typedef struct tagIMAGEINFO
{
	int magicNumber; // 0x12345678
	int orig_x;	// virtual original x axis
	int orig_y;	// virtual original y axis
	int x;	// x position
	int y;	// y position
	int w;	// width
	int h;	// height
	char* vpImageBMP;	// image data(BMP)
	char* voImageBMP;	// image data(BMP)
	sSWAPbuf sb;	// swap buffer
	int SAVEON;	// save on flag
}sIMAGEINFO;

//LCD config

#define MODE_TFT_16BIT    (0x4104)


#define HOZVAL		(LCD_XSIZE-1)
#define LINEVAL		(LCD_YSIZE-1)

#define VBPD (10)	
#define VFPD (1)	
#define VSPW (1)	
#define HBPD (43)	
#define HFPD (1)	
#define HSPW (1)	

#define CLKVAL	13 

// Macro

#define COPY(A,B) for(loop=0;loop<32;loop++) *(B+loop)=*(A+loop);
#define OR(A,B) for(loop=0;loop<32;loop++) *(B+loop)|=*(A+loop);

// LCD
void Lcd_Init(void);
void Lcd_Envid_On_Off(int onoff);
void Lcd_Power_Enable(int invpwren,int pwren);
void Lcd_Port_Init(void);
void Lcd_Set_Address(unsigned int fp);

// Graphic

void Lcd_Han_Putch(int x,int y,int color,int bkcolor,int data, int zx, int zy);
void Lcd_Eng_Putch(int x,int y,int color,int bkcolor,int data, int zx, int zy);
void Lcd_Puts(int x, int y, int color, int bkcolor, char *str, int zx, int zy);

void Graphic_Init(void);
void Lcd_Wait_Blank(void);
void Lcd_Copy(unsigned from, unsigned int to);
void Lcd_Select_Frame_Buffer(unsigned int id);
void Lcd_Display_Frame_Buffer(unsigned int id);
void Lcd_Set_Tran_Mode(int mode);
void Lcd_Put_Pixel(int x,int y,int c);
void Lcd_Clr_Screen(unsigned long color);
void Lcd_Hline(int y, int x1, int x2, int color);
void Lcd_Vline(int x, int y1, int y2, int color);
void Lcd_Line(int x1,int y1,int x2,int y2,int color);
void Lcd_Rectangular(int x1, int y1, int x2, int y2, int color);
void Lcd_Fill_Rectangular(int x1, int y1, int x2, int y2, int color);
void Lcd_Printf(int x, int y, int color, int bkcolor, int zx, int zy, char *fmt,...);
void Lcd_Get_Info_BMP(int * x, int  * y, const unsigned char *fp);
void Lcd_Draw_BMP(int x, int y, const unsigned char *fp);
void Lcd_GetINFO_BMP(int x, int y, const unsigned char *fp);
void Lcd_Draw_BUTTON(int button);

// virtual lcd printf
void VLcd_Printf(int x, int y, int color, int bkcolor, int zx, int zy, char *fmt,...);

#endif /*__2440LCD_H__*/