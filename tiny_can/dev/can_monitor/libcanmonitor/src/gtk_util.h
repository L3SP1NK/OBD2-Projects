#ifndef __GTK_UTIL_H__
#define __GTK_UTIL_H__

#include "can_types.h"
#include <gtk/gtk.h>
// #include <glade/glade.h>


void UpdateGTK(void);
void WidgetSetColor(GtkWidget *w, GdkColor *text_color, GdkColor *bg_color);
// void CreateWidgetList(GladeXML *ui, char *str_list[], GtkWidget *widget_list[]);
void SetSensitive(GtkWidget **widget_list[], gboolean sensitive);
// void SetSensitiveByNames(GladeXML *ui, char *str_list[], gboolean sensitive);
void SetSensitiveList(GtkWidget **widget_list[], unsigned char *sen_list);
//void SetSensitiveListByNames(GladeXML *ui, char *str_list[], unsigned char *sen_list);
gchar *GetDefaultFont(GtkWidget *widget, int monospace);
void TextLoadFromFile(GtkWidget *widget, const char *filename);
char *CreateFileName(char *dir, char *file_name);
int SplitFileName(char *full_file_name, char **dir_name, char **file_name);
GtkWidget *TextPageNewFromFile(const char *filename);

#endif
