/***************************************************************************
                          drv_scan.c  -  description
                             -------------------
    begin             : 08.09.2008
    copyright         : (C) 2008 by MHS-Elektronik GmbH & Co. KG, Germany
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/
#include "can_monitor.h"
#include "string.h"
#include <gmodule.h>
#include "drv_scan.h"


//struct TDriverList *DriverList = NULL;

typedef char *(*TCanDrvInfoCB)(void);


void DriverListDestroy(struct TDriverList **driver_list)
{
struct TDriverList *l, *h;

if (!driver_list)
  return;
l = *driver_list;
while (l)
  {
  h = l->Next;
  save_free(l->Name);
  save_free(l->Version);
  save_free(l->Summary);
  save_free(l->Description);
  save_free(l->Info);
  save_free(l->Filename);
  g_free(l);
  l = h;
  }
*driver_list = NULL;
}


static struct TDriverList *DriverCreate(struct TDriverList **driver_list)
{
struct TDriverList *l;

l = *driver_list;
if (!l)
  {
  // Liste ist leer
  l = (struct TDriverList *)g_malloc0(sizeof(struct TDriverList));
  *driver_list = l;
  }
else
  { // Neues Element anhängen
  while (l->Next != NULL) l = l->Next;
  l->Next = (struct TDriverList *)g_malloc0(sizeof(struct TDriverList));
  l = l->Next;
  if (!l)
    return(NULL);        // Nicht genügend Arbetsspeicher
  }
l->Next = NULL;
return(l);
}


static gboolean IsSharedObject(const char *filename)
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


static void ExtractDriverListInfo(const char *str, struct TDriverList *driver)
{
int match;
char *key, *val, *info, *tmp, *tmpstr, *tmpptr;

info = NULL;
tmpstr = g_strdup(str);
tmpptr = tmpstr;
do
  {
  // Bezeichner auslesen
  key = GetItemAsString(&tmpptr, ":=", &match);
  if (match <= 0)
    break;
  // Value auslesen
  val = GetItemAsString(&tmpptr, ";", &match);
  if (match < 0)
    break;

  if (!g_ascii_strcasecmp(key, "HARDWARE"))
    {
    driver->Summary = g_strdup(val);
    continue;
    }
  else if (!g_ascii_strcasecmp(key, "VERSION"))
    {
    driver->Version = g_strdup(val);
    continue;
    }
  else if (!g_ascii_strcasecmp(key, "DESCRIPTION"))
    {
    driver->Description = g_strdup(val);
    continue;
    }
  if (info)
    {
    tmp = g_strdup_printf("%s\n<span weight=\"bold\">%s</span>\t%s", info, key, val);
    g_free(info);
    info = tmp;
    }
  else
    info = g_strdup_printf("<span weight=\"bold\">%s</span>\t%s", key, val);
  }
while(1);

driver->Info = info;
if (!driver->Summary)
  driver->Summary = g_strdup("Unbekannt");
if (!driver->Version)
  driver->Version = g_strdup("Unbekannt");
save_free(tmpstr);
}


static void ExtractDriverInfo(const char *str, struct TDriverInfo *info)
{
unsigned int i;
int match;
char *key, *val, *s, *tmpstr, *tmpptr;

s = NULL;
tmpstr = g_strdup(str);
tmpptr = tmpstr;
do
  {
  // Bezeichner auslesen
  key = GetItemAsString(&tmpptr, ":=", &match);
  if (match <= 0)
    break;
  // Value auslesen
  val = GetItemAsString(&tmpptr, ";", &match);
  if (match < 0)
    break;

  if (!g_ascii_strcasecmp(key, "INTERFACE TYPE"))
    {
    save_free(s);
    s = g_ascii_strup(val, -1);
    i = 0;
    if (g_strstr_len(s, -1, "USB"))
      i |= INTERFACE_TYPE_USB;
    if (g_strstr_len(s, -1, "SERIAL"))
      i |= INTERFACE_TYPE_SERIAL;
    info->InterfaceType = i;
    save_free(s);
    continue;
    }
  }
while(1);
save_free(tmpstr);
}


struct TDriverList *DriverScan(char *dir_name)
{
GDir *dir;
GModule *module;
G_CONST_RETURN gchar *filename;
gchar *full_filename;
struct TDriverList *driver;
struct TDriverList *driver_list;
TCanDrvInfoCB drv_info_cb;
const char *info_string;

driver_list = NULL;
// Scan tiny-can-api directory
if (g_module_supported() == FALSE)
  return(NULL);
dir = g_dir_open(dir_name, 0, NULL);
if (dir == NULL)
  return(NULL);

drv_info_cb = NULL;
while ((filename = g_dir_read_name(dir)) != NULL)
  {
  if (!strcmp(filename, "."))
    continue;
  if (!strcmp(filename, ".."))
    continue;
  if (!IsSharedObject(filename))
    continue;
  full_filename = g_build_filename(dir_name, filename, NULL);
  module = g_module_open(full_filename, G_MODULE_BIND_LAZY); //0);
  if (module)
    {
    if (g_module_symbol(module, "CanDrvInfo", (gpointer)&drv_info_cb) == TRUE)
      {
      if (drv_info_cb)
        {
        info_string = (drv_info_cb)();
        if (info_string)
          {
          driver = DriverCreate(&driver_list);
          driver->Name = g_strdup(filename);
          driver->Filename = g_strdup(filename);
          //driver->Filename = g_strdup(full_filename);
          ExtractDriverListInfo(info_string, driver);
          }
        }
      }
    (void)g_module_close(module);
    }
  save_free(full_filename);
  }
g_dir_close(dir);
return(driver_list);
}


struct TDriverInfo *GetDriverInfo(char *filename)
{
GModule *module;
TCanDrvInfoCB drv_info_cb;
const char *info_string;
struct TDriverInfo *info;

info = NULL;
if (g_module_supported() == FALSE)
  return(NULL);
if (!g_file_test(filename, G_FILE_TEST_IS_REGULAR))
  return(NULL);
module = g_module_open(filename, (GModuleFlags)0); //G_MODULE_BIND_LAZY); //0);
if (module)
  {
  if (g_module_symbol(module, "CanDrvInfo", (gpointer)&drv_info_cb) == TRUE)
    {
    if (drv_info_cb)
      {
      info_string = (drv_info_cb)();
      if (info_string)
        {
        info = (struct TDriverInfo *)g_malloc0(sizeof(struct TDriverInfo));
        ExtractDriverInfo(info_string, info);
        }
      }
    }
  (void)g_module_close(module);
  }
return(info);
}


void DriverInfoFree(struct TDriverInfo **info)
{
if ((info) && (*info))
  {
  g_free(*info);
  *info = NULL;
  }
}


int IsDriverDir(char *dir_name)
{
int res;
GDir *dir;
GModule *module;
G_CONST_RETURN gchar *filename;
gchar *full_filename;
TCanDrvInfoCB drv_info_cb;

// Scan tiny-can-api directory
if (g_module_supported() == FALSE)
  return(0);
dir = g_dir_open(dir_name, 0, NULL);
if (dir == NULL)
  return(0);

res = 0;
while ((filename = g_dir_read_name(dir)) != NULL)
  {
  if (!strcmp(filename, "."))
    continue;
  if (!strcmp(filename, ".."))
    continue;
  if (!IsSharedObject(filename))
    continue;
  full_filename = g_build_filename(dir_name, filename, NULL);
  module = g_module_open(full_filename, G_MODULE_BIND_LAZY); //0);
  if (module)
    {
    if (g_module_symbol(module, "CanDrvInfo", (gpointer)&drv_info_cb) == TRUE)
      res = 1;
    (void)g_module_close(module);
    }
  save_free(full_filename);
  if (res)
    break;
  }
g_dir_close(dir);
return(res);
}
