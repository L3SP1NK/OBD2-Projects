#ifndef __UL_FIFO_H__
#define __UL_FIFO_H__

#include "global.h"

#ifdef __cplusplus
  extern "C" {
#endif


#define FIFO_PEND  1
#define FIFO_FULL  2
#define FIFO_OV    4

#define FIFO_PEND_FULL 3
#define FIFO_FULL_OV   7


typedef struct _TULongFifo TULongFifo;

struct _TULongFifo
  {
  uint8_t status;
  uint8_t overrun;
  uint32_t *base;
  uint32_t *end;
  uint32_t *read;
  uint32_t *write;
  };
  
/*****************************/
/*        FIFO status        */
/*****************************/
#define fifo_full(fifo) ((fifo)->status & FIFO_FULL)
#define fifo_overrun(fifo) ((fifo)->overrun)
#define fifo_overrun_clear(fifo) (fifo)->overrun = 0
#define fifo_status(fifo) (fifo)->status  

TULongFifo *ulong_fifo_create(int32_t size);
void ulong_fifo_destroy(TULongFifo *fifo); 
void ulong_fifo_clear(TULongFifo *fifo);
int32_t ulong_fifo_read(TULongFifo *fifo, uint32_t *data);
void ulong_fifo_write(TULongFifo *fifo, uint32_t data);

#ifdef __cplusplus
  }
#endif

#endif
