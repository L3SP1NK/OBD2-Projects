/*******************************************************************************
                         can_fifo.c  -  description
                             -------------------
    begin             : 18.12.2010
    last modified     : 30.05.2020
    copyright         : (C) 2008 - 2020 by MHS-Elektronik GmbH & Co. KG, Germany
    author            : Klaus Demlehner, klaus@mhs-elektronik.de
 ******************************************************************************/

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
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "can_core.h"
#include "mhs_class.h"
#include "mhs_obj.h"
#include "can_fifo.h"


static void mhs_object_destroy_cb(TMhsObj *obj);


static const struct TObjFuncs ObjFifoFuncs =
  {
  mhs_object_destroy_cb,
  NULL,
  NULL
  };


/***************************************************************/
/*                                                             */
/***************************************************************/
static void mhs_object_destroy_cb(TMhsObj *obj)
{
TObjCanFifo *fifo;

fifo = (TObjCanFifo *)obj;
safe_free(fifo->fd_base);
safe_free(fifo->base);
}


/***************************************************************************/
/*                                                  */
/***************************************************************************/
TObjCanFifo *mhs_can_fifo_create_unlocked(uint32_t index, const char *name, int32_t size, uint8_t fd, TMhsObjContext *context)
{
TMhsObj *obj;
TObjCanFifo *fifo;

if (mhs_object_get_by_index_unlocked(index, context))
  return(NULL);
#ifdef MHS_OBJ_USE_INDEX
obj = mhs_object_new_unlocked(OBJ_CAN_FIFO, index, name, &ObjFifoFuncs, sizeof(TObjCanFifo), context);
#else
obj = mhs_object_new_unlocked(OBJ_CAN_FIFO, name, &ObjFifoFuncs, sizeof(TObjCanFifo), context);
#endif
if (!obj)
  return(NULL);
fifo = (TObjCanFifo *)obj;

// FIFO Puffer anlegen
if (fd)
  {
  fifo->base = NULL;
  if (!(fifo->fd_base = mhs_malloc(size * sizeof(struct TCanFdMsg))))
    {
    mhs_obj_remove_by_index(index, context);
    return(NULL);
    }
  }
else
  {    
  fifo->fd_base = NULL;
  if (!(fifo->base = mhs_malloc(size * sizeof(struct TCanMsg))))
    {
    mhs_obj_remove_by_index(index, context);
    return(NULL);
    }
  }  
fifo->bufsize = size;
fifo->head = 0;
fifo->tail = 0;
fifo->status = 0;
fifo->overrun = 0;

return(fifo);
}


TObjCanFifo *mhs_can_fifo_create(uint32_t index, const char *name, int32_t size, uint8_t fd)
{
TMhsObjContext *context;
TObjCanFifo *obj;

context = can_core_get_context();
mhs_enter_critical(context->Event);
obj = mhs_can_fifo_create_unlocked(index, name, size, fd, context);
mhs_leave_critical(context->Event);
return(obj);
}


void mhs_can_fifo_destry(uint32_t index)
{
TMhsObjContext *context;

context = can_core_get_context();
mhs_obj_remove_by_index(index, context);
}


/***************************************************************/
/*                                                             */
/***************************************************************/
int32_t mhs_can_fifo_clear_unlocked(TObjCanFifo *fifo)
{
if (!fifo)
  return(-1);
fifo->head = 0;
fifo->tail = 0;
fifo->status = 0;
fifo->overrun = 0;
return(0);
}


int32_t mhs_can_fifo_clear(TObjCanFifo *fifo)
{
int32_t res;
TMhsObjContext *context;

if (!fifo)
  return(-1);
context = ((TMhsObj *)fifo)->Context;
mhs_enter_critical(context->Event);
res = mhs_can_fifo_clear_unlocked(fifo);
mhs_leave_critical(context->Event);
return(res);
}


