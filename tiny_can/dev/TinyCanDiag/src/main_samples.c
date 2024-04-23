/***************************************************************************
                          main_samples.c  -  description
                             -------------------
    begin             : 28.08.2017
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
#include "gtk_util.h"
#include "paths.h"
#include "project.h"
#include "configfile.h"
#include "candiag_icons.h"
#include "main_welcome.h"
#include "main_samples.h"


static const gchar SAMPLES_FILE_NAME[] = "samples.ini";
static GList *FileList = NULL;


static gboolean sample_project_cb(GtkWidget *w, GdkEventButton *event, gpointer user_data)
{
struct TMainWin *main_win;
gchar *filename;
(void)event;

filename = (gchar *)g_object_get_data(G_OBJECT(w), "user_data2");
main_win = (struct TMainWin *)user_data;
LoadPrj(main_win, filename);
return(FALSE);
}

void fill_sample_projects(struct TMainWin *main_win, GtkWidget *box)
{
GtkWidget *w;
ConfigFile *cfgfile;
gchar *full_filename, *filename, *section, *name, *description;
guint i;

if (!(filename = CreateFileName(Paths.samples_dir, SAMPLES_FILE_NAME)))
  return;
if (!(cfgfile = cfg_open_file(filename)))
  {
  g_free(filename);
  return;
  }
g_free(filename);
for (i = 0; i < 20; i++)
  {
  section = g_strdup_printf("SAMPLE%u", i);
  if (cfg_read_string(cfgfile, section, "Name", &name))
    {
    cfg_read_string(cfgfile, section, "Description", &description);
    cfg_read_string(cfgfile, section, "File", &filename);
    full_filename = CreateFileName(Paths.samples_dir, filename);
    w = welcome_button(SAMPLES_ICONE, name, description, NULL, sample_project_cb, (gpointer)main_win, full_filename);
    gtk_box_pack_start(GTK_BOX(box), w, FALSE, FALSE, 0);
    FileList = g_list_append(FileList, full_filename);
    safe_free(filename);
    safe_free(section);
    safe_free(name);
    safe_free(description);
    }
  else
    break;
  }
cfg_free(cfgfile);
}


void sample_projects_destroy(void)
{
GList *item;
gchar *name;

item = FileList;
while (item)
  {
  if ((name = (gchar *)item->data))
    g_free(name);
  item = g_list_next(item);
  }
g_list_free(FileList);
FileList = NULL;
}
