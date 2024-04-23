/* *********** Tiny-CAN Monitor **************                            */
/* Copyright (C) 2007 Klaus Demlehner (klaus@mhs-elektronik.de)           */
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
/*                                                                        */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    :                      */
/*                                                                        */
/*  Version         : 1.00                                                */
/*  Datei Name      : can_widget.c                                        */
/* ---------------------------------------------------------------------- */
/*  Datum           : 19.10.07                                            */
/*  Author          : Demlehner Klaus                                     */
/* ---------------------------------------------------------------------- */
/*  Compiler        : GNU C Compiler                                      */
/**************************************************************************/
#include "global.h"
#include <ctype.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dialogs.h"
#include "util.h"
#include "can_types.h"
#include "gtk_util.h"
#include "hex_entry.h"
#include "can_widget.h"


// Hellrot
const GdkColor ENTRY_INVALID_COLOR = {/* pixel */ 0,
                                      /* red   */ 0xFFFF,
                                      /* green */ 0xAFFF,
                                      /* blue  */ 0xAFFF};

const GdkColor ENTRY_VALID_COLOR   = {/* pixel */ 0,
                                      /* red   */ 0xAFFF,
                                      /* green */ 0xFFFF,
                                      /* blue  */ 0xAFFF};
// Hellgelb
const GdkColor ENTRY_UNUSED_COLOR = {/* pixel */ 0,
                                      /* red   */ 0xF000,
                                      /* green */ 0xF000,
                                      /* blue  */ 0x5A00};




static const gchar FORMAT_EFF_RTR_STR[] = {"EFF/RTR"};
static const gchar FORMAT_EFF_STR[]     = {"EFF"};
static const gchar FORMAT_STD_RTR_STR[] = {"STD/RTR"};
static const gchar FORMAT_STD_STR[]     = {"STD"};


void CanTxSetShowing(struct TCanTxWidget *ctw);
void CanTxSetDataEntrys(struct TCanTxWidget *ctw);
void CanTxSetDisplayMessage(struct TCanTxWidget *ctw);
void CanTxSetEditModeEx(struct TCanTxWidget *ctw);
void EffCheckToggledCB(GtkToggleButton *togglebutton, gpointer user_data);
void RtrCheckToggledCB(GtkToggleButton *togglebutton, gpointer user_data);
void SendBtnClickedCB(GtkToolButton *toolbutton, gpointer user_data);
void EditBtnClickedCB(GtkButton *button, gpointer user_data);
void HexSpinBtnChangeValueCB(GtkSpinButton *spinbutton, gpointer user_data);
//void HexEntryEventCB(struct THexEntry *he, int event, unsigned long value, gpointer user_data);
void HexEntryEventCB0(struct THexEntry *he, int event, unsigned long value, gpointer user_data);
void HexEntryEventCB1(struct THexEntry *he, int event, unsigned long value, gpointer user_data);
void HexEntryEventCB2(struct THexEntry *he, int event, unsigned long value, gpointer user_data);
void HexEntryEventCB3(struct THexEntry *he, int event, unsigned long value, gpointer user_data);
void HexEntryEventCB4(struct THexEntry *he, int event, unsigned long value, gpointer user_data);
void HexEntryEventCB5(struct THexEntry *he, int event, unsigned long value, gpointer user_data);
void HexEntryEventCB6(struct THexEntry *he, int event, unsigned long value, gpointer user_data);
void HexEntryEventCB7(struct THexEntry *he, int event, unsigned long value, gpointer user_data);
void HexEntryEventCB8(struct THexEntry *he, int event, unsigned long value, gpointer user_data);



