#ifndef __GTK_UTIL_H__
#define __GTK_UTIL_H__

#include <gtk/gtk.h>

#ifdef __cplusplus
  extern "C" {
#endif

#define lo(x)  (x & 0xFF)
#define hi(x)  ((x >> 8) & 0xFF)

#define safe_free(d) do { \
  if ((d)) \
    { \
    g_free((d)); \
    (d) = NULL; \
    } \
  } while(0)

GdkColor *GetColorByIndex(guint index);

char *get_item_as_string(char **str, char *trenner, int *result);

void UpdateGTK(void);
void WidgetSetColor(GtkWidget *w, GdkColor *text_color, GdkColor *bg_color);
void SetSensitive(GtkWidget **widget_list[], gboolean sensitive);
void SetSensitiveList(GtkWidget **widget_list[], unsigned char *sen_list);;
gchar *GetDefaultFont(GtkWidget *widget, int monospace);
void TextLoadFromFile(GtkWidget *widget, const char *filename);
char *CreateFileName(const char *dir, const char *file_name);
int SplitFileName(char *full_file_name, char **dir_name, char **file_name);
GtkWidget *TextPageNewFromFile(const char *filename);

void gtk_combo_box_clear(GtkComboBox *combo_box);
gint gtk_combo_box_set_text_item(GtkComboBox *combo_box, const gchar *item);
gchar *gtk_combo_box_get_active_text(GtkComboBox *combo_box);

GtkWidget *ws_gtk_box_new(GtkOrientation orientation, gint spacing, gboolean homogeneous);
void gtk_entry_set_text_ex(GtkEntry *entry, const gchar *text);
void gtk_text_buffer_set_text_ex(GtkTextBuffer *buffer, const gchar *text, gint len);
gint get_default_col_size(GtkWidget *view, const gchar *str);

#endif
