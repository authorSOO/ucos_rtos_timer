#include "includes.h"
#include <sys/reent.h>

/* semaphore to protect the heap */

static OS_EVENT *heapsem;


/* id of the task that is

   currently manipulating the heap */

static int lockid;


/* number of times

   __malloc_lock has recursed */

static int locks= 0;

void malloc_lock_init(void)
{
   locks= 0;
   heapsem= OSSemCreate(1);
   if(!heapsem){ printf("ERROR OSSemCreate!\n"); }
}
#if 1
void __malloc_lock ( struct _reent *_r )
{
#if 1
   OS_TCB tcb;
   OS_SEM_DATA semdata;
#endif
   INT8U err;
   int id;

//   OSSemPend( heapsem, 0, &err );
//   return;

#if 1
   /* use our priority as a task id */
   OSTaskQuery( OS_PRIO_SELF, &tcb );
   id = tcb.OSTCBPrio;

   /* see if we own the heap already */
   OSSemQuery( heapsem, &semdata );

   if( semdata.OSEventGrp && (id == lockid) ) {
      /* we do; just count the recursion */
      Uart_Printf("<$>");
      locks++;
   }
   else {
      /* wait on the other task to yield the
         heap, then claim ownership of it */
 //     Uart_Printf("<P>");
      OSSemPend( heapsem, 0, &err );
      locks= 1;
      lockid = id;
   }
//  printf("call __malloc_lock(%d)\n", locks);
   return;
#endif
}


void __malloc_unlock ( struct _reent *_r )
{
   /* release the heap once the number of
      locks == the number of unlocks */
//   OSSemPost( heapsem );
//   return;

   if( (--locks) == 0 ) {
 //     Uart_Printf("<V>");
      lockid = -1;
      OSSemPost( heapsem );
   }
 //  printf("call __malloc_unlock(%d)\n", locks);
}

#endif