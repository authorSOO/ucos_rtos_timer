#include "stop-watch.h"
#include "common.h"

typedef struct {
    unsigned char GPIO_PIN_0    : 1;
    unsigned char GPIO_PIN_1    : 1;
    unsigned char GPIO_PIN_2    : 1;
    unsigned char GPIO_PIN_3    : 1; 
    unsigned char LED           : 4;
//     unsigned char GPIO_PIN_4    : 1;
//     unsigned char GPIO_PIN_5    : 1;
//     unsigned char GPIO_PIN_6    : 1;
//     unsigned char GPIO_PIN_7    : 1;
    unsigned char GPIO_PIN_8    : 1;
    unsigned char GPIO_PIN_9    : 1;
    unsigned char GPIO_PIN_10   : 1;
    unsigned char GPIO_PIN_11   : 1;
    unsigned char GPIO_PIN_12   : 1; 
    unsigned char GPIO_PIN_13   : 1;
    unsigned char GPIO_PIN_14   : 1;
    unsigned char GPIO_PIN_15   : 1;
} GPIOG; 

typedef struct {
    unsigned char GPIO_PIN_0    : 1;
    unsigned char GPIO_PIN_1    : 1;
    unsigned char GPIO_PIN_2    : 1;
    unsigned char GPIO_PIN_3    : 1;
    unsigned char GPIO_PIN_4    : 1;
    unsigned char GPIO_PIN_5    : 1;
    unsigned char GPIO_PIN_6    : 1;
    unsigned char GPIO_PIN_7    : 1;
    unsigned char res           : 8;
} GPIOF;

enum IO_MODE {
    INPUT = (0x0),
    OUTPUT = (0x1),
    EINT = (0x2),
    RESERVED = (0x3)
};

enum EXTI_MODE {
    LOW_LEVEL = (0x0),
    HIGH_LEVEL = (0x1),
    FALLING_EDGE = (0x2),
    RISING_EDGE = (0x4),
    BOTH_EDGE = (0x6)
};

typedef struct {
    unsigned char GPIO_PIN_0   : 2;
    unsigned char GPIO_PIN_1   : 2;
    unsigned char GPIO_PIN_2   : 2;
    unsigned char GPIO_PIN_3   : 2;
    unsigned char GPIO_PIN_4   : 2;
    unsigned char GPIO_PIN_5   : 2;
    unsigned char GPIO_PIN_6   : 2;
    unsigned char GPIO_PIN_7   : 2;
    unsigned char GPIO_PIN_8   : 2;
    unsigned char GPIO_PIN_9   : 2;
    unsigned char GPIO_PIN_10  : 2;
    unsigned char GPIO_PIN_11  : 2;
    unsigned char GPIO_PIN_12  : 2;
    unsigned char GPIO_PIN_13  : 2;
    unsigned char GPIO_PIN_14  : 2;
    unsigned char GPIO_PIN_15  : 2;
} GPCON;

#define GPGCON    (*(volatile GPCON *)0x56000060)
#define GPGDAT    (*(volatile GPIOG *)0x56000064)

#define GPFCON (*(volatile GPCON *)0x56000050)
#define GPFDAT (*(volatile GPIOF *)0x56000054)

#define GPHCON (*(volatile unsigned *)0x56000070)

#define APBCLOCK (*(volatile unsigned *)0x4C000034)

#define ULCON1 (*(volatile unsigned *)0x50004000)
#define UCON1 (*(volatile unsigned *)0x50004004)
#define UFCON1 (*(volatile unsigned *)0x50004008)
#define UMCON1 (*(volatile unsigned *)0x5000400C)
#define UTRSTAT1 (*(volatile unsigned *)0x50004010)
#define UFSTAT1 (*(volatile unsigned *)0x50004018)
#define TX_BUFFER (*(volatile unsigned *)0x50004020)
#define RX_BUFFER (*(volatile unsigned *)0x50004024)
#define UBRDIV1 (*(volatile unsigned *)0x50004028)
#define UDIVSLOT1 (*(volatile unsigned *)0x5000402C)


