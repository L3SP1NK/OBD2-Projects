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
/*                    Kommandozeilen Argumente auswerten                  */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : Kommandozeilen Argumente auswerten                  */
/* Unterstützte Argumente:                                                */
/* --setup -s : Verzeichnis in dem sich die Konfigurationsfiles befinden  */
/* --help -h : Hilfstext anzeigen                                         */
/*                                                                        */
/*  Version         : 1.00                                                */
/*  Datei Name      : cmdline.c                                           */
/* ---------------------------------------------------------------------- */
/*  Datum           : 17.10.05                                            */
/*  Author          : Demlehner Klaus                                     */
/* ---------------------------------------------------------------------- */
/*  Compiler        : GNU C Compiler                                      */
/**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <glib.h>
#include "util.h"
#include "paths.h"
#include "cmdline.h"
#include "support.h"


static struct option LongOptions[] = {
    {"setup", 1, 0, 's'},
    {"help",  0, 0, 'h'},
    {0, 0, 0, 0}};


void PrintHelp(void)
{
printf(_("\nTiny CAN Monitor\n"
         "(c) by MHS-Elektronik 2008, Version 2.00\n"
         "     www.mhs-elektronik.de\n\n"
         "Options:\n"
         "  --setup -s : Select directory for configuration files\n"
         "  --help -h  :  Display this usage\n"));
}


int ReadCommandLine(int argc, char **argv)
{
int c;
int option_index = 0;

while(1)
  {
  c = getopt_long(argc, argv, "s:h", LongOptions, &option_index);

  if(c == -1)
    break;

  switch(c)
    {
    case 's' : {
               if (Paths.setup_dir)
                 g_free(Paths.setup_dir);
               Paths.setup_dir = g_strdup(optarg);
               break;
               }
    case 'h' : {
               PrintHelp();
               return(-1);
               }
    default  : {
               PrintHelp();
               printf(_("invalid option\n"));
               return(-1);
               }
    }
  }
return(0);
}

