/* Assembly for GNU */
/* ROM, SDRAM ¹× GCS ½ÅÈ£ ÃÊ±âÈ­ ¼³Á¤ */
/* Global º¯¼ö¿µ¿ª ÃÊ±âÈ­ */
/* Stack ÃÊ±âÈ­ */

#include "2450addr.inc"

	.extern main  
	.extern MMU_Init
	
	.equ _ISR_STARTADDRESS,		0x33ffff00

	/* The location of stacks
	 * stack layout design
	 * »ç¿ëÀÚ ÀÓÀÇ·Î ¸ñÀû¿¡ ¸Â°Ô Àç¼³°è µÇ¾î¾ß ÇÑ´Ù
	 * ÇöÀç ¼³°è´Â ZI-LIMITÀ» ½ºÅÃÀÇ Á¦ÀÏ À§·Î º»´Ù
	 */
	
	.equ	STACK_BASE_ADDR, 0x33ff8000
	.equ	SVCStack,		(STACK_BASE_ADDR - 0x3800)
	.equ	UserStack,		(STACK_BASE_ADDR - 0x2800)
	.equ	UndefStack,	(STACK_BASE_ADDR - 0x2400)
	.equ	AbortStack,	(STACK_BASE_ADDR - 0x2000)
	.equ	IRQStack,		(STACK_BASE_ADDR - 0x1000)
	.equ	FIQStack,		(STACK_BASE_ADDR - 0x0)

	/* Exception Ã³¸®¸¦ À§ÇÏ¿© °¢ ¸ðµåÀÇ ½ºÅÃÀ» ¼³Á¤ÇÏ¿©¾ß ÇÑ´Ù
	 * ¿ì¼± °¢ ¸ðµåÀÇ ½ºÅÃÀ» ¼³Á¤ÇÏ±â À§ÇÏ¿©´Â ¸ðµå °­Á¦ ÀüÈ¯ÀÌ ÇÊ¿äÇÏ´Ù
	 * ÀÌ´Â SVC¸ðµåºÎÅÍ ½ÃÀÛÇÏ¿© °¢ ¸ðµå·Î CPSRÀ» ¼öÁ¤ÇÏ¿© ÀüÈ¯ÇÑ´Ù
	 */
	.equ	Mode_USR,		0x10
	.equ	Mode_FIQ,		0x11
	.equ	Mode_IRQ,		0x12
	.equ	Mode_SVC,		0x13
	.equ	Mode_ABT,		0x17
	.equ	Mode_UND,	0x1b
	.equ	Mode_SYS,		0x1f
	.equ	Mode_MASK,	0x1f
	
	.equ	I_Bit,			0x80	/* when I bit is set, IRQ is disabled */
	.equ	F_Bit,			0x40	/* when F bit is set, FIQ is disabled */
	.equ	NOINT,			0xc0
	
	/* CONFIG_CLK_534_133_66	*/
	.equ	Startup_MDIV,			267
	.equ	Startup_PDIV,			3
	.equ	Startup_SDIV,			1
	.equ	Startup_EPLLSTOP,		0
	.equ	Startup_ARMCLKdiv,	0
	.equ	Startup_PREdiv,		0x1
	.equ	Startup_HCLKdiv,		0x1
	.equ	Startup_PCLKdiv,		1
	
	.equ	CLK_DIV_VAL,	((Startup_ARMCLKdiv<<9)|(Startup_PREdiv<<4)|(Startup_PCLKdiv<<2)|(Startup_HCLKdiv)|(1<<3))
	.equ	MPLL_VAL,	((Startup_EPLLSTOP<<24)|(Startup_MDIV<<14)|(Startup_PDIV<<5)|(Startup_SDIV))
	.equ	EPLL_VAL,	(32<<16)|(1<<8)|(2<<0)
	
	
	/* macro Á¤ÀÇ */
	.macro HANDLER, HandlerLabel, HandleLabel
