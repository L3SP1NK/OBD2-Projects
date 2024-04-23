#ifndef __PLUGIN_BASE_H__
#define __PLUGIN_BASE_H__  

#include <glib.h>
#include "can_types.h"
#include "can_drv.h"
#include "mhs_signal.h"
#include "mhs_signals.h"
#include "filter.h"

typedef void(*TVoidCB)(void);
typedef int(*TStandartCB)(void);
typedef void(*TMenueCreateCB)(GtkWidget *menueitem);

typedef int(*TSendMessageProc)(gpointer plugin, unsigned long index, struct TCanMsg *msg, int count);
typedef int(*TMakroSetProc)(gpointer plugin, char *name, struct TCanMsg *msg, unsigned long intervall);
typedef int(*TMakroGetProc)(gpointer plugin, char *name, struct TCanMsg *msg, unsigned long *intervall);
typedef int(*TMakroClearProc)(gpointer plugin, char* name);
typedef int(*TMakroSendProc)(gpointer plugin, char *name, unsigned long index);
typedef int(*TFilterSetProc)(gpointer plugin, char *name, struct TFilter *filter);
typedef int(*TFilterGetProc)(gpointer plugin, char *name, struct TFilter *filter);
typedef int(*TFilterClearProc)(gpointer plugin, char *name);
typedef int(*TFilterEditingFinish)(gpointer plugin);

struct TPluginInfo
  {  
  char *Id;
  char *Name;
  char *Version;
  char *Summary;
  char *Description;
  char *Author;
  char *Homepage;
  };

struct TPluginData
  {
  char *MenueLabel;
  TMenueCreateCB MenueCreateCB;
  TVoidCB ExecuteSetupCB;
  };

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



#endif