#define	ClearPending1(bit) {\
			rSRCPND1 = bit;\
			rINTPND1 = bit;\
			rINTPND1;\
		}		

void __attribute__((interrupt("IRQ"))) isr_eint_0(void);
void __attribute__((interrupt("IRQ"))) isr_eint_1(void);
void __attribute__((interrupt("IRQ"))) isr_eint_4_7(void);


int RX_Count = 0;
int mutex = 1;
int time_count = 0;
int stop1,stop2,stop3,stop4,stop5,stop6,stop7=0;




void gpio_init(){
    // LED INIT
    GPGCON.GPIO_PIN_4 = OUTPUT;
    GPGCON.GPIO_PIN_5 = OUTPUT;
    GPGCON.GPIO_PIN_6 = OUTPUT;
    GPGCON.GPIO_PIN_7 = OUTPUT;

    GPGDAT.LED = (0xF);    

    // KEY INIT
    GPFCON.GPIO_PIN_0 = EINT;
    GPFCON.GPIO_PIN_1 = EINT;
}

void exti_init(){
    // Set Interrupt Mod to IRQ
    rINTMOD1 = (0x0);
        
    // Reset Interrupt Mask
    rINTMSK1 = BIT_ALLMSK;              // (0xffffffff)

    // Clear Source Pending Bit 
    rSRCPND1 = BIT_EINT4_7 | BIT_EINT1 | BIT_EINT0;

    // Clear Interrupt Pending Bit 
    rINTPND1 = BIT_EINT4_7 | BIT_EINT0 | BIT_EINT1;     

    // Set Interrupt Mask
    rINTMSK1 = ~(BIT_EINT4_7 | BIT_EINT0 | BIT_EINT1);

    // Set External Interrupt Edge Trigger
    rEXTINT0 = (rEXTINT0 & ~(0x7 << 5)) | (FALLING_EDGE << 5); 
    rEXTINT0 |= (rEXTINT0 & ~(0x7 << 4)) | (FALLING_EDGE << 4);
    rEXTINT0 |= (rEXTINT0 & ~(0x7 << 1)) | (FALLING_EDGE << 1);
    rEXTINT0 |= (rEXTINT0 & ~(0x7 << 0)) | (FALLING_EDGE << 0);

    // Clear External Interrupt Pending Bit
    rEINTPEND = (0x3 << 4);
    
    // Set External Interrupt MAsk
    rEINTMASK = (0xFFFFC << 4);
    
    // ISR    
    pISR_EINT0 = (unsigned)isr_eint_0; 
    pISR_EINT1 = (unsigned)isr_eint_1;
    pISR_EINT4_7= (unsigned)isr_eint_4_7;
}

void timer0_init(){
    rTCFG0 = (rTCFG0 & ~0xFF) | (33 - 1);   // 66,000,000 / 33
    rTCFG1 = (rTCFG1 & ~0xF);               // 66,000,000 / 33 / 2
    rTCNTB0 =  (0xFFFF);                    // Max size of buffer 
    rTCON |= (0x02);                        // update TCNTB0
    rTCON = (rTCON & ~(0xf) | (1 << 3) | (1 << 0)); // auto reload & start timer0
}



void  __attribute__((interrupt("IRQ"))) isr_eint_0(void)
{
    ClearPending1(BIT_EINT0);
 if(stop1==1)
     	stop1=0;
     	else
	 	stop1=1;
}

void __attribute__((interrupt("IRQ"))) isr_eint_1(void)
{
    ClearPending1(BIT_EINT1);
     	if(stop2==1)
     	stop2=0;
     	else
	 	stop2=1;
}

