#ifndef __MAIN_WELCOME_H__
#define __MAIN_WELCOME_H__

#include <gtk/gtk.h>
#include "gui.h"

#ifdef __cplusplus
  extern "C" {
#endif

/* Foreground colors are set using Pango markup */
#if GTK_CHECK_VERSION(3,0,0)
extern const GdkRGBA rgba_welcome_bg;
extern const GdkRGBA rgba_header_bar_bg;
extern const GdkRGBA rgba_topic_header_bg;
extern const GdkRGBA rgba_topic_content_bg;
extern const GdkRGBA rgba_topic_item_entered_bg;
#else
extern const GdkColor welcome_bg;
extern const GdkColor header_bar_bg;
extern const GdkColor topic_header_bg;
extern const GdkColor topic_content_bg;
extern const GdkColor topic_item_entered_bg;
#endif

typedef gboolean (*welcome_button_callback_t)  (GtkWidget      *widget,
                                                GdkEventButton *event,
                                                gpointer        user_data);

GtkWidget *welcome_button(const gchar *image,
               const gchar *title, const gchar *subtitle, const gchar *tooltip,
               welcome_button_callback_t callback, gpointer user_data, gpointer user_data2);

/*gboolean welcome_item_enter_cb(GtkWidget *eb, GdkEventCrossing *event, gpointer user_data);
gboolean welcome_item_leave_cb(GtkWidget *eb, GdkEventCrossing *event, gpointer user_data);*/

GtkWidget *welcome_new(struct TMainWin *main_win);

#ifdef __cplusplus
  }
#endif

#endif
