/***************************************************************************
                           abaut.c  -  description
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
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dialogs.h"
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


//const GdkColor GdkColorBlack = { 0, 0, 0, 0};

#define DefColorListSize 18

static const gchar *DefColorList[DefColorListSize] = {
  "#7373DE", // 115, 115, 222
	"#FF7F73", // 255, 127, 115
	"#C100E0", // 193, 0,   224
	"#75F267", // 117, 242, 103
	"#00C3C3", // 0,   195, 195
	"#D5FD33", // 213, 253, 51
	"#D11B68", // 209, 27,  104
	"#FFC533", // 255, 197, 51
	"#BABABA", // 186, 186, 186
	"#D3D3FF", // 211, 211, 255
	"#FDD2CE", // 253, 210, 206
	"#ECC2F2", // 236, 194, 242
	"#D0F9CC", // 208, 249, 204
	"#B7FFFF", // 183, 255, 255
	"#F1FFB7", // 241, 255, 183
	"#FFCAE1", // 255, 202, 225
	"#FDEEC5", // 253, 238, 197
	"#E2E2E2"}; // 226, 226, 226

static GdkColor *GdkColorList = NULL;


GdkColor *GetColorByIndex(guint index)
{
guint i;

if (!GdkColorList)
  {
  GdkColorList = (GdkColor *)g_malloc0(DefColorListSize * sizeof(GdkColor));
  for (i = 0; i < DefColorListSize; i++)
    gdk_color_parse(DefColorList[i], &GdkColorList[i]);
  }
if (index >= DefColorListSize)
  index = DefColorListSize-1;
return(&GdkColorList[index]);
}


#define SET_RESULT(result, value) if ((result)) *(result) = (value)

char *get_item_as_string(char **str, char *trenner, int *result)
{
int hit, cnt, l;
char *s, *t, *start, *end, *item;

if ((!str) || (!trenner))
  {
  SET_RESULT(result, -1);
  return(NULL);
  }
s = *str;
if (!s)
  {
  SET_RESULT(result, -1);
  return(NULL);
  }
// Führende Leerzeichen überspringen
if (*s == '\0')
  {
  SET_RESULT(result, -1);
  return(NULL);
  }
SET_RESULT(result, 0);
end = s;
item = s;
start = s;

hit = 0;
for (s = end; *s; s++)
  {
  cnt = 0;
  for (t = trenner; *t; t++)
    {
    cnt++;
    if (*s == *t)
      {
      *s++ = '\0';
      end = s;
      SET_RESULT(result, cnt);
      hit = 1;
      break;
      }
    }
  if (hit)
    break;
  }
// Abschliesende Leerzeichen löschen
if ((l = strlen(item)))
  {
  s = item + (l-1);
  while ((l) && ((isspace((int)*s)) || (*s == '\n') || (*s == '\r')))
  {
    l--;
    s--;
  }
  if (l)
    s++;
  *s = 0;
  }
if (end == start)
  *str = start + strlen(end);
else
  *str = end;
return(item);
}


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


void SetSensitive(GtkWidget **widget_list[], gboolean sensitive)
{
GtkWidget **widget;
int i;

i = 0;
while ((widget = widget_list[i++]))
  gtk_widget_set_sensitive(*widget, sensitive);
}


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
size_t size;
char *text;

text_file = fopen(filename, "rb");
if (!text_file)
  g_error("Datei \"%s\" kann nicht geladen werden", filename);
fseek(text_file, 0L, SEEK_END);
size = ftell(text_file);
rewind(text_file);
text = (char *)g_malloc(size+1);
if (!text)
  g_error("Nicht genuegend Speicher");
if (fread(text, 1, size, text_file) != size)
  g_error("Fehler beim lesen der Datei: \"%s\"", filename);
text[size] = '\0';
fclose(text_file);
gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget)), text, -1);
g_free(text);
}


char *CreateFileName(const char *dir, const char *file_name)
{
size_t dir_len;

if (!file_name)
  return(NULL);
if (!dir)
  return(g_strdup(file_name));
dir_len = strlen(dir);
if ((!dir_len) || (strchr(file_name, G_DIR_SEPARATOR)))
  return(g_strdup(file_name));
else
  {
  if (dir[dir_len-1] == G_DIR_SEPARATOR)
    return(g_strconcat(dir, file_name, NULL));
  else
    return(g_strconcat(dir, DIR_SEPARATOR_STR, file_name, NULL));
  }
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


void gtk_combo_box_clear(GtkComboBox *combo_box)
{
GtkListStore *store;

store = GTK_LIST_STORE(gtk_combo_box_get_model(combo_box));
gtk_list_store_clear(store);
}


gint gtk_combo_box_set_text_item(GtkComboBox *combo_box, const gchar *item)
{
GtkTreeIter iter;
GtkTreeModel *model;
gint hit, idx;
gchar *str;

str = NULL;
idx = 0;
model = gtk_combo_box_get_model(combo_box);
if (item)
  {
  for (hit = gtk_tree_model_get_iter_first(model, &iter); hit; hit = gtk_tree_model_iter_next(model, &iter))
    {
    gtk_tree_model_get(model, &iter, 0, &str, -1);
    if (!str)
      break;
    if (!strcmp(str, item))
      break;
    idx++;
    safe_free(str);
    }
  }
if (!str)
  idx = -1;
safe_free(str);
gtk_combo_box_set_active(combo_box, idx);
return(idx);
}


gchar *gtk_combo_box_get_active_text(GtkComboBox *combo_box)
{
GtkTreeIter iter;
GtkTreeModel *model;
gint idx;
gchar *str;

str = NULL;
model = gtk_combo_box_get_model(combo_box);
if ((idx = gtk_combo_box_get_active(combo_box)) > -1)
  {
  if (gtk_tree_model_iter_nth_child(model, &iter, NULL, idx))
    gtk_tree_model_get(model, &iter, 0, &str, -1);
  }
return(str);
}


GtkWidget *ws_gtk_box_new(GtkOrientation orientation, gint spacing, gboolean homogeneous)
{
#if !GTK_CHECK_VERSION(3,0,0)
if (orientation == GTK_ORIENTATION_HORIZONTAL)
  return(gtk_hbox_new(homogeneous, spacing));
else
  return(gtk_vbox_new(homogeneous, spacing));
#else
GtkWidget *widget;

widget = gtk_box_new(orientation, spacing);
gtk_box_set_homogeneous(GTK_BOX(widget), homogeneous);
return(widget);
#endif /* GTK_CHECK_VERSION(3,0,0) */
}


void gtk_entry_set_text_ex(GtkEntry *entry, const gchar *text)
{
if (text)
  gtk_entry_set_text(entry, text);
else
  gtk_editable_delete_text(GTK_EDITABLE(entry), 0, -1);
}


void gtk_text_buffer_set_text_ex(GtkTextBuffer *buffer, const gchar *text, gint len)
{
GtkTextIter start, end;

if (text)
  gtk_text_buffer_set_text(buffer, text, len);
else
  {
  // Start und Stop Iter abfragen
  gtk_text_buffer_get_bounds(buffer, &start, &end);
  // Buffer löschen
  gtk_text_buffer_delete(buffer, &start, &end);
  }
}


gint get_default_col_size(GtkWidget *view, const gchar *str)
{
PangoLayout *layout;
gint col_width;

layout = gtk_widget_create_pango_layout(view, str);
pango_layout_get_pixel_size(layout, &col_width, NULL);
g_object_unref(G_OBJECT(layout));
// Add a single character's width to get some spacing between columns
return(col_width); //+ (pango_font_description_get_size(user_font_get_regular()) / PANGO_SCALE); <*> ?
}
