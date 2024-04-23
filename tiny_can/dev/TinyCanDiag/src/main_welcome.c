/***************************************************************************
                          main_welcome.c  -  description
                             -------------------
    begin             : 23.08.2017
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
#include "candiag_icons.h"
#include "tcan_mx_gui.h"
#include "project.h"
#include "open_ext.h"
#include "main_samples.h"
#include "main_welcome.h"

#define DOC_HOMEPAGE  0
#define DOC_M2_MANUAL 100
#define DOC_M3_MANUAL 101

/*static const gchar WelcomeHeaderString[] = { <*>
  "<span weight=\"bold\" size=\"x-large\" foreground=\"white\">"
  "Tiny-CAN Diag" \
  "</span>\n<span size=\"large\" foreground=\"white\">" \
  "Version: 1.00" \
  "</span>"};*/

/* Foreground colors are set using Pango markup */
#if GTK_CHECK_VERSION(3,0,0)
const GdkRGBA rgba_welcome_bg = {0.901, 0.901, 0.901, 1.0 };
const GdkRGBA rgba_header_bar_bg = { 0.094, 0.360, 0.792, 1.0 };
const GdkRGBA rgba_topic_header_bg = {  0.004, 0.224, 0.745, 1.0 };
const GdkRGBA rgba_topic_content_bg = { 1, 1, 1, 1.0 };
GdkRGBA rgba_topic_item_idle_bg;
const GdkRGBA rgba_topic_item_entered_bg =  { 0.827, 0.847, 0.854, 1.0 };
#else
const GdkColor welcome_bg = { 0, 0xe6e6, 0xe6e6, 0xe6e6 };
const GdkColor header_bar_bg = { 0, 0x1818, 0x5c5c, 0xcaca };
const GdkColor topic_header_bg = { 0, 0x0101, 0x3939, 0xbebe };
const GdkColor topic_content_bg = { 0, 0xffff, 0xffff, 0xffff };
GdkColor topic_item_idle_bg;
const GdkColor topic_item_entered_bg = { 0, 0xd3d3, 0xd8d8, 0xdada };
#endif



static gboolean new_project(GtkWidget *w, GdkEventButton *event, gpointer user_data)
{
struct TMainWin *main_win;
guint idx;
(void)event;

idx = (guint)g_object_get_data(G_OBJECT(w), "user_data2");
main_win = (struct TMainWin *)user_data;
NewPrj(main_win, idx);
return(TRUE);
}


static gboolean open_project(GtkWidget *w, GdkEventButton *event, gpointer user_data)
{
struct TMainWin *main_win;
(void)event;
(void)w;

main_win = (struct TMainWin *)user_data;
OpenProjectFile(main_win);
return(TRUE);
}


static gboolean open_ext(GtkWidget *w, GdkEventButton *event, gpointer user_data)
{
//struct TMainWin *main_win;
guint idx;
const gchar *file_url;
gchar *str;
gboolean res;
(void)event;
(void)user_data;

res = FALSE;
idx = (guint)g_object_get_data(G_OBJECT(w), "user_data2");
//main_win = (struct TMainWin *)user_data;
if (idx >= 100)
  {
  switch (idx)
    {
    case DOC_M2_MANUAL :
       {
       file_url = "tiny_can_m2_manual.pdf";
       break;
       }
    case DOC_M3_MANUAL :
       {
       file_url = "tiny_can_m3_manual.pdf";
       break;
       }
    default : file_url = NULL;
    }
  if ((str = CreateDocFileName(file_url)))
    {
    res = OpenExternel(str);
    g_free(str);
    }
  }
else
  {
  switch (idx)
    {
    case DOC_HOMEPAGE :
       {
       file_url = "http://www.mhs-elektronik.de";
       break;
       }
    default : file_url = NULL;
    }
  res = OpenExternel(file_url);
  }
return(res);
}


/* mouse entered this widget - change background color */
static gboolean welcome_item_enter_cb(GtkWidget *eb, GdkEventCrossing *event, gpointer user_data)
{
(void)event;
(void)user_data;

#if GTK_CHECK_VERSION(3,0,0)
gtk_widget_override_background_color(eb, GTK_STATE_FLAG_NORMAL, &rgba_topic_item_entered_bg);
#else
gtk_widget_modify_bg(eb, GTK_STATE_NORMAL, &topic_item_entered_bg);
#endif
return(FALSE);
}