struct TCanTxWidget *CanTxWidgetNew(GtkWidget *parent, unsigned int flags, TCanTxWidgetEventCB event_proc)
{
int i;
unsigned long event_flags;
GtkWidget *hbox;
GtkWidget *vbox;
GtkWidget *base_vbox;
GtkWidget *label;
GtkWidget *table;
GtkWidget *tmp_image;
GtkWidget *alignment;
GtkWidget *vseparator;
GtkIconSize tmp_toolbar_icon_size;
struct TCanTxWidget *ctw;

ctw = (struct TCanTxWidget *)g_malloc0(sizeof(struct TCanTxWidget));
if(!ctw)
  return(NULL);
if (flags & CAN_TX_WDG_EDIT_MODE)
  ctw->EditMode = 1;
else
  ctw->EditMode = 0;
if (flags & CAN_TX_WDG_ENABLE_RETURN_SEND)
  ctw->EnableReturnSend = 1;
else
  ctw->EnableReturnSend = 0;
if (flags & CAN_TX_EDIT_FINISH_EVENT)
  {
  event_flags = EDIT_EVENT_RETURN | EDIT_EVENT_CHANGE_VALUE;
  ctw->EditFinishEvent = 1;
  }
else
  {
  event_flags = EDIT_EVENT_RETURN;
  ctw->EditFinishEvent = 0;
  }
memset(&ctw->CanMsg, 0, sizeof(struct TCanMsg));
ctw->EventProc = event_proc;

// Frame
ctw->BaseWdg = gtk_frame_new(NULL);
gtk_widget_show(ctw->BaseWdg);
gtk_box_pack_start(GTK_BOX(parent), ctw->BaseWdg, FALSE, FALSE, 0);
gtk_frame_set_shadow_type(GTK_FRAME(ctw->BaseWdg), GTK_SHADOW_IN);
// VBox
base_vbox = gtk_vbox_new(FALSE, 0);
gtk_widget_show(base_vbox);
gtk_container_add(GTK_CONTAINER(ctw->BaseWdg), base_vbox);

if (flags & CAN_TX_WDG_DISPLAY_ENABLE)
  {
  // HBox
  hbox = gtk_hbox_new(FALSE, 5);
  ctw->MainDisplayFrame = hbox;
  gtk_widget_show(hbox);
  gtk_box_pack_start(GTK_BOX(base_vbox), hbox, FALSE, FALSE, 0);
  // Edit Button
  ctw->EditBtn = gtk_button_new();
  gtk_widget_show(ctw->EditBtn);
  gtk_box_pack_start(GTK_BOX(hbox), ctw->EditBtn, FALSE, FALSE, 0);
  gtk_button_set_relief(GTK_BUTTON(ctw->EditBtn), GTK_RELIEF_NONE);

  tmp_image = gtk_image_new_from_stock("gtk-go-forward", GTK_ICON_SIZE_MENU);
  gtk_widget_show (tmp_image);
  gtk_container_add(GTK_CONTAINER(ctw->EditBtn), tmp_image);
  // BezeichnerLabel
  ctw->BezeichnerLabel = gtk_label_new("Message: ");
  gtk_widget_show(ctw->BezeichnerLabel);
  gtk_box_pack_start(GTK_BOX(hbox), ctw->BezeichnerLabel, FALSE, FALSE, 0);
  // Display HBox
  ctw->DisplayFrame = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(ctw->DisplayFrame);
  gtk_box_pack_start(GTK_BOX(hbox), ctw->DisplayFrame, TRUE, TRUE, 0);
  // Display Text
  ctw->DisplayLabel = gtk_label_new("");
  gtk_widget_show(ctw->DisplayLabel);
  gtk_box_pack_start(GTK_BOX(ctw->DisplayFrame), ctw->DisplayLabel, FALSE, FALSE, 0);
  gtk_label_set_use_markup(GTK_LABEL (ctw->DisplayLabel), TRUE);
  // Senden Button
  ctw->DispSendBtn = gtk_button_new();
  gtk_widget_show(ctw->DispSendBtn);
  gtk_box_pack_end(GTK_BOX(ctw->DisplayFrame), ctw->DispSendBtn, FALSE, FALSE, 0);

  alignment = gtk_alignment_new(0.5, 0.5, 0, 0);
  gtk_widget_show (alignment);
  gtk_container_add(GTK_CONTAINER(ctw->DispSendBtn), alignment);

  hbox = gtk_hbox_new(FALSE, 2);
  gtk_widget_show(hbox);
  gtk_container_add(GTK_CONTAINER(alignment), hbox);

  tmp_image = gtk_image_new_from_stock("gtk-redo", GTK_ICON_SIZE_MENU);
  gtk_widget_show(tmp_image);
  gtk_box_pack_start(GTK_BOX(hbox), tmp_image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic("Senden");
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
  }
else
  ctw->MainDisplayFrame = NULL;
// **** HBox erzeugen
hbox = gtk_hbox_new(FALSE, 2);
gtk_widget_show(hbox);
ctw->MainEditFrame = hbox;
gtk_box_pack_start(GTK_BOX(base_vbox), hbox, FALSE, FALSE, 0);
// **** Tabelle für RTR & EFF Check Box
table = gtk_table_new(2, 2, FALSE);
gtk_widget_show(table);
gtk_box_pack_start(GTK_BOX(hbox), table, FALSE, FALSE, 0);
gtk_table_set_col_spacings(GTK_TABLE(table), 4);
// ****  RTR Check Box
ctw->RtrCheckBtn = gtk_check_button_new();
gtk_widget_show(ctw->RtrCheckBtn);
gtk_table_attach(GTK_TABLE(table), ctw->RtrCheckBtn, 0, 1, 1, 2, (GtkAttachOptions) (0),
                          (GtkAttachOptions) (0), 0, 0);
label = gtk_label_new("<span size=\"small\"><b>RTR</b></span>");
gtk_widget_show(label);
gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);

label = gtk_label_new ("<span size=\"small\"><b>EFF</b></span>");
gtk_widget_show(label);
gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);

