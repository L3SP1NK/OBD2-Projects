/***************************************************************************
                           paths.c  -  description
                             -------------------
    begin             : 08.04.2008
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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include "gtk_util.h"
#include "paths.h"

/**************************************************************************/
/*                        C O N S T A N T E N                             */
/**************************************************************************/
#if defined(G_OS_WIN32)
#ifdef _WIN64
static const char REG_TINY_CAN_API[] = {"Software\\Wow6432Node\\Tiny-CAN\\API"};
#else
static const char REG_TINY_CAN_API[] = {"Software\\Tiny-CAN\\API"};
#endif
static const char REG_TINY_CAN_API_PATH_ENTRY[] = {"PATH"};
#endif

struct TPaths Paths;

#if defined(G_OS_WIN32)
static char *MhsRegGetString(HKEY key, const char *value_name)
{
DWORD type, data_size;
char *str;

type = 0;
data_size = 0;
if ((!key) || (!value_name))
  return(NULL);
// Länge des Strings ermitteln
if (RegQueryValueExA(key, value_name, 0, &type, NULL, &data_size) != ERROR_SUCCESS)
  return(NULL);
// Wert ist String ?
if (type != REG_SZ)
  return(NULL);
str = malloc(data_size+1);
if (!str)
  return(NULL);
// String lesen
if (RegQueryValueExA(key, value_name, 0, &type, (LPBYTE)str, &data_size) != ERROR_SUCCESS)
  {
  free(str);
  return(NULL);
  }
str[data_size] = '\0';
return(str);
}


static char *MhsRegReadStringEntry(const char *path, const char *entry)
{
HKEY key;
char *value;

value = NULL;
// HKEY_CURRENT_USER ?
if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, path, 0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS) // KEY_READ
  {
  value = MhsRegGetString(key, entry);
  RegCloseKey(key);
  }
return(value);
}
#endif


gchar *CreateDocFileName(const gchar *doc_file)
{
#if defined(G_OS_WIN32)
gchar *out;

if (!doc_file)
  return(NULL);
if (Paths.doc_dir)
  {
  out = g_strconcat(Paths.doc_dir, doc_file, NULL);
  if (GetFileAttributes(out) == INVALID_FILE_ATTRIBUTES)
    g_free(out);
  else
    return(out);
  }
if (Paths.base_dir)
  {
  out = g_strconcat(Paths.base_dir, doc_file, NULL);
  if (GetFileAttributes(out) == INVALID_FILE_ATTRIBUTES)
    g_free(out);
  else
    return(out);
  }
return(NULL);
#else
return(g_strconcat(Paths.doc_dir, doc_file, NULL));
#endif
}


void PathsInit(gchar *prog)
{
#if defined(G_OS_WIN32)
char *path, *s;

Paths.base_dir = g_path_get_dirname(prog);
Paths.setup_dir = g_strdup(Paths.base_dir);

if ((path = MhsRegReadStringEntry(REG_TINY_CAN_API, REG_TINY_CAN_API_PATH_ENTRY)))
  {
  if ((s = strrchr(path, G_DIR_SEPARATOR)))
    *s = '\0';
  Paths.doc_dir = g_strconcat(path, "\\doku\\", NULL);
  g_free(path);
  }
Paths.samples_dir = g_strconcat(Paths.base_dir, "\\samples\\", NULL);
g_chdir(Paths.base_dir);
#else
Paths.base_dir = g_path_get_dirname(prog);
Paths.setup_dir = g_strdup(Paths.base_dir);

Paths.doc_dir = g_strdup("/opt/tiny_can/doku/");
Paths.samples_dir = g_strconcat(Paths.base_dir, "/samples/", NULL);;
#endif
}


void PathsFree(void)
{
safe_free(Paths.base_dir);
safe_free(Paths.doc_dir);
safe_free(Paths.samples_dir);
safe_free(Paths.setup_dir);
}
