#ifndef __PLUGIN__
#define __PLUGIN__  

#include <glib.h>
#include <gtk/gtk.h>
#include "plugin_call.h"

typedef void(*TVoidCB)(void);
typedef int(*TStandartCB)(void);
typedef int(*TPluginLoadCB)(gpointer plugin, char *plugin_path, struct TPluginMainCalls *main_calls);
typedef void(*TMenueCreateCB)(GtkWidget *menueitem);

#define PLUGIN_STATUS_ERROR 0
#define PLUGIN_STATUS_SCAN  1
#define PLUGIN_STATUS_LOAD  2


/* struct TPluginMenueItem
  {
  struct TPluginMenueItem *Next;
  //struct TPluginMenueItem *SubMenue;
  GtkWidget *Widget;
  char *MenueLabel;

  }; */

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


struct TPlugin
  {
  struct TPlugin *Next;
  int Status;
  GModule *Module;
  gchar *Filename;
  struct TPluginInfo Info;
  struct TPluginData *Data;
  GtkWidget *Menue;
  //struct TPluginMenue *Menue;
  TPluginLoadCB LoadCB;
  TStandartCB UnloadCB;  
  };


extern struct TPlugin *Plugins;


void PluginDestroyAll(void);
int PluginValid(struct TPlugin *plugin);
int PluginsScan(void);
int PluginLoad(struct TPlugin *plugin);
int PluginUnload(struct TPlugin *plugin);

#endif
