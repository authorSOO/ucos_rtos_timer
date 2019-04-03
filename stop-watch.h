#ifndef __STOPWATCH_H__
#define __STOPWATCH_H__

#include "2450addr.h"
#include "includes.h"

/* Size of each task's stacks (# of WORDs) */
#define  TASK_STK_SIZE 		512    

/* Number of identical tasks */             
#define  NO_TASKS           9                  
#define  MAZE_SIZE  		19

// If you HIGH_QUALITY 1, CPU usage is 40 or over
#define HIGH_QUALITY		1

 /* Tasks stacks */
OS_STK   TaskStk[NO_TASKS][TASK_STK_SIZE];     
OS_STK   TaskStartStk[TASK_STK_SIZE];

/* Parameters to pass to each task */
char     TaskData[NO_TASKS];                    

void Timer0_ISR(void);
void Inval_ISR(void);
void C_IRQHandler(void);

void Task1 (void *data);
void Task2 (void *data);
void Task3 (void *data);
void Task4 (void *data);
void Task5 (void *data);
void Task6 (void *data);
void Task7 (void *data);
void Task8 (void *data);
void Task9 (void *data);

void TaskStart (void *data);

void (*irqtbl[32])(void)={
	Inval_ISR,	//	EINT0
	Inval_ISR,	//	EINT1
	Inval_ISR,	//	EINT2
	Inval_ISR,	//	EINT3
	Inval_ISR,	//	EINT4_7
	Inval_ISR,	//	EINT8_23
	Inval_ISR,	//	pISR_CAM
	Inval_ISR,	//	BAT_FLT	      
	Inval_ISR,	//	TICK	      
	Inval_ISR,	//	WDT_AC97T_AC97
	Timer0_ISR,	//	TIMER0	      
	Inval_ISR,	//	TIMER1	      
	Inval_ISR,	//	TIMER2	      
	Inval_ISR,	//	TIMER3	      
	Inval_ISR,	//	TIMER4	      
	Inval_ISR,	//      UART2	      
    Inval_ISR,      //      LCD	      
	Inval_ISR,      //      DMA0	      
	Inval_ISR,	//	DMA1	 														     
	Inval_ISR,	//	DMA2	 														     
	Inval_ISR,	//	DMA3	 														     
	Inval_ISR,	//	SDI	 														     
	Inval_ISR,	//	SPI0	 														     
	Inval_ISR,	//	UART1	 														     // Added for 2440.
	Inval_ISR,	//	NFCON	 														     
	Inval_ISR,	//	USBD	 														     
	Inval_ISR,	//	USBH	 														     
	Inval_ISR,	//	IIC	 														     
	Inval_ISR,	//	UART0	 																     
	Inval_ISR,	//	SPI1	 																     
	Inval_ISR,	//	RTC	 																     
	Inval_ISR	//	ADC	 																     
};

#endif
