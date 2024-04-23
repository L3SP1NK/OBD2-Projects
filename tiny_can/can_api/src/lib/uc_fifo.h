#ifndef __CHAR_FIFO_H__
#define __CHAR_FIFO_H__

#include "global.h"

#ifdef __cplusplus
  extern "C" {
#endif


#define CHAR_FIFO_PEND  1
#define CHAR_FIFO_FULL  2
#define CHAR_FIFO_OV    4

#define CHAR_FIFO_PEND_FULL 3
#define CHAR_FIFO_FULL_OV   7


typedef struct _TCharFifo TCharFifo;

struct _TCharFifo
  {
  uint8_t status;
  uint8_t overrun;
  uint8_t *base;
  uint8_t *end;
  uint8_t *read;
  uint8_t *write;
  };
  
/*****************************/
/*        FIFO status        */
/*****************************/
#define char_fifo_full(fifo) ((fifo)->status & FIFO_FULL)
#define char_fifo_overrun(fifo) ((fifo)->overrun)
#define char_fifo_overrun_clear(fifo) (fifo)->overrun = 0
#define char_fifo_status(fifo) (fifo)->status  

TCharFifo *char_fifo_create(int32_t size);
void char_fifo_destroy(TCharFifo *fifo); 
void char_fifo_clear(TCharFifo *fifo);
int32_t char_fifo_get(TCharFifo *fifo, uint8_t *data);
void char_fifo_put(TCharFifo *fifo, uint8_t data);

uint32_t char_fifo_read(TCharFifo *fifo, uint8_t *data, uint32_t size);
uint32_t char_fifo_write(TCharFifo *fifo, uint8_t *data, uint32_t size);

#ifdef __cplusplus
  }
#endif

#endif
