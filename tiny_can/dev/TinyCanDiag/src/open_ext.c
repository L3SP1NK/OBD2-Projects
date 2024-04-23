/***************************************************************************
                            open_ext.c  -  description
                             -------------------
    begin             : 26.08.2017
    copyright         : (C) 2017 by MHS-Elektronik GmbH & Co. KG, Germany
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/
#include <glib.h>
#include <gtk/gtk.h>

#if defined(G_OS_WIN32)
  #include <windows.h>
  #include <tchar.h>
  #include <shellapi.h>
#else

#endif
#include "open_ext.h"


gboolean OpenExternel(const gchar *url_file)
{
//gunichar2 *utf16_str; <*>

#if defined(G_OS_WIN32)
(void)ShellExecute(HWND_DESKTOP, _T("open"), (LPCSTR)url_file, NULL, NULL, SW_SHOWNORMAL);
/*if ((utf16_str = g_utf8_to_utf16(url_file, -1, NULL, NULL, NULL))) <*>
  {
  (void)ShellExecute(HWND_DESKTOP, _T("open"), (LPCSTR)utf16_str, NULL, NULL, SW_SHOWNORMAL);
  g_free(utf16_str);
  } */
return(TRUE); // <*>

#else
GError *error;
gchar *argv[3];
gboolean retval;

error = NULL;
argv[0] = "xdg-open";
//DIAG_OFF(cast-qual) <*>
argv[1] = (gchar *)url_file;
//DIAG_ON(cast-qual) <*>
argv[2] = NULL;

retval = g_spawn_async (NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &error);
if (retval)
  return TRUE;
g_debug("Could not execute xdg-open: %s", error->message);
g_error_free(error);
return(FALSE);
#endif
}
