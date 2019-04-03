/*
 * =====================================================================
 * NAME: 
 *       libc.c
 *
 * Descriptions : 
 *       C library for S3C2450
 *
 * Developing Environment : 
 *       CodeWarrior 2.0 or Later
 *
 * =====================================================================
 */

#include "includes.h"
#include "libc.h"
#include "MMU.h"

void Graphic_Init(void);
void Touch_Isr_Init(void (*fp)(void));
void TOUCH_IntHandler(void);
static void StartTimeTick(void);
void ucos_IntroTitle(void);

extern unsigned int HandleTIMER4;
extern unsigned int HandleTIMER0;

/*
 * =====================================================================
 * Handling MMU Librarues
 * =====================================================================
 *   1) Only the section table is used. 
 *   2) The cachable/non-cachable area can be changed by MMT_DEFAULT value.
 *      The section size is 1MB.
 */

	/* 
	 * void MMU_Init(void)
	 * -----------------------------------------------------------------
	 */
void MMU_Init(void)
{
	unsigned long i,j;
	//========================== IMPORTANT NOTE =========================
	//The current stack and code area can't be re-mapped in this routine.
	//If you want memory map mapped freely, your own sophiscated MMU
	//initialization code is needed.
	//===================================================================

	MMU_DisableDCache();
	MMU_DisableICache();

	//If write-back is used,the DCache should be cleared.
	for(i=0;i<64;i++)
	for(j=0;j<8;j++)
    	    MMU_CleanInvalidateDCacheIndex((i<<26)|(j<<5));
	MMU_InvalidateICache();
       
	MMU_DisableMMU();
	MMU_InvalidateTLB();

	/* MMU_SetMTT(int vaddrStart,int vaddrEnd,int paddrStart,int attr) */
	MMU_SetMTT(0x00000000,0x07f00000,0x30000000,RW_NCNB); //bank0
	MMU_SetMTT(0x08000000,0x0ff00000,0x08000000,RW_NCNB); //bank1
	MMU_SetMTT(0x10000000,0x17f00000,0x10000000,RW_NCNB); //bank2
	MMU_SetMTT(0x18000000,0x1ff00000,0x18000000,RW_NCNB); //bank3
	MMU_SetMTT(0x20000000,0x27f00000,0x20000000,RW_NCNB); //bank4
	MMU_SetMTT(0x28000000,0x2ff00000,0x28000000,RW_NCNB); //bank5
	
	/* 
	 * USER WILL TEST WITH BANK 6
	 * the each attribute value such as RW_CB, RW_NCNB, .... was decleared on libc.h
	 *
	 * 1. Please IMPLEMENT MMU_SetMTT function
	 */
	MMU_SetMTT(0x30000000,0x31f00000,0x30000000,RW_CB);   //bank6-1, TEXT/DATA
	MMU_SetMTT(0x32000000,0x33e00000,0x32000000,RW_NCNB);  //bank6-2, TEST DATA,LCD FrameBuffer
	MMU_SetMTT(0x33f00000,0x33f00000,0x33f00000,RW_NCNB);   //bank6-3
	MMU_SetMTT(0x34000000,0x37f00000,0x34000000,RW_NCNB);   //bank6-3
	MMU_SetMTT(0x38000000,0x3ff00000,0x38000000,RW_NCNB); //bank7
    
	MMU_SetMTT(0x40000000,0x5af00000,0x40000000,RW_NCNB);//SFR+StepSram    
	MMU_SetMTT(0x5b000000,0xfff00000,0x5b000000,RW_FAULT);//not used

	/* 
	 * 2. Please IMPLEMENT MMU_SetTTBase function : libs.S 
	 */
	MMU_SetTTBase(_MMUTT_STARTADDRESS);
	
	/* DOMAIN1: no_access, DOMAIN0,2~15=client(AP is checked) */
	MMU_SetDomain(0x55555550|DOMAIN1_ATTR|DOMAIN0_ATTR); 

	MMU_SetProcessId(0x0);
	MMU_EnableAlignFault();

	/* 
	 * 3. Please IMPLEMENT MMU_EnableMMU,MMU_EnableICache and MMU_EnableDCache
	 *     : libs.S 
	 */
	MMU_EnableMMU();
	MMU_EnableICache();
	MMU_EnableDCache(); /* IMPORTANT : DCache should be turned on after MMU is turned on. */
}   

	/* 
	 * void MMU_SetMTT(int vaddrStart,int vaddrEnd,int paddrStart,int attr)
	 * -----------------------------------------------------------------
	 * Attribute : RW_CB,RW_CNB,RW_NCNB,RW_FAULT
	 */
