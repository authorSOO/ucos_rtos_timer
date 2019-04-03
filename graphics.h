#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#define BLACK		0x0000
#define WHITE		0xffff
#define BLUE		0x001f
#define GREEN		0x07e0
#define RED			0xf800
#define YELLOW	(RED|GREEN)
#define CYAN		(GREEN|BLUE)
#define MAGENTA	(RED|BLUE)

typedef enum
{
	PTN1, PTN2
}PTN_TYPE;

void Lcd_PutPixel(int x,int y,u16 color);
void Lcd_Clrscr(void);

void Lcd_Hline(int y, int x1, int x2, int color);
void Lcd_Vline(int x, int y1, int y2, int color);
void Lcd_Rectangular(int x1, int y1, int x2, int y2, int color);
void Lcd_Bar(int x1, int y1, int x2, int y2, int color);

void Lcd_EngPutchxyh(int x,int y,int color,int bkcolor,int data, int zx, int zy);
void Lcd_HanPutchxyh(int x,int y,int color,int bkcolor, int data, int zx, int zy);
void Lcd_Putsxyh(int x, int y, int color, int bkcolor, char *str, int zx, int zy);
void Lcd_Printf(int x, int y, int color, int bkcolor, int zx, int zy, char *fmt,...);

//void Lcd_DrawBMP(int x, int y, const unsigned char *fp);

/* Newly added functions */
void Lcd_SetTranMode(int mode);
void Lcd_DrawBackColor(int color);
void Lcd_Line(int x1, int y1, int x2, int y2, int color);
//void Lcd_GetInfoBMP(int * x, int  * y, const unsigned char *fp);
void Lcd_GetInfoBMP(int * x, int  * y, const unsigned short int *fp);
void Lcd_DrawBMP(int x, int y, const unsigned short int *fp);

void Lcd_DrawLine(int x1, int y1, int x2, int y2, u32 cvalue);
void Lcd_FillRectangle(int x1, int y1, int dx, int dy, u32 cvalue);
void Lcd_PutRectangle(int x1, int y1, int dx, int dy, u32 cvalue);
void Lcd_DrawPattern(u32 uPosX, u32 uPosY, u32 uHsz, u32 uVsz, PTN_TYPE eType);

extern unsigned int bFbSel;
extern unsigned int ArrFbSel[MAX_WINDOW_NUM];
extern unsigned int Trans_Mode;


#endif