int32_t mhs_can_fifo_clear_by_index(uint32_t index)
{
TObjCanFifo *fifo;

if (!(fifo = (TObjCanFifo *)mhs_object_get_by_index(index, can_core_get_context())))
  return(0);
return(mhs_can_fifo_clear(fifo));
}


void mhs_can_fifo_overrun_clear(TObjCanFifo *fifo)
{
if (!fifo)
  return;
fifo->overrun = 0;
}


void mhs_can_fifo_overrun_clear_by_index(uint32_t index)
{
TObjCanFifo *fifo;

if (!(fifo = (TObjCanFifo *)mhs_object_get_by_index(index, can_core_get_context())))
  return;
fifo->overrun = 0;
}


int32_t mhs_can_fifo_count(TObjCanFifo *fifo)
{
TMhsObjContext *context;
uint32_t rd_cnt;

if (!fifo->status)
  return(0);
context = ((TMhsObj *)fifo)->Context;
mhs_enter_critical(context->Event);
rd_cnt = fifo->head < fifo->tail ?
  (fifo->tail - fifo->head) :
  (fifo->tail - fifo->head + fifo->bufsize);
mhs_leave_critical(context->Event);
return(rd_cnt);
}


int32_t mhs_can_fifo_count_by_index(uint32_t index)
{
TObjCanFifo *fifo;

if (!(fifo = (TObjCanFifo *)mhs_object_get_by_index(index, can_core_get_context())))
  return(0);
return(mhs_can_fifo_count(fifo));
}



static int32_t can_fifo_get(TObjCanFifo *fifo, struct TCanMsg *msg_buf, int32_t count)
{
TMhsObjContext *context;
uint32_t rd_cnt, read;

if ((!fifo->status) || (count <= 0))
  return(0);
read = 0;
context = ((TMhsObj *)fifo)->Context;
mhs_enter_critical(context->Event);
if (fifo->status)
  {
  rd_cnt = fifo->head < fifo->tail ?
    (fifo->tail - fifo->head) :
    (fifo->tail - fifo->head + fifo->bufsize);

  if (rd_cnt > (uint32_t)count)
    rd_cnt = (uint32_t)count;
  while (read < rd_cnt)
    {
    if (fifo->fd_base)
      CanFdToCan(msg_buf, &fifo->fd_base[fifo->head]);
    else
      memcpy(msg_buf, &fifo->base[fifo->head], sizeof(struct TCanMsg));
    read++;
    msg_buf++;
    fifo->head++;
    fifo->head = fifo->head % fifo->bufsize;
    if (fifo->head == fifo->tail)
      {
      fifo->status = CANMSG_BUF_EMPTY;
      break;
      }
    fifo->status &= ~(CANMSG_BUF_FULL | CANMSG_BUF_OVR);
    }
  }
mhs_leave_critical(context->Event);
return(read);
}


static int32_t can_fifo_get_fd(TObjCanFifo *fifo, struct TCanFdMsg *msg_buf, int32_t count)
{
TMhsObjContext *context;
uint32_t rd_cnt, read;

if ((!fifo->status) || (count <= 0))
  return(0);
read = 0;
context = ((TMhsObj *)fifo)->Context;
mhs_enter_critical(context->Event);
if (fifo->status)
  {
  rd_cnt = fifo->head < fifo->tail ?
    (fifo->tail - fifo->head) :
    (fifo->tail - fifo->head + fifo->bufsize);

  if (rd_cnt > (uint32_t)count)
    rd_cnt = (uint32_t)count;
  while (read < rd_cnt)
    {
    if (fifo->base)
      CanToCanFd(msg_buf, &fifo->base[fifo->head]);
    else
      memcpy(msg_buf, &fifo->fd_base[fifo->head], sizeof(struct TCanFdMsg));
    read++;
    msg_buf++;
    fifo->head++;
    fifo->head = fifo->head % fifo->bufsize;
    if (fifo->head == fifo->tail)
      {
      fifo->status = CANMSG_BUF_EMPTY;
      break;
      }
    fifo->status &= ~(CANMSG_BUF_FULL | CANMSG_BUF_OVR);
    }
  }
mhs_leave_critical(context->Event);
return(read);
}


