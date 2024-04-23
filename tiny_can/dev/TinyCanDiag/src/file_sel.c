/***************************************************************************
                          file_sel.c  -  description
                             -------------------
    begin             : 20.07.2008
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
#include <stdlib.h>
#include <string.h>
#include "gtk_util.h"
#include "file_sel.h"


int SelectFileDlg(const gchar *title, unsigned char action, char **file_name)
{
GtkWidget *win;
GtkFileChooserAction gtk_action;
const gchar *ok_button_text;
gchar *full_file_name, *file, *dir;
int result;
/* int filename_len;
gchar *new_filename; */

result = 0;
file = NULL;
dir = NULL;
switch (action)
  {
  case FILE_SELECTION_OPEN : {
                             gtk_action = GTK_FILE_CHOOSER_ACTION_OPEN;
                             ok_button_text = GTK_STOCK_OPEN;
                             break;
                             }
  case FILE_SELECTION_READ_BROWSE :
                             {
                             gtk_action = GTK_FILE_CHOOSER_ACTION_OPEN;
                             ok_button_text = GTK_STOCK_OK;
                             break;
                             }
  case FILE_SELECTION_SAVE : {
                             gtk_action = GTK_FILE_CHOOSER_ACTION_SAVE;
                             ok_button_text = GTK_STOCK_SAVE;
                             break;
                             }
  case FILE_SELECTION_WRITE_BROWSE :
                             {
                             gtk_action = GTK_FILE_CHOOSER_ACTION_SAVE;
                             ok_button_text = GTK_STOCK_OK;
                             break;
                             }
  default                  : return(-1);
  }

win = gtk_file_chooser_dialog_new(title, NULL, gtk_action,
                                    ok_button_text, GTK_RESPONSE_ACCEPT,
                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                    NULL);
gtk_dialog_set_default_response(GTK_DIALOG(win), GTK_RESPONSE_ACCEPT);

/* filename_len = strlen(filename)
// trim filename, so gtk_file_chooser_set_current_folder() likes it, see below
if ((filename[filename_len -1] == G_DIR_SEPARATOR)
#ifdef _WIN32
   && (filename_len > 3))    // e.g. "D:\"
#else
   && (filename_len > 1))    // e.g. "/"
#endif
  {
  new_filename = g_strdup(filename);
  new_filename[filename_len-1] = '\0';
else
  new_filename = g_strdup(filename);

// this function is very pedantic about it's filename parameter
// no trailing '\' allowed, unless a win32 root dir "D:\"
(void)gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(win), new_filename);
g_free(new_filename); */

full_file_name = *file_name;
if (full_file_name)
  {
  if (g_file_test(full_file_name, G_FILE_TEST_IS_REGULAR))
    gtk_file_chooser_select_filename(GTK_FILE_CHOOSER(win), full_file_name);
  else
    {
    if (g_file_test(full_file_name, G_FILE_TEST_IS_DIR))
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(win), full_file_name);
    else
      {
      SplitFileName(full_file_name, &dir, &file);
      if ((dir) && (g_file_test(dir, G_FILE_TEST_IS_DIR)))
        gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(win), dir);
      if ((file) && ((action == FILE_SELECTION_WRITE_BROWSE) || (action == FILE_SELECTION_SAVE)))
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(win), file);
      }
    }
  }
if (gtk_dialog_run(GTK_DIALOG (win)) == GTK_RESPONSE_ACCEPT)
  {
  safe_free(*file_name);
  *file_name = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(win));
  result = 1;
  }
safe_free(dir);
safe_free(file);
gtk_widget_destroy(win);
return(result);
}

