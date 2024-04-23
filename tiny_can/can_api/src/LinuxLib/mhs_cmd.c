/***************************************************************************
                          tar_drv.c  -  description
                             -------------------
    begin             : 01.11.2010
    last modified     : 02.01.2016     
    copyright         : (C) 2010 - 2016 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include "errors.h"
//#include "tcan_drv.h" <*>
#include "global.h"
#include "can_core.h" // can_main_get_context()
#include "mhs_event.h"
#include "mhs_cmd.h"


#ifdef __WIN32__
// ****** Windows
#define cmd_enter_critical(l) EnterCriticalSection((l))
#define cmd_leave_critical(l) LeaveCriticalSection((l))
#else
// ****** Linux
#define cmd_enter_critical(l) pthread_mutex_lock((l))
#define cmd_leave_critical(l) pthread_mutex_unlock((l))
#endif



TMhsCmd *mhs_cmd_create(void)
{
TMhsCmd *cmd;

if ((cmd = (TMhsCmd *)mhs_event_create_ex(sizeof(TMhsCmd))))
  {
#ifdef __WIN32__   
  InitializeCriticalSection(&cmd->CmdLock);
#else  
  pthread_mutex_init((&cmd->CmdLock), NULL);
#endif  
  mhs_event_set_event_mask((TMhsEvent *)cmd, 0xFFFFFFFF);
  }
return(cmd);
}


void mhs_cmd_destroy(TMhsCmd **cmd)
{
TMhsCmd *c;

if (!cmd)
  return;
if ((c = *cmd))
  {
#ifdef __WIN32__   
  DeleteCriticalSection(&c->CmdLock);
#else  
  pthread_mutex_destroy(&c->CmdLock);
#endif  
  mhs_event_destroy(((TMhsEvent **)cmd));
  }
}




/***************************************************************/
/* ExecuteCommand                                              */
/***************************************************************/
int32_t mhs_cmd_execute(TCanDevice *dev, uint32_t command, void *param, uint32_t param_size, int32_t timeout)
{
TMhsObjContext *context, *main_context;
TMhsCmd *cmd;
int32_t res;
uint32_t wait, events;

if (!dev)
  return(-1);
cmd = dev->Cmd;
context = dev->Context;
main_context = can_main_get_context();
if (!dev->MainThread)
  return(0);
if (!dev->MainThread->Thread)
  return(0);
if (timeout > -1)
  wait = timeout;
else
  {    
  if (mhs_value_get_status("ExecuteCommandTimeout", context) & MHS_VAL_FLAG_WRITE)
    wait = mhs_value_get_as_ulong("ExecuteCommandTimeout", ACCESS_INIT, context);
  else
    wait = mhs_value_get_as_ulong("ExecuteCommandTimeout", ACCESS_INIT, main_context);
  }  
cmd_enter_critical(&cmd->CmdLock);
if (dev->MainThread->Run)
  {
  // Kommando variablen laden
  cmd->Command = command;
  cmd->ParamCount = param_size;
  cmd->Param = param;
  // Main Thread Event auslösen
  mhs_event_set((TMhsEvent *)dev->MainThread, MAIN_CMD_EVENT);
  // Ausführung des Kommandos abwarten
  events = mhs_wait_for_event((TMhsEvent *)cmd, wait);
  if (!events)   // Timeout
    {
    cmd->Command = 0;
    res = ERR_MAIN_THREAD_IS_BUSY;
    }
  else if (events & 0x7FFFFFFF)
    res = cmd->Result;
  else  
    res = ERR_MAIN_THREAD_NOT_RUN;             
  }
else
  res = ERR_MAIN_THREAD_NOT_RUN;
cmd_leave_critical(&cmd->CmdLock);
return(res);
}


void mhs_cmd_finish(TCanDevice *dev, int32_t result)
{
TMhsCmd *cmd;

if (!dev)
  return;
cmd = dev->Cmd; 
cmd->Result = result;
mhs_event_set((TMhsEvent *)cmd, 1);
}


void mhs_cmd_exit_thread(TCanDevice *dev)
{
TMhsCmd *cmd;

if (!dev)
  return; 
cmd = dev->Cmd;  
mhs_event_set((TMhsEvent *)cmd, MHS_TERMINATE);
}


void mhs_cmd_clear(TCanDevice *dev)
{
TMhsCmd *cmd;

if (!dev)
  return; 
cmd = dev->Cmd;
mhs_enter_critical(cmd);
cmd->Command = 0;
mhs_event_clear_unlocked((TMhsEvent *)cmd, MHS_ALL_EVENTS);  
mhs_leave_critical(cmd);
}