ctw->EffCheckBtn = gtk_check_button_new();
gtk_widget_show(ctw->EffCheckBtn);
gtk_table_attach (GTK_TABLE (table), ctw->EffCheckBtn, 1, 2, 1, 2, (GtkAttachOptions) (0),
                  (GtkAttachOptions) (0), 0, 0);
/*
// **** VBox für EFF & RTR Check Button
vbox = gtk_vbox_new(FALSE, 0);
gtk_widget_show(vbox);
gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
// **** EFF Check Box
ctw->EffCheckBtn = gtk_check_button_new_with_mnemonic("EFF");
gtk_widget_show(ctw->EffCheckBtn);
gtk_box_pack_start(GTK_BOX(vbox), ctw->EffCheckBtn, FALSE, FALSE, 0);
// ****  RTR Check Box
ctw->RtrCheckBtn = gtk_check_button_new_with_mnemonic("RTR");
gtk_widget_show(ctw->RtrCheckBtn);
gtk_box_pack_start(GTK_BOX(vbox), ctw->RtrCheckBtn, FALSE, FALSE, 0);
*/
// **** Id Label
vbox = gtk_vbox_new(FALSE, 0);
gtk_widget_show(vbox);
gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
ctw->IdLabel = gtk_label_new("<span size=\"small\"><b>Id:</b></span>");
gtk_label_set_use_markup(GTK_LABEL(ctw->IdLabel), TRUE);
gtk_widget_show(ctw->IdLabel);
gtk_box_pack_start(GTK_BOX(vbox), ctw->IdLabel, FALSE, FALSE, 0);
// **** Id Hex-Entry
ctw->IdEntry = HexEntryNew(vbox, 0, EDIT_SIZE_32_BIT, EDIT_MODE_HEX, EDIT_MASK_HEX | EDIT_MASK_DEZIMAL, event_flags, HexEntryEventCB0, ctw);
// **** Trennlinie
alignment = gtk_alignment_new(0.5, 0.5, 1, 1);
gtk_widget_show(alignment);
gtk_box_pack_start(GTK_BOX(hbox), alignment, FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(alignment), 3);
gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 3, 3, 0, 0);
vseparator = gtk_vseparator_new();
gtk_widget_show(vseparator);
gtk_container_add(GTK_CONTAINER(alignment), vseparator);
// **** Dlc Label
vbox = gtk_vbox_new(FALSE, 0);
gtk_widget_show(vbox);
gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
ctw->DlcLabel = gtk_label_new("<span size=\"small\"><b>DLC:</b></span>");
gtk_label_set_use_markup(GTK_LABEL(ctw->DlcLabel), TRUE);
gtk_widget_show(ctw->DlcLabel);
gtk_box_pack_start(GTK_BOX(vbox), ctw->DlcLabel, FALSE, FALSE, 0);
// **** Dlc Entry
ctw->DlcEntryAdj = gtk_adjustment_new(0, 0, 8, 1, 1, 0);
ctw->DlcEntry = gtk_spin_button_new(GTK_ADJUSTMENT(ctw->DlcEntryAdj), 1, 0);
gtk_widget_show(ctw->DlcEntry);
gtk_box_pack_start(GTK_BOX(vbox), ctw->DlcEntry, FALSE, FALSE, 0);
gtk_widget_set_size_request(ctw->DlcEntry, 35, -1);
gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(ctw->DlcEntry), TRUE);
gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(ctw->DlcEntry), GTK_UPDATE_IF_VALID);
// **** Trennlinie
alignment = gtk_alignment_new(0.5, 0.5, 1, 1);
gtk_widget_show(alignment);
gtk_box_pack_start(GTK_BOX(hbox), alignment, FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(alignment), 3);
gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 3, 3, 0, 0);
vseparator = gtk_vseparator_new();
gtk_widget_show(vseparator);
gtk_container_add(GTK_CONTAINER(alignment), vseparator);
// **** Can Daten Hex-Entry´s
// **** Daten Label
table = gtk_table_new(2, 8, FALSE);
gtk_widget_show (table);
gtk_box_pack_start(GTK_BOX(hbox), table, FALSE, FALSE, 0);
gtk_table_set_col_spacings(GTK_TABLE(table), 2);

