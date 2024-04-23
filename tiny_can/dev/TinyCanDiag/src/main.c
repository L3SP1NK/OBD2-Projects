/***************************************************************************
                           main.c  -  description
                             -------------------
    begin             : 24.08.2017
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
#include <gtk/gtk.h>

#include "hw_types.h"
#include "dialogs.h"
#include "paths.h"
#include "gtk_util.h"
#include "setup.h"
#include "gui.h"
#include "main_samples.h"
#include "main.h"


/*************************/
/* Programm wird beendet */
/*************************/
void ExitApplikation(struct TMainWin *main_win)
{
SaveConfigFile(main_win);
}


int main (int argc, char *argv[])
{
struct TMainWin *main_win;

//gtk_set_locale();
gtk_init(&argc, &argv);
PathsInit(argv[0]);

main_win = CreateMainWin();
LoadConfigFile(main_win);

gtk_main();

DestroyMainWinData(main_win);
sample_projects_destroy();
PathsFree();
return(0);
}
