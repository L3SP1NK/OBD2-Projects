/***************************************************************************
                          gui_help.c  -  description
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
#include "hw_types.h"
#include "gui_help.h"


static const gchar *ConfigComboStrings[] =
                                    {"Disable Pin",   // 0 = Nicht verwendet
                                     "Input",         // 1 = Digital Input
                                     "Output",        // 2 = Digital Output
                                     "Analog-In",     // 3 = Analog Input
                                     "Analog-Out",    // 4 = Analog Output
                                     "Taster",        // 5 = Taster
                                     "Schalter",      // 6 = Schalter
                                     "Encoder-Pin A", // 7 = Encoder
                                     "Encoder-Pin B",
                                     NULL};

/*******************************************************************************/
/*                            Hilfsfunktionen                                  */
/*******************************************************************************/
void IoPinSetDefaultName(struct TCanHw *hw, int pin_idx)
{
struct TIoPin *io_pin;

io_pin = &hw->IoPin[pin_idx];
safe_free(io_pin->Name);
if (io_pin->Mode == PIN_CFG_NOT_USED)
  io_pin->Name = g_strdup_printf("%s - Disabled", hw->IoPinDefs[pin_idx].CfgName);
if (io_pin->Mode == PIN_CFG_DIG_IN)
  io_pin->Name = g_strdup(hw->IoPinDefs[pin_idx].InName);
else if (io_pin->Mode == PIN_CFG_DIG_OUT)
  io_pin->Name = g_strdup(hw->IoPinDefs[pin_idx].OutName);
else if (io_pin->Mode == PIN_CFG_KEY)
  io_pin->Name = g_strdup(hw->IoPinDefs[pin_idx].KeyName);
else if (io_pin->Mode == PIN_CFG_SWITCH)
  io_pin->Name = g_strdup(hw->IoPinDefs[pin_idx].SwitchName);
else
  io_pin->Name = g_strdup(hw->IoPinDefs[pin_idx].AnalogName);
}



gint GetConfigComboSel(GtkWidget *widget)
{
gchar *str;
const gchar *s;
gint i;

if (!(str = gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget))))
  return(-1);
for (i = 0; (s = ConfigComboStrings[i]); i++)
  {
  if (!strcmp(s, str))
    break;
  }
safe_free(str);
return(i);
}


void GetConfigForPin(struct TCanHw *hw, int pin_idx)
{
unsigned int mode;

mode = GetConfigComboSel(hw->IoPin[pin_idx].ConfigWidget);
hw->IoPin[pin_idx].Mode = mode;
if (hw->IoPin->DefaultName)
  IoPinSetDefaultName(hw, pin_idx);
}


void SetWidgetLabel(GtkWidget *label_wdg, const gchar *str)
{
gchar *s;

if ((label_wdg) && (str))
  {
  s = g_strdup_printf("<span weight=\"bold\" size=\"large\">%s</span>", str);
  gtk_label_set_markup(GTK_LABEL(label_wdg), s);
  g_free(s);
  }
}


void SetButtonWidgetLabel(GtkWidget *wdg, const gchar *str)
{
gchar *s;
GList *list;
GtkWidget *label;

if ((wdg) && (str))
  {
  list = gtk_container_get_children(GTK_CONTAINER(wdg));
  label = list->data;
  if ((label) && (GTK_IS_LABEL(label)))
    {
    s = g_strdup_printf("<span weight=\"bold\" size=\"large\">%s</span>", str);
    gtk_label_set_markup(GTK_LABEL(label), s);
    g_free(s);
    }
  }
}


void SetWidgetLabelAsInt(GtkWidget *label_wdg, gint value)
{
gchar *s;

if (label_wdg)
  {
  s = g_strdup_printf("<span weight=\"bold\" size=\"large\">%5d</span>", value);
  gtk_label_set_markup(GTK_LABEL(label_wdg), s);
  g_free(s);
  }
}