/* mouse has left this widget - change background color  */
static gboolean welcome_item_leave_cb(GtkWidget *eb, GdkEventCrossing *event, gpointer user_data)
{
(void)event;
(void)user_data;

#if GTK_CHECK_VERSION(3,0,0)
gtk_widget_override_background_color(eb, GTK_STATE_FLAG_NORMAL, &rgba_topic_item_idle_bg);
#else
gtk_widget_modify_bg(eb, GTK_STATE_NORMAL, &topic_item_idle_bg);
#endif
return(FALSE);
}


/* create a "button widget" */
GtkWidget *welcome_button(const gchar *image,
               const gchar *title, const gchar *subtitle, const gchar *tooltip,
               welcome_button_callback_t callback, gpointer user_data, gpointer user_data2)
{
GtkWidget *eb, *w, *item_hb, *text_vb;
gchar *formatted_text, *filename;

item_hb = ws_gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1, FALSE);
/* event box (for background color and events) */
eb = gtk_event_box_new();
gtk_container_add(GTK_CONTAINER(eb), item_hb);
#if GTK_CHECK_VERSION(3,0,0)
gtk_widget_override_background_color(eb, GTK_STATE_FLAG_NORMAL, &rgba_topic_item_idle_bg);
#else
gtk_widget_modify_bg(eb, GTK_STATE_NORMAL, &topic_item_idle_bg);
#endif
if(tooltip != NULL)
  gtk_widget_set_tooltip_text(eb, tooltip);

g_object_set_data(G_OBJECT(eb), "user_data2", user_data2);
g_signal_connect(eb, "enter-notify-event", G_CALLBACK(welcome_item_enter_cb), NULL);
g_signal_connect(eb, "leave-notify-event", G_CALLBACK(welcome_item_leave_cb), NULL);
g_signal_connect(eb, "button-release-event", G_CALLBACK(callback), user_data);

/* icon */
if (image)
  {
  filename = CreateFileName(Paths.base_dir, image);
  w = gtk_image_new_from_file(filename);
  g_free(filename);
  gtk_box_pack_start(GTK_BOX(item_hb), w, FALSE, FALSE, 5);
  }

text_vb = ws_gtk_box_new(GTK_ORIENTATION_VERTICAL, 3, FALSE);

/* title */
w = gtk_label_new(title);
gtk_misc_set_alignment (GTK_MISC(w), 0.0f, 0.5f);
formatted_text = g_strdup_printf("<span weight=\"bold\" size=\"x-large\" foreground=\"black\">%s</span>", title);
gtk_label_set_markup(GTK_LABEL(w), formatted_text);
g_free(formatted_text);
gtk_box_pack_start(GTK_BOX(text_vb), w, FALSE, FALSE, 1);

/* subtitle */
w = gtk_label_new(subtitle);
gtk_misc_set_alignment (GTK_MISC(w), 0.0f, 0.5f);
formatted_text = g_strdup_printf("<span size=\"small\" foreground=\"black\">%s</span>", subtitle);
gtk_label_set_markup(GTK_LABEL(w), formatted_text);
g_free(formatted_text);
gtk_box_pack_start(GTK_BOX(text_vb), w, FALSE, FALSE, 1);

gtk_box_pack_start(GTK_BOX(item_hb), text_vb, TRUE, TRUE, 5);
return(eb);
}


