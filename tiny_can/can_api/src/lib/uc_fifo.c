/***************************************************************************
                         ul_fifo.c  -  description
                             -------------------
    begin             : 14.07.2022
    last modify       : 14.07.2022
    copyright         : (C) 2022 by MHS-Elektronik GmbH & Co. KG, Germany
    author            : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/

/**
    Library to talk to Tiny-CAN devices. You find the latest versions at
       http://www.tiny-can.com/
**/
#include "global.h"
#include "util.h"
#include "uc_fifo.h"


/*****************************/
/*    FIFO Initialisieren    */
/*****************************/
TCharFifo *char_fifo_create(int32_t size) 
{
TCharFifo *fifo;
uint8_t *data;

fifo = (TCharFifo *)mhs_calloc(1, sizeof(TCharFifo));
data = mhs_malloc(size);
fifo->base = data;
fifo->end = &data[(size-1)];
fifo->read = fifo->base;
fifo->write = fifo->base;
fifo->status = 0;
fifo->overrun = 0;
return(fifo);
}


void char_fifo_destroy(TCharFifo *fifo)
{
if (fifo)
  {
  mhs_free(fifo->base);
  mhs_free(fifo);
  }
}


/*****************************/
/*        FIFO löschen       */
/*****************************/
void char_fifo_clear(TCharFifo *fifo) 
{
fifo->read = fifo->base;
fifo->write = fifo->base;
fifo->status = 0;
fifo->overrun = 0;
}


/*****************************/
/*        FIFO lesen         */
/*****************************/
int32_t char_fifo_get(TCharFifo *fifo, uint8_t *data) 
{
if (fifo->status) 
  { 
  *data = *fifo->read;
  if ((++fifo->read) > fifo->end)
    fifo->read = fifo->base;
  if (fifo->read == fifo->write)
    fifo->status = 0;
  else
    fifo->status = CHAR_FIFO_PEND;
  return(1);
  }
else
  return(0);
}


uint32_t char_fifo_read(TCharFifo *fifo, uint8_t *data, uint32_t size) 
{
uint32_t rd_count;

rd_count = 0;
for (; size; size--)
  {
  if (fifo->status) 
    { 
    *data++ = *fifo->read;
    if ((++fifo->read) > fifo->end)
      fifo->read = fifo->base;
    if (fifo->read == fifo->write)
      fifo->status = 0;
    else
      fifo->status = CHAR_FIFO_PEND;
    rd_count++;
    }
  else
    break;
  }
return(rd_count);
}


/*****************************/
/*       FIFO schreiben      */
/*****************************/
void char_fifo_put(TCharFifo *fifo, uint8_t data) 
{
if (fifo->status & CHAR_FIFO_FULL)
  {
  fifo->status = CHAR_FIFO_FULL_OV;
  fifo->overrun = 1;
  }
else
  {
  *fifo->write = data;
  if ((++fifo->write) > fifo->end)
    fifo->write = fifo->base;
  if (fifo->write == fifo->read)
    fifo->status = CHAR_FIFO_PEND_FULL;
  else
    fifo->status = CHAR_FIFO_PEND;
  }
}


uint32_t char_fifo_write(TCharFifo *fifo, uint8_t *data, uint32_t size) 
{
uint32_t wr_count;

wr_count = 0;
for (; size; size--)
  {
  if (fifo->status & CHAR_FIFO_FULL)
    return(wr_count);
  *fifo->write = *data++;
  if ((++fifo->write) > fifo->end)
    fifo->write = fifo->base;
  if (fifo->write == fifo->read)
    fifo->status = CHAR_FIFO_PEND_FULL;
  else
    fifo->status = CHAR_FIFO_PEND;
  wr_count++;
  }
return(wr_count);  
}

