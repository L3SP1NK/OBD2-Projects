/* *********** Tiny-CAN Monitor **************                            */
/* Copyright (C) 2006 Klaus Demlehner (klaus@mhs-elektronik.de)           */
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
#include <gmodule.h>
#include <string.h>
#include "mhs_signal.h"
#include "filter.h"
#include "makro.h"
#include "plugin.h"
#include "main.h"
#include "plugin_call.h"
#include "support.h"
#include "dialogs.h"

struct TPlugin *Plugins = NULL;


struct TPlugin *PluginCreate(void);
int PluginDestroy(struct TPlugin *plugin);
void PluginDestroyMenues(void);
void PluginCreateMenues(void);

void DestroyPluginInfo(struct TPluginInfo *info);


static gboolean IsPluginFile(const char *filename)
{
const char *last_period;

last_period = strrchr(filename, '.');
if (last_period == NULL)
  return FALSE;
#ifdef __WIN32__
if (strcmp(last_period, ".dll") == 0)
  return(TRUE);
#else
if (strcmp(last_period, ".dylib") == 0)
  return(TRUE);
if (strcmp(last_period, ".sl") == 0)
  return(TRUE);
if (strcmp(last_period, ".so") == 0)
  return(TRUE);
#endif
return(FALSE);
}


void PluginDestroyAll(void)
{
struct TPlugin *h;

while (Plugins)
  {
  h = Plugins->Next;
  PluginUnload(Plugins);
  if (Plugins->Filename)
    g_free(Plugins->Filename);
  DestroyPluginInfo(&Plugins->Info);
  g_free(Plugins);
  Plugins = h;
  }
}


struct TPlugin *PluginCreate(void)
{
struct TPlugin *l;

l = Plugins;
if (!l)
  {
  // Liste ist leer  
  l = (struct TPlugin *)g_malloc0(sizeof(struct TPlugin));  
  Plugins = l;
  }
else
  { // Neues Element anhängen    
  while (l->Next != NULL) l = l->Next;
  l->Next = (struct TPlugin *)g_malloc0(sizeof(struct TPlugin));
  l = l->Next;
  if (!l)
    return(NULL);        // Nicht genügend Arbetsspeicher  
  }
l->Next = NULL;
return(l);
}


int PluginDestroy(struct TPlugin *plugin)
{
struct TPlugin *l, *prev;
  
prev = NULL;
for (l = Plugins; l; l = l->Next)
  {
  if (l == plugin)
    {  
    if (prev)
      prev->Next = l->Next;
    else
      Plugins = l->Next;
    if (l->Filename)
      g_free(l->Filename);
    DestroyPluginInfo(&l->Info);
    g_free(l);   
    return(0);
    }
  prev = l;
  }
return(-1);
}


int PluginValid(struct TPlugin *plugin)
{
struct TPlugin *l;

for (l = Plugins; l; l = l->Next)
  {
  if (l == plugin)    
    return(0);   
  }
return(-1);
}


void CopyPluginInfo(struct TPluginInfo *info_dst, struct TPluginInfo *info_src)
{
DestroyPluginInfo(info_dst);
info_dst->Id = g_strdup(info_src->Id);
info_dst->Name = g_strdup(info_src->Name);
info_dst->Version = g_strdup(info_src->Version);
info_dst->Summary = g_strdup(info_src->Summary);
info_dst->Description = g_strdup(info_src->Description);
info_dst->Author = g_strdup(info_src->Author);
info_dst->Homepage = g_strdup(info_src->Homepage);
}


void DestroyPluginInfo(struct TPluginInfo *info)
{
if (info->Id)
  {
  g_free(info->Id);
  info->Id = NULL;
  }
if (info->Name)
  {
  g_free(info->Name);
  info->Name = NULL;
  }
if (info->Version)
  {
  g_free(info->Version);
  info->Version = NULL;
  }
if (info->Summary)
  {
  g_free(info->Summary);
  info->Summary = NULL;
  }
if (info->Description)
  {
  g_free(info->Description);
  info->Description = NULL;
  }
  {
  g_free(info->Author);
  info->Author = NULL;
  }
if (info->Homepage)
  {
  g_free(info->Homepage);
  info->Homepage = NULL;
  }
}


