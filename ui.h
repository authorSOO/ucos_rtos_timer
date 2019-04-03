#ifndef _UI_H_
#define _UI_H_

#include "os_cpu.h"

// Color Define
// [D15-D11]RED, [D10-D6]GREEN, [D5-D1]BLUE
#define BLACK	0x0000
#define WHITE	0xfffe
#define BLUE	0x003e
#define GREEN	0x07c0
#define RED		0xf800
#define YELLOW	0xffc0

enum {
		POS_0= 0,	// 76,199(42,27) 6
		POS_1,	// 76,169(42,27) 11
		POS_2,	// 122,169(42,27) 12
		POS_3,	// 167,169(42,27) 13
		POS_4,	// 76,139(42,27) 16
		POS_5,	// 122,139(42,27) 17
		POS_6,	// 167,139(42,27) 18				
		POS_7,	// 76,109(42,27) 21
		POS_8,	// 122,109(42,27) 22
		POS_9,	// 167,109(42,27) 23
		
		POS_INPUT,	// 22,43(278,21) 
		POS_BACKSPACE,	// 76,76(74,27) 10
		POS_CE,	// 153,76(74,27) 20
		POS_C,	// 228,76(74,27) 0

		POS_PLUSMINUS,	// 122,199(42,27) 7
		POS_POINT,	// 167,199(42,27) 8
		
		POS_DIV,	// 213,109(42,27) 24
		POS_MUL,	// 213,139(42,27) 19
		POS_MINUS,	// 213,169(42,27) 14
		POS_PLUS,	// 213,199(42,27) 9
		
		POS_SQRT,	// 258,109(42,27) 1
		POS_MODULA,	// 258,139(42,27) 2
		POS_1_X,	// 258,169(42,27) 3
		POS_RESULT,	// 258,199(42,27) 4
		POS_MAX
};

// UI('LCD') Ä¿¸Çµå
enum {
	UI_BUTTON_PRESSED,
	UI_LCDPRINTF,
	MAX_UI_COMMAND
};

typedef struct {
	INT16U command;	// 'LCD' command
	INT16U x;	// (x,y)
	INT16U y;	// (x,y)
	INT16U Color;	// fore ground color
	INT16U bgColor; // back ground color
	INT16U param;	// param
	char* msg;	// message buffer pointer
}LCD_DISPLAY_MSG;

#endif // _UI_H_