void MMU_SetMTT(int vaddrStart,int vaddrEnd,int paddrStart,int attr)
{
	unsigned long *pTT;
	int i,nSec;
	
	/* 
	 * 1. Get Translation Table base address on Physical memory
	 *    Translation table held each 1 word translation descriptor for each 1MB section
	 *    The Level 1 translation descript location for each 1M section can be calcurated
	 *    from Translation Table Base address and virtual address 
	 *
	 *    The variable for actual translation descriptor location : pTT
	 *    MMU Translation Table Base Address : _MMUTT_STARTADDRESS
	 *	  Address bit [31:20] specify M byte address
	 */
	pTT=(unsigned long *)_MMUTT_STARTADDRESS+(vaddrStart>>20);
	
	/*
	 * 2. We must configure each 1M section descriptor
	 *    So, we must get a number of section from start virtual address (vaddrStart)
	 *        to end of virtual address (vaddrEnd)
	 *    The number of scetion : nSec
	 */ 
	nSec=(vaddrEnd>>20)-(vaddrStart>>20);
	
	for(i=0;i<=nSec;i++) 
	{
		/*
		 * 3. We must write down physical address and attribute information to 
		 *    each section descriptor
		 *    Section descriptor location : *pTT
		 *    Configuration Value : (physical address information) | (attribute)
		 *        The physical address information was held from address bit 20 (1M byte address)
		 */
		*pTT++ = (((paddrStart>>20)+i)<<20) | attr ;
	}
} 

	/* 
	 * void ChangeRomCacheStatus(int attr)
	 * -----------------------------------------------------------------
	 * Attribute : RW_CB,RW_CNB,RW_NCNB,RW_FAULT
	 */
void ChangeRomCacheStatus(int attr)
{
	int i,j;
	MMU_DisableDCache();
	MMU_DisableICache();

	//If write-back is used,the DCache should be cleared.
	for(i=0;i<64;i++)
    	for(j=0;j<8;j++)
    	    MMU_CleanInvalidateDCacheIndex((i<<26)|(j<<5));

	MMU_InvalidateICache();
	MMU_DisableMMU();
	MMU_InvalidateTLB();
	MMU_SetMTT(0x00000000,0x07f00000,0x00000000,attr);	//bank0
	MMU_SetMTT(0x08000000,0x0ff00000,0x08000000,attr);	//bank1
	MMU_EnableMMU();
	MMU_EnableICache();
	MMU_EnableDCache();
}    
    
/*
 * =====================================================================
 * LED Display Librarues
 * =====================================================================
 */
void Led_Init(void)
{	
	/* 
	 * LED On : Active Low 
	 * Initialize GPGDAT[7:4] : high
	 * Setup GPGCON[15:8] : 01-> GPG7~4 Output mode
	 * GPGUP pull up function disable
	 * 
	 */
	/* YOUR CODE HERE */  
	rGPGDAT |= (0xf<<4);
	
	rGPGCON &= ~(0xff<<8);
	rGPGCON |= (0x55<<8);
	rGPGUDP &= 0xf<<4;
}

