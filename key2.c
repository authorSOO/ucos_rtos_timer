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


//void Main()
//{   
//    gpio_init();
//
//    // BUS INIT
//    APBCLOCK = (0xFFFFFFFF);
//    
//    Uart_Init(115200);
//
//    exti_init();
//
//    timer0_init();
//
//    putstr("Program Started!!\r\n");
//
//    while(1){
//        GPGDAT.LED = ~(0xF);
//        delay_ms(1000);
//        GPGDAT.LED = (0xF);
//        delay_ms(1000);
//    }
//}

// __irq =>  __attribute__((interrupt("IRQ")))

void  __attribute__((interrupt("IRQ"))) isr_eint_0(void)
{
    ClearPending1(BIT_EINT0);
    //putstr("e0\r\n");
}

void __attribute__((interrupt("IRQ"))) isr_eint_1(void)
{
    ClearPending1(BIT_EINT1);
    //putstr("e1\r\n");

}

void __attribute__((interrupt("IRQ"))) isr_eint_4_7(void){
    ClearPending1(BIT_EINT4_7);
    if(rEINTPEND & (0x1 <<4)){
        rEINTPEND = 0x1 << 4;

    }
    if(rEINTPEND & (0x1 <<5)){
        rEINTPEND = 0x1 << 5;

    }
}
