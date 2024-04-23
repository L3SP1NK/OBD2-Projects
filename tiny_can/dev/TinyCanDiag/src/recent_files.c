/***************************************************************************
                         recent_files.c  -  description
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
#include "project.h"
#include "gui.h"
#include "main_welcome.h"
#include "configfile.h"
#include "recent_files.h"

#define UTF8_HORIZONTAL_ELLIPSIS "\xe2\x80\xa6" /*  8230 / 0x2026 */


static gboolean activate_link_cb(GtkLabel *label, gchar *uri, gpointer user_data)
{
gchar *file_name;
struct TMainWin *main_win;
(void)uri;

main_win = (struct TMainWin *)user_data;
file_name = (gchar *)g_object_get_data(G_OBJECT(label), "file_name");

LoadPrj(main_win, file_name);
return(TRUE);
}


static void welcome_filename_destroy_cb(GtkWidget *widget, gpointer user_data)
{
gchar *filename;
(void)widget;

if (!(filename = (gchar *)user_data))
  return;
g_free(filename);
}

/* create a "file link widget" */
static GtkWidget *welcome_filename_link_new(struct TMainWin *main_win, const gchar *filename)
{
GtkWidget *w;
GString *str;
gchar *str_escaped, *filename_copy, *link_name, *link_str;
glong uni_len;
gsize uni_start, uni_end;
const glong  max = 60;

/* filename */
str = g_string_new(filename);
uni_len = g_utf8_strlen(str->str, str->len);

/* cut max filename length */
if (uni_len > max)
  {
  uni_start = g_utf8_offset_to_pointer(str->str, 20) - str->str;
  uni_end = g_utf8_offset_to_pointer(str->str, uni_len - max) - str->str;
  g_string_erase(str, uni_start, uni_end);
  g_string_insert(str, uni_start, " " UTF8_HORIZONTAL_ELLIPSIS " ");
  }

/* escape the possibly shortened filename before adding pango language */
str_escaped = g_markup_escape_text(str->str, -1);
g_string_free(str, TRUE);
link_name = g_markup_escape_text(filename, -1);
link_str = g_strdup_printf("<a href=\"%s\">%s</a>", link_name, str_escaped);
safe_free(str_escaped);
safe_free(link_name);

/* label */
w = gtk_label_new(NULL);
gtk_label_set_markup(GTK_LABEL(w), link_str);
gtk_misc_set_padding(GTK_MISC(w), 5, 2);
gtk_misc_set_alignment (GTK_MISC(w), 0.0f, 0.0f);

filename_copy = g_strdup(filename);
g_object_set_data(G_OBJECT(w), "file_name", (gpointer)filename_copy);
g_signal_connect(w, "destroy", G_CALLBACK(welcome_filename_destroy_cb), filename_copy);
gtk_widget_set_tooltip_text(w, filename);
g_signal_connect(w, "activate-link", G_CALLBACK(activate_link_cb), (struct TMainWin *)main_win);

return(w);
}


/* reset the list of recent files */
void main_welcome_reset_recent_capture_files(struct TMainWin *main_win)
{
GList* list, * item;

if (main_win->FileBox)
  {
  list = gtk_container_get_children(GTK_CONTAINER(main_win->FileBox));
  item = list;

  while (item)
    {
    gtk_container_remove(GTK_CONTAINER(main_win->FileBox), (GtkWidget *)item->data);
    item = g_list_next(item);
    }
  g_list_free(list);
  }
}


/* add a new file to the list of recent files */
void main_welcome_add_recent_capture_file(struct TMainWin *main_win, const gchar *filename)
{
GtkWidget *w;

w = welcome_filename_link_new(main_win, filename);
gtk_box_pack_start(GTK_BOX(main_win->FileBox), w, FALSE, FALSE, 0);
gtk_widget_show_all(w);
}


static void RecentFileCB(GtkMenuItem *menuitem, gpointer user_data)
{
gchar *file_name;
struct TMainWin *main_win;

main_win = (struct TMainWin *)user_data;
file_name = (gchar *)g_object_get_data(G_OBJECT(menuitem), "file_name");
LoadPrj(main_win, file_name);
}


void ClearRecentFileList(struct TMainWin *main_win)
{
GList *list;

list = (GList *)g_object_get_data(G_OBJECT(main_win->RecentFilesMenu), "recent-files-list");
g_list_free(list);
g_object_set_data(G_OBJECT(main_win->RecentFilesMenu), "recent-files-list", NULL);
UpdateRecentItems(main_win);
}


