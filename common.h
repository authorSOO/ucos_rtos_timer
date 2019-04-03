#ifndef __COMMON_H__
#define __COMMON_H__

#define LCD_WIDTH		480
#define LCD_HEIGHT		272

#define BLACK	0x0000
#define WHITE	0xfffe
#define BLUE	0x003e
#define GREEN	0x07c0
#define RED		0xf800

void hardware_init(void);
void clear_screen(const int color);

#endif