int PluginsScan(void)
{
GDir *dir;
GModule *module;
G_CONST_RETURN gchar *filename;
gchar *full_filename;
struct TPluginInfo *plugin_info;
struct TPlugin *plugin;

PluginDestroyAll();  // Alle Plugins löschen
// Scan plug-ins directory
if (g_module_supported() == FALSE) 
  return(-1);
dir = g_dir_open(Paths.plugin_dir, 0, NULL);
if (dir == NULL) 
  return(-1);
  
while ((filename = g_dir_read_name(dir)) != NULL) 
  {
  if (!strcmp(filename, "."))
    continue;
  if (!strcmp(filename, ".."))
    continue;
  if (!IsPluginFile(filename))
    continue;
  full_filename = g_strconcat(Paths.plugin_dir, "/", filename, NULL);      
  module = g_module_open(full_filename, 0); //G_MODULE_BIND_LAZY); //0);
  if (module) 
    {
    if (g_module_symbol(module, "PluginInfo", (gpointer)&plugin_info) == TRUE)    
      {
      plugin = PluginCreate();
      CopyPluginInfo(&plugin->Info, plugin_info);
      plugin->Filename = g_strdup(full_filename);
      plugin->Status = PLUGIN_STATUS_SCAN;
      }
    (void)g_module_close(module);
    }
  else
    msg_box(MSG_TYPE_ERROR, _("Error"), _("module error: %s"), g_module_error());       
  }
g_dir_close(dir);
return(0);
}


int PluginLoad(struct TPlugin *plugin)
{
int err;
GModule *module;
TPluginLoadCB load_cb;

if (!plugin)
  return(-1);
if (plugin->Module)
  (void)PluginUnload(plugin);

module = g_module_open(plugin->Filename, 0); //G_MODULE_BIND_LAZY); 
if (!module)       
  return(-1);
err = 0;        
if (g_module_symbol(module, "PluginLoadCB", (gpointer) &plugin->LoadCB) == FALSE) 
  err = -1;         
else if (g_module_symbol(module, "PluginUnloadCB", (gpointer) &plugin->UnloadCB) == FALSE) 
  err = -1;    
else if (g_module_symbol(module, "PluginData", (gpointer) &plugin->Data) == FALSE)
  err = -1;
else
  {  
  load_cb = plugin->LoadCB;
  if (load_cb)
    err = (load_cb)((gpointer)plugin, Paths.plugin_dir, (struct TPluginMainCalls *)&PluginMainCalls);
  else
    err = -1;
  }
if (err < 0)
  {
  plugin->Status = PLUGIN_STATUS_ERROR;
  plugin->Module = NULL;
  (void)g_module_close(module);
  }
else
  {
  if (plugin->Data->MenueLabel)
    PluginCreateMenues();
  plugin->Status = PLUGIN_STATUS_LOAD;
  plugin->Module = module;
  }
return(err);
} 


int PluginUnload(struct TPlugin *plugin)
{
int err;

err = 0;
if (plugin->Module)
  {
  mhs_signal_disconnect_owner(plugin);
  if (plugin->UnloadCB)
    err = (plugin->UnloadCB)();
  UpdateGTK();
  if ((plugin->Data) && (plugin->Data->MenueLabel))
    {
    plugin->Data = NULL;
    PluginCreateMenues();
    }
  MakClearPlugins(plugin);
  FilClearPlugins(plugin);
  (void)g_module_close(plugin->Module);
  }
if (plugin->Status == PLUGIN_STATUS_LOAD)
  plugin->Status = PLUGIN_STATUS_SCAN;
plugin->Module = NULL;
plugin->LoadCB = NULL;
plugin->UnloadCB = NULL;
return(err);
}


/**************************************************************/
/* Plugin Menues erzeugen / löschen                           */
/**************************************************************/
void PluginDestroyMenues(void)
{
struct TPlugin *plugin;

for (plugin = Plugins; plugin; plugin = plugin->Next)
  {
  if (plugin->Menue)
    {
    gtk_widget_destroy(plugin->Menue);
    plugin->Menue = NULL;
    }
  }
}
  

void PluginCreateMenues(void)
{
struct TPlugin *plugin;
GtkWidget *menuitem;
TMenueCreateCB menue_cb;

PluginDestroyMenues();
for (plugin = Plugins; plugin; plugin = plugin->Next)
  {
  if (plugin->Data)
    {
    if (plugin->Data->MenueLabel)
      {
      menuitem = gtk_image_menu_item_new_with_label(plugin->Data->MenueLabel);
      gtk_menu_shell_append(GTK_MENU_SHELL(MainWin.MPlugins_menu), menuitem);
      gtk_widget_show(menuitem);
      plugin->Menue = menuitem;
      menue_cb = plugin->Data->MenueCreateCB;
      if (menue_cb)
        (menue_cb)(menuitem);
      }
    }
  }
}