label = gtk_label_new("<span size=\"small\">0</span>");
gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
gtk_widget_show (label);
gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
label = gtk_label_new("<span size=\"small\">7</span>");
gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
gtk_widget_show(label);
gtk_table_attach(GTK_TABLE(table), label, 7, 8, 0, 1, (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
ctw->DatenLabel = gtk_label_new("<span size=\"small\"><b>Daten</b></span>");
gtk_label_set_use_markup(GTK_LABEL(ctw->DatenLabel), TRUE);
gtk_widget_show(ctw->DatenLabel);
gtk_table_attach(GTK_TABLE(table), ctw->DatenLabel, 1, 7, 0, 1, (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

ctw->CanDataEntry[0] = HexEntryNew(NULL, 0, EDIT_SIZE_8_BIT, EDIT_MODE_HEX, EDIT_MASK_ALL, event_flags, HexEntryEventCB1, ctw);
ctw->CanDataEntry[1] = HexEntryNew(NULL, 0, EDIT_SIZE_8_BIT, EDIT_MODE_HEX, EDIT_MASK_ALL, event_flags, HexEntryEventCB2, ctw);
ctw->CanDataEntry[2] = HexEntryNew(NULL, 0, EDIT_SIZE_8_BIT, EDIT_MODE_HEX, EDIT_MASK_ALL, event_flags, HexEntryEventCB3, ctw);
ctw->CanDataEntry[3] = HexEntryNew(NULL, 0, EDIT_SIZE_8_BIT, EDIT_MODE_HEX, EDIT_MASK_ALL, event_flags, HexEntryEventCB4, ctw);
ctw->CanDataEntry[4] = HexEntryNew(NULL, 0, EDIT_SIZE_8_BIT, EDIT_MODE_HEX, EDIT_MASK_ALL, event_flags, HexEntryEventCB5, ctw);
ctw->CanDataEntry[5] = HexEntryNew(NULL, 0, EDIT_SIZE_8_BIT, EDIT_MODE_HEX, EDIT_MASK_ALL, event_flags, HexEntryEventCB6, ctw);
ctw->CanDataEntry[6] = HexEntryNew(NULL, 0, EDIT_SIZE_8_BIT, EDIT_MODE_HEX, EDIT_MASK_ALL, event_flags, HexEntryEventCB7, ctw);
ctw->CanDataEntry[7] = HexEntryNew(NULL, 0, EDIT_SIZE_8_BIT, EDIT_MODE_HEX, EDIT_MASK_ALL, event_flags, HexEntryEventCB8, ctw);
for (i = 0; i < 8; i++)
  {
  //ctw->CanDataEntry[i] = HexEntryNew(NULL, 0, EDIT_SIZE_8_BIT, EDIT_MODE_HEX, EDIT_MASK_ALL, event_flags, HexEntryEventCB, ctw);
  gtk_table_attach(GTK_TABLE(table), ctw->CanDataEntry[i]->BaseWdg, i, i+1, 1, 2, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  HexEntryCreateCursor(ctw->CanDataEntry[i]);
  }
if (flags & CAN_TX_WDG_END_SEPERATROR)
  {
  // **** Trennlinie
  alignment = gtk_alignment_new(0.5, 0.5, 1, 1);
  gtk_widget_show(alignment);
  gtk_box_pack_start(GTK_BOX(hbox), alignment, FALSE, FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(alignment), 3);
  gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 3, 3, 0, 0);
  vseparator = gtk_vseparator_new();
  gtk_widget_show(vseparator);
  gtk_container_add(GTK_CONTAINER(alignment), vseparator);
  }
if (flags & CAN_TX_WDG_SEND_TOOLBAR)
  {
  // **** Senden Toolbar
  ctw->Toolbar = gtk_toolbar_new();
  gtk_widget_show(ctw->Toolbar);
  gtk_box_pack_start(GTK_BOX(hbox), ctw->Toolbar, FALSE, FALSE, 0);
  gtk_toolbar_set_style(GTK_TOOLBAR(ctw->Toolbar), GTK_TOOLBAR_BOTH);
  gtk_toolbar_set_show_arrow(GTK_TOOLBAR(ctw->Toolbar), FALSE);

  if (flags & CAN_TX_WDG_SEND_BTN)
    {
    // **** Senden Button
    tmp_toolbar_icon_size = gtk_toolbar_get_icon_size(GTK_TOOLBAR(ctw->Toolbar));
    tmp_image = gtk_image_new_from_stock("gtk-redo", tmp_toolbar_icon_size);
    gtk_widget_show(tmp_image);
    ctw->SendBtn = (GtkWidget*)gtk_tool_button_new(tmp_image, "Senden");
    gtk_widget_show(ctw->SendBtn);
    gtk_container_add(GTK_CONTAINER(ctw->Toolbar), ctw->SendBtn);

    g_signal_connect((gpointer)ctw->SendBtn, "clicked", G_CALLBACK(SendBtnClickedCB), ctw);
    }
  }
else
  {
  if (flags & CAN_TX_WDG_SEND_BTN)
    {
    // Senden Button
    ctw->SendBtn = gtk_button_new();
    gtk_widget_show(ctw->SendBtn);
    gtk_box_pack_end(GTK_BOX(hbox), ctw->SendBtn, FALSE, FALSE, 0);

    alignment = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_widget_show (alignment);
    gtk_container_add(GTK_CONTAINER(ctw->SendBtn), alignment);

    hbox = gtk_hbox_new(FALSE, 2);
    gtk_widget_show(hbox);
    gtk_container_add(GTK_CONTAINER(alignment), hbox);

    tmp_image = gtk_image_new_from_stock("gtk-redo", GTK_ICON_SIZE_BUTTON);
    gtk_widget_show(tmp_image);
    gtk_box_pack_start(GTK_BOX(hbox), tmp_image, FALSE, FALSE, 0);

    label = gtk_label_new_with_mnemonic("Senden");
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);

    g_signal_connect((gpointer)ctw->SendBtn, "clicked", G_CALLBACK(SendBtnClickedCB), ctw);
    }
  }
ctw->EffCheckSig = g_signal_connect((gpointer)ctw->EffCheckBtn, "toggled", G_CALLBACK(EffCheckToggledCB), ctw);
ctw->RtrCheckSig = g_signal_connect((gpointer)ctw->RtrCheckBtn, "toggled", G_CALLBACK(RtrCheckToggledCB), ctw);
ctw->DlcEntrySig = g_signal_connect((gpointer)ctw->DlcEntry, "value_changed", G_CALLBACK(HexSpinBtnChangeValueCB), ctw);
if (flags & CAN_TX_WDG_DISPLAY_ENABLE)
  {
  g_signal_connect((gpointer)ctw->EditBtn, "clicked", G_CALLBACK(EditBtnClickedCB), ctw);
  g_signal_connect((gpointer)ctw->DispSendBtn, "clicked", G_CALLBACK(SendBtnClickedCB), ctw);
  }
CanTxSetShowing(ctw);
CanTxSetEditModeEx(ctw);
CanTxSetDataEntrys(ctw);
if ((flags & CAN_TX_WDG_DISPLAY_ENABLE) && (!ctw->EditMode))
  CanTxSetDisplayMessage(ctw);
return(ctw);
}



void CanTxWidgetDestroy(struct TCanTxWidget *ctw)
{
int i;

if (!ctw)
  return;
for (i = 0; i < 8; i++)
  {
  HexEntryDestroy(ctw->CanDataEntry[i]);
  ctw->CanDataEntry[i] = NULL;
  }
gtk_widget_destroy(ctw->BaseWdg);
/*if (ctw->MainDisplayFrame)
  gtk_widget_destroy(ctw->MainDisplayFrame);
gtk_widget_destroy(ctw->MainEditFrame);*/
g_free(ctw);
}


void CanTxWidgetSetMsg(struct TCanTxWidget *ctw, struct TCanMsg *can_msg)
{
int i;

if (!ctw)
  return;
if (can_msg)
  memcpy(&ctw->CanMsg, can_msg, sizeof(struct TCanMsg));
can_msg = &ctw->CanMsg;
if ((ctw->EditMode) || (!ctw->DisplayFrame))
  {
  g_signal_handler_block(ctw->EffCheckBtn, ctw->EffCheckSig);
  g_signal_handler_block(ctw->RtrCheckBtn, ctw->RtrCheckSig);
  g_signal_handler_block(ctw->DlcEntry, ctw->DlcEntrySig);
  // **** EFF
  if (can_msg->MsgEFF)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctw->EffCheckBtn), TRUE);
  else
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctw->EffCheckBtn), FALSE);
  // **** RTR
  if (can_msg->MsgRTR)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctw->RtrCheckBtn), TRUE);
  else
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctw->RtrCheckBtn), FALSE);
  CanTxSetShowing(ctw);
  // **** ID
  HexEntrySetValue(ctw->IdEntry, can_msg->Id);
  // **** DLC
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(ctw->DlcEntry), can_msg->MsgLen);

  g_signal_handler_unblock(ctw->EffCheckBtn, ctw->EffCheckSig);
  g_signal_handler_unblock(ctw->RtrCheckBtn, ctw->RtrCheckSig);
  g_signal_handler_unblock(ctw->DlcEntry, ctw->DlcEntrySig);
  // **** Daten
  for (i=0; i < 8; i++)
    {
    HexEntrySetValue(ctw->CanDataEntry[i], can_msg->MsgData[i]);
    }
  CanTxSetDataEntrys(ctw);
  }
