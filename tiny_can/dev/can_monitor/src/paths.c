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

/**************************************************************************/
/*                            Programm Paths                              */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : Programm Paths Variablen (base_dir, setup_dir)      */
/*                    entsprechend "Programm-Exe" Directory               */
/*                    setzen                                              */
/*                                                                        */
/*  Version         : 1.00                                                */
/*  Datei Name      : paths.c                                             */
/* ---------------------------------------------------------------------- */
/*  Datum           : 17.10.05                                            */
/*  Author          : Demlehner Klaus                                     */
/* ---------------------------------------------------------------------- */
/*  Compiler        : GNU C Compiler                                      */
/**************************************************************************/
#include <stdio.h>
#include <string.h>
//#include <unistd.h>
#include <glib/gstdio.h>
#include "util.h"
#include "paths.h"


struct TPaths Paths;



void PathsInit(gchar *prog)
{
gchar *path;

path = g_path_get_dirname(prog);
if (path)
  {
  g_chdir(path);
  save_free(path);
  }
Paths.base_dir = g_get_current_dir();
//g_error("Path: %s", Paths.base_dir);
Paths.setup_dir = g_strdup(Paths.base_dir);
Paths.plugin_dir = g_build_filename(Paths.base_dir, "plugins", NULL);; 
Paths.locale_dir = g_build_filename(Paths.base_dir, "locale", NULL);
}


void PathsFree(void)
{
save_free(Paths.base_dir);
save_free(Paths.plugin_dir);
save_free(Paths.setup_dir);
save_free(Paths.locale_dir);
}
