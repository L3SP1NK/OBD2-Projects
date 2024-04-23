/***************************************************************************
                         tcan_mx_gui.c  -  description
                             -------------------
    begin             : 13.07.2017
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
#include "can_drv.h"
#include "dialogs.h"
#include "giwled.h"
#include "paths.h"
#include "gtk_util.h"
#include "std_op.h"
#include "gui.h"
#include "gui_help.h"
#include "hw_types.h"
#include "io.h"
#include "tcan_m2.h"
#include "tcan_m3.h"
#include "tcan_mx_gui.h"



/* <*>
// Foreground colors are set using Pango markup
#if GTK_CHECK_VERSION(3,0,0)
static GdkRGBA rgba_value_bg = { 0.094, 0.360, 0.792, 1.0 };
#else
static GdkColor value_bg = { 0, 0x1818, 0x5c5c, 0xcaca };
#endif */

/******************************************************************************/
/*                         Callback-Funktionen                                */
/******************************************************************************/
static void OptionChangeCB(GtkButton *button, gpointer user_data)
{
struct TCanHw *hw;
unsigned int index;

if (!(hw = (struct TCanHw *)g_object_get_data(G_OBJECT(button), "can_hw")))
  return;
index = (unsigned int)user_data;
GetConfigForPin(hw, index);
StdOpDlgExecute(hw, index);
}


static void OutChangeProc(GtkCheckButton *button, gpointer user_data)
{
struct TCanHw *hw;
guint index, pin, state;

if (!(hw = (struct TCanHw *)g_object_get_data(G_OBJECT(button), "can_hw")))
  return;
index = (int)user_data;
g_assert(index < hw->IoPinsCount);
pin = hw->IoPinDefs[index].Pin;
if ((!(hw->IoPinDefs[index].Flags & PIN_OUT)) || (hw->IoPin[index].Mode != PIN_CFG_DIG_OUT))
  return;
if (GTK_TOGGLE_BUTTON(button)->active == TRUE)
  state = 1;
else
  state = 0;
SetOutPin(hw, pin, state);
}


static void AOutChangeProc(GtkSpinButton *spinbutton, gpointer user_data)
{
struct TCanHw *hw;
uint16_t value;
guint index, pin;

if (!(hw = (struct TCanHw *)g_object_get_data(G_OBJECT(spinbutton), "can_hw")))
  return;
index = (int)user_data;
g_assert(index < hw->IoPinsCount);
pin = hw->IoPinDefs[index].Pin;
if ((!(hw->IoPinDefs[index].Flags & PIN_A_OUT)) || (hw->IoPin[index].Mode != PIN_CFG_AN_OUT))
  return;
value = (uint16_t)gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbutton));
SetAnalogOutPin(hw, pin, value);
}


static void CanGuiDestroy(GtkWidget *widget, gpointer data)
{
struct TCanHw *hw;
(void)widget;

hw = (struct TCanHw *)data;
TCanM2Destroy(hw);
}