/* create the banner "above our heads" */
/*static GtkWidget *welcome_header_new(void) <*>
{
GtkWidget *item_vb, *item_hb, *eb, *widget;
gchar *filename;

item_vb = ws_gtk_box_new(GTK_ORIENTATION_VERTICAL, 0, FALSE);
// colorize vbox
eb = gtk_event_box_new();
gtk_container_add(GTK_CONTAINER(eb), item_vb);
#if GTK_CHECK_VERSION(3,0,0)
gtk_widget_override_background_color(eb, GTK_STATE_FLAG_NORMAL, &rgba_header_bar_bg);
#else
gtk_widget_modify_bg(eb, GTK_STATE_NORMAL, &header_bar_bg);
#endif
item_hb = ws_gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0, FALSE);
gtk_box_pack_start(GTK_BOX(item_vb), item_hb, FALSE, FALSE, 10);

filename = CreateFileName(Paths.base_dir, TINY_CAN_DIAG_ICONE);
widget = gtk_image_new_from_file(filename);
g_free(filename);
gtk_box_pack_start(GTK_BOX(item_hb), widget, FALSE, FALSE, 10);

widget = gtk_label_new(NULL);
gtk_label_set_markup(GTK_LABEL(widget), WelcomeHeaderString);
gtk_misc_set_alignment(GTK_MISC(widget), 0.0f, 0.5f);
gtk_box_pack_start(GTK_BOX(item_hb), widget, TRUE, TRUE, 5);
gtk_widget_show_all(eb);
return(eb);
}*/


/* create a "topic widget" */
static GtkWidget *welcome_topic_new(const char *header, GtkWidget **to_fill)
{
GtkWidget *topic_vb, *layout_vb, *eb, *w;
gchar *str;

topic_vb = ws_gtk_box_new(GTK_ORIENTATION_VERTICAL, 0, FALSE);
// **** Header
w = gtk_label_new(NULL);
str = g_strdup_printf("<span weight=\"bold\" size=\"x-large\" foreground=\"white\">%s</span>", header);
gtk_label_set_markup(GTK_LABEL(w), str);
g_free(str);
eb = gtk_event_box_new();
gtk_container_add(GTK_CONTAINER(eb), w);
#if GTK_CHECK_VERSION(3,0,0)
gtk_widget_override_background_color(eb, GTK_STATE_FLAG_NORMAL, &rgba_topic_header_bg);
#else
gtk_widget_modify_bg(eb, GTK_STATE_NORMAL, &topic_header_bg);
#endif
gtk_box_pack_start(GTK_BOX(topic_vb), eb, FALSE, FALSE, 0);

layout_vb = ws_gtk_box_new(GTK_ORIENTATION_VERTICAL, 5, FALSE);
gtk_container_set_border_width(GTK_CONTAINER(layout_vb), 10);
gtk_box_pack_start(GTK_BOX(topic_vb), layout_vb, FALSE, FALSE, 0);

/* colorize vbox (we need an event box for this!) */
eb = gtk_event_box_new();
gtk_container_add(GTK_CONTAINER(eb), topic_vb);
#if GTK_CHECK_VERSION(3,0,0)
gtk_widget_override_background_color(eb, GTK_STATE_FLAG_NORMAL, &rgba_topic_content_bg);
#else
gtk_widget_modify_bg(eb, GTK_STATE_NORMAL, &topic_content_bg);
#endif
*to_fill = layout_vb;
return(eb);
}


static void fill_new_projects(struct TMainWin *main_win, GtkWidget *box)
{
GtkWidget *w;

w = welcome_button(TINY_CAN_M2_ICONE, "Tiny-CAN M2", "", NULL, new_project, (gpointer)main_win, (gpointer)HARDWARE_TYPE_TCAN_M2);
gtk_box_pack_start(GTK_BOX(box), w, FALSE, FALSE, 5);
w = welcome_button(TINY_CAN_M3_ICONE, "Tiny-CAN M3", "", NULL, new_project, (gpointer)main_win, (gpointer)HARDWARE_TYPE_TCAN_M3);
gtk_box_pack_start(GTK_BOX(box), w, FALSE, FALSE, 5);
}


/* create the welcome page */
GtkWidget *welcome_new(struct TMainWin *main_win)
{
GtkWidget *welcome_eb, *welcome_vb, *welcome_hb, *widget, *box, *column_vb;

#if GTK_CHECK_VERSION(3,0,0)
rgba_topic_item_idle_bg = rgba_topic_content_bg;
#else
topic_item_idle_bg = topic_content_bg;
#endif

welcome_vb = ws_gtk_box_new(GTK_ORIENTATION_VERTICAL, 0, FALSE);

welcome_eb = gtk_event_box_new();
gtk_container_add(GTK_CONTAINER(welcome_eb), welcome_vb);
#if GTK_CHECK_VERSION(3,0,0)
gtk_widget_override_background_color(welcome_eb, GTK_STATE_FLAG_NORMAL, &rgba_welcome_bg);
#else
gtk_widget_modify_bg(welcome_eb, GTK_STATE_NORMAL, &welcome_bg);
#endif
/* header */
//widget = welcome_header_new(); <*>
//gtk_box_pack_start(GTK_BOX(welcome_vb), widget, FALSE, FALSE, 0);

/* content */
welcome_hb = ws_gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10, FALSE);
gtk_container_set_border_width(GTK_CONTAINER(welcome_hb), 10);
gtk_box_pack_start(GTK_BOX(welcome_vb), welcome_hb, TRUE, TRUE, 0);

