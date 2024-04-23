#ifndef __GUI_HELP_H__
#define __GUI_HELP_H__

#include "gui.h"

#ifdef __cplusplus
  extern "C" {
#endif

void IoPinSetDefaultName(struct TCanHw *hw, int pin_idx);
gint GetConfigComboSel(GtkWidget *widget);
void GetConfigForPin(struct TCanHw *hw, int pin_idx);
void SetWidgetLabel(GtkWidget *label_wdg, const gchar *str);
void SetButtonWidgetLabel(GtkWidget *wdg, const gchar *str);
void SetWidgetLabelAsInt(GtkWidget *label_wdg, gint value);
void SetGuiShowing(struct TCanHw *hw, int config);
void GetConfigFromGui(struct TCanHw *hw);
void SetConfigToGui(struct TCanHw *hw);
void SetConfigComboBox(struct TCanHw *hw, GtkWidget *widget, guint pin_idx);

#ifdef __cplusplus
  }
#endif

#endif
