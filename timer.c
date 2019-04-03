#include "2450addr.h"
#include "option.h"
#include "my_lib.h"

void Timer_Init(void)
{
	rTCFG0 = ((rTCFG0 & (~0xff)) | (0xff));				//timer0,1 
	rTCFG0 = ((rTCFG0 & (~(0xff<<8))) | (0xff<<8));		//timer2,3,4

	rTCFG1 = ((rTCFG1 & (~(0xf<<4))) | (0x3<<4));		//timer1
	rTCFG1 = ((rTCFG1 & (~(0xf<<8))) | (0x3<<8));		//timer2



	rTCON |= (0x1<<11);		//timer1 auto_reload
	rTCON |= (0x1<<15);		//timer2 auto_reload
	
	rTCNTB0 = 0;
	rTCMPB0 = 0;
	
	rTCNTB1 = 0;
	rTCMPB1 = 0;
		
	rTCNTB2 = 0;
	rTCMPB2 = 0;

	rINTMSK1 &= ~(0x1<<10);

}

void Timer_Delay(int msec)
{
	/*
	* 1) TCNTB0���� : �Ѱܹ޴� data�� ������ msec�̴�.
	*                  ���� msec�� �״�� TCNTB0������ ������ ���� ����.
	* 2) manual update�Ŀ�  timer0�� start��Ų��. 
	* 	 note : The bit has to be cleared at next writing.
	* 3) TCNTO0���� 0�� �ɶ����� ��ٸ���. 	
	*/
	rTCNTB0 = 16.113*msec;	

	rTCON |= (1<<1)|(0);
	rTCON &= ~(1<<1);
	
	rTCON |= 1;	
	
	while(rTCNTO0 != 0);
	
}