// Spalte: Neu
column_vb = ws_gtk_box_new(GTK_ORIENTATION_VERTICAL, 5, FALSE);
#if GTK_CHECK_VERSION(3,0,0)
gtk_widget_override_background_color(column_vb, GTK_STATE_FLAG_NORMAL, &rgba_welcome_bg);
#else
gtk_widget_modify_bg(column_vb, GTK_STATE_NORMAL, &welcome_bg);
#endif
gtk_box_pack_start(GTK_BOX(welcome_hb), column_vb, TRUE, TRUE, 0);

widget = welcome_topic_new("Neues Projekt", &box);
gtk_box_pack_start(GTK_BOX(column_vb), widget, TRUE, TRUE, 0);
fill_new_projects(main_win, box);

// Spalte: Files / Samples
column_vb = ws_gtk_box_new(GTK_ORIENTATION_VERTICAL, 5, FALSE);
gtk_box_pack_start(GTK_BOX(welcome_hb), column_vb, TRUE, TRUE, 0);

widget = welcome_topic_new("Files", &box);
gtk_box_pack_start(GTK_BOX(column_vb), widget, TRUE, TRUE, 0);

widget = welcome_button(OPEN_ICONE, "Öffnen", "Projekt File öffnen",
        NULL, open_project, (gpointer)main_win, NULL);
gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
// prepare list of recent files
widget = gtk_label_new(NULL);
gtk_label_set_markup(GTK_LABEL(widget), "<span foreground=\"black\">Zuletzt geöffnete Dateien:</span>");
gtk_misc_set_alignment(GTK_MISC(widget), 0.0f, 0.0f);
gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 5);

main_win->FileBox = ws_gtk_box_new(GTK_ORIENTATION_VERTICAL, 1, FALSE);
gtk_box_pack_start(GTK_BOX(box), main_win->FileBox, FALSE, FALSE, 0);

widget = welcome_topic_new("Samples", &box);
gtk_box_pack_start(GTK_BOX(column_vb), widget, TRUE, TRUE, 0);
fill_sample_projects(main_win, box);

// Spalte: Online / Help
column_vb = ws_gtk_box_new(GTK_ORIENTATION_VERTICAL, 5, FALSE);
gtk_box_pack_start(GTK_BOX(welcome_hb), column_vb, TRUE, TRUE, 0);

/* topic Online */
widget = welcome_topic_new("Online", &box);
gtk_box_pack_start(GTK_BOX(column_vb), widget, TRUE, TRUE, 0);

widget = welcome_button(WEB_ICONE, "Website", "Visit the project's website", NULL, open_ext,
        (gpointer)main_win, (gpointer)DOC_HOMEPAGE);
gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);

/* topic Help */
widget = welcome_topic_new("Help", &box);
gtk_box_pack_start(GTK_BOX(column_vb), widget, TRUE, TRUE, 0);

widget = welcome_button(TINY_CAN_M2_MANUAL_ICONE, "Tiny-CAN M2 User Manual", "tiny_can_m2_manual.pdf", NULL, open_ext,
        (gpointer)main_win, (gpointer)DOC_M2_MANUAL);
gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);

widget = welcome_button(TINY_CAN_M3_MANUAL_ICONE, "Tiny-CAN M3 User Manual", "tiny_can_m3_manual.pdf", NULL, open_ext,
        (gpointer)main_win, (gpointer)DOC_M3_MANUAL);
gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);

/* the end */
gtk_widget_show_all(welcome_eb);

return(welcome_eb);
}


