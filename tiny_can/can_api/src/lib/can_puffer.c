/*******************************************************************************
                         can_puffer.c  -  description
                             -------------------
    begin             : 07.01.2011
    copyright         : (C) 2011 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include "mhs_class.h"
#include "mhs_obj.h"
#include "can_core.h"
#include "can_obj.h"
#include "can_puffer.h"



static const struct TObjFuncs ObjFifoFuncs =
  {
  NULL,
  NULL,
  NULL
  };


/***************************************************************************/
/*                                                  */
/***************************************************************************/
TObjCanPuffer *mhs_can_puffer_create_unlocked(uint32_t obj_class, uint32_t index, const char *name, TMhsObjContext *context)
{
TMhsObj *obj;
TObjCanPuffer *puffer;

if (!context)
  return(NULL);
//if (mhs_object_get_by_name_unlocked(name, context))
//  return(NULL);
#ifdef MHS_OBJ_USE_INDEX
obj = mhs_object_new_unlocked(obj_class, index, name, &ObjFifoFuncs, sizeof(TObjCanPuffer), context);
#else
obj = mhs_object_new_unlocked(obj_class, name, &ObjFifoFuncs, sizeof(TObjCanPuffer), context);
#endif
if (!obj)
  return(NULL);
puffer = (TObjCanPuffer *)obj;
puffer->Update = -1;
return(puffer);
}


TObjCanPuffer *mhs_can_puffer_create(uint32_t obj_class, uint32_t index, const char *name, TMhsObjContext *context)
{
TObjCanPuffer *obj;

if (!context)
  return(NULL);
mhs_enter_critical(context->Event);
obj = mhs_can_puffer_create_unlocked(obj_class, index, name, context);
mhs_leave_critical(context->Event);
return(obj);
}


/***************************************************************/
/*                                                             */
/***************************************************************/
int32_t mhs_can_puffer_clear(TObjCanPuffer *puffer)
{
TMhsObjContext *context;

if (!puffer)
  return(-1);
context = ((TMhsObj *)puffer)->Context;
mhs_enter_critical(context->Event);
puffer->Update = 0;
mhs_leave_critical(context->Event);
return(0);
}


int32_t mhs_can_puffer_clear_by_index(uint32_t index, TMhsObjContext *context)
{
TObjCanPuffer *puffer;

if (!(puffer = (TObjCanPuffer *)mhs_object_get_by_index(index, context)))
  return(0);
return(mhs_can_puffer_clear(puffer));
}


/***************************************************************/
/*                                                             */
/***************************************************************/
int32_t mhs_can_puffer_put_fd_unlocked(TObjCanPuffer *puffer, struct TCanFdMsg *fd_msg)
{
int32_t res;

res = 0;
if (memcmp(&puffer->FdMsg, fd_msg, sizeof(struct TCanFdMsg)))
  {
  memcpy(&puffer->FdMsg, fd_msg, sizeof(struct TCanFdMsg));
  res = 1;
  }
if (puffer->Update < 0)
  {
  puffer->Update = 1;
  res = 1;
  }
else if (puffer->Update != 0xFFFF)
  puffer->Update++;  // Puffer geschrieben
return(res);
}


int32_t mhs_can_puffer_put_fd(TObjCanPuffer *puffer, struct TCanFdMsg *fd_msg)
{
int32_t res;
TMhsObjContext *context;

if (!puffer)
  return(0);
context = ((TMhsObj *)puffer)->Context;
mhs_enter_critical(context->Event);
res = mhs_can_puffer_put_fd_unlocked(puffer, fd_msg);
mhs_leave_critical(context->Event);
if (res > 0)
  mhs_object_set_event((TMhsObj *)puffer);
return(res);
}


int32_t mhs_can_puffer_put_fd_by_index(uint32_t index, struct TCanFdMsg *fd_msg, TMhsObjContext *context)
{
int32_t res;
TObjCanPuffer *puffer;

if (!(puffer = (TObjCanPuffer *)mhs_object_get_by_index(index, context)))
  return(0);
context = ((TMhsObj *)puffer)->Context;
mhs_enter_critical(context->Event);
res = mhs_can_puffer_put_fd_unlocked(puffer, fd_msg);
mhs_leave_critical(context->Event);
if (res > 0)
  mhs_object_set_event((TMhsObj *)puffer);
return(res);
}


/***************************************************************/
/*                                                             */
/***************************************************************/
int32_t mhs_can_puffer_get_fd(TObjCanPuffer *puffer, struct TCanFdMsg *fd_msg)
{
int32_t res;
TMhsObjContext *context;

context = ((TMhsObj *)puffer)->Context;
mhs_enter_critical(context->Event);
res = puffer->Update;
if (res >= 0)
  {
  memcpy(fd_msg, &puffer->FdMsg, sizeof(struct TCanFdMsg));
  puffer->Update = 0;
  }
mhs_leave_critical(context->Event);
return(res);
}