void UpdateRecentItems(struct TMainWin *main_win)
{
GList *list, * item;
GtkWidget *widget;
gchar *name;

main_welcome_reset_recent_capture_files(main_win);

if (main_win->RecentFilesMenu)
  {
  list = gtk_container_get_children(GTK_CONTAINER(main_win->RecentFilesMenu));
  item = list;

  while (item)
    {
    gtk_container_remove(GTK_CONTAINER(main_win->RecentFilesMenu), (GtkWidget *)item->data);
    item = g_list_next(item);
    }
  g_list_free(list);
  }

list = (GList *)g_object_get_data(G_OBJECT(main_win->RecentFilesMenu), "recent-files-list");

if (!list)
  {
  widget = gtk_menu_item_new_with_label("kein Dateien vorhanden");
  gtk_container_add(GTK_CONTAINER(main_win->RecentFilesMenu), widget);
  gtk_widget_show(widget);
  return;
  }

for (item = list; item; item = g_list_next(item))
  {
  name = (gchar *)item->data;
  widget = gtk_menu_item_new_with_label(name);
  g_object_set_data(G_OBJECT(widget), "file_name", (gpointer)name);
  g_signal_connect((gpointer)widget, "activate", G_CALLBACK(RecentFileCB), (gpointer)main_win);
  gtk_container_add(GTK_CONTAINER(main_win->RecentFilesMenu), widget);
  gtk_widget_show(widget);
  /* Add the file name to the recent files list on the Welcome screen */
  main_welcome_add_recent_capture_file(main_win, name);
  }
/* Add a Separator */

/* Add a clear Icon */
//g_signal_connect (action, "activate", G_CALLBACK (recent_clear_cb), NULL);
}


void RecentFileAddToList(struct TMainWin *main_win, const gchar *filename)
{
GList *list, *item;
guint cnt;
gchar *curr, *normalized_filename, *name;

if (g_path_is_absolute(filename))
  normalized_filename = g_strdup(filename);
else
  {
  curr = g_get_current_dir();
  normalized_filename = g_strdup_printf("%s%s%s", curr, G_DIR_SEPARATOR_S, filename);
  g_free(curr);
  }
#ifdef _WIN32
    g_strdelimit(normalized_filename, "/", '\\'); // replace all slashes by backslashes
#endif
list = (GList *)g_object_get_data(G_OBJECT(main_win->RecentFilesMenu), "recent-files-list");
cnt = 1;
for (item = list; item; cnt++)
  {
  name = (gchar *)item->data;
  /* Find the next element BEFORE we (possibly) free the current one below */
  item = item->next;
  if (cnt >= 10)  // <*>
    {
    list = g_list_remove(list, name);
    break;
    }
#ifdef _WIN32
  if (!g_ascii_strncasecmp(name, normalized_filename, 1000))
#else   /* _WIN32 */
  if (!strncmp(name, normalized_filename, 1000))
#endif
    {
    list = g_list_remove(list, name);
    break;
    }
  }
list = g_list_prepend(list, normalized_filename);
g_object_set_data(G_OBJECT(main_win->RecentFilesMenu), "recent-files-list", (gpointer)list);
}


void RecentFileLoadList(struct TMainWin *main_win, ConfigFile *cfgfile)
{
gchar *key, *tmpbuf;
guint i;

key = NULL;
tmpbuf = NULL;
for (i = 0; i < 10; i++)
  {
  key = g_strdup_printf("File%u", i);
  if (cfg_read_string(cfgfile, "RECENT_FILES", key, &tmpbuf))
    {
    RecentFileAddToList(main_win, tmpbuf);
    safe_free(tmpbuf);
    safe_free(key);
    }
  else
    break;
  }
safe_free(tmpbuf);
safe_free(key);
UpdateRecentItems(main_win);
}


/* write all capture filenames of the menu to the user's recent file */
void RecentFileSaveAndDestroyList(struct TMainWin *main_win, ConfigFile *cfgfile)
{
gchar *name, *key;
GList *list, *item;
guint idx;

list = (GList *)g_object_get_data(G_OBJECT(main_win->RecentFilesMenu), "recent-files-list");
item =  g_list_last(list);
idx = 0;
cfg_clear_section(cfgfile, "RECENT_FILES");
while (item)
  {
  if ((name = (gchar *)item->data))
    {
    key = g_strdup_printf("File%u", idx++);
    cfg_write_string(cfgfile, "RECENT_FILES", key, name);
    g_free(key);
    }
  item = g_list_previous(item);
  }
g_list_free(list);
g_object_set_data(G_OBJECT(main_win->RecentFilesMenu), "recent-files-list", NULL);
}