\HandlerLabel:
	sub		sp,sp,#4		/* decrement sp(to store jump address) */
	stmfd	sp!,{r0}			/* PUSH the work register to stack(lr doesn`t push because */ 
						   	/* it return to original address) */
	ldr		r0,=\HandleLabel	/* load the address of HandleXXX to r0 */
	ldr		r0,[r0]         		/* load the contents(service routine start address) of HandleXXX */
	str		r0,[sp,#4]      	/* store the contents(ISR) of HandleXXX to stack */
	ldmfd	sp!,{r0,pc}     	/* POP the work register and pc(jump to ISR) */
	.endm

	.text
	.globl _start
_start:

	/* ResetHandler°¡ Ã³À½ºÎÅÍ ³ª¿À´Â °ÍÀÌ ¾Æ´Ï¶ó 
	 * vector ÁÖ¼Ò ¿µ¿ª¿¡´Â reset vector°¡ Á¸ÀçÇØ¾ß ÇÑ´Ù
	 * exceptionÀÌ ¹ß»ýÇÏ¸é ARMÀº ÇÏµå¿þ¾îÀûÀ¸·Î ´ÙÀ½ ÁÖ¼Ò·Î ºÐ±âµÈ´Ù
	 */
	b	ResetHandler
	b	HandlerUndef			/* handler for Undefined mode */
	b	HandlerSWI			/* handler for SWI interrupt */
	b	HandlerPabort			/* handler for PAbort */
	b	HandlerDabort			/* handler for DAbort */
	b	.						/* reserved */
	b	HandlerIRQ				/* handler for IRQ interrupt */
	b	HandlerFIQ				/* handler for FIQ interrupt */

	.ltorg
HANDLER HandlerFIQ, HandleFIQ
HANDLER HandlerIRQ, HandleIRQ
HANDLER HandlerUndef, HandleUndef
HANDLER HandlerSWI, HandleSWI
HANDLER HandlerDabort, HandleDabort
HANDLER HandlerPabort, HandlePabort

	/* Reset HandlerÀÇ ½ÃÀÛ */
	.globl ResetHandler
ResetHandler:
	ldr	r0,=WTCON		/* watch dog disable  */
	ldr	r1,=0x0         
	str	r1,[r0]

	ldr	r0,=INTMSK1
	ldr	r1,=0xffffffff		/* all interrupt disable */
	str	r1,[r0]
	ldr	r0,=INTMSK2
	ldr	r1,=0xffffffff
	str	r1,[r0]

	ldr	r0,=INTSUBMSK
	ldr	r1,=0x7fff			/* all sub interrupt disable */
	str	r1,[r0]

	/* Set clock PLL */	
	ldr	r0,=CLKDIV0CON	/* Configure Clock Divide 	for FCLK:HCLK:PCLK  */
	ldr	r1,=CLK_DIV_VAL
	str	r1,[r0]
	
	ldr	r0,=LOCKCON0		/* Set MPLL LockTime*/
	ldr	r1,=0xe10
	str	r1,[r0]

	ldr	r0,=MPLLCON		/* Configure MPLL */
	ldr	r1,=MPLL_VAL   	/* 532MHz @ 12MHz */
	str	r1,[r0]
	
	ldr	r0,=LOCKCON1		/*Set EPLL LockTime*/
	ldr	r1, =0x1780
	str	r1, [r0]
	
	ldr	r0,=EPLLCON		/*Set EPLL*/
	ldr	r1, =EPLL_VAL		
	str	r1, [r0]
	
	ldr	r0,=CLKSRCCON	/* set CLKSRC to fit MPLL and USB */
	ldr	r1, [r0]
	orr	r1, r1, #0x50
	str	r1, [r0]
	
	/* Set memory control registers */
       ldr	r0,=SMRDATA
	ldr	r1,=BANKCFG	/* BANKCFG Address (Start Address) */
	add	r2, r0, #20		/* End address of SMRDATA => total registers = 20/4 = 5°³ */
0:
	ldr	r3, [r0], #4   	/* post increment */
	str	r3, [r1], #4  	/* post increment */
	cmp	r2, r0
	bne	0b

	/* C¾ð¾îÀÇ º¯¼ö ¿µ¿ªÀ» ¼³Á¤ÇÏ±â À§ÇÑ ·çÆ¾
	 * ±âº»ÀûÀÎ ¸Þ¸ð¸® ¼³Á¤ÀÌ ¿Ï·áµÇ°í ¼öÇàÇÏ¿©¾ß ÇÑ´Ù
	 * stackÀÇ »ý¼º
	 * ±âº»ÀûÀ¸·Î C¾ð¾î ÇÁ·Î±×·¥¿¡¼­ »ç¿ëÇÏ´Â ½ºÅÃÀÌ »ý¼ºµÇ¾î¾ß
	 * ÇÔ¼öÈ£Ãâ°ú Áö¿ªº¯¼öÀÇ »ç¿ëÀÌ °¡´ÉÇÏ´Ù
	 */

	/* Initialize stacks */
	bl	InitStacks
	
	/* Copy and paste RW data zero initialized data */
	ldr	r0, =rodata_end_addr 	/* text_end_addr Get pointer to ROM data */
	ldr	r1, =data_start_addr 	/* bss_end_addr and RAM copy */
	ldr	r3, =data_end_addr  	/* bss_start_addr  */
	mov r2, r0, lsr #12
 	mov r4, r1, lsr #12

	/* Zero init base => top of initialised data */
	cmp	r2, r4      		/* Check that they are different */
	beq	2f
1:       
	cmp	r1, r3      		/* Copy init data */
	ldrcc	r2, [r0], #4    /* --> LDRCC r2, [r0] + ADD r0, r0, #4 */         
	strcc	r2, [r1], #4    /* --> STRCC r2, [r1] + ADD r1, r1, #4 */
	bcc	1b
2:       
	mov	r2, #0 
       ldr r1,=bss_end_addr 
       ldr r3,=bss_start_addr
3:       
	cmp	r3, r1      /* Zero init */
	strcc	r2, [r3], #4
	bcc	3b
	
	/* ¿©±â¼­ IRQ°¡ ¹ß»ýÇÒ¶§ À§¿¡¼­ ¸¸µç 
	 * IsrIRQ ·çÆ¾À¸·Î ºÐ±âÇÏ±â À§ÇÑ ISR_VECTOR ¼³Á¤À» ÇÑ´Ù 
	 */
	/* Setup IRQ handler */
	ldr	r0,=HandleIRQ  
	ldr	r1,=IsrIRQ			
	str	r1,[r0]

	/* MMU_Init */
	bl MMU_Init

	/* ÄÄÆÄÀÏ·¯ÀÇ startup ·çÆ¾À» È£ÃâÇÏ°Å³ª È¤Àº ÀÓÀÇÀÇ MainÀ» È£Ãâ
	 * b MainÇÏ¸é C ÇÔ¼ö¿¡¼­ µ¹¾Æ¿ÀÁö ¾ÊÀ½À» °¡Á¤ÇÏ´Â °ÍÀÌ¸ç
	 * bl ÇÏ¸é µÇµ¹¾Æ ¿Ã¼öµµ ÀÖÀ½À» °¡Á¤ÇÑ´Ù.
	 * µû¶ó¼­ µÇ·Ï¾Æ ¿Â ÈÄ ÇÒ ÀÏÀÌ ¾øÀ¸¸é b %B0 ÇÏ¸é ¹«ÇÑ ·çÇÁ°¡ Çü¼ºµÈ´Ù.
	 * ÀÌ¶§, Àý´ë·Î ÀÓº£µðµå ½Ã½ºÅÛÀº ÇÁ·Î±×·¥ Á¾·á°¡ ¾ø¾î¾ß ÇÏ¹Ç·Î
	 * bl mainÇÑ ÈÄ µ¹¾Æ¿Í¼­ ¹«ÇÑ·çÇÁ¸¦ Çü¼ºÇÏÁö ¾Ê´Â ÀÏÀ» ¾øµµ·Ï ÁÖÀÇÇØ¾ß ÇÑ´Ù
	 */
	
	bl Exception_Init

	BL  main	
	b	.

	/* IRQ Handler »ý¼º
	 * IRQ´Â ¹ß»ý ¼Ò½º°¡ ´Ù¾çÇÏ±â ¶§¹®¿¡ ÇØ´ç C·çÆ¾ÀÇ ÁÖ¼Ò¸¦ È¹µæÇÏ¿©¾ß ÇÑ´Ù
	 */
	.globl IsrIRQ
IsrIRQ: 
        sub     sp,sp,#4       //reserved for PC
        stmfd   sp!,{r8-r9}   
 
 // check  INTOFFSET1
 chk_off1:
        ldr     r9,=INTOFFSET1
        ldr     r9,[r9]
        cmp r9,#0
 	 beq   chk_off2
        ldr     r8,=HandleEINT0
        add     r8,r8,r9,lsl #2
        b end_off

 // check  INTOFFSET2
 chk_off2:
        ldr     r9,=INTOFFSET2
        ldr     r9,[r9]
        cmp r9,#0
 	 beq   abt_return
        ldr     r8,=Handle2D
        add     r8,r8,r9,lsl #2

end_off: // generic interrupt
        ldr     r8,[r8]
        str     r8,[sp,#8]
        ldmfd   sp!,{r8-r9,pc} 

abt_return: // no interrupt
	 sub lr,lr,#4
        str   lr,[sp,#8]
        ldmfd   sp!,{r8-r9,pc}^

	/* function initializing stacks */
InitStacks:
	/* Don't use DRAM,such as stmfd,ldmfd......
	 * SVCstack is initialized before
	 * Under toolkit ver 2.5, 'msr cpsr,r1' can be used instead of 'msr cpsr_cxsf,r1'
	 */
	mrs	 r0,cpsr
	bic	r0,r0,#Mode_MASK
	orr	r1,r0,#Mode_UND|NOINT
	msr	 cpsr_cxsf,r1		/* UndefMode */
	ldr	sp,=UndefStack
	
	orr	r1,r0,#Mode_ABT|NOINT
	msr	 cpsr_cxsf,r1		/* AbortMode */
	ldr	sp,=AbortStack

	orr	r1,r0,#Mode_IRQ|NOINT
	msr	 cpsr_cxsf,r1		/* IRQMode */
	ldr	sp,=IRQStack
    
	orr	r1,r0,#Mode_FIQ|NOINT
	msr	 cpsr_cxsf,r1		/* FIQMode */
	ldr	sp,=FIQStack

	orr	r1,r0,#Mode_SYS|NOINT
	msr	 cpsr_cxsf,r1		/* SystemMode */
	ldr	sp,=UserStack
	
	bic	r0,r0,#I_Bit
	orr	r1,r0,#Mode_SVC
	msr cpsr_cxsf,r1		/* SVCMode interrupt enable */
	ldr	sp,=SVCStack
	
	/* USER mode has not be initialized. */
	
	mov pc,lr 
	/* The LR register won't be valid if the current mode is not SVC mode. */

	.globl _sbrk
_sbrk:
	nop
	mov pc,lr

	/* MEMORY ¼³Á¤À» À§ÇÑ Table
	 * ±¸µ¿ÇÏ°íÀÚ ÇÏ´Â ½Ã½ºÅÛ¿¡ µû¶ó ¼³Á¤Àº ´Þ¶óÁø´Ù
	 */
	
;	.pool
;	.align 2
	.ltorg
SMRDATA:
	.word	0x0004920d
	.word	0x04000040
	.word	0x00570039
	.word	0x80000030
	.word	0x00000313	

	/* C¿¡¼­ Ã³¸®ÇÒ Exception°ú Interrupt ÇÔ¼ö ÁÖ¼Ò¸¦ ÀúÀåÇÒ À§Ä¡
	 * startupÀº ÀÌ ÁÖ¼Ò¿¡¼­ ÇÔ¼öÁÖ¼Ò¸¦ È¹µæÇÏ¿© ºÐ±âÇÑ´Ù.
	 */

	.ltorg

	.align 4
	.section int_vect, "w"

	//0x33ffff00
	.globl HandleReset
	.globl HandleUndef
	.globl HandleSWI
	.globl HandlePabort
	.globl HandleDabort
	.globl HandleReserved
	.globl HandleIRQ
	.globl HandleFIQ
HandleReset:		.long   (_ISR_STARTADDRESS + 0x0)
HandleUndef:		.long   (_ISR_STARTADDRESS + 0x4)
HandleSWI:			.long   (_ISR_STARTADDRESS + 0x8)
HandlePabort:		.long   (_ISR_STARTADDRESS + 0xc)
HandleDabort:		.long   (_ISR_STARTADDRESS + 0x10)
HandleReserved:	.long   (_ISR_STARTADDRESS + 0x14)
HandleIRQ:			.long   (_ISR_STARTADDRESS + 0x18)
HandleFIQ:			.long   (_ISR_STARTADDRESS + 0x1c)

	//0x33ffff20
	.globl HandleEINT0
	.globl HandleEINT1
	.globl HandleEINT2
	.globl HandleEINT3
HandleEINT0:		.long	(_ISR_STARTADDRESS + 0x20)
HandleEINT1:		.long	(_ISR_STARTADDRESS + 0x24)
HandleEINT2:		.long	(_ISR_STARTADDRESS + 0x28)
HandleEINT3:		.long	(_ISR_STARTADDRESS + 0x2c)

	//0x33ffff30
	.globl HandleEINT4_7
	.globl HandleEINT8_23
	.globl HandleCAM
	.globl HandleBATFLT
HandleEINT4_7:	.long	(_ISR_STARTADDRESS + 0x30)
HandleEINT8_23:	.long	(_ISR_STARTADDRESS + 0x34)
HandleCAM:		.long	(_ISR_STARTADDRESS + 0x38)
HandleBATFLT:		.long	(_ISR_STARTADDRESS + 0x3c)

	//0x33ffff40
	.globl HandleTICK
	.globl HandleWDT
	.globl HandleTIMER0
	.globl HandleTIMER1
HandleTICK:		.long	(_ISR_STARTADDRESS + 0x40)
HandleWDT:		.long	(_ISR_STARTADDRESS + 0x44)
HandleTIMER0:		.long	(_ISR_STARTADDRESS + 0x48)
HandleTIMER1:		.long	(_ISR_STARTADDRESS + 0x4c)

	//0x33ffff50
	.globl HandleTIMER2
	.globl HandleTIMER3
	.globl HandleTIMER4
	.globl HandleUART2
HandleTIMER2:		.long	(_ISR_STARTADDRESS + 0x50)
HandleTIMER3:		.long	(_ISR_STARTADDRESS + 0x54)
HandleTIMER4:		.long	(_ISR_STARTADDRESS + 0x58)
HandleUART2:		.long	(_ISR_STARTADDRESS + 0x5c)

	//0x33ffff60
	.globl HandleLCD
	.globl HandleDMA
	.globl HandleUART3
	.globl HandleCFCON
HandleLCD:		.long	(_ISR_STARTADDRESS + 0x60)
HandleDMA:		.long	(_ISR_STARTADDRESS + 0x64)
HandleUART3:		.long	(_ISR_STARTADDRESS + 0x68)
HandleCFCON:		.long	(_ISR_STARTADDRESS + 0x6c)

	//0x33ffff70
	.globl HandleSDI1
	.globl HandleSDI0
	.globl HandleSPI0
	.globl HandleUART1
HandleSDI1:		.long	(_ISR_STARTADDRESS + 0x70)
HandleSDI0:		.long	(_ISR_STARTADDRESS + 0x74)
HandleSPI0:		.long	(_ISR_STARTADDRESS + 0x78)
HandleUART1:		.long	(_ISR_STARTADDRESS + 0x7c)

	//0x33ffff80
	.globl HandleNFCON
	.globl HandleUSBD
	.globl HandleUSBH
	.globl HandleIIC
HandleNFCON:		.long	(_ISR_STARTADDRESS + 0x80)
HandleUSBD:		.long	(_ISR_STARTADDRESS + 0x84)
HandleUSBH:		.long	(_ISR_STARTADDRESS + 0x88)
HandleIIC:			.long	(_ISR_STARTADDRESS + 0x8c)

	//0x33ffff90
	.globl HandleUART0
	.globl HandleSPI1
	.globl HandleRTC
	.globl HandleADC
HandleUART0:		.long	(_ISR_STARTADDRESS + 0x90)
HandleSPI1:		.long	(_ISR_STARTADDRESS + 0x94)
HandleRTC:		.long	(_ISR_STARTADDRESS + 0x98)
HandleADC:		.long	(_ISR_STARTADDRESS + 0x9c)

	//0x33ffffA0
	.globl Handle2D
	.globl HandleIIC1
	.globl HandleRSV1
	.globl HandleRSV2
Handle2D:			.long	(_ISR_STARTADDRESS + 0xA0)
HandleIIC1:			.long	(_ISR_STARTADDRESS + 0xA4)
HandleRSV1:		.long	(_ISR_STARTADDRESS + 0xA8)
HandleRSV2:		.long	(_ISR_STARTADDRESS + 0xAc)

	//0x33ffffB0
	.globl HandlePCM0
	.globl HandlePCM1
	.globl HandleI2S0
	.globl HandleI2S1
HandlePCM0:		.long	(_ISR_STARTADDRESS + 0xB0)
HandlePCM1:		.long	(_ISR_STARTADDRESS + 0xB4)
HandleI2S0:		.long	(_ISR_STARTADDRESS + 0xB8)
HandleI2S1:		.long	(_ISR_STARTADDRESS + 0xBc)
