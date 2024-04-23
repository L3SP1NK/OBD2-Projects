/***************************************************************************
                       can_speed_win.c  -  description
                             -------------------
    begin             : 13.09.2008
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
#include "can_monitor.h"
#include "support.h"
#include "file_sel.h"
#include "paths.h"
#include "main.h"
#include "main_can.h"
#include "can_speed_db.h"
#include "can_speed_win.h"



struct TCanSpeedTab
  {
  unsigned long Speed;
  char *Name;
  };


static const struct TCanSpeedTab CanSpeedTab[] = {{10,   "10 kBit/s"},
                                                  {20,   "20 kBit/s"},
                                                  {50,   "50 kBit/s"},
                                                  {100,  "100 kBit/s"},
                                                  {125,  "125 kBit/s"},
                                                  {250,  "250 kBit/s"},
                                                  {500,  "500 kBit/s"},
                                                  {800,  "800 kBit/s"},
                                                  {1000, "1 MBit/s"},
                                                  {0,     NULL}};      // Ende der Liste


void gtk_combo_box_clear(GtkComboBox *combo_box)
{
GtkListStore *store;

store = GTK_LIST_STORE(gtk_combo_box_get_model(combo_box));
gtk_list_store_clear(store);
}


static void CanSpeedWinReadFromUi(struct TCanSpeedWin *can_speed_win)
{
int i, idx;
struct TCanSpeedList *list;

if (!can_speed_win)
  return;
idx = gtk_combo_box_get_active(GTK_COMBO_BOX(can_speed_win->ComboBox));
if (idx < 0)
  return;
if (!can_speed_win->Mode)
  can_speed_win->CanSpeed = CanSpeedTab[idx].Speed;
else
  {
  i = 0;
  for (list = UserCanSpeedList; list; list = list->Next)
    {
    if (i == idx)
      break;
    i++;
    }
  if (list)
    can_speed_win->UserCanSpeed = list->Value;
  }
}


static void CanSpeedWinSetStandart(struct TCanSpeedWin *can_speed_win)
{
const struct TCanSpeedTab *l;
char *s;
int idx, i;

CanSpeedWinReadFromUi(can_speed_win);
idx = 0;
i = 0;
can_speed_win->Mode = 0;
gtk_combo_box_clear(GTK_COMBO_BOX(can_speed_win->ComboBox));
for (l = CanSpeedTab; (s = l->Name); l++)
  {
  gtk_combo_box_append_text(GTK_COMBO_BOX(can_speed_win->ComboBox), s);
  if (l->Speed == can_speed_win->CanSpeed)
    idx = i;
  i++;
  }
gtk_combo_box_set_active(GTK_COMBO_BOX(can_speed_win->ComboBox), idx);
}


static void CanSpeedWinSetUser(struct TCanSpeedWin *can_speed_win)
{
struct TCanSpeedList *list;
int idx, i;

CanSpeedWinReadFromUi(can_speed_win);
idx = 0;
i = 0;
can_speed_win->Mode = 1;
gtk_combo_box_clear(GTK_COMBO_BOX(can_speed_win->ComboBox));
for (list = UserCanSpeedList; list; list = list->Next)
  {
  gtk_combo_box_append_text(GTK_COMBO_BOX(can_speed_win->ComboBox), list->Description);
  if (list->Value == can_speed_win->UserCanSpeed)
    idx = i;
  i++;
  }
gtk_combo_box_set_active(GTK_COMBO_BOX(can_speed_win->ComboBox), idx);
}


static void UserSpeedCheckCB(GtkCheckButton *button, gpointer user_data)
{
struct TCanSpeedWin *can_speed_win;

can_speed_win = (struct TCanSpeedWin *)user_data;
if (GTK_TOGGLE_BUTTON(button)->active == TRUE)
  CanSpeedWinSetUser(can_speed_win);
else
  CanSpeedWinSetStandart(can_speed_win);
}


static void CanOpModeSet(struct TCanSpeedWin *can_speed_win, unsigned int mode)
{
if (mode > 2)
  mode = 2;
gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(can_speed_win->DatenListe[mode]), TRUE);
}


static void ModulInitCB(gulong signal, gpointer event_data, gpointer user_data)
{
struct TCanSpeedWin *can_speed_win;

if (!user_data)
  return;
can_speed_win = (struct TCanSpeedWin *)user_data;
// Automatic Retransmission disable
if (CanModul.CanFeaturesFlags & CAN_FEATURE_ARD)
  gtk_widget_set_sensitive(can_speed_win->DatenListe[1], TRUE);
else
  gtk_widget_set_sensitive(can_speed_win->DatenListe[1], FALSE);
// Silent Mode (LOM = Listen only Mode)
if (CanModul.CanFeaturesFlags & CAN_FEATURE_LOM)
  gtk_widget_set_sensitive(can_speed_win->DatenListe[2], TRUE);
else
  gtk_widget_set_sensitive(can_speed_win->DatenListe[2], FALSE);
}


struct TCanSpeedWin *CanSpeedWinNew(struct TCanSpeed *can_speed)
{
struct TCanSpeedWin *can_speed_win;
GtkWidget *widget;
GtkWidget *vbox;
GtkWidget *frame;

can_speed_win = (struct TCanSpeedWin *)g_malloc0(sizeof(struct TCanSpeedWin));
if (!can_speed_win)
  return(NULL);

can_speed_win->Mode = can_speed->Mode;
can_speed_win->CanSpeed = can_speed->CanSpeed;
can_speed_win->UserCanSpeed = can_speed->UserCanSpeed;

can_speed_win->Base = gtk_vbox_new(FALSE, 0);
// **** Frame CAN Speed
frame = gtk_frame_new (NULL);
gtk_box_pack_start(GTK_BOX(can_speed_win->Base), frame, FALSE, FALSE, 0);
gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
gtk_frame_set_label_align (GTK_FRAME (frame), 0.05, 0.5);

widget = gtk_label_new (_("CAN transfer rate"));
gtk_frame_set_label_widget (GTK_FRAME (frame), widget);

widget = gtk_alignment_new (0.5, 0.5, 1, 1);
gtk_container_add (GTK_CONTAINER (frame), widget);
gtk_alignment_set_padding (GTK_ALIGNMENT (widget), 4, 4, 7, 4);

vbox = gtk_vbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(widget), vbox);
// **** Check Button User Speeds
if (UserCanSpeedList)
  {
  widget = gtk_check_button_new_with_mnemonic (_("User CAN speeds"));
  if (can_speed_win->Mode)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
  else
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), FALSE);
  gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
  (void)g_signal_connect(widget, "toggled", G_CALLBACK(UserSpeedCheckCB), (gpointer)can_speed_win);
  }
// **** Can Speed Combo
can_speed_win->ComboBox = gtk_combo_box_new_text ();
gtk_box_pack_start(GTK_BOX(vbox), can_speed_win->ComboBox, FALSE, FALSE, 0);
if (can_speed_win->Mode)
  CanSpeedWinSetUser(can_speed_win);
else
  CanSpeedWinSetStandart(can_speed_win);

frame = gtk_frame_new (NULL);
gtk_box_pack_start(GTK_BOX(can_speed_win->Base), frame, FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
gtk_frame_set_label_align(GTK_FRAME(frame), 0.05, 0.5);

widget = gtk_label_new(_("CAN operating mode"));
gtk_frame_set_label_widget(GTK_FRAME(frame), widget);
gtk_label_set_use_markup(GTK_LABEL(widget), TRUE);

widget = gtk_alignment_new (0.5, 0.5, 1, 1);
gtk_container_add(GTK_CONTAINER (frame), widget);
gtk_alignment_set_padding(GTK_ALIGNMENT(widget), 0, 0, 7, 0);

vbox = gtk_vbox_new(FALSE, 0);
gtk_container_add (GTK_CONTAINER (widget), vbox);

can_speed_win->DatenListe[0] = gtk_radio_button_new_with_mnemonic (NULL, _("Normal"));
gtk_box_pack_start (GTK_BOX (vbox), can_speed_win->DatenListe[0], FALSE, FALSE, 0);
gtk_radio_button_set_group (GTK_RADIO_BUTTON (can_speed_win->DatenListe[0]), can_speed_win->DatenListe_group);
can_speed_win->DatenListe_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (can_speed_win->DatenListe[0]));

can_speed_win->DatenListe[1] = gtk_radio_button_new_with_mnemonic (NULL, _("Automatic Retransmission disable"));
gtk_box_pack_start (GTK_BOX (vbox), can_speed_win->DatenListe[1], FALSE, FALSE, 0);
gtk_radio_button_set_group (GTK_RADIO_BUTTON (can_speed_win->DatenListe[1]), can_speed_win->DatenListe_group);
can_speed_win->DatenListe_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (can_speed_win->DatenListe[1]));

can_speed_win->DatenListe[2] = gtk_radio_button_new_with_mnemonic (NULL, _("Silent Mode (LOM = Listen only Mode)"));
gtk_box_pack_start (GTK_BOX (vbox), can_speed_win->DatenListe[2], FALSE, FALSE, 0);
gtk_radio_button_set_group (GTK_RADIO_BUTTON (can_speed_win->DatenListe[2]), can_speed_win->DatenListe_group);
can_speed_win->DatenListe_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (can_speed_win->DatenListe[2]));

CanOpModeSet(can_speed_win, can_speed->CanOpMode);

gtk_widget_show_all(can_speed_win->Base);
ModulInitCB(SIG_CAN_MODUL_INIT, NULL, can_speed_win);
mhs_signal_connect(NULL, SIGC_PUBLIC, SIG_CAN_MODUL_INIT, ModulInitCB, can_speed_win);
return(can_speed_win);
}


void CanSpeedWinGet(struct TCanSpeedWin *can_speed_win, struct TCanSpeed *can_speed)
{
int i;

CanSpeedWinReadFromUi(can_speed_win);
can_speed->Mode = can_speed_win->Mode;
can_speed->CanSpeed = can_speed_win->CanSpeed;
can_speed->UserCanSpeed = can_speed_win->UserCanSpeed;

for (i = 0; i < 3; i++)
  {
  if (GTK_TOGGLE_BUTTON(can_speed_win->DatenListe[i])->active == TRUE)
    {
    can_speed->CanOpMode = i;
    break;
    }
  }
if (i == 3)
  can_speed->CanOpMode = 0;
}


void CanSpeedWinDestroy(struct TCanSpeedWin *can_speed_win)
{
mhs_signal_disconnect(SIGC_PUBLIC, SIG_CAN_MODUL_INIT, ModulInitCB);
if (can_speed_win)
  g_free(can_speed_win);
}