int32_t mhs_can_puffer_get_fd_by_index(uint32_t index, struct TCanFdMsg *fd_msg, TMhsObjContext *context)
{
TObjCanPuffer *puffer;

if (!(puffer = (TObjCanPuffer *)mhs_object_get_by_index(index, context)))
  return(0);
return(mhs_can_puffer_get_fd(puffer, fd_msg));
}


int32_t mhs_can_puffer_get_fd_copy(TObjCanPuffer *puffer, struct TCanFdMsg *fd_msg)
{
int32_t res;
TMhsObjContext *context;

context = ((TMhsObj *)puffer)->Context;
mhs_enter_critical(context->Event);
if (puffer->Update >= 0)
  {
  memcpy(fd_msg, &puffer->FdMsg, sizeof(struct TCanMsg));
  res = 1;
  }
else
  res = 0;
mhs_leave_critical(context->Event);
return(res);
}


/***************************************************************/
/*                                                             */
/***************************************************************/
int32_t mhs_can_puffer_put_unlocked(TObjCanPuffer *puffer, struct TCanMsg *msg)
{
int32_t res;

res = 0;
if (CanFdCanCmp(&puffer->FdMsg, msg))
  {
  CanToCanFd(&puffer->FdMsg, msg);
  res = 1;
  }
if (puffer->Update < 0)
  {
  puffer->Update = 1;
  res = 1;
  }
else if (puffer->Update != 0xFFFF)
  puffer->Update++;  // Puffer geschrieben
return(res);
}


int32_t mhs_can_puffer_put(TObjCanPuffer *puffer, struct TCanMsg *msg)
{
int32_t res;
TMhsObjContext *context;

if (!puffer)
  return(0);
context = ((TMhsObj *)puffer)->Context;
mhs_enter_critical(context->Event);
res = mhs_can_puffer_put_unlocked(puffer, msg);
mhs_leave_critical(context->Event);
if (res > 0)
  mhs_object_set_event((TMhsObj *)puffer);
return(res);
}


int32_t mhs_can_puffer_put_by_index(uint32_t index, struct TCanMsg *msg, TMhsObjContext *context)
{
int32_t res;
TObjCanPuffer *puffer;

if (!(puffer = (TObjCanPuffer *)mhs_object_get_by_index(index, context)))
  return(0);
context = ((TMhsObj *)puffer)->Context;
mhs_enter_critical(context->Event);
res = mhs_can_puffer_put_unlocked(puffer, msg);
mhs_leave_critical(context->Event);
if (res > 0)
  mhs_object_set_event((TMhsObj *)puffer);
return(res);
}


/***************************************************************/
/*                                                             */
/***************************************************************/
int32_t mhs_can_puffer_get(TObjCanPuffer *puffer, struct TCanMsg *msg)
{
int32_t res;
TMhsObjContext *context;

context = ((TMhsObj *)puffer)->Context;
mhs_enter_critical(context->Event);
res = puffer->Update;
if (res >= 0)
  {
  CanFdToCan(msg, &puffer->FdMsg);
  puffer->Update = 0;
  }
mhs_leave_critical(context->Event);
return(res);
}


int32_t mhs_can_puffer_get_by_index(uint32_t index, struct TCanMsg *msg, TMhsObjContext *context)
{
TObjCanPuffer *puffer;

if (!(puffer = (TObjCanPuffer *)mhs_object_get_by_index(index, context)))
  return(0);
return(mhs_can_puffer_get(puffer, msg));
}


int32_t mhs_can_puffer_get_copy(TObjCanPuffer *puffer, struct TCanMsg *msg)
{
int32_t res;
TMhsObjContext *context;

context = ((TMhsObj *)puffer)->Context;
mhs_enter_critical(context->Event);
if (puffer->Update >= 0)
  {
  CanFdToCan(msg, &puffer->FdMsg);
  res = 1;
  }
else
  res = 0;
mhs_leave_critical(context->Event);
return(res);
}


int32_t mhs_can_puffer_count(TObjCanPuffer *puffer)
{
int32_t res;
TMhsObjContext *context;

context = ((TMhsObj *)puffer)->Context;
mhs_enter_critical(context->Event);
res = puffer->Update;
mhs_leave_critical(context->Event);
return(res);
}


uint32_t mhs_can_puffer_get_intervall(TObjCanPuffer *puffer)
{
if (!puffer)
  return(0);
else
  return(puffer->Intervall);
}


void mhs_can_puffer_set_intervall(TObjCanPuffer *puffer, uint32_t intervall)
{
if (puffer)
  {
  if (puffer->Intervall != intervall)
    {
    puffer->Intervall = intervall;
    mhs_object_set_cmd_event((TMhsObj *)puffer);
    }
  }
}

