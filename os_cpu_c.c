/*
*********************************************************************************************************
*                                               uC/OS-II
*                                        The Real-Time Kernel
*
*                           (c) Copyright 1992-2003, Micrium, Inc., Weston, FL
*                                          All Rights Reserved
*
*                                               ARM9 Port
*
* File : OS_CPU_C.C
*********************************************************************************************************
*/

#define  OS_CPU_GLOBALS
#include "includes.h"
#include "ui.h"

extern TASK_USER_DATA  TaskUserData[7];
extern void DispTaskStat (INT8U id);
extern void PC_DispStr (INT8U x, INT8U y, char *s, INT8U color);

// 1초마다 콘솔에 동작중을 표시(.)
INT32U USE_OSTimeTickHook = 0;

/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                            (BEGINNING)
*
* Description: This function is called by OSInit() at the beginning of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void  OSInitHookBegin (void)
{

}
#endif

/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                               (END)
*
* Description: This function is called by OSInit() at the end of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void  OSInitHookEnd (void)
{
	int i;

	// Clear a Running State for Current Task
	for(i=0; i<7; i++)
	{
		TaskUserData[i].TaskID = 0xff;
	}
}
#endif

/*
*********************************************************************************************************
*                                          TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being created.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0 
void  OSTaskCreateHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                       /* Prevent compiler warning                                     */
}
#endif


/*
*********************************************************************************************************
*                                           TASK DELETION HOOK
*
* Description: This function is called when a task is deleted.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0 
void  OSTaskDelHook (OS_TCB *ptcb)
{
	int i;

    ptcb = ptcb;                       /* Prevent compiler warning                                     */

    	// Clear a Running State for Current Task
	for(i=0; i<7; i++)
	{
		if(TaskUserData[i].TaskID == ptcb->OSTCBId)
			PC_DispChar(((LCD_XSIZE/8)/2) - (LCD_XSIZE/8/2)+0, i + 4, 'x', BLUE);
	}

}
#endif

/*
*********************************************************************************************************
*                                             IDLE TASK HOOK
*
* Description: This function is called by the idle task.  This hook has been added to allow you to do  
*              such things as STOP the CPU to conserve power.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are enabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION >= 251
void  OSTaskIdleHook (void)
{
}
#endif

/*
*********************************************************************************************************
*                                           STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/OS-II's statistics task.  This allows your 
*              application to add functionality to the statistics task.
*
* Arguments  : none
*********************************************************************************************************
*/

#if OS_CPU_HOOKS_EN > 0 
void  OSTaskStatHook (void)
{
    INT8U  i;
    INT32U total;
    INT8U  pct;
	int end;
	char s[80];

    total = 0L;                                          /* Totalize TOT. EXEC. TIME for each task */
    for (i = 0; i < 7; i++) {
        total += TaskUserData[i].TaskTotExecTime;
        DispTaskStat(i);                                 /* Display task data                      */
    }
    if (total > 0) {
        for (i = 0; i < 7; i++) {                        /* Derive percentage of each task         */
            pct = 100 * TaskUserData[i].TaskTotExecTime / total;
            sprintf(s, "%3d %%", pct);
			end= strlen(s);
			s[end]='%';s[end+1]=0;
            PC_DispStr(((LCD_XSIZE/8)/2) - (LCD_XSIZE/8/2)+32, i + 4, s, BLUE);
        }
    }
    if (total > 1000000000L) {                           /* Reset total time counters at 1 billion */
        for (i = 0; i < 7; i++) {
            TaskUserData[i].TaskTotExecTime = 0L;
        }
    }

	// CPU Usage Display
    sprintf(s, "%5d", OSTaskCtr);  /* Display #tasks running                   */
    PC_DispStr(((LCD_XSIZE/8)/2) - (LCD_XSIZE/8/2)+18, 12, s, BLUE);
    sprintf(s, "%3d", OSCPUUsage); /* Display CPU usage in %                   */
    PC_DispStr(((LCD_XSIZE/8)/2) - (LCD_XSIZE/8/2)+35, 12, s, BLUE);
    sprintf(s, "%5d", OSCtxSwCtr); /* Display #context switches per second     */
    PC_DispStr(((LCD_XSIZE/8)/2) - (LCD_XSIZE/8/2)+18, 13, s, BLUE);
}
#endif