int32_t mhs_can_fifo_get(TObjCanFifo *fifo, struct TCanMsg *msg_buf, int32_t count)
{
int32_t read_count, read_total, res;

read_total = 0;
while (count)
  {
  if (count > 25)
    read_count = 25;
  else
    read_count = count;
  if ((res = can_fifo_get(fifo, msg_buf, read_count)) < 0)
    return(res);
  read_total += res;
  if (res < read_count)
    break;
  count -= res;
  msg_buf += res;
  }
return(read_total);
}


int32_t mhs_can_fifo_get_fd(TObjCanFifo *fifo, struct TCanFdMsg *msg_buf, int32_t count)
{
int32_t read_count, read_total, res;

read_total = 0;
while (count)
  {
  if (count > 25)
    read_count = 25;
  else
    read_count = count;
  if ((res = can_fifo_get_fd(fifo, msg_buf, read_count)) < 0)
    return(res);
  read_total += res;
  if (res < read_count)
    break;
  count -= res;
  msg_buf += res;
  }
return(read_total);
}


int32_t mhs_can_fifo_get_by_index(uint32_t index, struct TCanMsg *msg_buf, int32_t count)
{
TObjCanFifo *fifo;

if (!(fifo = (TObjCanFifo *)mhs_object_get_by_index(index, can_core_get_context())))
  return(0);
return(mhs_can_fifo_get(fifo, msg_buf, count));
}



int32_t mhs_can_fifo_get_fd_by_index(uint32_t index, struct TCanFdMsg *msg_buf, int32_t count)
{
TObjCanFifo *fifo;

if (!(fifo = (TObjCanFifo *)mhs_object_get_by_index(index, can_core_get_context())))
  return(0);
return(mhs_can_fifo_get_fd(fifo, msg_buf, count));
}


static __inline int32_t can_fifo_out_ref(TObjCanFifo *fifo, struct TCanMsg **msg_ref)
{
if ((!fifo->status) || (!fifo->base) || (!msg_ref))
  return(0);
*msg_ref = &fifo->base[fifo->head];
return(1);
}


static __inline int32_t can_fifo_out_fd_ref(TObjCanFifo *fifo, struct TCanFdMsg **msg_ref)
{
if ((!fifo->status) || (!fifo->fd_base) || (!msg_ref))
  return(0);
*msg_ref = &fifo->fd_base[fifo->head];
return(1);
}


static int32_t can_fifo_out_finish(TObjCanFifo *fifo)
{
TMhsObjContext *context;
uint32_t rd_cnt;

context = ((TMhsObj *)fifo)->Context;
mhs_enter_critical(context->Event);
if (fifo->status)
  {
  rd_cnt = fifo->head < fifo->tail ?
    (fifo->tail - fifo->head) :
    (fifo->tail - fifo->head + fifo->bufsize);
  if (rd_cnt)
    {  
    fifo->head++;
    fifo->head = fifo->head % fifo->bufsize;
    if (fifo->head == fifo->tail)      
      fifo->status = CANMSG_BUF_EMPTY;
    else        
      fifo->status &= ~(CANMSG_BUF_FULL | CANMSG_BUF_OVR);
    }
  }
mhs_leave_critical(context->Event);
return(1);
}