void __attribute__((interrupt("IRQ"))) isr_eint_4_7(void){
    ClearPending1(BIT_EINT4_7);
   
   	if(rEINTPEND & (0x1 <<3)){
        rEINTPEND = 0x1 << 3;
	if(stop2==1)
     	stop2=0;
     	else
	 	stop2=1;
     
    }
   
   
    if(rEINTPEND & (0x1 <<4)){
        rEINTPEND = 0x1 <<4;
	if(stop3==1)
     	stop3=0;
     	else
	 	stop3=1;
     
    }
    if(rEINTPEND & (0x1 <<5)){
        rEINTPEND = 0x1 <<5;
	if(stop4==1)
     	stop4=0;
     	else
	 	stop4=1;
 
    }
     if(rEINTPEND & (0x1 <<6)){
        rEINTPEND = 0x1 << 6;
	if(stop5==1)
     	stop5=0;
     	else
	 	stop5=1;
 
    }
     if(rEINTPEND & (0x1 <<7)){
        rEINTPEND = 0x1 << 7;
	if(stop6==1)
     	stop6=0;
     	else
	 	stop6=1;
 
    }
}



extern INT32U USE_OSTimeTickHook;
void main()
{
	Uart_Init(115200);
	// HW init
//	hardware_init();
	Graphic_Init();
	// Clear screen to black
//	clear_screen(BLACK);	
	 gpio_init();
	 exti_init();

	Uart_Send_String("Stop Watch in uC/OS-II. Designed by Hong\n");


	OSInit();                               /* Initialize uC/OS-II */

	//RandomSem = OSSemCreate(1);             /* Random number semaphore */
	OSTaskCreate(TaskStart, (void *)0, (void *)&TaskStartStk[TASK_STK_SIZE - 1], 0);
	Lcd_Printf(10*8,0,WHITE,BLACK,1,1,"Stop Watch. Designed by Hong");  

	OSStart();                              /* Start multitasking */

	while(1);	
}

void Timer0_ISR(void)
{
	
}


void Inval_ISR(void)
{
	Uart_Printf("Invalid Interrupt");
}


void C_IRQHandler(void)
{
	irqtbl[rINTOFFSET1]();
}


void Task1 (void *data)
{
	int  hour=0, min=0, sec=0, mm=0;

	while(1)
	{
		if(stop1)
			OSTaskSuspend(1);
		if(!stop2)
			OSTaskResume(2);
		if(!stop3)
			OSTaskResume(3);
		if(!stop4)
			OSTaskResume(4);
		if(!stop5)
			OSTaskResume(5);
		if(!stop6)
			OSTaskResume(6);
		if(!stop7)
			OSTaskResume(7);
			
		
#if HIGH_QUALITY
		Lcd_Printf(80, 50 ,WHITE,BLACK,1,1,"%02d:%02d:%02d:%02d",hour, min, sec, mm);
#else
	//	Lcd_Printf(80, 50 ,WHITE,BLACK,1,1,"%02d:%02d:%02d",hour, min, sec);
#endif


#if HIGH_QUALITY
		OSTimeDlyHMSM(0,0,0,20);
		if( mm++ == 60 )
		{
			sec++; mm = 0 ;
		}
#else
		OSTimeDlyHMSM(0,0,1,0);
		sec++;
#endif
		if( sec == 60 )
		{
			min++; sec = 0 ;
		}
		if( min == 60 )
		{
			hour++; min = 0 ;
		}
	}
}

void Task2(void *data)
{
	int  hour=0, min=0, sec=0, mm=0;



	while(1)
	{
			if(stop2)
			OSTaskSuspend(2);	
		
		
		
#if HIGH_QUALITY
		Lcd_Printf(80, 70 ,WHITE,BLACK,1,1,"%02d:%02d:%02d:%02d",hour, min, sec, mm);
#else
	//	Lcd_Printf(80, 70 ,WHITE,BLACK,1,1,"%02d:%02d:%02d",hour, min, sec);
#endif

#if HIGH_QUALITY
		OSTimeDlyHMSM(0,0,0,30);
		if( mm++ == 60 )
		{
			sec++; mm = 0 ;
		}
#else
		OSTimeDlyHMSM(0,0,2,0);
		sec++;
#endif
		if( sec == 60 )
		{
			min++; sec = 0 ;
		}
		if( min == 60 )
		{
			hour++; min = 0 ;
		}
	}
}

