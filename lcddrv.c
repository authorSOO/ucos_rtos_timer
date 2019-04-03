/*******************************************************************************
**	File:
**
**		lcddrv.c
**
**  Contents:
**		uC/OS-II programming
**
**	History:
**		Date        Name              Description
**		------------------------------------------------------------------------
**
*******************************************************************************/

#include "2450addr.h"
#include "option.h"
#include "macro.h"
#include <stdio.h>
#include <stdarg.h>
//#include <alloca.h>
#include <limits.h>
#include "includes.h"
#include "ui.h"
#include "lcddrv.h"

#include "fonts/ENG8X16.H"
#include "fonts/HAN16X16.H"
#include "fonts/HANTABLE.H"

#define  NonPal_Fb   ((volatile unsigned short(*)[480]) FRAME_BUFFER)

// button key for calculator {x,y,width,height}
const unsigned int 
TouchKey_pTable[][4]= {
	{76,199,42,27}, {76,169,42,27}, {122,169,42,27}, {167,169,42,27}, 
	{76,139,42,27},	{122,139,42,27}, {167,139,42,27}, {76,109,42,27}, 
	{122,109,42,27}, {167,109,42,27}, {0,0,0,0}, {76,76,74,27}, 
	{153,76,74,27}, {228,76,74,27},	{122,199,42,27}, {0,0,0,0},
	{213,109,42,27}, {213,139,42,27}, {213,169,42,27}, {213,199,42,27},
	{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {258,199,42,27}
};

static unsigned char _first[]={0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19 };
static unsigned char _middle[]={0,0,0,1,2,3,4,5,0,0,6,7,8,9,10,11,0,0,12,13,14,15,16,17,0,0,18,19,20,21};
static unsigned char _last[]={0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,0,17,18,19,20,21,22,23,24,25,26,27};
static unsigned char cho[]={0,0,0,0,0,0,0,0,0,1,3,3,3,1,2,4,4,4,2,1,3,0};
static unsigned char cho2[]={0,5,5,5,5,5,5,5,5,6,7,7,7,6,6,7,7,7,6,6,7,5};
static unsigned char jong[]={0,0,2,0,2,1,2,1,2,3,0,2,1,3,3,1,2,1,3,3,1,1};

// 4MB align ÇÊ¿ä (0x400000ÀÇ ¹è¼ö)
static unsigned int Fbuf[2] = {0x33800000, 0x33C00000};
unsigned int (* Fb_ptr)[LCD_XSIZE/2];

//unsigned int (* Fb_ptr)[480/2];

static int Trans_mode= 0;

static unsigned short bfType;
static unsigned int bfSize;
static unsigned int bfOffbits;
static unsigned int biWidth, biWidth2;
static unsigned int biHeight;

sIMAGEINFO calcImg;

// Functions

void Lcd_Envid_On_Off(int onoff)
{
	if(onoff)
	{
		rVIDCON0 |= (0x3<<0);
	}
	else
	{
		rVIDCON0 |=  (0x1<<1);
		rVIDCON0 &= ~(0x1<<0);
	}
}    

void Lcd_Power_Enable(int invpwren,int pwren)
{
	(void)invpwren;
	(void)pwren;
}    

void Lcd_Port_Init(void)
{
	rGPCUDP = 0xffffffff; 
	rGPCCON &= ~(0xffffffff);
	rGPCCON |= 0xaaaa02aa; //GPC5,6,7 = output
	
	rGPDUDP = 0xffffffff; 
	rGPDCON &= ~(0xffffffff);
	rGPDCON |= 0xaaaaaaaa;
	
	/* GPG12 is setted as LCD_Display ON/OFF */
	rGPGCON=rGPGCON&(~(3<<24))|(1<<24); //GPG12=OUTPUT
	rGPGDAT|=(1<<12);
	
	/* GPB0 backlight Enable */
	rGPBCON=rGPBCON&(~(3<<0))|(1<<0);
	rGPBDAT|= (1<<0);
	
	
}

void NonPal_Lcd_Init()
{		
	rVIDW00ADD0B0 = FRAME_BUFFER;
	rVIDW00ADD1B0 = 0;
	rVIDW00ADD2B0 = (0<<13)|((LCD_XSIZE*4*2)&0x1fff);

	rVIDW00ADD1B0 = 0+(LCD_XSIZE*LCD_YSIZE);
	
	/* TO DO : setting for LCD control 
	* RGB I/F,PNRMODE(BGR),VCLK=9MHz,VCLK=Enable,CLKSEL=HCLK,CLKDIR=Divided,ENVID=disable
	*/
	rVIDCON0=(0x0<<22)+(0x1<<13)+((CLKVAL)<<6)+(1<<5)+(1<<4)+(0<<2);
	rVIDCON1= (1<<6)+(1<<5);
	
	rVIDTCON0=((VBPD)<<16)+((VFPD)<<8)+(VSPW);
	rVIDTCON1=((HBPD)<<16)+((HFPD)<<8)+(HSPW);
	rVIDTCON2=(LINEVAL<<11)+(HOZVAL);
	
	rVIDOSD0A		= 	(((0)&0x7FF)<<11) | (((0)&0x7FF)<<0);
	rVIDOSD0B	 	= 	(((LCD_XSIZE-1)&0x7FF)<<11) | (((LCD_YSIZE-1)&0x7FF)<<0);

	rVIDOSD1A 		= 	(((0)&0x7FF)<<11) | (((0)&0x7FF)<<0);
	rVIDOSD1B	 	= 	(((LCD_XSIZE-1)&0x7FF)<<11) | (((LCD_YSIZE-1)&0x7FF)<<0);
	/* TO DO 
	* Half swap Enable, 5:5:5:I format, Window0 ON
	*/
	rWINCON0=(0x0<<16)+(0x7<<2)+(1<<0); 
	
	/* TO DO : ENVID Enable for video output and LCD control signal */
	rVIDCON0 |= (3<<0);	

     	Lcd_Select_Frame_Buffer(0);
     	Lcd_Display_Frame_Buffer(0);
}

void Graphic_Init(void)
{    
	int i,j;

	Lcd_Port_Init();
	NonPal_Lcd_Init();

	Lcd_Power_Enable(1,1);
	Lcd_Clr_Screen(WHITE);
	Lcd_Envid_On_Off(1);   
}

#define M5D(n) ((n) & 0x1fffff)

void Lcd_Set_Address(unsigned int fp)
{
	rVIDW00ADD0B0=((fp>>22)<<21);
	rVIDW00ADD1B0=M5D(fp+(LCD_XSIZE*LCD_YSIZE));
	rVIDW00ADD2B0=LCD_XSIZE;
}

void Lcd_Copy(unsigned from, unsigned int to)
{
	unsigned int i;

	for(i=0; i< (LCD_XSIZE / 2 * LCD_YSIZE) ; i++)
	{
		*((unsigned int *)(Fbuf[to])+i) = *((unsigned int *)(Fbuf[from])+i);
	}
}

void Lcd_Select_Frame_Buffer(unsigned int id)
{
     	Fb_ptr = (unsigned int (*)[LCD_XSIZE/2])Fbuf[id];
}

void Lcd_Display_Frame_Buffer(unsigned int id)
{
//	Lcd_Set_Address(Fbuf[id]);
}

void Lcd_Put_Pixel(int x,int y,int color)
{
    if(!Trans_mode)
    {
    	if((x%2)) // odd number
    	{
        	Fb_ptr[y][x/2]=( Fb_ptr[y][x/2] & 0xffff0000) | (color & 0x0000ffff);
        }
        else
        {
        	Fb_ptr[y][x/2]=( Fb_ptr[y][x/2] & 0x0000ffff) | ((color & 0x0000ffff) << 16);
        }
    }
    
    else
    {
    	if((x%2)) // odd number
    	{
        	Fb_ptr[y][x/2]= Fb_ptr[y][x/2] | (color & 0x0000ffff);
        }
        else
        {
        	Fb_ptr[y][x/2]= Fb_ptr[y][x/2] | ((color & 0x0000ffff) << 16);
        }
    }    
}

// Lcd_SetTranMode
void Lcd_Set_Tran_Mode(int mode)
{
	if(mode) Trans_mode = 1;
	else Trans_mode = 0;	
}

// Clear screen
void Lcd_Clr_Screen(unsigned long color)
{
	int i,j;

	for(j=0;j<SCR_YSIZE;j++)
	{
		for(i=0;i<SCR_XSIZE;i++)
		{
	        Lcd_Put_Pixel(i,j,(int)color);
		}
	}
}

// Lcd_Line
void Lcd_Line(int x1,int y1,int x2,int y2,int color)
{
	int dx,dy,e;
	dx=x2-x1; 
	dy=y2-y1;
    
	if(dx>=0)
	{
		if(dy >= 0) // dy>=0
		{
			if(dx>=dy) // 1/8 octant
			{
				e=dy-dx/2;
				while(x1<=x2)
				{
					Lcd_Put_Pixel(x1,y1,color);
					if(e>0){y1+=1;e-=dx;}	
					x1+=1;
					e+=dy;
				}
			}
			else		// 2/8 octant
			{
				e=dx-dy/2;
				while(y1<=y2)
				{
					Lcd_Put_Pixel(x1,y1,color);
					if(e>0){x1+=1;e-=dy;}	
					y1+=1;
					e+=dx;
				}
			}
		}
		else		   // dy<0
		{
			dy=-dy;   // dy=abs(dy)

			if(dx>=dy) // 8/8 octant
			{
				e=dy-dx/2;
				while(x1<=x2)
				{
					Lcd_Put_Pixel(x1,y1,color);
					if(e>0){y1-=1;e-=dx;}	
					x1+=1;
					e+=dy;
				}
			}
			else		// 7/8 octant
			{
				e=dx-dy/2;
				while(y1>=y2)
				{
					Lcd_Put_Pixel(x1,y1,color);
					if(e>0){x1+=1;e-=dy;}	
					y1-=1;
					e+=dx;
				}
			}
		}	
	}
	else //dx<0
	{
		dx=-dx;		//dx=abs(dx)
		if(dy >= 0) // dy>=0
		{
			if(dx>=dy) // 4/8 octant
			{
				e=dy-dx/2;
				while(x1>=x2)
				{
					Lcd_Put_Pixel(x1,y1,color);
					if(e>0){y1+=1;e-=dx;}	
					x1-=1;
					e+=dy;
				}
			}
			else		// 3/8 octant
			{
				e=dx-dy/2;
				while(y1<=y2)
				{
					Lcd_Put_Pixel(x1,y1,color);
					if(e>0){x1-=1;e-=dy;}	
					y1+=1;
					e+=dx;
				}
			}
		}
		else		   // dy<0
		{
			dy=-dy;   // dy=abs(dy)

			if(dx>=dy) // 5/8 octant
			{
				e=dy-dx/2;
				while(x1>=x2)
				{
					Lcd_Put_Pixel(x1,y1,color);
					if(e>0){y1-=1;e-=dx;}	
					x1-=1;
					e+=dy;
				}
			}
			else		// 6/8 octant
			{
				e=dx-dy/2;
				while(y1>=y2)
				{
					Lcd_Put_Pixel(x1,y1,color);
					if(e>0){x1-=1;e-=dy;}	
					y1-=1;
					e+=dx;
				}
			}
		}	
	}
}

// Lcd_Hline
void Lcd_Hline(int y, int x1, int x2, int color)
{
     int i, xx1, xx2;
     
     if(x1<x2)
     {
     	xx1=x1;
     	xx2=x2;
     }
     else 
     {
     	xx1=x2;
     	xx2=x1;
     } 
     for(i=xx1;i<=xx2;i++)
     {
         Lcd_Put_Pixel(i,y,color);
     }
} 

// Lcd_Vline
void Lcd_Vline(int x, int y1, int y2, int color)
{
     int i, yy1, yy2;
     
     if(y1<y2)
     {
     	yy1=y1;
     	yy2=y2;
     }
     else 
     {
     	yy1=y2;
     	yy2=y1;
     }	
     for(i=yy1;i<=yy2;i++)
     {
         Lcd_Put_Pixel(x,i,color);	
     }
}	

// Lcd_Rectangular
void Lcd_Rectangular(int x1, int y1, int x2, int y2, int color)
{
     int xx1, yy1, xx2, yy2;
     
     if(x1<x2)
     {
     	xx1=x1;
     	xx2=x2;
     }
     else 
     {
     	xx1=x2;
     	xx2=x1;
     } 
     
     if(y1<y2)
     {
     	yy1=y1;
     	yy2=y2;
     } 
     else 
     {
     	yy1=y2;
     	yy2=y1;
     } 
	 
     Lcd_Hline(yy1,xx1,xx2,color);
     Lcd_Hline(yy2,xx1,xx2,color);
     Lcd_Vline(xx1,yy1,yy2,color);
     Lcd_Vline(xx2,yy1,yy2,color);
} 

// Lcd_Bar
void Lcd_Fill_Rectangular(int x1, int y1, int x2, int y2, int color)
{
     int i, j;
     int xx1, yy1, xx2, yy2;
     
     if(x1<x2)
     {
     	xx1=x1;
     	xx2=x2;
     }
     else 
     {
     	xx1=x2;
     	xx2=x1;
     }
     
     if(y1<y2)
     {
     	yy1=y1;
     	yy2=y2;
     } 
     else 
     {
     	yy1=y2;
     	yy2=y1;
     }
     
     for(i=yy1;i<=yy2;i++)
     {
         for(j=xx1;j<=xx2;j++)
         {
             Lcd_Put_Pixel(j,i,color);
         }
     }
}


// Lcd_Han_Putch
void Lcd_Han_Putch(int x,int y,int color,int bkcolor, int data, int zx, int zy)
{
	unsigned int first,middle,last;	
	unsigned int offset,loop;
	unsigned char xs,ys;
	unsigned char temp[32];
	unsigned char bitmask[]={128,64,32,16,8,4,2,1};     

	first=(unsigned)((data>>8)&0x00ff);
	middle=(unsigned)(data&0x00ff);
	offset=(first-0xA1)*(0x5E)+(middle-0xA1);
	first=*(HanTable+offset*2);
	middle=*(HanTable+offset*2+1);
	data=(int)((first<<8)+middle);    

	first=_first[(data>>10)&31];
	middle=_middle[(data>>5)&31];
	last=_last[(data)&31];     

	if(last==0)
	{
		offset=(unsigned)(cho[middle]*640); 
		offset+=first*32;
		COPY(han16x16+offset,temp);

		if(first==1||first==24) offset=5120;  
		else offset=5120+704;
		offset+=middle*32;
		OR(han16x16+offset,temp);
	}
	else 
	{
		offset=(unsigned)(cho2[middle]*640); 
		offset+=first*32;
		COPY(han16x16+offset,temp);

		if(first==1||first==24) offset=5120+704*2; 
		else offset=5120+704*3;
		offset+=middle*32;
		OR(han16x16+offset,temp);

		offset=(unsigned)(5120+2816+jong[middle]*896);
		offset+=last*32;
		OR(han16x16+offset,temp);
	}

	for(ys=0;ys<16;ys++)
	{
		for(xs=0;xs<8;xs++)
		{
			if(temp[ys*2]&bitmask[xs])
			{
				if( (zx==1)&&(zy==1) ) Lcd_Put_Pixel(x+xs,y+ys,color);
				else if( (zx==2)&&(zy==1) )
				{
					Lcd_Put_Pixel(x+2*xs,y+ys,color);
					Lcd_Put_Pixel(x+2*xs+1,y+ys,color);
				}
				else if( (zx==1)&&(zy==2) )
				{
					Lcd_Put_Pixel(x+xs,y+2*ys,color);
					Lcd_Put_Pixel(x+xs,y+2*ys+1,color);
				}
				else if( (zx==2)&&(zy==2) )
				{
					Lcd_Put_Pixel(x+2*xs,y+2*ys+1,color);
					Lcd_Put_Pixel(x+2*xs+1,y+2*ys,color);
					Lcd_Put_Pixel(x+2*xs,y+2*ys,color);
					Lcd_Put_Pixel(x+2*xs+1,y+2*ys+1,color);
				}
			}
			else
			{
				if( (zx==1)&&(zy==1) ) Lcd_Put_Pixel(x+xs,y+ys,bkcolor);
				else if( (zx==2)&&(zy==1) )
				{
					Lcd_Put_Pixel(x+2*xs,y+ys,bkcolor);
					Lcd_Put_Pixel(x+2*xs+1,y+ys,bkcolor);
				}
				else if( (zx==1)&&(zy==2) )
				{
					Lcd_Put_Pixel(x+xs,y+2*ys,bkcolor);
					Lcd_Put_Pixel(x+xs,y+2*ys+1,bkcolor);
				}
				else if( (zx==2)&&(zy==2) )
				{
					Lcd_Put_Pixel(x+2*xs,y+2*ys+1,bkcolor);
					Lcd_Put_Pixel(x+2*xs+1,y+2*ys,bkcolor);
					Lcd_Put_Pixel(x+2*xs,y+2*ys,bkcolor);
					Lcd_Put_Pixel(x+2*xs+1,y+2*ys+1,bkcolor);
				}	   	
			}
		}

		for(xs=0;xs<8;xs++)
		{
			if(temp[ys*2+1]&bitmask[xs])
			{
				if( (zx==1)&&(zy==1) )
				Lcd_Put_Pixel(x+xs+8,y+ys,color);
				else if( (zx==2)&&(zy==1) ){
				Lcd_Put_Pixel(x+2*(xs+8),y+ys,color);
				Lcd_Put_Pixel(x+2*(xs+8)+1,y+ys,color);
				}
				else if( (zx==1)&&(zy==2) ){
				Lcd_Put_Pixel(x+(xs+8),y+2*ys,color);
				Lcd_Put_Pixel(x+(xs+8),y+2*ys+1,color);
				}
				else if( (zx==2)&&(zy==2) ){
				Lcd_Put_Pixel(x+2*(xs+8),y+2*ys+1,color);
				Lcd_Put_Pixel(x+2*(xs+8)+1,y+2*ys,color);
				Lcd_Put_Pixel(x+2*(xs+8),y+2*ys,color);
				Lcd_Put_Pixel(x+2*(xs+8)+1,y+2*ys+1,color);
				}
			}

			else
			{	   	
				if( (zx==1)&&(zy==1) ) Lcd_Put_Pixel(x+xs+8,y+ys,bkcolor);
				else if( (zx==2)&&(zy==1) )
				{
					Lcd_Put_Pixel(x+2*(xs+8),y+ys,bkcolor);
					Lcd_Put_Pixel(x+2*(xs+8)+1,y+ys,bkcolor);
				}
				else if( (zx==1)&&(zy==2) )
				{
					Lcd_Put_Pixel(x+(xs+8),y+2*ys,bkcolor);
					Lcd_Put_Pixel(x+(xs+8),y+2*ys+1,bkcolor);
				}
				else if( (zx==2)&&(zy==2) )
				{
					Lcd_Put_Pixel(x+2*(xs+8),y+2*ys+1,bkcolor);
					Lcd_Put_Pixel(x+2*(xs+8)+1,y+2*ys,bkcolor);
					Lcd_Put_Pixel(x+2*(xs+8),y+2*ys,bkcolor);
					Lcd_Put_Pixel(x+2*(xs+8)+1,y+2*ys+1,bkcolor);
				}	   	
			}
		}
	}
}

// Lcd_Eng_Putch
void Lcd_Eng_Putch(int x,int y,int color,int bkcolor,int data, int zx, int zy)
{
	unsigned offset,loop;
	unsigned char xs,ys;
	unsigned char temp[32];
	unsigned char bitmask[]={128,64,32,16,8,4,2,1};     

	offset=(unsigned)(data*16);
	COPY(eng8x16+offset,temp);

	for(ys=0;ys<16;ys++)
	{
		for(xs=0;xs<8;xs++)
		{
			if(temp[ys]&bitmask[xs])
			{
				if( (zx==1)&&(zy==1) ) Lcd_Put_Pixel(x+xs,y+ys,color);
				else if( (zx==2)&&(zy==1) )
				{
					Lcd_Put_Pixel(x+2*xs,y+ys,color);
					Lcd_Put_Pixel(x+2*xs+1,y+ys,color);
				}
				else if( (zx==1)&&(zy==2) )
				{
					Lcd_Put_Pixel(x+xs,y+2*ys,color);
					Lcd_Put_Pixel(x+xs,y+2*ys+1,color);
				}
				else if( (zx==2)&&(zy==2) )
				{
					Lcd_Put_Pixel(x+2*xs,y+2*ys+1,color);
					Lcd_Put_Pixel(x+2*xs+1,y+2*ys,color);
					Lcd_Put_Pixel(x+2*xs,y+2*ys,color);
					Lcd_Put_Pixel(x+2*xs+1,y+2*ys+1,color);
				}
			} 
			else
			{
				if( (zx==1)&&(zy==1) ) Lcd_Put_Pixel(x+xs,y+ys,bkcolor);
				else if( (zx==2)&&(zy==1) )
				{
					Lcd_Put_Pixel(x+2*xs,y+ys,bkcolor);
					Lcd_Put_Pixel(x+2*xs+1,y+ys,bkcolor);
				}
				else if( (zx==1)&&(zy==2) )
				{
					Lcd_Put_Pixel(x+xs,y+2*ys,bkcolor);
					Lcd_Put_Pixel(x+xs,y+2*ys+1,bkcolor);
				}
				else if( (zx==2)&&(zy==2) )
				{
					Lcd_Put_Pixel(x+2*xs,y+2*ys+1,bkcolor);
					Lcd_Put_Pixel(x+2*xs+1,y+2*ys,bkcolor);
					Lcd_Put_Pixel(x+2*xs,y+2*ys,bkcolor);
					Lcd_Put_Pixel(x+2*xs+1,y+2*ys+1,bkcolor);
				}	   	
			} 
		}
	}
}

// Lcd_Puts
void Lcd_Puts(int x, int y, int color, int bkcolor, char *str, int zx, int zy)
{
     unsigned data;
   
     while(*str)
     {
        data=*str++;
        if(data>=128) 
        { 
             data*=256;
             data|=*str++;
             Lcd_Han_Putch(x, y, color, bkcolor, (int)data, zx, zy);
             x+=zx*16;
        }
        else 
        {
             Lcd_Eng_Putch(x, y, color, bkcolor, (int)data, zx, zy);
             x+=zx*8;
        }
     } 
} 

// Lcd_Printf
void Lcd_Printf(int x, int y, int color, int bkcolor, int zx, int zy, char *fmt,...)
{
	va_list ap;
	char string[256];

	va_start(ap,fmt);
	vsprintf(string,fmt,ap);
	Lcd_Puts(x, y, color, bkcolor, string, zx, zy);
	va_end(ap);
}

// Lcd_GetInfoBMP
void Lcd_Get_Info_BMP(int * x, int  * y, const unsigned char *fp)
{
	*x =*(unsigned short *)(fp+18);    
	*y =*(unsigned short *)(fp+22); 
}

// Lcd_DrawBMP
void Lcd_Draw_BMP(int x, int y, const unsigned char *fp)
{
     int xx=0, yy=0;	
     unsigned int tmp;
     unsigned char tmpR, tmpG, tmpB;

     bfType=*(unsigned short *)(fp+0);
     bfSize=*(unsigned short *)(fp+2);
     tmp=*(unsigned short *)(fp+4);
     bfSize=(tmp<<16)+bfSize;
     bfOffbits=*(unsigned short *)(fp+10);
     biWidth=*(unsigned short *)(fp+18);    
     biHeight=*(unsigned short *)(fp+22);    
     biWidth2=(bfSize-bfOffbits)/biHeight;	

// virtual origin
	calcImg.orig_x = x;
	calcImg.orig_y = y;
	calcImg.voImageBMP = (char*)(fp)+bfOffbits;
	
	for(yy=0;yy<biHeight;yy++)
	{
		for(xx=0;xx<biWidth;xx++)
		{
			tmpB=*(unsigned char *)(fp+bfOffbits+(biHeight-yy-1)*biWidth*3+xx*3+0);
			tmpG=*(unsigned char *)(fp+bfOffbits+(biHeight-yy-1)*biWidth*3+xx*3+1);
			tmpR=*(unsigned char *)(fp+bfOffbits+(biHeight-yy-1)*biWidth*3+xx*3+2);

			tmpR>>=3;
			tmpG>>=3;
			tmpB>>=3;

			if(xx<biWidth2) Lcd_Put_Pixel(x+xx,y+yy,(tmpR<<11)+(tmpG<<6)+(tmpB<<1)+1);
		} 
	}

}

//
// Lcd_GetINFO_BMP
//
void Lcd_GetINFO_BMP(int x, int y, const unsigned char *fp)
{
     unsigned int tmp;

     bfType=*(unsigned short *)(fp+0);
     bfSize=*(unsigned short *)(fp+2);
     tmp=*(unsigned short *)(fp+4);
     bfSize=(tmp<<16)+bfSize;
     bfOffbits=*(unsigned short *)(fp+10);
     biWidth=*(unsigned short *)(fp+18);    
     biHeight=*(unsigned short *)(fp+22);    
     biWidth2=(bfSize-bfOffbits)/biHeight;	

// calcImg memory alloc for future use
	if(calcImg.magicNumber != MAGICNUMBER)
	{
		calcImg.magicNumber = MAGICNUMBER;
//		calcImg.sb.P = malloc(1024*10);	// mem allocation
		calcImg.sb.size = 1024*10;
		calcImg.sb.use = 0;	// buffer use flag
	}
	calcImg.sb.use = 0;	// buffer use flag
		
	calcImg.x = x; // x position for CALC IMAGE
	calcImg.y = y; // y position for CALC IMAGE
	calcImg.w = biWidth; //width for CALC IMAGE
	calcImg.h = biHeight; // height for CALC IMAGE
	calcImg.vpImageBMP = (char*)(fp)+bfOffbits;
	calcImg.SAVEON = 1;
}

//
// Lcd_Draw_BUTTON
//
void Lcd_Draw_BUTTON(int button)
{
	int xx=0, yy=0;
	int x,y,w,h;
//	unsigned int tmp;
	unsigned char tmpR, tmpG, tmpB;
	unsigned int *pV;

// get key position
	x= TouchKey_pTable[button][0];
	y= TouchKey_pTable[button][1];
	w= TouchKey_pTable[button][2];
	h= TouchKey_pTable[button][3];

// buffer Image restore
	if((calcImg.SAVEON != 0) && (calcImg.sb.use != 0))
	{
		pV = calcImg.sb.P;
		
		for(yy=0;yy<calcImg.sb.h;yy++)
		{
			for(xx=0;xx<calcImg.sb.w;xx++)
			{
				tmpB=*(unsigned char *)(calcImg.voImageBMP+(biHeight-yy-calcImg.sb.y-1)*biWidth*3+(calcImg.sb.x+xx)*3+0);
				tmpG=*(unsigned char *)(calcImg.voImageBMP+(biHeight-yy-calcImg.sb.y-1)*biWidth*3+(calcImg.sb.x+xx)*3+1);
				tmpR=*(unsigned char *)(calcImg.voImageBMP+(biHeight-yy-calcImg.sb.y-1)*biWidth*3+(calcImg.sb.x+xx)*3+2);

				tmpR>>=3;
				tmpG>>=3;
				tmpB>>=3;
			
				Lcd_Put_Pixel(calcImg.orig_x+calcImg.sb.x+xx,calcImg.orig_y+calcImg.sb.y+yy,(tmpR<<11)+(tmpG<<6)+(tmpB<<1)+1);
			} 
		}
	}

	pV = calcImg.sb.P;
	for(yy=0;yy<h;yy++)
	{
		for(xx=0;xx<w;xx++)
		{
			*pV = Fb_ptr[calcImg.orig_y+y+yy][(calcImg.orig_x+x+xx)/2];
		}
	}
	
	pV = calcImg.sb.P;
	
	for(yy=0;yy<h;yy++)
	{
		for(xx=0;xx<w;xx++)
		{
			tmpB=*(unsigned char *)(calcImg.vpImageBMP+(biHeight-yy-y-1)*biWidth*3+(x+xx)*3+0);
			tmpG=*(unsigned char *)(calcImg.vpImageBMP+(biHeight-yy-y-1)*biWidth*3+(x+xx)*3+1);
			tmpR=*(unsigned char *)(calcImg.vpImageBMP+(biHeight-yy-y-1)*biWidth*3+(x+xx)*3+2);

			tmpR>>=3;
			tmpG>>=3;
			tmpB>>=3;

		
			Lcd_Put_Pixel(calcImg.orig_x+x+xx,calcImg.orig_y+y+yy,(tmpR<<11)+(tmpG<<6)+(tmpB<<1)+1);
		} 
	}

	calcImg.sb.x = x;
	calcImg.sb.y = y;
	calcImg.sb.w = w;
	calcImg.sb.h = h;
	calcImg.sb.size = x * y * 4;
	calcImg.sb.use = 1;
}

/*-----------------------------------------------------------------------------
 * Program : lcddrv.C
-----------------------------------------------------------------------------*/
