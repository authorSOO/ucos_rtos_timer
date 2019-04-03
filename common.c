#include "common.h"

void
hardware_init(void)
{
	// Uart Initialize
	Uart_Init(115200);

	// LCD Initialize
	Lcd_Port_Init();
	NonPal_Lcd_Init();
}

void
clear_screen(const int color)
{
	int width=0;
	int height=0;

	for(height=0; height<LCD_HEIGHT; height++)
	{
		for(width=0; width<LCD_WIDTH; width++)
		{
			NonPal_Put_Pixel(width, height, color);
		}
	}
}
	
