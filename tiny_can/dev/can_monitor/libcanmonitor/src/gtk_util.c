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
/*                            GTK Utilitis                                */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : Hilfsfunktionen zu GTK/Glade                        */
/*                                                                        */
/*  Version         : 1.00                                                */
/*  Datei Name      : gtk_util.c                                          */
/* ---------------------------------------------------------------------- */
/*  Datum           : 17.10.05                                            */
/*  Author          : Demlehner Klaus                                     */
/* ---------------------------------------------------------------------- */
/*  Compiler        : GNU C Compiler                                      */
/**************************************************************************/
#include "global.h"
#include <gtk/gtk.h>
//#include <glade/glade.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "support.h"
#include "can_types.h"
#include "dialogs.h"
#include "util.h"
#include "gtk_util.h"

#ifdef __WIN32__
  #define DEF_FONT_NAME_NONMONO "sans"
  #define DEF_FONT_NAME_MONO    "courier new"
  #define DIR_SEPARATOR '\\'
  #define DIR_SEPARATOR_STR "\\"
#else
  #define DEF_FONT_NAME_NONMONO "sans"
  #define DEF_FONT_NAME_MONO    "monospace"
  #define DIR_SEPARATOR '/'
  #define DIR_SEPARATOR_STR "/"
#endif



void UpdateGTK(void)
{
while (gtk_events_pending())
  gtk_main_iteration();
}


void WidgetSetColor(GtkWidget *w, GdkColor *text_color, GdkColor *bg_color)
{
GtkStyle *style;

style = gtk_style_copy(gtk_widget_get_style(w));
if (text_color)
  style->text[GTK_STATE_NORMAL] = *text_color;
if (bg_color)
style->base[GTK_STATE_NORMAL] = *bg_color;
gtk_widget_set_style(w, style);
gtk_style_unref(style);
}


/* void CreateWidgetList(GladeXML *ui ,char *str_list[], GtkWidget *widget_list[])
{
GtkWidget *widget;
int i;
char *str;

i = 0;
while (*(str = str_list[i]))
  {
  widget = glade_xml_get_widget(ui, str);
  if (!widget)
    g_error("widget %s nicht gefunden", str);
  widget_list[i++] = widget;
  }
widget_list[i] = NULL;
}*/


void SetSensitive(GtkWidget **widget_list[], gboolean sensitive)
{
GtkWidget **widget;
int i;

i = 0;
while ((widget = widget_list[i++]))
  gtk_widget_set_sensitive(*widget, sensitive);
}


/* void SetSensitiveByNames(GladeXML *ui, char *str_list[], gboolean sensitive)
{
GtkWidget *widget;
int i;
char *str;

i = 0;
while (*(str = str_list[i++]))
  {
  widget = glade_xml_get_widget(ui, str);
  if (!widget)
    g_error("widget %s nicht gefunden", str);
  gtk_widget_set_sensitive(widget, sensitive);
  }
} */


void SetSensitiveList(GtkWidget **widget_list[], unsigned char *sen_list)
{
GtkWidget **widget;
int i;
unsigned char s;

i = 0;
while ((widget = widget_list[i++]))
  {
  s = *sen_list++;
  if (s == 1)
    gtk_widget_set_sensitive(*widget, TRUE);
  else if (s == 0)
    gtk_widget_set_sensitive(*widget, FALSE);
  }
}


/*void SetSensitiveListByNames(GladeXML *ui, char *str_list[], unsigned char *sen_list)
{
GtkWidget *widget;
int i;
unsigned char s;
char *str;

i = 0;
while (*(str = str_list[i++]))
  {
  widget = glade_xml_get_widget(ui, str);
  if (!widget)
    g_error("widget %s nicht gefunden", str);
  s = *sen_list++;
  if (s == 1)
    gtk_widget_set_sensitive(widget, TRUE);
  else if (s == 0)
    gtk_widget_set_sensitive(widget, FALSE);
  }
} */


