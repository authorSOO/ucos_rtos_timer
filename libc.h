/*
 * =====================================================================
 * NAME: 
 *       libc.h
 *
 * Descriptions : 
 *       Definition of S3C2410 Library prototype
 *
 * Developing Environment : 
 *       CodeWarrior 2.0 or Later
 *
 * Author : 
 *       Ahn, HyoBok  (hbahn@dignsys.com)
 *       DIGNSYS Inc. (www.dignsys.com)
 * =====================================================================
 */

#ifndef __libc_h__
#define __libc_h__

#ifdef __cplusplus
extern "C" {
#endif

void InitSystem(void);
void Led_Display(int data);
void TFT_DrawLine(int xs, int xe, int yy, int color);
// virtual lcd printf
void VLcd_Printf(int x, int y, int color, int bkcolor, int zx, int zy, char *fmt,...);
void VLcd_Draw_BUTTON(int button);
void Buzzer_Init(void);

/*
 * =====================================================================
 * Variable Definition
 * =====================================================================
 */
#define min(x1,x2) (((x1)<(x2))? (x1):(x2))
#define max(x1,x2) (((x1)>(x2))? (x1):(x2))

#define ONESEC0 (62500)		//16us resolution, max 1.04 sec
#define ONESEC1 (31250)		//32us resolution, max 2.09 sec
#define ONESEC2 (15625)		//64us resolution, max 4.19 sec
#define ONESEC3 (7812)		//128us resolution, max 8.38 sec
#define ONESEC4 (PCLK/128/(0xff+1))  //@60Mhz, 128*4us resolution, max 32.53 sec

/*
 * =====================================================================
 * Libraries on the libc.c
 * =====================================================================
 */
void Delay(int time);	//Watchdog Timer is used.
void ChangeMPllValue(int m,int p,int s);
void ChangeClockDivider(int hdivn,int pdivn);
void ChangeUPllValue(int m,int p,int s);

void MMU_Init(void);
void ChangeRomCacheStatus(int attr);
void MMU_SetMTT(int vaddrStart,int vaddrEnd,int paddrStart,int attr);

void *malloc(unsigned nbyte); 
void free(void *pt);

void Port_Init(void);
void Led_Display(int data);

void Uart_SendByte(int data);
void Uart_SendString(char *pt);

int  Uart_GetIntNum(void);

void Timer_Start(int divider);    //Watchdog Timer is used.
int  Timer_Stop(void);            //Watchdog Timer is used.

void MemFill(unsigned long ptr, unsigned long pattern, int size);
void MemDump(unsigned long ptr, int size);

/*
 * =====================================================================
 * Libraries on the libs.s
 * =====================================================================
 */

/*
 * =====================================================================
 * Assembly testing 
 * =====================================================================
 */
/* Testing sum of decimal */
int SUM_OF_DEC(int start, int end);

/*
 * =====================================================================
 *  device testing
 * =====================================================================
 */
/* Buzzer */
void Buzzer(void);
void BuzzerTest(void);

/* UART */
void Uart_TxEmpty(int ch);
char Uart_Getch(void);
char Uart_GetKey(void);

/* RTC */
void Display_Rtc(void);
void Rtc_Init(void);
void Rtc_TimeSet(void);

#ifdef __cplusplus
}
#endif

#endif  /* __libc_h__ */
