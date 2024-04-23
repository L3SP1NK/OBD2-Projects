/* *********** Tiny-CAN Monitor **************                            */
/* Copyright (C) 2005 Klaus Demlehner (klaus@mhs-elektronik.de)           */
/*   Tiny-CAN Project Homepage: http://www.mhs-elektronik.de              */
/*                                                                        */
/* This program is free software; you can redistribute it and/or modify   */
/* it under the terms of the GNU General Public License as published by   */
/* the Free Software Foundation; either version 2 of the License, or      */
/* (at your option) any later version.                                    */
/*                                                                        */
/* This program is distributed in the hope that it will be useful,        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/* GNU General Public License for more details.                           */
/*                                                                        */
/* You should have received a copy of the GNU General Public License      */
/* along with this program; if not, write to the Free Software            */
/* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */
#include "can_monitor.h"
#include "filter.h"
#include "makro.h"
#include "paths.h"
#include "main.h"
#include "main_can.h"
#include "plugin_call.h"


int PlgSendMessage(gpointer plugin, unsigned long index, struct TCanMsg *msg, int count);
int PlgMakroSet(gpointer plugin, char *name, struct TCanMsg *msg, unsigned long intervall);
int PlgMakroGet(gpointer plugin, char *name, struct TCanMsg *msg, unsigned long *intervall);
int PlgMakroClear(gpointer plugin, char* name);
int PlgMakroSend(gpointer plugin, char *name, unsigned long index);
int PlgFilterSet(gpointer plugin, char *name, struct TFilter *filter);
int PlgFilterGet(gpointer plugin, char *name, struct TFilter *filter);
int PlgFilterClear(gpointer plugin, char *name);
int PlgFilterEditingFinish(gpointer plugin);


const struct TPluginMainCalls PluginMainCalls =
  {
  /* .PlCanDevStatus = */ &CanModul.DeviceStatus,
  /* .PlSignals = */ &MhsCanViewSignals,
  /* .SendMessageProc = */ PlgSendMessage,
  /* .MakroSetProc = */ PlgMakroSet,
  /* .MakroGetProc = */ PlgMakroGet,
  /* .MakroClearProc = */ PlgMakroClear,
  /* .MakroSendProc = */ PlgMakroSend,
  /* .FilterSetProc = */ PlgFilterSet,
  /* .FilterGetProc = */ PlgFilterGet,
  /* .FilterClearProc = */ PlgFilterClear,
  /* .FilterEditingFinishProc = */ PlgFilterEditingFinish,
  };


int PlgSendMessage(gpointer plugin, unsigned long index, struct TCanMsg *msg, int count)
{
return(CanTransmit(index, msg, count));
}


int PlgMakroSet(gpointer plugin, char *name, struct TCanMsg *msg, unsigned long intervall)
{
int res;
struct TPlugin *plg;

plg = (struct TPlugin *)plugin;
if (PluginValid(plugin) < 0)
  return(-1);
res = MakSetByPlugin(plg, name, msg, intervall);
mhs_signal_emit(SIGC_PROTECTED, SIG_MAKRO_DLG_REPAINT, NULL);
return(res);
}


int PlgMakroGet(gpointer plugin, char *name, struct TCanMsg *msg, unsigned long *intervall)
{
struct TPlugin *plg;

plg = (struct TPlugin *)plugin;
if (PluginValid(plg) < 0)
  return(-1);
return(MakGetByPlugin(plg, name, msg, intervall));
}


int PlgMakroClear(gpointer plugin, char* name)
{
int res;
struct TPlugin *plg;

plg = (struct TPlugin *)plugin;
if (PluginValid(plg) < 0)
  return(-1);
res = MakClearByPlugin(plg, name);
mhs_signal_emit(SIGC_PROTECTED, SIG_MAKRO_DLG_REPAINT, NULL);
return(res);
}


int PlgMakroSend(gpointer plugin, char *name, unsigned long index)
{
struct TPlugin *plg;

plg = (struct TPlugin *)plugin;
if (PluginValid(plg) < 0)
  return(-1);
return(MakSendByPlugin(plg, name, index));
}


int PlgFilterSet(gpointer plugin, char *name, struct TFilter *filter)
{
struct TPlugin *plg;

plg = (struct TPlugin *)plugin;
if (PluginValid(plg) < 0)
  return(-1);
return(FilSetByPlugin(plg, name, filter));
}


int PlgFilterGet(gpointer plugin, char *name, struct TFilter *filter)
{
struct TPlugin *plg;

plg = (struct TPlugin *)plugin;
if (PluginValid(plg) < 0)
  return(-1);
return(FilGetByPlugin(plg, name, filter));
}


int PlgFilterClear(gpointer plugin, char *name)
{
struct TPlugin *plg;

plg = (struct TPlugin *)plugin;
if (PluginValid(plg) < 0)
  return(-1);
return(FilClearByPlugin(plg, name));
}


int PlgFilterEditingFinish(gpointer plugin)
{
struct TPlugin *plg;

plg = (struct TPlugin *)plugin;
if (PluginValid(plg) < 0)
  return(-1);
FilEditingFinish();
return(0);
}
