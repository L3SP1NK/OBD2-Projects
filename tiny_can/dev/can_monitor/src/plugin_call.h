#ifndef __PLUGIN_CALL__
#define __PLUGIN_CALL__

#include <glib.h>
#include "can_types.h"
#include "mhs_signal.h"
#include "filter.h"
#include "plugin.h"


typedef int(*TSendMessageProc)(gpointer plugin, unsigned long index, struct TCanMsg *msg, int count);
typedef int(*TMakroSetProc)(gpointer plugin, char *name, struct TCanMsg *msg, unsigned long intervall);
typedef int(*TMakroGetProc)(gpointer plugin, char *name, struct TCanMsg *msg, unsigned long *intervall);
typedef int(*TMakroClearProc)(gpointer plugin, char* name);
typedef int(*TMakroSendProc)(gpointer plugin, char *name, unsigned long index);
typedef int(*TFilterSetProc)(gpointer plugin, char *name, struct TFilter *filter);
typedef int(*TFilterGetProc)(gpointer plugin, char *name, struct TFilter *filter);
typedef int(*TFilterClearProc)(gpointer plugin, char *name);
typedef int(*TFilterEditingFinish)(gpointer plugin);


struct TPluginMainCalls 
  {    
  struct TDeviceStatus *PlCanDevStatus;
  struct TMhsSignal **PlSignals;  
  TSendMessageProc SendMessageProc;
  TMakroSetProc MakroSetProc;
  TMakroGetProc MakroGetProc;
  TMakroClearProc MakroClearProc;
  TMakroSendProc MakroSendProc;
  TFilterSetProc FilterSetProc;
  TFilterGetProc FilterGetProc;
  TFilterClearProc FilterClearProc;
  TFilterEditingFinish FilterEditingFinishProc;  
  };

extern const struct TPluginMainCalls PluginMainCalls;

#endif