/*
*********************************************************************************************************
*                                        INITIALIZE A TASK'S STACK
*
* Description: This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
*              stack frame of the task being created.  This function is highly processor specific.
*
* Arguments  : task          is a pointer to the task code
*
*              p_arg         is a pointer to a user supplied data area that will be passed to the task
*                            when the task first executes.
*
*              ptos          is a pointer to the top of stack.  It is assumed that 'ptos' points to
*                            a 'free' entry on the task stack.  If OS_STK_GROWTH is set to 1 then 
*                            'ptos' will contain the HIGHEST valid address of the stack.  Similarly, if
*                            OS_STK_GROWTH is set to 0, the 'ptos' will contains the LOWEST valid address
*                            of the stack.
*
*              opt           specifies options that can be used to alter the behavior of OSTaskStkInit().
*                            (see uCOS_II.H for OS_TASK_OPT_???).
*
* Returns    : Always returns the location of the new top-of-stack' once the processor registers have
*              been placed on the stack in the proper order.
*
* Note(s)    : 1) Interrupts are enabled when your task starts executing. 
*              2) All tasks run in SVC mode.
*********************************************************************************************************
*/

OS_STK *OSTaskStkInit (void (*task)(void *pd), void *p_arg, OS_STK *ptos, INT16U opt)
{
    OS_STK *stk;

    opt      = opt;                 /* 'opt' is not used, prevent warning                      */
    
    stk      = ptos;                /* Load stack pointer                                      */
    
    *(stk)   = (OS_STK)task;        /* Entry Point                                             */
    *(--stk) = (INT32U)0;         	/* LR                                                      */
    *(--stk) = (INT32U)0;         	/* R12                                                     */
    *(--stk) = (INT32U)0;         	/* R11                                                     */
    *(--stk) = (INT32U)0;         	/* R10                                                     */
    *(--stk) = (INT32U)0;         	/* R9                                                      */
    *(--stk) = (INT32U)0;         	/* R8                                                      */
    *(--stk) = (INT32U)0;         	/* R7                                                      */
    *(--stk) = (INT32U)0;         	/* R6                                                      */
    *(--stk) = (INT32U)0;         	/* R5                                                      */
    *(--stk) = (INT32U)0;         	/* R4                                                      */
    *(--stk) = (INT32U)0;         	/* R3                                                      */
    *(--stk) = (INT32U)0;         	/* R2                                                      */
    *(--stk) = (INT32U)0;         	/* R1                                                      */
    *(--stk) = (INT32U)p_arg;		/* R0 : argument                                           */
    *(--stk) = (INT32U)0x00000013L; /* CPSR  (SVC mode, Enable both IRQ and FIQ interrupts)    */
    *(--stk) = (INT32U)0x00000013L; /* CPSR  (SVC mode, Enable both IRQ and FIQ interrupts)    */

    return (stk);
}

/*
*********************************************************************************************************
*                                           TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other
*              operations during a context switch.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                 will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the 
*                 task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0) && (OS_TASK_SW_HOOK_EN > 0)
void  OSTaskSwHook (void)
{
    INT16U          time;
    TASK_USER_DATA *puser;

    time  = PC_ElapsedStop();                    /* This task is done                                  */
    PC_ElapsedStart();                           /* Start for next task                                */

	/* TODO #1: 
		EXAMPLE "04_STAT.C" 실습 예제
		태스크 사용자 자료 구조(TASK_USER_DATA) 업데이트
		use 'OSTCBCur' : 5 line */
#if 1
	puser = OSTCBCur->OSTCBExtPtr;               /* Point to used data                                 */
    if (puser != (void *)0) {
		puser->TaskID = OSTCBCur->OSTCBId;		 /* Task ID */
        puser->TaskCtr++;                        /* Increment task counter                             */
        puser->TaskExecTime     = time;          /* Update the task's execution time                   */
        puser->TaskTotExecTime += time;          /* Update the task's total execution time             */
    }
#endif // TODO #1
}
#endif

/*
*********************************************************************************************************
*                                           OS_TCBInit() HOOK
*
* Description: This function is called by OS_TCBInit() after setting up most of the TCB.
*
* Arguments  : ptcb    is a pointer to the TCB of the task being created.
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void  OSTCBInitHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                                           /* Prevent Compiler warning                 */
}
#endif


/*
*********************************************************************************************************
*                                               TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0) && (OS_TIME_TICK_HOOK_EN > 0)
void  OSTimeTickHook (void)
{
	// 1초마다 콘솔에 동작중을 표시(.)
	if( USE_OSTimeTickHook )
	{
		if((OSTime % OS_TICKS_PER_SEC) == 0)
			Uart_Printf(".");
	}
}
#endif


/*
*********************************************************************************************************
*                                         ARM SPECIFIC CODE
*********************************************************************************************************
*/