gchar *GetDefaultFont(GtkWidget *widget, int monospace)
{
PangoFontFamily **families;
PangoFontFamily *nonmono_family, *mono_family, *sel_family;
PangoFontFace **faces;
PangoFontDescription *font_desc;
const gchar *name;
gchar *font_name;
gint n, i;

pango_context_list_families (gtk_widget_get_pango_context(widget), &families, &n);
nonmono_family = NULL;
mono_family = NULL;
sel_family = NULL;
for (i=0; i < n; i++)
  {
  name = pango_font_family_get_name(families[i]);
  if (!g_ascii_strcasecmp (name, DEF_FONT_NAME_NONMONO))
    nonmono_family = families[i];
  if (!g_ascii_strcasecmp (name, DEF_FONT_NAME_MONO))
    mono_family = families[i];
  if (monospace && !pango_font_family_is_monospace (families[i]))
     continue;
  sel_family = families[i];
  }
if (monospace)
  {
  if (mono_family)
    sel_family = mono_family;
  }
else
  {
  if (nonmono_family)
    sel_family = nonmono_family;
  }
if (!sel_family)
  sel_family = families[0];
pango_font_family_list_faces (sel_family, &faces, &n);
font_desc = pango_font_face_describe(faces[0]);
pango_font_description_set_size (font_desc, 10 * PANGO_SCALE);
font_name = pango_font_description_to_string (font_desc);
g_free (faces);
pango_font_description_free (font_desc);
g_free (families);
return(font_name);
}


void TextLoadFromFile(GtkWidget *widget, const char *filename)
{
FILE *text_file;
int size;
char *text;

text_file = fopen(filename, "rb");
if (!text_file)
  g_error(_("Error loading file \"%s\""), filename);
fseek(text_file, 0L, SEEK_END);
size = ftell(text_file);
rewind(text_file);
text = (char *)g_malloc(size+1);
if (!text)
  g_error(_("Out of Memory"));
if (fread(text, 1, size, text_file) != size)
  g_error(_("Error reading data from file: \"%s\""), filename);
text[size] = '\0';
fclose(text_file);
gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget)), text, -1);
g_free(text);
}


char *CreateFileName(char *dir, char *file_name)
{
if ((!dir) || (!file_name))
  return(NULL);
if (strchr(file_name, DIR_SEPARATOR))
  return(g_strdup(file_name));
else
  return(g_strconcat(dir, DIR_SEPARATOR_STR, file_name, NULL));
}


int SplitFileName(char *full_file_name, char **dir_name, char **file_name)
{
int len;
char *base;
*dir_name = NULL;
*file_name = NULL;

if ((!file_name) || (!dir_name) || (!file_name))
  return(-1);
if (!strlen(full_file_name))
  return(-1);
base = strrchr(full_file_name, G_DIR_SEPARATOR);
if (!base)
  {
  if (strlen(full_file_name))
    *file_name = g_strdup(full_file_name);
  }
else
  {
  if (strlen(base+1))
      *file_name = g_strdup(base+1);
  while ((base > full_file_name) && (*base == G_DIR_SEPARATOR))
    base--;
  len = base - full_file_name + 1;

  base = (char *)g_malloc(len + 1);
  if (!base)
    return(-1);
  memcpy(base, full_file_name, len);
  base[len] = 0;
  *dir_name = base;
  }
return(0);
}


GtkWidget *TextPageNewFromFile(const char *filename)
{
GtkWidget *page_vb, *txt_scrollw, *txt;

page_vb = gtk_vbox_new(FALSE, 0);
gtk_container_border_width(GTK_CONTAINER(page_vb), 1);
txt_scrollw = gtk_scrolled_window_new(NULL, NULL);
gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(txt_scrollw),
                                 GTK_SHADOW_IN);
gtk_box_pack_start(GTK_BOX(page_vb), txt_scrollw, TRUE, TRUE, 0);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(txt_scrollw),
				 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
txt = gtk_text_view_new();
gtk_text_view_set_editable(GTK_TEXT_VIEW(txt), FALSE);
gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(txt), GTK_WRAP_WORD);
gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(txt), FALSE);

TextLoadFromFile(txt, filename);
gtk_container_add(GTK_CONTAINER(txt_scrollw), txt);
gtk_widget_show(txt_scrollw);
gtk_widget_show(txt);
gtk_widget_show(page_vb);

return(page_vb);
}

/* void push_appdir_to_path (void)
{
#ifdef __WIN32__
    char *appdir;
    const char *path;
    char *new_path;

    appdir = moo_win32_get_app_dir ();
    g_return_if_fail (appdir != NULL);

    path = g_getenv ("Path");

    if (path)
        new_path = g_strdup_printf ("%s;%s", appdir, path);
    else
        new_path = g_strdup (appdir);

    g_setenv ("Path", new_path, TRUE);

    g_free (new_path);
    g_free (appdir);
#endif
} */