else
  CanTxSetDisplayMessage(ctw);
}


void CanTxWidgetGetMsg(struct TCanTxWidget *ctw, struct TCanMsg *can_msg)
{
int i;
struct TCanMsg *hlp_msg;

if (!ctw)
  return;

hlp_msg = &ctw->CanMsg;
if (ctw->EditMode)
  {
  hlp_msg->Flags.Long = 0L;
  // **** EFF
  if (GTK_TOGGLE_BUTTON(ctw->EffCheckBtn)->active == TRUE)
    hlp_msg->MsgEFF = 1;
  // **** RTR
  if (GTK_TOGGLE_BUTTON(ctw->RtrCheckBtn)->active == TRUE)
    hlp_msg->MsgRTR = 1;
  // **** ID
  hlp_msg->Id = HexEntryGetValue(ctw->IdEntry);
  // **** DLC
  gtk_spin_button_update(GTK_SPIN_BUTTON(ctw->DlcEntry));
  hlp_msg->MsgLen = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ctw->DlcEntry));
  // **** Daten
  for (i=0; i < 8; i++)
    {
    hlp_msg->MsgData[i] = (unsigned char)HexEntryGetValue(ctw->CanDataEntry[i]);
    }
  }
if (can_msg)
  memcpy(can_msg, hlp_msg, sizeof(struct TCanMsg));
}