void SetGuiShowing(struct TCanHw *hw, int config)
{
gchar *str;
guint i;
int mode;

for (i = 0; i < hw->IoPinsCount; i++)
  {
  if (config)
    {
    mode = 100;
#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_override_background_color(hw->IoPin[i].Frame, GTK_STATE_FLAG_NORMAL, &rgba_value_bg); // <*>
#else
    gtk_widget_modify_bg(hw->IoPin[i].Frame, GTK_STATE_NORMAL, GetColorByIndex(10));
#endif // GTK_CHECK_VERSION
    }
  else
    {
    mode = hw->IoPin[i].Mode;
#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_override_background_color(hw->IoPin[i].Frame, GTK_STATE_FLAG_NORMAL, &rgba_value_bg); // <*>
#else
    gtk_widget_modify_bg(hw->IoPin[i].Frame, GTK_STATE_NORMAL, GetColorByIndex(mode));
    gtk_widget_modify_bg(hw->IoPin[i].InWidget, GTK_STATE_NORMAL, GetColorByIndex(mode));
#endif // GTK_CHECK_VERSION
    }
  switch (mode)
    {
    case PIN_CFG_DIG_IN  :
              {  // Digital Input
              if (hw->IoPin[i].OutWidget)
                gtk_widget_hide(hw->IoPin[i].OutWidget);
              if (hw->IoPin[i].InBoxWidget)
                gtk_widget_show(hw->IoPin[i].InBoxWidget);
              if (hw->IoPin[i].AInBoxWidget)
                gtk_widget_hide(hw->IoPin[i].AInBoxWidget);
              if (hw->IoPin[i].AOutBoxWidget)
                gtk_widget_hide(hw->IoPin[i].AOutBoxWidget);
              SetWidgetLabel(hw->IoPin[i].InNameWidget, hw->IoPin[i].Name);
              gtk_widget_hide(hw->IoPin[i].ConfigBoxWidget);
              gtk_widget_hide(hw->IoPin[i].DisableWidget);
              break;
              }
    case PIN_CFG_DIG_OUT :
              {  // Digital Output
              SetButtonWidgetLabel(hw->IoPin[i].OutWidget, hw->IoPin[i].Name);
              gtk_widget_show(hw->IoPin[i].OutWidget);
              if (hw->IoPin[i].InBoxWidget)
                gtk_widget_hide(hw->IoPin[i].InBoxWidget);
              if (hw->IoPin[i].AInBoxWidget)
                gtk_widget_hide(hw->IoPin[i].AInBoxWidget);
              if (hw->IoPin[i].AOutBoxWidget)
                gtk_widget_hide(hw->IoPin[i].AOutBoxWidget);
              gtk_widget_hide(hw->IoPin[i].ConfigBoxWidget);
              gtk_widget_hide(hw->IoPin[i].DisableWidget);
              break;
              }
    case PIN_CFG_AN_OUT :
              {  // Analog Output
              if (hw->IoPin[i].OutWidget)
                gtk_widget_hide(hw->IoPin[i].OutWidget);
              if (hw->IoPin[i].InBoxWidget)
                gtk_widget_hide(hw->IoPin[i].InBoxWidget);
              if (hw->IoPin[i].AInBoxWidget)
                gtk_widget_hide(hw->IoPin[i].AInBoxWidget);
              if (hw->IoPin[i].AOutBoxWidget)
                gtk_widget_show(hw->IoPin[i].AOutBoxWidget);
              SetWidgetLabel(hw->IoPin[i].AOutNameWidget, hw->IoPin[i].Name);
              gtk_widget_hide(hw->IoPin[i].ConfigBoxWidget);
              gtk_widget_hide(hw->IoPin[i].DisableWidget);
              break;
              }
    case PIN_CFG_NOT_USED :  // Nicht verwendet
              {
              if (hw->IoPin[i].OutWidget)
                gtk_widget_hide(hw->IoPin[i].OutWidget);
              if (hw->IoPin[i].InBoxWidget)
                gtk_widget_hide(hw->IoPin[i].InBoxWidget);
              if (hw->IoPin[i].AInBoxWidget)
                gtk_widget_hide(hw->IoPin[i].AInBoxWidget);
              if (hw->IoPin[i].AOutBoxWidget)
                gtk_widget_hide(hw->IoPin[i].AOutBoxWidget);
              str = g_strdup_printf("<span weight=\"bold\" size=\"large\">%s - Disabled</span>", hw->IoPinDefs[i].CfgName);
              gtk_label_set_markup(GTK_LABEL(hw->IoPin[i].DisableWidget), str);
              g_free(str);
              gtk_widget_hide(hw->IoPin[i].ConfigBoxWidget);
              gtk_widget_show(hw->IoPin[i].DisableWidget);
              break;
              }
    case PIN_CFG_KEY :       // Taster
    case PIN_CFG_SWITCH :    // Schalter
              {
              if (hw->IoPin[i].OutWidget)
                gtk_widget_hide(hw->IoPin[i].OutWidget);
              if (hw->IoPin[i].InBoxWidget)
                gtk_widget_hide(hw->IoPin[i].InBoxWidget);
              if (hw->IoPin[i].AInBoxWidget)
                gtk_widget_hide(hw->IoPin[i].AInBoxWidget);
              if (hw->IoPin[i].AOutBoxWidget)
                gtk_widget_hide(hw->IoPin[i].AOutBoxWidget);
              str = g_strdup_printf("<span weight=\"bold\" size=\"large\">%s</span>", hw->IoPinDefs[i].KeyName);
              gtk_label_set_markup(GTK_LABEL(hw->IoPin[i].DisableWidget), str);
              g_free(str);
              gtk_widget_hide(hw->IoPin[i].ConfigBoxWidget);
              gtk_widget_show(hw->IoPin[i].DisableWidget);
              break;
              }          
    case PIN_CFG_AN_IN :     // Analog Input
    case PIN_CFG_ENCODER :   // Encoder
              {
              if (hw->IoPin[i].OutWidget)
                gtk_widget_hide(hw->IoPin[i].OutWidget);
              if (hw->IoPin[i].InBoxWidget)
                gtk_widget_hide(hw->IoPin[i].InBoxWidget);
              if (hw->IoPin[i].AOutBoxWidget)
                gtk_widget_hide(hw->IoPin[i].AOutBoxWidget);
              if (hw->IoPin[i].AInBoxWidget)
                gtk_widget_show(hw->IoPin[i].AInBoxWidget);
              SetWidgetLabel(hw->IoPin[i].AInNameWidget, hw->IoPin[i].Name);
              gtk_widget_hide(hw->IoPin[i].ConfigBoxWidget);
              gtk_widget_hide(hw->IoPin[i].DisableWidget);
              break;
              }
    case PIN_CFG_ENCODER+1 :   // Encoder - Pin B
              {
              if (hw->IoPin[i].OutWidget)
                gtk_widget_hide(hw->IoPin[i].OutWidget);
              if (hw->IoPin[i].InBoxWidget)
                gtk_widget_hide(hw->IoPin[i].InBoxWidget);
              if (hw->IoPin[i].AOutBoxWidget)
                gtk_widget_hide(hw->IoPin[i].AOutBoxWidget);
              if (hw->IoPin[i].AInBoxWidget)
                gtk_widget_hide(hw->IoPin[i].AInBoxWidget);
              str = g_strdup_printf("<span weight=\"bold\" size=\"large\">%s - %s</span>", hw->IoPinDefs[i].CfgName, hw->IoPinDefs[i].AnalogName);
              gtk_label_set_markup(GTK_LABEL(hw->IoPin[i].DisableWidget), str);
              g_free(str);
              gtk_widget_hide(hw->IoPin[i].ConfigBoxWidget);
              gtk_widget_show(hw->IoPin[i].DisableWidget);
              break;
              }
    default : {  // Config
              if (hw->IoPin[i].OutWidget)
                gtk_widget_hide(hw->IoPin[i].OutWidget);
              if (hw->IoPin[i].InBoxWidget)
                gtk_widget_hide(hw->IoPin[i].InBoxWidget);
              if (hw->IoPin[i].AInBoxWidget)
                gtk_widget_hide(hw->IoPin[i].AInBoxWidget);
              if (hw->IoPin[i].AOutBoxWidget)
                gtk_widget_hide(hw->IoPin[i].AOutBoxWidget);
              gtk_widget_show(hw->IoPin[i].ConfigBoxWidget);
              gtk_widget_hide(hw->IoPin[i].DisableWidget);
              break;
              }
    }
  }
}