void Led_Display(int data)
{
	/* 
 	 * LED On : Active high 
     * LED Off: Active low
	 * GPGDAT[7:4]
	 */
	
	rGPGDAT |= (0xf<<4);

	if(data & 0x01)  rGPGDAT &= ~(0x1<<7);
	if(data & 0x02)  rGPGDAT &= ~(0x1<<6);
	if(data & 0x04)  rGPGDAT &= ~(0x1<<5);
	if(data & 0x08)  rGPGDAT &= ~(0x1<<4);  

}

//
// UART0 인터럽트 서비스 루틴
//
#ifndef __BUFFERED_IO
void ISR_UART0(void)
{    
	rSUBSRCPND	= BIT_SUB_RXD1;
	rSRCPND1		= BIT_UART1;
	rINTPND1		= BIT_UART1;
}
#else
void  CommTxIntEn (INT8U ch);
void  CommRxIntEn (INT8U ch);
void  CommTxIntDis (INT8U ch);
void  CommRxIntDis (INT8U ch);

#define RX_READY	(1)
#define TX_EMPTY	(1<<1)

void ISR_UART0(void)
{
	INT8U err;
	INT8U c;

	if(rUTRSTAT1 & TX_EMPTY)
	{// tx
		rSUBSRCPND	= BIT_SUB_TXD1;
		rSRCPND1		= BIT_UART1;
		rINTPND1		= BIT_UART1;
		c = CommGetTxChar (2, &err);

		// Carriage return check!
		if(c=='\n')
		{
			while(!(rUTRSTAT1 & 0x2))
				;

			rUTXH1 = '\r';
		}
		while(!(rUTRSTAT1 & 0x2))	//Wait until THR is empty.
			;

		if(c)
		{
			rUTXH1 = c;
		}
		else
			CommTxIntDis (2);	// tx int disable
	}

	if(rUTRSTAT1 & RX_READY)
	{// rx
		rSUBSRCPND	= BIT_SUB_RXD1;
		rSRCPND1		= BIT_UART1;
		rINTPND1		= BIT_UART1;

		c = rURXH1;

		CommPutRxChar (2, c);
	}
}
#endif // __BUFFERED_IO

/*
 * =====================================================================
 * PORTS Librarues
 * =====================================================================
 */
void Port_Init(void)
{
}

void InitSystem(void)
{
	int i,j;

	Port_Init();    
	Uart_Init(115200);
	
// UART 인터럽트 등록
//@@@	pISR_UART0 	= (unsigned int)UART0_IntHandler;
#ifdef __XDEF_REPORT_STAT
	Graphic_Init();
#endif
	
#ifdef _MINI_CALC_
//	Buzzer_Init();
	Touch_Isr_Init((void (*)(void))TOUCH_IntHandler);
#endif // _MINI_CALC_

#ifdef __BUFFERED_IO
// UART BUFFERED I/O 초기화
//	CommInit();
//	CommTxIntEn(2);
//	CommRxIntEn(2);
#endif
	// start time TICK
	StartTimeTick();
	// display uc/os-ii intro
	ucos_IntroTitle();
}