void Task3 (void *data)
{
	int  hour=0, min=0, sec=0, mm=0;

	while(1)
	{
		
			if(stop3)
			OSTaskSuspend(3);	
		
#if HIGH_QUALITY
		Lcd_Printf(80, 90 ,WHITE,BLACK,1,1,"%02d:%02d:%02d:%02d",hour, min, sec, mm);
#else
	//	Lcd_Printf(80, 90 ,WHITE,BLACK,1,1,"%02d:%02d:%02d",hour, min, sec);
#endif

#if HIGH_QUALITY
		OSTimeDlyHMSM(0,0,0,40);
		if( mm++ == 60 )
		{
			sec++; mm = 0 ;
		}
#else
		OSTimeDlyHMSM(0,0,3,0);
		sec++;
#endif
		if( sec == 60 )
		{
			min++; sec = 0 ;
		}
		if( min == 60 )
		{
			hour++; min = 0 ;
		}
	}
}

void Task4 (void *data)
{
	int  hour=0, min=0, sec=0, mm=0;

	while(1)
	{
		
				if(stop4)
			OSTaskSuspend(4);	
		
#if HIGH_QUALITY
		Lcd_Printf(80, 110 ,WHITE,BLACK,1,1,"%02d:%02d:%02d:%02d",hour, min, sec, mm);
#else
	//	Lcd_Printf(80, 110 ,WHITE,BLACK,1,1,"%02d:%02d:%02d",hour, min, sec);
#endif

#if HIGH_QUALITY
		OSTimeDlyHMSM(0,0,0,50);
		if( mm++ == 60 )
		{
			sec++; mm = 0 ;
		}
#else
		OSTimeDlyHMSM(0,0,4,0);
		sec++;
#endif
		if( sec == 60 )
		{
			min++; sec = 0 ;
		}
		if( min == 60 )
		{
			hour++; min = 0 ;
		}
	}
}

void Task5 (void *data)
{
	int  hour=0, min=0, sec=0, mm=0;

	while(1)
	{
#if HIGH_QUALITY
		if(stop5)
		OSTaskSuspend(5);
		Lcd_Printf(80, 130 ,WHITE,BLACK,1,1,"%02d:%02d:%02d:%02d",hour, min, sec, mm);
#else
	//	Lcd_Printf(80, 130 ,WHITE,BLACK,1,1,"%02d:%02d:%02d",hour, min, sec);
#endif

#if HIGH_QUALITY
		OSTimeDlyHMSM(0,0,0,60);
		if( mm++ == 60 )
		{
			sec++; mm = 0 ;
		}
#else 
		OSTimeDlyHMSM(0,0,5,0);
		sec++;
#endif
		if( sec == 60 )
		{
			min++; sec = 0 ;
		}
		if( min == 60 )
		{
			hour++; min = 0 ;
		}
	}
}

void Task6 (void *data)
{
	int  hour=0, min=0, sec=0, mm=0;

	while(1)
	{
#if HIGH_QUALITY
		if(stop6)
			OSTaskSuspend(6);
		Lcd_Printf(80, 150 ,WHITE,BLACK,1,1,"%02d:%02d:%02d:%02d",hour, min, sec, mm);
#else
	//	Lcd_Printf(80, 150 ,WHITE,BLACK,1,1,"%02d:%02d:%02d",hour, min, sec);
#endif

#if HIGH_QUALITY
		OSTimeDlyHMSM(0,0,0,70);
		if( mm++ == 60 )
		{
			sec++; mm = 0 ;
		}
#else
		OSTimeDlyHMSM(0,0,6,0);
		sec++;
#endif
		if( sec == 60 )
		{
			min++; sec = 0 ;
		}
		if( min == 60 )
		{
			hour++; min = 0 ;
		}
	}
}

