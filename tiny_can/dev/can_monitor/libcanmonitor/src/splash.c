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
/*                           Splash Window                                */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : "Splash" Window anzeigen                            */
/*                                                                        */
/*  Version         : 1.00                                                */
/*  Datei Name      : splash.c                                            */
/* ---------------------------------------------------------------------- */
/*  Datum           : 25.10.05                                            */
/*  Author          : Demlehner Klaus                                     */
/* ---------------------------------------------------------------------- */
/*  Compiler        : GNU C Compiler                                      */
/**************************************************************************/
#include <gtk/gtk.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include "support.h"
#include "util.h"
#include "gtk_util.h"
#include "splash.h"


static GtkWidget *SplashWin = NULL;
static GtkWidget *SplashLabel = NULL;
static GTimeVal SplashStartTime;
static const struct TSplashData *SplashData;




void SplashStart(gchar *file_path, const struct TSplashData *splash_data, char *message)
{
GtkWidget *image;
GtkHBox *vbox;
GdkColor color;
GdkPixbuf *pixbuf;
GError *error = NULL;
gchar *filename;
gchar *str;

if ((!file_path) || (!splash_data))
  return;
SplashData = splash_data;
SplashWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
gtk_window_set_position(GTK_WINDOW(SplashWin),
         GTK_WIN_POS_CENTER_ALWAYS);
gtk_window_set_decorated(GTK_WINDOW(SplashWin), FALSE);
gtk_window_set_role(GTK_WINDOW(SplashWin), "splash");
gtk_window_set_resizable(GTK_WINDOW(SplashWin), FALSE);
gtk_widget_realize(SplashWin);
// gtk_window_set_default_size(GTK_WINDOW(splashscreen.window), 150, 150);
color.red = 65535;
color.blue = 65535;
color.green = 65535;
gtk_widget_modify_bg(SplashWin, GTK_STATE_NORMAL,
           &color);
// **** VBox erzeugen
vbox = g_object_new(GTK_TYPE_VBOX, "border-width", 10, "spacing", 10, NULL);
gtk_container_add(GTK_CONTAINER(SplashWin), GTK_WIDGET(vbox));
// **** Grafik anzeigen
filename = CreateFileName(file_path, splash_data->Grafik);
pixbuf = gdk_pixbuf_new_from_file(filename, &error);
if (!pixbuf)
  g_warning(_("Loading file \"%s\" error: %s"), filename, error->message);
g_free(filename);
image = gtk_image_new_from_pixbuf(pixbuf);
gtk_box_pack_start(GTK_BOX(vbox), image, FALSE, FALSE, 0);
g_object_unref(pixbuf);
if (message)
  {
  // **** Label erzeugen
  str = g_strdup_printf("<span size=\"x-large\" weight=\"ultrabold\">%s</span>", message);
  SplashLabel = g_object_new(GTK_TYPE_LABEL,
                     "wrap", TRUE,
                     "use-markup", TRUE,
                     "label", str, NULL);
  g_free(str);
  gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(SplashLabel), FALSE, FALSE, 0);
  }
// **** Fenster anzeigen
gtk_widget_show_all(SplashWin);
UpdateGTK();
g_get_current_time(&SplashStartTime);
}


void SplashUpdate(char *message)
{
char *str;

if (!SplashWin)
  return;
gtk_window_present(GTK_WINDOW(SplashWin));
if ((message) && (SplashLabel))
  {
  // **** Label erzeugen
  str = g_strdup_printf("<span size=\"x-large\" weight=\"ultrabold\">%s</span>", message);
  gtk_label_set_markup(GTK_LABEL(SplashLabel), str);
  g_free(str);
  }
UpdateGTK();
}


/*******************************/
/* Timeout Funktion            */
/*******************************/
/*gint SplashTimeout(gpointer data)
{
if (SplashWin)
  {
  gtk_widget_destroy(SplashWin);
  SplashWin = NULL;
  }
return(FALSE);   // Funktion nicht mehr aufrufen
}


void SplashStop(void)
{
unsigned int i;
GTimeVal splash_end_time;

if (SplashWin)
  {
  g_get_current_time(&splash_end_time);
  i = splash_end_time.tv_sec - SplashStartTime.tv_sec;
  if (i < SplashData->ShowTime)
    {
    i = SplashData->ShowTime - i;
    g_timeout_add(i * 1000, (GtkFunction)SplashTimeout, NULL);
    }
  else
    gtk_widget_destroy(SplashWin);
  }
} */

void SplashStop(void)
{
unsigned int i;
GTimeVal splash_end_time;

if (SplashWin)
  {
  if (SplashData->ShowTime)
    {
    g_get_current_time(&splash_end_time);
    i = splash_end_time.tv_sec - SplashStartTime.tv_sec;
    if (i < SplashData->ShowTime)
      {
      i = SplashData->ShowTime - i;
      mhs_sleep(i * 1000);
      }
    }
  gtk_widget_destroy(SplashWin);
  SplashWin = NULL;
  }
}