void CanTxSetEditMode(struct TCanTxWidget *ctw, int edit_mode)
{
if (!ctw)
  return;
if ((edit_mode == ctw->EditMode) || (!ctw->DisplayFrame))
  return;
if (ctw->EditMode)
  CanTxWidgetGetMsg(ctw, NULL);
ctw->EditMode = edit_mode;
CanTxWidgetSetMsg(ctw, NULL);
CanTxSetEditModeEx(ctw);
}


void CanTxSetReturnAutoSend(struct TCanTxWidget *ctw, int mode)
{
if (!ctw)
  return;
ctw->EnableReturnSend = mode;
}


void CanTxSetShowing(struct TCanTxWidget *ctw)
{
gboolean sensitive;
int i;

if(!ctw)
  return;
if (GTK_TOGGLE_BUTTON(ctw->RtrCheckBtn)->active == TRUE)
  sensitive = FALSE;
else
  sensitive = TRUE;
for (i = 0; i < 8; i++)
  gtk_widget_set_sensitive(ctw->CanDataEntry[i]->BaseWdg, sensitive);
if (GTK_TOGGLE_BUTTON(ctw->EffCheckBtn)->active == TRUE)
  HexEntrySetValueWidth(ctw->IdEntry, EDIT_SIZE_32_BIT);
else
  HexEntrySetValueWidth(ctw->IdEntry, EDIT_SIZE_12_BIT);
}


void CanTxSetDataEntrys(struct TCanTxWidget *ctw)
{
int dlc, i;

if (!ctw)
  return;
// **** DLC
gtk_spin_button_update(GTK_SPIN_BUTTON(ctw->DlcEntry));
dlc = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ctw->DlcEntry));
for (i = 0; i < 8; i++)
  {
  if (i < dlc)
    HexEntrySetColor(ctw->CanDataEntry[i], NULL, (GdkColor *)&ENTRY_VALID_COLOR);
  else
    HexEntrySetColor(ctw->CanDataEntry[i], NULL, (GdkColor *)&ENTRY_UNUSED_COLOR);
  }
}


