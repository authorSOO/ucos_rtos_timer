#ifndef __INCLUDES_H__
#define __INCLUDES_H__

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>

#include "os_cpu.h"
#include "os_cfg.h"
#include "uCOS_II.H"

#define printf Uart_Printf

#define TASK_STK_SIZE     1024       /* Size of each task's stacks (# of WORDs)            */

/* Application task ID */
enum {
		TASK_START_ID= 0,
		TASK_1_ID,
		TASK_2_ID,
		TASK_3_ID,
		TASK_4_ID,
		TASK_5_ID,
		TASK_6_ID,
		TASK_7_ID,
		TASK_8_ID,
		TASK_9_ID,
		TASK_10_ID,
		MAX_TASK_ID = 55
};

/* Application tasks priorities */
enum {
		TASK_START_PRIO= 4,	// High priority
		TASK_1_PRIO,
		TASK_2_PRIO,
		TASK_3_PRIO,
		TASK_4_PRIO,
		TASK_5_PRIO,
		TASK_6_PRIO,
		TASK_7_PRIO,
		TASK_8_PRIO,
		TASK_9_PRIO,
		TASK_10_PRIO,
		MAX_TASK_PRIO = 59	// Low priority
};

typedef struct {
    char    TaskName[30];
	INT8U	TaskID;
    INT16U  TaskCtr;
    INT16U  TaskExecTime;
    INT32U  TaskTotExecTime;
} TASK_USER_DATA;

/********************************************
*			사용자 헤더 추가				*
********************************************/
#include "2450addr.h"
#include "option.h"
#include "stdarg.h"
#include "libc.h"
#include "uartdrv.h"
#include "lcddrv.h"
#include "drv/COMMRTOS.H"
#include "PC.h"
#include "ui.h"

void UART0_IntHandler(void);


#endif /* __INCLUDES_H__ */