void Task7 (void *data)
{
	int  hour=0, min=0, sec=0, mm=0;

	while(1)
	{
#if HIGH_QUALITY
		Lcd_Printf(80, 170 ,WHITE,BLACK,1,1,"%02d:%02d:%02d:%02d",hour, min, sec, mm);
#else
	//	Lcd_Printf(80, 170 ,WHITE,BLACK,1,1,"%02d:%02d:%02d",hour, min, sec);
#endif

#if HIGH_QUALITY
		OSTimeDlyHMSM(0,0,0,80);
		if( mm++ == 60 )
		{
			sec++; mm = 0 ;
		}
#else
		OSTimeDlyHMSM(0,0,7,0);
		sec++;
#endif
		if( sec == 60 )
		{
			min++; sec = 0 ;
		}
		if( min == 60 )
		{
			hour++; min = 0 ;
		}
	}
}

void Task8 (void *data)
{
	int  hour=0, min=0, sec=0, mm=0;

	while(1)
	{
#if HIGH_QUALITY
		Lcd_Printf(80, 190 ,WHITE,BLACK,1,1,"%02d:%02d:%02d:%02d",hour, min, sec, mm);
#else
	//	Lcd_Printf(80, 190 ,WHITE,BLACK,1,1,"%02d:%02d:%02d",hour, min, sec);
#endif

#if HIGH_QUALITY
		OSTimeDlyHMSM(0,0,0,90);
		if( mm++ == 60 )
		{
			sec++; mm = 0 ;
		}
#else
		OSTimeDlyHMSM(0,0,8,0);
		sec++;
#endif
		if( sec == 60 )
		{
			min++; sec = 0 ;
		}
		if( min == 60 )
		{
			hour++; min = 0 ;
		}
	}
}

void Task9 (void *data)
{
	int  hour=0, min=0, sec=0, mm=0;

	while(1)
	{
#if HIGH_QUALITY
		Lcd_Printf(80, 210 ,WHITE,BLACK,1,1,"%02d:%02d:%02d:%02d",hour, min, sec, mm);
#else
	//	Lcd_Printf(80, 210 ,WHITE,BLACK,1,1,"%02d:%02d:%02d",hour, min, sec);
#endif

#if HIGH_QUALITY
		OSTimeDlyHMSM(0,0,0,100);
		if( mm++ == 60 )
		{
			sec++; mm = 0 ;
		}
#else
		OSTimeDlyHMSM(0,0,9,0);
		sec++;
#endif
		if( sec == 60 )
		{
			min++; sec = 0 ;
		}
		if( min == 60 )
		{
			hour++; min = 0 ;
		}
	}
}

void TaskStart (void *data)
{
	int i;
	char key;
	int idx=0;

	/* Prevent compiler warning */
	data = data;
	InitSystem();	

	/* Initialize uC/OS-II's statistics */

	
	for(idx=0; idx<NO_TASKS; idx++)
	{
		TaskData[idx] = 1 + idx;
		Lcd_Printf(308,50+(20*idx),WHITE,BLACK,1,1,"Task %d",idx+1);
	}

	USE_OSTimeTickHook = 1;
	
	OSTaskCreate(Task1, (void *)0, (void *)&TaskStk[0][TASK_STK_SIZE - 1], 1);
	OSTaskCreate(Task2, (void *)0, (void *)&TaskStk[1][TASK_STK_SIZE - 1], 2);
	OSTaskCreate(Task3, (void *)0, (void *)&TaskStk[2][TASK_STK_SIZE - 1], 3);
	OSTaskCreate(Task4, (void *)0, (void *)&TaskStk[3][TASK_STK_SIZE - 1], 4);
	OSTaskCreate(Task5, (void *)0, (void *)&TaskStk[4][TASK_STK_SIZE - 1], 5);
	OSTaskCreate(Task6, (void *)0, (void *)&TaskStk[5][TASK_STK_SIZE - 1], 6);
	OSTaskCreate(Task7, (void *)0, (void *)&TaskStk[6][TASK_STK_SIZE - 1], 7);
	OSTaskCreate(Task8, (void *)0, (void *)&TaskStk[7][TASK_STK_SIZE - 1], 8);
	OSTaskCreate(Task9, (void *)0, (void *)&TaskStk[8][TASK_STK_SIZE - 1], 9);

	Uart_Printf("<-POWER OFF TO QUIT->\n");

	while(1)
	{
				
		OSTimeDlyHMSM(0, 0, 1, 0);     
	}
}