void CanTxSetDisplayMessage(struct TCanTxWidget *ctw)
{
struct TCanMsg *msg;
int cnt, i;
char *hex_ptr;
char *type_str;
char id_str[9];
char dlc_str[2];
char data_str[25];
char buf[200];

if (!ctw)
  return;
if (!ctw->DisplayFrame)
  return;
msg = &ctw->CanMsg;
if ((msg->MsgRTR) && (msg->MsgEFF))
  type_str = (char *)FORMAT_EFF_RTR_STR;
else if (msg->MsgEFF)
  type_str = (char *)FORMAT_EFF_STR;
else if (msg->MsgRTR)
  type_str = (char *)FORMAT_STD_RTR_STR;
else
  type_str = (char *)FORMAT_STD_STR;
if (msg->MsgEFF)
  g_snprintf(id_str, 9, "%08X", (unsigned int)msg->Id);
else
  g_snprintf(id_str, 9, "%03X", (unsigned int)msg->Id);
cnt = msg->MsgLen;
g_snprintf(dlc_str, 2, "%1u", cnt);
if (msg->MsgRTR)
  cnt = 0;
if (cnt)
  {
  i = 0;
  hex_ptr = data_str;
  for (; cnt; cnt--)
    {
    g_snprintf(hex_ptr, 4, "%02X ", msg->MsgData[i++]);
    hex_ptr += 3;
    }
  *hex_ptr = '\0';
  }
else
  strcpy(data_str, "keine Daten");
if (strlen(ctw->CanMakroName))
  {
  g_snprintf(buf, 200, "<b>%s</b> <span font_desc=\"Courier Medium 10\">(<b>%s</b>  Id:<b>%s</b>  Dlc:<b>%s</b>  Daten:<b>%s</b>)</span>  ",
    ctw->CanMakroName, type_str, id_str, dlc_str, data_str);
  gtk_label_set_markup(GTK_LABEL(ctw->DisplayLabel), buf);
  }
else
  {
  g_snprintf(buf, 200, "<span font_desc=\"Courier Medium 10\"><b>%s</b>  Id:<b>%s</b>  Dlc:<b>%s</b>  Daten:<b>%s</b></span>  ",
    type_str, id_str, dlc_str, data_str);
  gtk_label_set_markup(GTK_LABEL(ctw->DisplayLabel), buf);

  }
}


void CanTxSetEditModeEx(struct TCanTxWidget *ctw)
{
GtkWidget *tmp_image;

if (!ctw)
  return;
if ((ctw->DisplayFrame) && (ctw->MainEditFrame))
  {
  if (ctw->EditMode)
    {
    tmp_image = gtk_image_new_from_stock("gtk-go-down", GTK_ICON_SIZE_MENU);
    gtk_widget_hide(ctw->DisplayFrame);
    gtk_widget_show(ctw->MainEditFrame);
    }
  else
    {
    tmp_image = gtk_image_new_from_stock("gtk-go-forward", GTK_ICON_SIZE_MENU);
    gtk_widget_show(ctw->DisplayFrame);
    gtk_widget_hide(ctw->MainEditFrame);
    }
  gtk_button_set_image(GTK_BUTTON(ctw->EditBtn), tmp_image);
  }
}


void EffCheckToggledCB(GtkToggleButton *togglebutton, gpointer user_data)
{
struct TCanTxWidget *ctw;

ctw = (struct TCanTxWidget *)user_data;
if(!ctw)
  return;
CanTxSetShowing(ctw);
if ((ctw->EventProc) && (ctw->EditFinishEvent))
  {
  // **** EFF
  if (GTK_TOGGLE_BUTTON(ctw->EffCheckBtn)->active == FALSE)
    ctw->CanMsg.MsgEFF = 0;
  else
    ctw->CanMsg.MsgEFF = 1;
  (ctw->EventProc)(ctw, CAN_TX_ENTRY_EDIT_FINISH, &ctw->CanMsg);
  }
}


void RtrCheckToggledCB(GtkToggleButton *togglebutton, gpointer user_data)
{
struct TCanTxWidget *ctw;

ctw = (struct TCanTxWidget *)user_data;
if(!ctw)
  return;
CanTxSetShowing(ctw);
if ((ctw->EventProc) && (ctw->EditFinishEvent))
  {
  // **** RTR
  if (GTK_TOGGLE_BUTTON(ctw->RtrCheckBtn)->active == FALSE)
    ctw->CanMsg.MsgRTR = 0;
  else
    ctw->CanMsg.MsgRTR = 1;
  (ctw->EventProc)(ctw, CAN_TX_ENTRY_EDIT_FINISH, &ctw->CanMsg);
  }
}


void SendBtnClickedCB(GtkToolButton *toolbutton, gpointer user_data)
{
struct TCanTxWidget *ctw;

ctw = (struct TCanTxWidget *)user_data;
if(!ctw)
  return;
if (ctw->EventProc)
  {
  CanTxWidgetGetMsg(ctw, NULL);
  if (ctw->EditFinishEvent)
    (ctw->EventProc)(ctw, CAN_TX_ENTRY_EDIT_FINISH, &ctw->CanMsg);
  (ctw->EventProc)(ctw, CAN_TX_ENTRY_SEND_BTN, &ctw->CanMsg);
  }
}