void GetConfigFromGui(struct TCanHw *hw)
{
guint i;

for (i = 0; i < hw->IoPinsCount; i++)
  GetConfigForPin(hw, i);
}


void SetConfigToGui(struct TCanHw *hw)
{
unsigned int i, mode;

for (i = 0; i < hw->IoPinsCount; i++)
  {
  mode = hw->IoPin[i].Mode;
  gtk_combo_box_set_text_item(GTK_COMBO_BOX(hw->IoPin[i].ConfigWidget), ConfigComboStrings[mode]);
  }
}


void SetConfigComboBox(struct TCanHw *hw, GtkWidget *widget, guint pin_idx)
{
guint flags;

flags = hw->IoPinDefs[pin_idx].Flags;
gtk_combo_box_append_text(GTK_COMBO_BOX(widget), ConfigComboStrings[PIN_CFG_NOT_USED]);
if (flags & PIN_IN)
  gtk_combo_box_append_text(GTK_COMBO_BOX(widget), ConfigComboStrings[PIN_CFG_DIG_IN]);
if (flags & PIN_OUT)
  gtk_combo_box_append_text(GTK_COMBO_BOX(widget), ConfigComboStrings[PIN_CFG_DIG_OUT]);
if (flags & PIN_A_IN)
  gtk_combo_box_append_text(GTK_COMBO_BOX(widget), ConfigComboStrings[PIN_CFG_AN_IN]);
if (flags & PIN_A_OUT)
  gtk_combo_box_append_text(GTK_COMBO_BOX(widget), ConfigComboStrings[PIN_CFG_AN_OUT]);
if (flags & PIN_KEY)
  gtk_combo_box_append_text(GTK_COMBO_BOX(widget), ConfigComboStrings[PIN_CFG_KEY]);
if (flags & PIN_SW)
  gtk_combo_box_append_text(GTK_COMBO_BOX(widget), ConfigComboStrings[PIN_CFG_SWITCH]);
if (flags & PIN_ENC_A)
  gtk_combo_box_append_text(GTK_COMBO_BOX(widget), ConfigComboStrings[PIN_CFG_ENCODER]);
if (flags & PIN_ENC_B)
  gtk_combo_box_append_text(GTK_COMBO_BOX(widget), ConfigComboStrings[PIN_CFG_ENCODER+1]);
}


