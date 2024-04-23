/***************************************************************************
                           setup.c  -  description
                             -------------------
    begin             : 19.07.2008
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
#include <gtk/gtk.h>
#include "main.h"
#include "gtk_util.h"
#include "paths.h"
#include "gui.h"
#include "configfile.h"
#include "recent_files.h"
#include "setup.h"

static const gchar CONFIG_FILE_NAME[] = "TinyCanDiag.ini";



gint LoadConfigFile(struct TMainWin *main_win)
{
ConfigFile *cfgfile;
gchar *filename;

if (!(filename = CreateFileName(Paths.setup_dir, CONFIG_FILE_NAME)))
  return(-1);
if (!(cfgfile = cfg_open_file(filename)))
  {
  g_free(filename);
  return(-1);
  }
RecentFileLoadList(main_win, cfgfile);
cfg_free(cfgfile);
g_free(filename);
return(0);
}


gint SaveConfigFile(struct TMainWin *main_win)
{
ConfigFile *cfgfile;
gchar *filename;
int res;

res = 0;
if (!(filename = CreateFileName(Paths.setup_dir, CONFIG_FILE_NAME)))
  return(-1);
if (!(cfgfile = cfg_open_file(filename)))
  cfgfile = cfg_new();
RecentFileSaveAndDestroyList(main_win, cfgfile);
if (cfg_write_file(cfgfile, filename) == FALSE)
  res = -1;
cfg_free(cfgfile);
g_free(filename);
return(res);
}
