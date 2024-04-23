/***************************************************************************
                         ul_fifo.c  -  description
                             -------------------
    begin             : 16.10.2017
    last modify       : 16.10.2017
    copyright         : (C) 2017 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include "ul_fifo.h"


/*****************************/
/*    FIFO Initialisieren    */
/*****************************/
TULongFifo *ulong_fifo_create(int32_t size) 
{
TULongFifo *fifo;
uint32_t *data;

fifo = (TULongFifo *)mhs_calloc(1, sizeof(TULongFifo));
data = mhs_malloc(size * sizeof(uint32_t));
fifo->base = data;
fifo->end = &data[(size-1)];
fifo->read = fifo->base;
fifo->write = fifo->base;
fifo->status = 0;
fifo->overrun = 0;
return(fifo);
}


void ulong_fifo_destroy(TULongFifo *fifo)
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
void ulong_fifo_clear(TULongFifo *fifo) 
{
fifo->read = fifo->base;
fifo->write = fifo->base;
fifo->status = 0;
fifo->overrun = 0;
}


/*****************************/
/*        FIFO lesen         */
/*****************************/
int32_t ulong_fifo_read(TULongFifo *fifo, uint32_t *data) 
{
if (fifo->status) 
  { 
  *data = *fifo->read;
  if ((++fifo->read) > fifo->end)
    fifo->read = fifo->base;
  if (fifo->read == fifo->write)
    fifo->status = 0;
  else
    fifo->status = FIFO_PEND;
  return(1);
  }
else
  return(0);
}


/*****************************/
/*       FIFO schreiben      */
/*****************************/
void ulong_fifo_write(TULongFifo *fifo, uint32_t data) 
{
if (fifo->status & FIFO_FULL)
  {
  fifo->status = FIFO_FULL_OV;
  fifo->overrun = 1;
  }
else
  {
  *fifo->write = data;
  if ((++fifo->write) > fifo->end)
    fifo->write = fifo->base;
  if (fifo->write == fifo->read)
    fifo->status = FIFO_PEND_FULL;
  else
    fifo->status = FIFO_PEND;
  }
}

