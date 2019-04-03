/*******************************************************************************
**	File:
**
**		PC.c
**
**  Contents:
**		uC/OS-II programming
**
**	History:
**		Date        Name              Description
**		------------------------------------------------------------------------
**
*******************************************************************************/
#include "../include/includes.h"
#include "PC.h"
#include "lcddrv.h"
#include "ui.h"

static INT16U    PC_ElapsedOverhead;

void ElapseTimer_Start(void);
unsigned int ElapseTimer_Stop(void);

// 메시지 큐 'LCD'
extern OS_EVENT *qLCD_id;	/* Message Queue Pointer */

TASK_USER_DATA  TaskUserData[7];

/*
*********************************************************************************************************
*                                       ELAPSED TIME INITIALIZATION
*
* Description : This function initialize the elapsed time module by determining how long the START and
*               STOP functions take to execute.  In other words, this function calibrates this module
*               to account for the processing time of the START and STOP functions.
*
* Arguments   : None.
*
* Returns     : None.
*********************************************************************************************************
*/
void PC_ElapsedInit(void)
{
    PC_ElapsedOverhead = 0;
}

/*
*********************************************************************************************************
*                                         INITIALIZE PC'S TIMER #2
*
* Description : This function initialize the PC's Timer #2 to be used to measure the time between events.
*               Timer #2 will be running when the function returns.
*
* Arguments   : None.
*
* Returns     : None.
*********************************************************************************************************
*/
void PC_ElapsedStart(void)
{
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr;
#endif    

    OS_ENTER_CRITICAL();
	ElapseTimer_Start();
    OS_EXIT_CRITICAL();
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                 STOP THE PC'S TIMER #2 AND GET ELAPSED TIME
*
* Description : This function stops the PC's Timer #2, obtains the elapsed counts from when it was
*               started and converts the elapsed counts to micro-seconds.
*
* Arguments   : None.
*
* Returns     : The number of micro-seconds since the timer was last started.
*
* Notes       : - The returned time accounts for the processing time of the START and STOP functions.
*               - 41943 represents 41943S-16 or, 0.6400 which is used to convert timer counts to
*                 micro-seconds.  The clock source for the PC's timer #2 is 1.5625 MHz (or 0.6400 uS)
*********************************************************************************************************
*/
INT16U PC_ElapsedStop(void)
{
#if OS_CRITICAL_METHOD == 3 /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr;
#endif    
    INT16U     cnts=0;

    OS_ENTER_CRITICAL();
	cnts = ElapseTimer_Stop();
    OS_EXIT_CRITICAL();
    return ((INT16U)((INT32U)cnts * 41943L >> 16) - PC_ElapsedOverhead);
}

/*
*********************************************************************************************************
*                                 DISPLAY A STRING  AT 'X' & 'Y' COORDINATE
*
* Description : This function writes an ASCII string anywhere on the PC's screen.  This function writes
*               directly to video RAM instead of using the BIOS for speed reasons.  It assumed that the 
*               video adapter is VGA compatible.  Video RAM starts at absolute address 0x000B8000.  Each 
*               character on the screen is composed of two bytes: the ASCII character to appear on the 
*               screen followed by a video attribute.  An attribute of 0x07 displays the character in 
*               WHITE with a black background.
*
* Arguments   : x      corresponds to the desired column on the screen.  Valid columns numbers are from
*                      0 to 79.  Column 0 corresponds to the leftmost column.
*               y      corresponds to the desired row on the screen.  Valid row numbers are from 0 to 24.
*                      Line 0 corresponds to the topmost row.
*               s      Is the ASCII string to display.  You can also specify a string containing 
*                      characters with numeric values higher than 128.  In this case, special character 
*                      based graphics will be displayed.
*               color  specifies the foreground/background color to use (see PC.H for available choices)
*                      and whether the characters will blink or not.
*
* Returns     : None
*********************************************************************************************************
*/
void PC_DispStr (INT8U x, INT8U y, char *s, INT8U color)
{
#ifdef __XDEF_REPORT_STAT
	Lcd_Printf(x*8, y*16, color, WHITE, 1,1, s);
#endif // __XDEF_REPORT_STAT
}

/*
*********************************************************************************************************
*                           DISPLAY A SINGLE CHARACTER AT 'X' & 'Y' COORDINATE
*
* Description : This function writes a single character anywhere on the PC's screen.  This function
*               writes directly to video RAM instead of using the BIOS for speed reasons.  It assumed 
*               that the video adapter is VGA compatible.  Video RAM starts at absolute address 
*               0x000B8000.  Each character on the screen is composed of two bytes: the ASCII character 
*               to appear on the screen followed by a video attribute.  An attribute of 0x07 displays 
*               the character in WHITE with a black background.
*
* Arguments   : x      corresponds to the desired column on the screen.  Valid columns numbers are from
*                      0 to 79.  Column 0 corresponds to the leftmost column.
*               y      corresponds to the desired row on the screen.  Valid row numbers are from 0 to 24.
*                      Line 0 corresponds to the topmost row.
*               c      Is the ASCII character to display.  You can also specify a character with a 
*                      numeric value higher than 128.  In this case, special character based graphics
*                      will be displayed.
*               color  specifies the foreground/background color to use (see PC.H for available choices)
*                      and whether the character will blink or not.
*
* Returns     : None
*********************************************************************************************************
*/
void PC_DispChar (INT8U x, INT8U y, char c, INT8U color)
{
#ifdef __XDEF_REPORT_STAT
	Lcd_Eng_Putch(x*16, y*32, color, WHITE, (int)c, 2, 2);
#endif // __XDEF_REPORT_STAT
}

//
// 'DispTaskStat'
// DISPLAY TASK RELATED STATISTICS
//
void DispTaskStat (INT8U id)
{
    char s[80];

    sprintf(s, " %3d   %05u   %5u %10ld",
            TaskUserData[id].TaskID,
            TaskUserData[id].TaskCtr,
            TaskUserData[id].TaskExecTime,
            (long)TaskUserData[id].TaskTotExecTime);
    PC_DispStr(1, id + 4, s, BLUE);
}

//
// 'TaskStatisticsTitleDisplay'
//
void TaskStatisticsTitleDisplay(char *name)
{
	//
	// __XDEF_REPORT_STAT(통계 기능 보고 활성화)
	//
#ifdef __XDEF_REPORT_STAT

    char   s[80];
	
    PC_DispStr( ((LCD_XSIZE/8)/4) - (strlen(name)/2),  0, name, BLACK);

    PC_DispStr(0,  2, "S TASK Counter Exe(us)   Tot(us) Rate(%%)", BLACK);
    PC_DispStr(0, 3,  "- ---- ------- -------  -------- -------", BLACK);

    PC_DispStr( 0, 12, "#Tasks          : xxxxx  CPU Usage: xx %%", BLACK);
    PC_DispStr( 0, 13, "#Task switch/sec: xxxxx", BLACK);
	sprintf(s, "uC/OS-II V%1d.%02d", OSVersion() / 100, OSVersion() % 100);
	PC_DispStr(26, 14, s, BLACK);
#endif // __XDEF_REPORT_STAT
}

/*-----------------------------------------------------------------------------
 * Program : PC.c
-----------------------------------------------------------------------------*/