/******************************************************************************/
/*                             Frame erzeugen                                 */
/******************************************************************************/
GtkWidget *TCanMxGuiCreate(guint hardware_type)
{
struct TCanHw *hw;
guint i;
gchar *filename, *str;
unsigned int z, s;
GtkWidget *eb, *table_wdg, *widget, *widget1, *widget2, *image1, *vbox2;
GtkObject *spinbutton_adj;

filename = NULL;
if (hardware_type == HARDWARE_TYPE_TCAN_M2)
  {
  if (!(hw = TCanM2Create()))
    return(NULL);
  filename = CreateFileName(Paths.base_dir, "tcanm2-conn.png");
  }
else if (hardware_type == HARDWARE_TYPE_TCAN_M3)
  {
  if (!(hw = TCanM3Create()))
    return(NULL);
  filename = CreateFileName(Paths.base_dir, "tcanm3-conn.png");
  }
else
  return(NULL);
table_wdg = gtk_table_new (hw->TableRows, hw->TableColumns, FALSE);
//gtk_widget_show(table_wdg);
hw->BaseWdg = table_wdg;
g_object_set_data(G_OBJECT(table_wdg), "can_hw", hw);
g_signal_connect(G_OBJECT(table_wdg), "destroy", G_CALLBACK(CanGuiDestroy), hw);
gtk_table_set_row_spacings(GTK_TABLE(table_wdg), 2);

image1 = gtk_image_new_from_file(filename);
g_free(filename);
//gtk_widget_show (image1);
gtk_table_attach (GTK_TABLE(table_wdg), image1, 2, 3, 0, 15,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
gtk_misc_set_padding (GTK_MISC(image1), 4, 4);

for (i = 0; i < hw->IoPinsCount; i++)
  {
  z = hw->IoPinDefs[i].Zeile;
  s = hw->IoPinDefs[i].Spalte;
  vbox2 = gtk_vbox_new(FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(vbox2), 4);
  /* colorize vbox */
  eb = gtk_event_box_new();
  gtk_container_add(GTK_CONTAINER(eb), vbox2);
  hw->IoPin[i].EventBox = eb;
#if GTK_CHECK_VERSION(3,0,0)
  gtk_widget_override_background_color(eb, GTK_STATE_FLAG_NORMAL, &rgba_value_bg); // <*>
#else
  gtk_widget_modify_bg(eb, GTK_STATE_NORMAL, GetColorByIndex(10));
#endif
  gtk_table_attach(GTK_TABLE(table_wdg), eb, s-1, s, z-1, z,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 4, 4);

  if (hw->IoPinDefs[i].Flags & PIN_IN)
    {
    // ** Input (Led)
    widget = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox2), widget, TRUE, TRUE, 0);
    hw->IoPin[i].InBoxWidget = widget;

    widget = giw_led_new();
    gtk_box_pack_start(GTK_BOX(hw->IoPin[i].InBoxWidget), widget, FALSE, FALSE, 0);
    hw->IoPin[i].InWidget = widget;
    widget = gtk_label_new(hw->IoPinDefs[i].InName);
    hw->IoPin[i].InNameWidget = widget;
    gtk_box_pack_start(GTK_BOX(hw->IoPin[i].InBoxWidget), widget, FALSE, FALSE, 0);
    }
  else
    {
    hw->IoPin[i].InBoxWidget = NULL;
    hw->IoPin[i].InWidget = NULL;
    hw->IoPin[i].InNameWidget = NULL;
    }
  if (hw->IoPinDefs[i].Flags & PIN_OUT)
    {
    // ** Output (Switch)
    widget = gtk_check_button_new_with_label(hw->IoPinDefs[i].OutName);
    g_object_set_data(G_OBJECT(widget), "can_hw", hw);
    (void)g_signal_connect(widget, "toggled", G_CALLBACK(OutChangeProc), (gpointer)i);
    hw->IoPin[i].OutWidget = widget;
    gtk_box_pack_start(GTK_BOX(vbox2), widget, FALSE, FALSE, 0);
    }
  else
    hw->IoPin[i].OutWidget = NULL;


  if (hw->IoPinDefs[i].Flags & (PIN_A_IN | PIN_ENC_A | PIN_ENC_B))
    {
    widget = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox2), widget, TRUE, TRUE, 0);
    // Frame
    widget1 = gtk_frame_new (NULL);
    gtk_box_pack_start(GTK_BOX(widget), widget1, FALSE, FALSE, 0);
    gtk_frame_set_shadow_type(GTK_FRAME(widget1), GTK_SHADOW_ETCHED_OUT);
    // Label
    widget2 = gtk_label_new(NULL);
    gtk_container_add(GTK_CONTAINER(widget1), widget2);
    gtk_label_set_use_markup(GTK_LABEL(widget2), TRUE);
    gtk_misc_set_alignment(GTK_MISC(widget2), 0, 0.5);
    gtk_label_set_width_chars(GTK_LABEL(widget2), 10);
    // Label
    widget1 = gtk_label_new(hw->IoPinDefs[i].AnalogName);
    gtk_box_pack_start(GTK_BOX(widget), widget1, FALSE, FALSE, 0);
    hw->IoPin[i].AInNameWidget = widget1;
    hw->IoPin[i].AInBoxWidget = widget;
    hw->IoPin[i].AInWidget = widget2;
    }
  else
    {
    hw->IoPin[i].AInBoxWidget = NULL;
    hw->IoPin[i].AInWidget = NULL;
    hw->IoPin[i].AInNameWidget = NULL;
    }
  if (hw->IoPinDefs[i].Flags & PIN_A_OUT)
    {
    widget = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox2), widget, TRUE, TRUE, 0);

    spinbutton_adj = gtk_adjustment_new(0, 0, 4095, 1, 1, 0);
    widget2 = gtk_spin_button_new(GTK_ADJUSTMENT(spinbutton_adj), 1, 0);
    g_object_set_data(G_OBJECT(widget2), "can_hw", hw);
    gtk_box_pack_start(GTK_BOX(widget), widget2, FALSE, FALSE, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(widget2), TRUE);
    gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(widget2), GTK_UPDATE_IF_VALID);
    (void)g_signal_connect((gpointer)widget2, "value_changed", G_CALLBACK(AOutChangeProc), (gpointer)i);
    (void)g_signal_connect((gpointer)widget2, "activate", G_CALLBACK(AOutChangeProc), (gpointer)i);

    widget1 = gtk_label_new (hw->IoPinDefs[i].AnalogName);
    gtk_box_pack_start (GTK_BOX(widget), widget1, FALSE, FALSE, 0);
    hw->IoPin[i].AOutNameWidget = widget1;
    hw->IoPin[i].AOutBoxWidget = widget;
    hw->IoPin[i].AOutWidget = widget2;
    }
  else
    {
    hw->IoPin[i].AOutBoxWidget = NULL;
    hw->IoPin[i].AOutWidget = NULL;
    hw->IoPin[i].AOutNameWidget = NULL;
    }
  // ** Text
  widget = gtk_label_new(NULL);
  gtk_box_pack_start(GTK_BOX(vbox2), widget, TRUE, TRUE, 0);
  str = g_strdup_printf("<span weight=\"bold\" size=\"large\">%s - Disabled</span>", hw->IoPinDefs[i].CfgName);
  gtk_label_set_markup(GTK_LABEL(widget), str);
  g_free(str);
  hw->IoPin[i].DisableWidget = widget;

  // ** Config
  widget = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox2), widget, TRUE, TRUE, 0);
  hw->IoPin[i].ConfigBoxWidget = widget;

  widget = gtk_combo_box_new_text();
  gtk_box_pack_start(GTK_BOX(hw->IoPin[i].ConfigBoxWidget), widget, TRUE, TRUE, 0);
  SetConfigComboBox(hw, widget, i);
  gtk_combo_box_set_active(GTK_COMBO_BOX(widget), 0);
  hw->IoPin[i].ConfigWidget = widget;

  widget = gtk_label_new(hw->IoPinDefs[i].CfgName);
  gtk_box_pack_start (GTK_BOX(hw->IoPin[i].ConfigBoxWidget), widget, FALSE, FALSE, 0);

  widget = gtk_button_new_with_label("...");
  g_object_set_data(G_OBJECT(widget), "can_hw", hw);
  gtk_box_pack_start (GTK_BOX(hw->IoPin[i].ConfigBoxWidget), widget, FALSE, FALSE, 0);
  (void)g_signal_connect(widget, "clicked", G_CALLBACK(OptionChangeCB), (gpointer)i);

  hw->IoPin[i].Mode = 0;
  }
gtk_widget_show_all(table_wdg);
SetGuiShowing(hw, 1);
return(table_wdg);
}

