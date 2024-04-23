/***************************************************************************
                           can_core.c  -  description
                             -------------------
    begin             : 07.01.2011
    last modified     : 13.10.2015    
    copyright         : (C) 2011 - 2015 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include "log.h"
#include "mhs_obj.h"
#include "mhs_event.h"
#include "index.h"
#include "can_fifo.h"
//#include "tcan_drv.h" <*>
#include "can_core.h"

                    // DLC = 0, 2, 2, 3, 4, 5, 6, 7, 8, 9,  A,  B,  C,  D,  E,  F
const uint8_t DlcLenTab[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};

struct TCanCore
  {
  TMhsObjContext *MainContext;
  TMhsObjContext *Context;
  TMhsObjContext *FilterContext;
  TMhsEvent *Event;
  };

static struct TCanCore *CanCore = NULL;



void can_core_init(void)
{
CanCore = NULL;
}


int32_t can_core_create(void)
{
if (CanCore)
  return(0);
CanCore = (struct TCanCore *)mhs_calloc(1, sizeof(struct TCanCore));
if (!CanCore)
  return(-1);
CanCore->Event = mhs_event_create();
CanCore->Context = mhs_obj_context_new("", CanCore->Event);
CanCore->FilterContext = mhs_obj_context_new("", CanCore->Event);
CanCore->MainContext = mhs_obj_context_new("", CanCore->Event);
return(0);
}


void can_core_destroy(void)
{
if (!CanCore)
  return;
mhs_obj_context_destroy(&CanCore->Context);
mhs_obj_context_destroy(&CanCore->FilterContext);
mhs_obj_context_destroy(&CanCore->MainContext);
mhs_event_destroy(&CanCore->Event);
mhs_free(CanCore);
CanCore = NULL;
}


TMhsObjContext *can_core_get_context(void)
{
if (!CanCore)
  return(NULL);
else
  return(CanCore->Context);
}


TMhsObjContext *can_filter_get_context(void)
{
if (!CanCore)
  return(NULL);
else
  return(CanCore->FilterContext);
}


TMhsObjContext *can_main_get_context(void)
{
if (!CanCore)
  return(NULL);
else
  return(CanCore->MainContext);
}


TMhsObjContext *can_get_context(uint32_t index)
{
#if (TX_PUFFER_MAX > 0) || (RX_FILTER_MAX > 0)
TCanDevice *dev;
#endif

if (index & INDEX_FIFO_VIRTUAL)
  return(can_core_get_context());
else if (index & INDEX_TXT_FLAG)
  {
  if (index & INDEX_FIFO_PUFFER_MASK)
    {
#if TX_PUFFER_MAX > 0
    if ((dev = get_device(index)))
      return(dev->TxPufferContext);
#else
    return(NULL);
#endif
    }
  else
    return(can_core_get_context());
  }
else if (index & INDEX_SOFT_FLAG)
  return(can_filter_get_context());
else
  {
  if (index & INDEX_FIFO_PUFFER_MASK)
    {
#if RX_FILTER_MAX > 0
    if ((dev = get_device(index)))
      return(dev->RxFilterContext);
#else
    return(NULL);
#endif
    }
  else
    return(can_core_get_context());
  }
return(NULL);
}


uint8_t LenToDlc(uint8_t len)
{
if (len <= 8)
  return(len);
if (len > 48)
  return(0xF);  // Datenlänge = 64 Byte
if (len > 32)
  return(0xE);  // Datenlänge = 48 Byte
if (len > 24)
  return(0xD);  // Datenlänge = 32 Byte
if (len > 20)
  return(0xC);  // Datenlänge = 24 Byte
if (len > 16)
  return(0xB);  // Datenlänge = 20 Byte
if (len > 12)
  return(0xA);  // Datenlänge = 16 Byte
return(0x9);  // Datenlänge = 12 Byte
}


int CanFdCanCmp(struct TCanFdMsg *fd_msg, const struct TCanMsg *msg) 
{
uint32_t flags;

if (fd_msg->Id != msg->Id)
  return(-1);
if (fd_msg->MsgLen != msg->MsgLen)
  return(-1);
flags = (fd_msg->MsgFlags >> 12) & 0x000000F0;
if (flags != (msg->MsgFlags & 0x000000F0))
  return(-1);
if ((fd_msg->MsgSource & 0x0F) != msg->MsgSource)
  return(-1);
if (fd_msg->MsgFilHit != msg->MsgFilHit)
  return(-1);    
if (fd_msg->Data.Longs[0] != msg->Data.Longs[0])
  return(-1); 
if (fd_msg->Data.Longs[1] != msg->Data.Longs[1])
  return(-1); 
if (fd_msg->Time.Sec != msg->Time.Sec)
  return(-1);
if (fd_msg->Time.USec != msg->Time.Sec)
  return(-1);
return(0);
}
