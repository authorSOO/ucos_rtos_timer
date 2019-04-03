/*=========================*/
#include "my_lib.h"
#include "option.h"
#include "2450addr.h"
#include "./images/bicycle.h"

int tick=0;
extern unsigned int HandleTIMER0;

// ISR Declaration
 __attribute__((interrupt("IRQ"))) void  Timer0_ISR(void)
{
	/* TODO : Pending Register Clear */
	rSRCPND1 = (1<<10);
	rINTPND1 = (1<<10);

	tick++;

	// Uart_Send_String("Timer ISR\n");  
}

void Main(void)
{
	int msec;

	Led_Port_Init(); //LED init
	Uart_Init(115200); // baudrate=115,200kbps
	Timer_Init();

	Led_Display(1); //LED is all 'on'
	Uart_Printf("Hello, UCOS~~\n");

	printf("Hello, UCOS~~ by printf\n");
 	
	Uart_Send_String("TIMER TEST !!!\n\n");

      HandleTIMER0 = (unsigned int)Timer0_ISR; //register a interrupt vector for timer0

      /* Interrupt Unmasking */
      rINTMSK1 &= ~(1<<10);

	msec= 10; // 0.01 second

	Timer_Delay(msec);

	while(1)
	{
		if((tick%100)==0) printf("tick=%d\n", tick);
	}	 
	return;
}
