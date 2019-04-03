
#ifndef __PC_H
#define __PC_H

//#include "os_cpu.h"

void PC_ElapsedInit(void);
void PC_ElapsedStart(void);
INT16U PC_ElapsedStop(void);
void PC_DispStr (INT8U x, INT8U y, char *s, INT8U color);
void PC_DispChar (INT8U x, INT8U y, char c, INT8U color);
void TaskStatisticsTitleDisplay(char *name);

#endif // __PC_H