// 타임틱 발생 함수 PWM의 Timer4를 사용함(Timer4가 PWM인가?)
static void StartTimeTick(void)
{
	INT32U ticf;				// Timer input clock Freauency

	// Timer input clock Freauency = PCLK / (prescaler value+1) / divider value
	ticf = PCLK / 16;
#if 1	// by TIMER4 
	rTCFG0 &= ~(0xff<<8);		// Prescaler1[15: 8]=0x00  (prescaler value=0)
	rTCFG1 = (0x3<<16);		//       MUX4[19:16]=0000b (divider   value=3)

	rTCNTB4 = ticf / OS_TICKS_PER_SEC;

	rTCON  = (1<<22)|(1<<21)|(0<<4)|(0<<3)|(0<<2)|(0<<1)|(0);
	rTCON &= ~(1<<21);
	rTCON |=  (1<<20);			// Start for Timer4

	// TimeTick 인터럽트 벡터 설치 & 인터럽트 마스킹 해제
       HandleTIMER4 = (unsigned int)OSTickISR;
       /* Interrupt Unmasking */
       rINTMSK1 &= ~(1<<14);
#else	// by TIMER0
	rTCFG0 = (0<<8)|(0); 
	rTCFG1 = (0<<20)|(3); 
	
	/* TCON설정 :Dead zone disable,  auto reload on, output inverter off
	*  manual update no operation, timer0 stop, TCNTB0=0, TCMPB0 =0
	*/
	rTCON  = (0<<4)|(1<<3)|(0<<2)|(0<<1)|(0);
	rTCNTB0 = 0;
	rTCMPB0 = 0;

	rTCNTB0 = ticf / OS_TICKS_PER_SEC;

	rTCON |= (1<<1)|(0);
	rTCON &= ~(1<<1);
	rTCON |= 1;

	// TimeTick 인터럽트 벡터 설치 & 인터럽트 마스킹 해제
       HandleTIMER0 = (unsigned int)OSTickISR;
       /* Interrupt Unmasking */
       rINTMSK1 &= ~(1<<10);
#endif
}

//
// Use Timer0 for PC_ElapsedStart
// Timer INPUT clock Resolution 1.5625MHz
void ElapseTimer_Start(void)
{
 rTCFG0 = (rTCFG0 & ~0xff) | 1;
 rTCFG1 = (rTCFG1 & ~0xf) | 3; 
 
 /* TCON설정 :Dead zone disable,  auto reload on, output inverter off
 *  manual update no operation, timer0 stop, TCNTB0=0, TCMPB0 =0
 */
 rTCON  = (rTCON & ~((1<<4)|(1<<2)|(1<<1)|(1))) | (1<<3);

 rTCNTB0 = 0;
 rTCMPB0 = 0;

 rTCNTB0 = 0xffff; // initial value(65535) for timer

 rTCON |= (1<<1)|(0);
 rTCON &= ~(1<<1);
 rTCON |= 1;
}

//
// Use Timer0 for PC_ElapsedStart
// Timer Resolution 1.5625MHz
unsigned int ElapseTimer_Stop(void)
{
 rTCON &= ~1;

 return (0xffff - rTCNTO0);
}


//
// 'ucos_IntroTitle'
//
void ucos_IntroTitle(void)
{
	Uart_Printf("]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]                    \n");
	Uart_Printf("]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]                     \n");
	Uart_Printf("]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]                      \n");
	Uart_Printf("]]]]]]]]]]]]]]]]      ]]]]]  ]]]     ]]]]]]     ]]]]]]]     ]]]   ]]]      (R)\n");
	Uart_Printf("]]]]]]  ]]  ]]]   ]]]  ]]]] ]]   ]]]   ]]]  ]]]]]]]]]]      ]]]   ]]]         \n");
	Uart_Printf("]]]]]]  ]]  ]]   ]]]]]]]]] ]]   ]]]]]  ]]]]     ]]]]]       ]]]   ]]]         \n");
	Uart_Printf("]]]]]]  ]]  ]]    ]]]] ]]] ]]]   ]]]   ]]]]]]]]] ]]] ]]]]]] ]]]   ]]]         \n");
	Uart_Printf("]]]]]]  ]]  ]]]]       ]] ]]]]]]     ]]]]]]    ]]]]         ]]]   ]]]         \n");
	Uart_Printf("]]]]]      ]]]]]]]]]]]]]] ]]]]]]]]]]]]]]]]]]]]]]]]          ]]]   ]]]         \n");
	Uart_Printf("]]]]  ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]                             \n");
	Uart_Printf("]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]   The Real Time Kernel \n");
	Uart_Printf("]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]\n");
	Uart_Printf("]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]     uC/OS-II version %1d.%02d\n", OSVersion() / 100, OSVersion() % 100);
	Uart_Printf("]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]\n");
	Uart_Printf("]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]   Copyright MDS Technology, Inc.,\n");
	Uart_Printf("\n");
	Uart_Printf("->\n");
}
