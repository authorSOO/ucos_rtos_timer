#ifndef __2440UART_H__
#define __2440UART_H__

// Function Declaration
void Uart_Init(int baud);
void Uart_Printf(const char *fmt,...);
void Uart_Send_String(char *pt);
void Uart_Send_Byte(int data);

char Uart_GetCh(void);

#endif /*__2440UART_H__*/