int32_t mhs_can_fifo_put_unlocked(TObjCanFifo *fifo, struct TCanMsg *msg_buf, int32_t count)
{
uint32_t i, w;

if (count <= 0)
  return(0);
if (fifo->status & CANMSG_BUF_FULL)
  {
  fifo->status |= CANMSG_BUF_OVR;
  fifo->overrun = 1;
  return(0);
  }
fifo->status |= CANMSG_BUF_PEND;
w = 0;
for (i = 0; i < (uint32_t)count; i++)
  {
  if (fifo->base)
    memcpy(&fifo->base[fifo->tail], msg_buf, sizeof(struct TCanMsg));
  else
    CanToCanFd(&fifo->fd_base[fifo->tail], msg_buf);
  msg_buf++;
  w++;
  fifo->tail++;
  fifo->tail = fifo->tail % fifo->bufsize;

  if (fifo->tail == fifo->head)
    {
    fifo->status |= CANMSG_BUF_FULL;
    if (i < ((uint32_t)count - 1))
      {
      fifo->status |= CANMSG_BUF_OVR;
      fifo->overrun = 1;
      }
    break;
    }
  }
return(w);
}


int32_t mhs_can_fifo_put_fd_unlocked(TObjCanFifo *fifo, struct TCanFdMsg *msg_buf, int32_t count)
{
uint32_t i, w;

if (count <= 0)
  return(0);
if (fifo->status & CANMSG_BUF_FULL)
  {
  fifo->status |= CANMSG_BUF_OVR;
  fifo->overrun = 1;
  return(0);
  }
fifo->status |= CANMSG_BUF_PEND;
w = 0;
for (i = 0; i < (uint32_t)count; i++)
  {
  if (fifo->base)
    CanFdToCan(&fifo->base[fifo->tail], msg_buf);
  else
    memcpy(&fifo->fd_base[fifo->tail], msg_buf, sizeof(struct TCanFdMsg));
  msg_buf++;
  w++;
  fifo->tail++;
  fifo->tail = fifo->tail % fifo->bufsize;

  if (fifo->tail == fifo->head)
    {
    fifo->status |= CANMSG_BUF_FULL;
    if (i < ((uint32_t)count - 1))
      {
      fifo->status |= CANMSG_BUF_OVR;
      fifo->overrun = 1;
      }
    break;
    }
  }
return(w);
}


int32_t mhs_can_fifo_put(TObjCanFifo *fifo, struct TCanMsg *msg_buf, int32_t count)
{
int32_t res;
TMhsObjContext *context;

if (!fifo)
  return(0);
context = ((TMhsObj *)fifo)->Context;
mhs_enter_critical(context->Event);
res = mhs_can_fifo_put_unlocked(fifo, msg_buf, count);
mhs_leave_critical(context->Event);
return(res);
}


int32_t mhs_can_fifo_put_fd(TObjCanFifo *fifo, struct TCanFdMsg *msg_buf, int32_t count)
{
int32_t res;
TMhsObjContext *context;

if (!fifo)
  return(0);
context = ((TMhsObj *)fifo)->Context;
mhs_enter_critical(context->Event);
res = mhs_can_fifo_put_fd_unlocked(fifo, msg_buf, count);
mhs_leave_critical(context->Event);
return(res);
}


// Nachfolgende Funktionen werden von socketcan verwendent
int32_t mhs_can_fifo_out_ref_by_index(uint32_t index, struct TCanMsg **msg_ref)
{
TObjCanFifo *fifo;

if (!(fifo = (TObjCanFifo *)mhs_object_get_by_index(index, can_core_get_context())))
  return(0);
return(can_fifo_out_ref(fifo, msg_ref));
}


int32_t mhs_can_fifo_out_finish_by_index(uint32_t index)
{
TObjCanFifo *fifo;

if (!(fifo = (TObjCanFifo *)mhs_object_get_by_index(index, can_core_get_context())))
  return(0);
return(can_fifo_out_finish(fifo));
}


int32_t mhs_can_fifo_out_fd_ref_by_index(uint32_t index, struct TCanFdMsg **msg_ref)
{
TObjCanFifo *fifo;

if (!(fifo = (TObjCanFifo *)mhs_object_get_by_index(index, can_core_get_context())))
  return(0);
return(can_fifo_out_fd_ref(fifo, msg_ref));
}