void EditBtnClickedCB(GtkButton *button, gpointer user_data)
{
struct TCanTxWidget *ctw;

ctw = (struct TCanTxWidget *)user_data;
if (!ctw)
  return;
if (ctw->EditMode)
  {
  CanTxWidgetGetMsg(ctw, NULL);
  CanTxSetDisplayMessage(ctw);
  ctw->EditMode = 0;
  }
else
  {
  CanTxWidgetSetMsg(ctw, NULL);
  ctw->EditMode = 1;
  }
CanTxSetDataEntrys(ctw);
CanTxSetEditModeEx(ctw);
}


void HexSpinBtnChangeValueCB(GtkSpinButton *spinbutton, gpointer user_data)
{
struct TCanTxWidget *ctw;

ctw = (struct TCanTxWidget *)user_data;
if (!ctw)
  return;
CanTxSetDataEntrys(ctw);
if ((ctw->EventProc) && (ctw->EditFinishEvent))
  {
  //CanTxWidgetGetMsg(ctw, NULL);
  // gtk_spin_button_update(GTK_SPIN_BUTTON(ctw->DlcEntry));
  ctw->CanMsg.MsgLen = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ctw->DlcEntry));
  (ctw->EventProc)(ctw, CAN_TX_ENTRY_EDIT_FINISH, &ctw->CanMsg);
  }
}



/* void HexEntryEventCB(struct THexEntry *he, int event, unsigned long value, gpointer user_data)
{
struct TCanTxWidget *ctw;

ctw = (struct TCanTxWidget *)user_data;
if (!ctw)
  return;
if (ctw->EventProc)
  {
  CanTxWidgetGetMsg(ctw, NULL);
  if ((event & EDIT_EVENT_FOUCUS_OUT) && (ctw->EditFinishEvent))
    (ctw->EventProc)(ctw, CAN_TX_ENTRY_EDIT_FINISH, &ctw->CanMsg);
  if ((event & EDIT_EVENT_RETURN) && (ctw->EnableReturnSend))
    (ctw->EventProc)(ctw, CAN_TX_ENTRY_RETURN, &ctw->CanMsg);
  }
} */


void HexEntryEventCB(int event, unsigned long value, gpointer user_data, int index)
{
struct TCanTxWidget *ctw;

ctw = (struct TCanTxWidget *)user_data;
if (!ctw)
  return;
if (event & EDIT_EVENT_CHANGE_VALUE)
  {
  if (!index)
    ctw->CanMsg.Id = value;
  else if (index < 9)
    ctw->CanMsg.MsgData[index-1] = (unsigned char)value;
  }
if (ctw->EventProc)
  {
  if ((event & EDIT_EVENT_CHANGE_VALUE) && (ctw->EditFinishEvent))
    (ctw->EventProc)(ctw, CAN_TX_ENTRY_EDIT_FINISH, &ctw->CanMsg);
  if ((event & EDIT_EVENT_RETURN) && (ctw->EnableReturnSend))
    {
    if (!(event & EDIT_EVENT_CHANGE_VALUE))
      CanTxWidgetGetMsg(ctw, NULL);
    (ctw->EventProc)(ctw, CAN_TX_ENTRY_RETURN, &ctw->CanMsg);
    }
  }
}



void HexEntryEventCB0(struct THexEntry *he, int event, unsigned long value, gpointer user_data)
{
HexEntryEventCB(event, value, user_data, 0);
}


void HexEntryEventCB1(struct THexEntry *he, int event, unsigned long value, gpointer user_data)
{
HexEntryEventCB(event, value, user_data, 1);
}


void HexEntryEventCB2(struct THexEntry *he, int event, unsigned long value, gpointer user_data)
{
HexEntryEventCB(event, value, user_data, 2);
}


void HexEntryEventCB3(struct THexEntry *he, int event, unsigned long value, gpointer user_data)
{
HexEntryEventCB(event, value, user_data, 3);
}


void HexEntryEventCB4(struct THexEntry *he, int event, unsigned long value, gpointer user_data)
{
HexEntryEventCB(event, value, user_data, 4);
}


void HexEntryEventCB5(struct THexEntry *he, int event, unsigned long value, gpointer user_data)
{
HexEntryEventCB(event, value, user_data, 5);
}


void HexEntryEventCB6(struct THexEntry *he, int event, unsigned long value, gpointer user_data)
{
HexEntryEventCB(event, value, user_data, 6);
}


void HexEntryEventCB7(struct THexEntry *he, int event, unsigned long value, gpointer user_data)
{
HexEntryEventCB(event, value, user_data, 7);
}


void HexEntryEventCB8(struct THexEntry *he, int event, unsigned long value, gpointer user_data)
{
HexEntryEventCB(event, value, user_data, 8);
}


