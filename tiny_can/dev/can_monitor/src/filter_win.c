/* *********** Tiny-CAN Monitor **************                            */
/* Copyright (C) 2007 Klaus Demlehner (klaus@mhs-elektronik.de)           */
/*                                                                        */
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
#include "can_monitor.h"
#include "hex_entry.h"
#include "dialogs.h"
#include "filter.h"
#include "support.h"
#include "filter_win.h"


#define UP_BUTTON   0
#define DOWN_BUTTON 1

enum
  {
  FILTER_COLUMN_AKTIV = 0,
  FILTER_COLUMN_SHOW,
  FILTER_COLUMN_NAME,
  FILTER_COLUMN_AKTIV_EN,
  FILTER_COLUMN_SHOW_EN,
  FILTER_COLUMN_FILTER,
  FILTER_COLUMN_NUMBER
  };


/**************************************************************************/
/*                         V A R I A B L E N                              */
/**************************************************************************/
struct TFilterWin FilterWin;


/**************************************************************************/
/*                        F U N K T I O N E N                             */
/**************************************************************************/
void FlWinListSelectCB(GtkTreeSelection *sel, gpointer  user_data);
void FlWinNewBtnCB(GtkWidget *w, gpointer data);
void FlWinDeleteBtnCB(GtkWidget *w, gpointer data);
void FlWinArrowChangeCB(GtkWidget *w, gpointer user_data);
void FlWinListAktiveToggledCB(GtkCellRendererToggle *cell, gchar *pth, gpointer data);
void FlWinShowToggledCB(GtkCellRendererToggle *cell, gchar *path_str, gpointer data);
void FlWinNameEditChangedCB(GtkEditable *te, gpointer data);
void FlWinHardwareFilterChangeCB(GtkToggleButton *w, gpointer user_data);
void FlWinIdModeChangeCB(GtkComboBox *w, gpointer user_data);
void FlWinEffCheckCB(GtkToggleButton *w, gpointer user_data);

void FlWinSetDataToUi(struct TFilter *filter);
void FlWinGetDataFromUi(struct TFilter *filter);
void FlWinSetSensitivity(void);

void FlWinSetIdMode(unsigned int mode);
void FlWinSetHardwareFilter(unsigned int hardware_filter);
void FlWinSetEff(unsigned int eff);

void ShowFilterDlg(void);

/**************************************************************/
/* Fenster darstellen                                         */
/**************************************************************/
void FlWinExecute(void)
{
ShowFilterDlg();
FilterWin.Id1Edit = HexEntryNew(FilterWin.Id1EditBase, 0, EDIT_SIZE_12_BIT, EDIT_MODE_HEX, EDIT_MASK_HEX | EDIT_MASK_DEZIMAL, 0, NULL, NULL);
FilterWin.Id2Edit = HexEntryNew(FilterWin.Id2EditBase, 0, EDIT_SIZE_12_BIT, EDIT_MODE_HEX, EDIT_MASK_HEX | EDIT_MASK_DEZIMAL, 0, NULL, NULL);

FlWinCreateTable();

(void)g_signal_connect(FilterWin.NewBtn, "clicked", G_CALLBACK(FlWinNewBtnCB), NULL);
(void)g_signal_connect(FilterWin.DeleteBtn, "clicked", G_CALLBACK(FlWinDeleteBtnCB), NULL);
(void)g_signal_connect(FilterWin.UpBtn, "clicked", G_CALLBACK(FlWinArrowChangeCB), (gpointer)UP_BUTTON);
(void)g_signal_connect(FilterWin.DownBtn, "clicked", G_CALLBACK(FlWinArrowChangeCB), (gpointer)DOWN_BUTTON);
(void)g_signal_connect(FilterWin.NameEdit, "changed", G_CALLBACK(FlWinNameEditChangedCB), NULL);
(void)g_signal_connect(FilterWin.HardwareFilterEdit, "toggled", G_CALLBACK(FlWinHardwareFilterChangeCB), NULL);
(void)g_signal_connect(FilterWin.IdModeEdit, "changed", G_CALLBACK(FlWinIdModeChangeCB), NULL);
(void)g_signal_connect(FilterWin.EffEdit, "toggled", G_CALLBACK(FlWinEffCheckCB), NULL);

FlWinRepaintTable();
//gtk_window_set_modal(GTK_WINDOW(FilterWin.MainWin), 1);
//gtk_widget_show_all(FilterWin.MainWin);
gtk_dialog_run(GTK_DIALOG(FilterWin.MainWin));
FlWinGetDataFromUi(FilterWin.SelectedRow);
gtk_widget_destroy(FilterWin.MainWin);
FilEditingFinish();
}


/**************************************************************/
/*                                                            */
/**************************************************************/
void FlWinCreateTable(void)
{
GtkListStore *store;
GtkCellRenderer *renderer;
GtkTreeSelection *sel;

store = gtk_list_store_new(FILTER_COLUMN_NUMBER, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_STRING,
   G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_POINTER);
gtk_tree_view_set_model(GTK_TREE_VIEW(FilterWin.FlListView), GTK_TREE_MODEL(store));

gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(FilterWin.FlListView), TRUE);
gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(FilterWin.FlListView), FALSE);

renderer = gtk_cell_renderer_toggle_new();
g_signal_connect (G_OBJECT(renderer), "toggled", G_CALLBACK(FlWinListAktiveToggledCB), GTK_TREE_MODEL(store));
gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(FilterWin.FlListView), -1,
  _("active"), renderer, "active", FILTER_COLUMN_AKTIV, "activatable", FILTER_COLUMN_AKTIV_EN, NULL);

renderer = gtk_cell_renderer_toggle_new();
g_signal_connect (G_OBJECT(renderer), "toggled", G_CALLBACK(FlWinShowToggledCB), GTK_TREE_MODEL(store));
gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(FilterWin.FlListView), -1,
  _("show"), renderer, "active", FILTER_COLUMN_SHOW, "activatable", FILTER_COLUMN_SHOW_EN, NULL);

renderer = gtk_cell_renderer_text_new();
gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(FilterWin.FlListView), -1,
  _("filter-name"), renderer, "markup", FILTER_COLUMN_NAME, NULL);

sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(FilterWin.FlListView));
gtk_tree_selection_set_mode(sel, GTK_SELECTION_SINGLE);

g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(FlWinListSelectCB), NULL);

g_object_unref(G_OBJECT(store));
}


/**************************************************************/
/* Filter Tabelle neu Zeichnen                                */
/**************************************************************/
void FlWinRepaintTable(void)
{
GtkListStore *store;
struct TFilter *filter;
GtkTreeIter iter;
GtkTreeIter first_iter;
gint row;
GtkTreeSelection *sel;
char puf[40];
gboolean show, aktiv, edit_show, edit_aktiv;

row = 0;
store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(FilterWin.FlListView)));
sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(FilterWin.FlListView));
FilterWin.SelectedRow = FilGetFirst(); // Filter;
for (filter = FilGetFirst(); filter; filter = filter->Next)
  {
  gtk_list_store_append(store, &iter);
  if (filter->Flags & FILTER_SHOW)
    show = TRUE; else show = FALSE;
  if (filter->Flags & FILTER_ENABLED)
    aktiv = TRUE; else aktiv = FALSE;
  if (filter->Flags & PLUGIN_FILTER)
    {
    if (filter->Flags & PLUGIN_FILTER_EDIT_AKTIV)
      edit_aktiv = TRUE; else edit_aktiv = FALSE;
    if (filter->Flags & PLUGIN_FILTER_EDIT_SHOW)
      edit_show = TRUE; else edit_show = FALSE;
    g_snprintf(puf, 40, "<i>%s</i>", filter->Name);
    gtk_list_store_set(store, &iter, FILTER_COLUMN_AKTIV, aktiv, FILTER_COLUMN_SHOW, show ,
       FILTER_COLUMN_NAME, puf, FILTER_COLUMN_FILTER, filter, FILTER_COLUMN_AKTIV_EN, edit_aktiv, FILTER_COLUMN_SHOW_EN, edit_show, -1);
    }
  else
    gtk_list_store_set(store, &iter, FILTER_COLUMN_AKTIV, aktiv, FILTER_COLUMN_SHOW, show,
       FILTER_COLUMN_NAME, filter->Name, FILTER_COLUMN_FILTER, filter, FILTER_COLUMN_AKTIV_EN, TRUE, FILTER_COLUMN_SHOW_EN, TRUE, -1);
  if (!row++)
    first_iter = iter;
  }
// 1. Zeile markieren
if (row)
  gtk_tree_selection_select_iter(sel, &first_iter);
FlWinSetDataToUi(FilGetFirst());
FlWinSetSensitivity();
}


/**************************************************************/
/* Filter in Tabelle auswählen Callback                       */
/**************************************************************/
void FlWinListSelectCB(GtkTreeSelection *sel, gpointer  user_data)
{
struct TFilter *filter;
GtkTreeModel *model;
GtkTreeIter iter;

model = gtk_tree_view_get_model(GTK_TREE_VIEW(FilterWin.FlListView));
if (gtk_tree_selection_get_selected(sel, &model, &iter))
  {
  gtk_tree_model_get(model, &iter, FILTER_COLUMN_FILTER, &filter, -1);
  g_assert(filter != NULL);
  if ((FilterWin.SelectedRow != filter) && (FilterWin.SelectedRow))
    FlWinGetDataFromUi(FilterWin.SelectedRow);
  FilterWin.SelectedRow = filter;
  FlWinSetDataToUi(filter);

  gtk_entry_set_text(GTK_ENTRY(FilterWin.NameEdit), filter->Name);

  gtk_editable_select_region(GTK_EDITABLE(FilterWin.NameEdit), 0, -1);
  gtk_widget_grab_focus(FilterWin.NameEdit);
  }
else
  FilterWin.SelectedRow = NULL;
FlWinSetSensitivity();
}


/**************************************************************/
/* Filter Neu Callback                                        */
/**************************************************************/
void FlWinNewBtnCB(GtkWidget *w, gpointer data)
{
struct TFilter *filter;
GtkTreeModel *model;
GtkTreeIter iter;

if (FilterWin.SelectedRow)
  FlWinGetDataFromUi(FilterWin.SelectedRow);
filter = FilCreateNew();
if (!filter)
  {
  msg_box(MSG_TYPE_ERROR, _("Error"), _("Error creating new filter"));
  return;
  }
filter->HardwareFilter = 0;
filter->PassMessage = 0;
filter->IdMode = 0;
filter->FormatEff = 0;
filter->Id1 = 0;
filter->Id2 = 0;
filter->Flags = STANDART_FILTER | FILTER_SHOW | FILTER_ENABLED;
strcpy(filter->Name, _("unnamed"));
FilterWin.SelectedRow = filter;
model = gtk_tree_view_get_model(GTK_TREE_VIEW(FilterWin.FlListView));
gtk_list_store_append(GTK_LIST_STORE(model), &iter);
gtk_list_store_set(GTK_LIST_STORE(model), &iter,
  FILTER_COLUMN_AKTIV, TRUE,
  FILTER_COLUMN_AKTIV_EN, TRUE,
  FILTER_COLUMN_SHOW, TRUE,
  FILTER_COLUMN_SHOW_EN, TRUE,
  FILTER_COLUMN_NAME, filter->Name,
  FILTER_COLUMN_FILTER, filter,  -1);
gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(FilterWin.FlListView)), &iter);
}


/**************************************************************/
/* Filter löschen Button Callback-Funktion                    */
/**************************************************************/
void FlWinDeleteBtnCB(GtkWidget *w, gpointer data)
{
struct TFilter *filter;
GtkTreeSelection *sel;
GtkTreeModel *model;
GtkTreeIter iter;

sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(FilterWin.FlListView));
if (gtk_tree_selection_get_selected(sel, &model, &iter))
  {
  gtk_tree_model_get(model, &iter, FILTER_COLUMN_FILTER, &filter, -1);
  g_assert(filter != NULL);
  if (filter->Flags & PLUGIN_FILTER)
    return;
  FilRemove(filter);
  gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
  }
FilterWin.SelectedRow = NULL;
}


/**************************************************************/
/* Filter Up/Down Button Callback-Funktion                    */
/**************************************************************/
void FlWinArrowChangeCB(GtkWidget *w, gpointer user_data)
{
GtkTreeSelection *sel;
GtkTreeModel *model;
struct TFilter *filter;
GtkTreeIter iter;
GtkTreeIter iter_next;
GtkTreeIter prev_iter;
GtkTreePath *path;
int aktion;

aktion = (int)user_data;
sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(FilterWin.FlListView));
if (gtk_tree_selection_get_selected(sel, &model, &iter))
  {
  gtk_tree_model_get(model, &iter, FILTER_COLUMN_FILTER, &filter, -1);
  if (aktion == UP_BUTTON)
    {
    path = gtk_tree_model_get_path(model, &iter);
    if (gtk_tree_path_prev(path))
      {
      if (gtk_tree_model_get_iter(model, &prev_iter, path))
        {
        gtk_list_store_move_before(GTK_LIST_STORE(model), &iter, &prev_iter);
        FilMoveBefore(filter);
        }
      }
    gtk_tree_path_free(path);
    }
  else
    {
    iter_next = iter;
    if (gtk_tree_model_iter_next(model, &iter_next))
      {
      gtk_list_store_move_after(GTK_LIST_STORE(model), &iter, &iter_next);
      FilMoveAfter(filter);
      }
    }
  FilterWin.SelectedRow = filter;
  }
FlWinSetSensitivity();
}


/**************************************************************/
/* Filter "Aktiv" Toggle Callback-Funktion                    */
/**************************************************************/
void FlWinListAktiveToggledCB(GtkCellRendererToggle *cell, gchar *path_str, gpointer data)
{
struct TFilter *filter;
GtkTreeModel *model;
GtkTreeIter  iter;
GtkTreePath *path;
gboolean aktiv;

model = (GtkTreeModel *)data;
path = gtk_tree_path_new_from_string (path_str);

gtk_tree_model_get_iter(model, &iter, path);
gtk_tree_model_get(model, &iter, FILTER_COLUMN_FILTER, &filter, -1);
g_assert(filter != NULL);
if ((!(filter->Flags & PLUGIN_FILTER)) || (filter->Flags & PLUGIN_FILTER_EDIT_AKTIV))
  {
  aktiv = !gtk_cell_renderer_toggle_get_active(cell);
  if (aktiv)
    filter->Flags |= FILTER_ENABLED;
  else
    filter->Flags &= (~FILTER_ENABLED);
  gtk_list_store_set(GTK_LIST_STORE (model), &iter, FILTER_COLUMN_AKTIV, aktiv, -1);
  }
gtk_tree_path_free(path);
}


/**************************************************************/
/* Filter "Show" Toggle Callback-Funktion                     */
/**************************************************************/
void FlWinShowToggledCB(GtkCellRendererToggle *cell, gchar *path_str, gpointer data)
{
struct TFilter *filter;
GtkTreeModel *model;
GtkTreeIter  iter;
GtkTreePath *path;
gboolean show;

model = (GtkTreeModel *)data;
path = gtk_tree_path_new_from_string (path_str);

gtk_tree_model_get_iter(model, &iter, path);
gtk_tree_model_get(model, &iter, FILTER_COLUMN_FILTER, &filter, -1);
g_assert(filter != NULL);
if ((!(filter->Flags & PLUGIN_FILTER)) || (filter->Flags & PLUGIN_FILTER_EDIT_SHOW))
  {
  show = !gtk_cell_renderer_toggle_get_active(cell);
  if (show)
    filter->Flags |= FILTER_SHOW;
  else
    filter->Flags &= (~FILTER_SHOW);
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, FILTER_COLUMN_SHOW, show, -1);
  }
gtk_tree_path_free(path);
}


/**************************************************************/
/* Filter Name Edit Callback-Funktion                         */
/**************************************************************/
void FlWinNameEditChangedCB(GtkEditable *te, gpointer data)
{
struct TFilter *filter;
GtkTreeSelection *sel;
GtkTreeModel *model;
GtkTreeIter iter;

sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(FilterWin.FlListView));
if (gtk_tree_selection_get_selected(sel, &model, &iter))
  {
  gtk_tree_model_get(model, &iter, FILTER_COLUMN_FILTER, &filter, -1);
  g_assert(filter != NULL);
  if (filter->Flags & STANDART_FILTER)
    {
    g_strlcpy(filter->Name, gtk_entry_get_text(GTK_ENTRY(te)), 40);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, FILTER_COLUMN_NAME, filter->Name, -1);
    }
  }
}


/**************************************************************/
/* Widget Sensitive setzen                                    */
/**************************************************************/
void FlWinSetSensitivity(void)
{
gint up_sens, dn_sens;
gboolean enabled, edit_sens, hw_sens, pass_sens;

up_sens = FALSE;
dn_sens = FALSE;
enabled = FALSE;
hw_sens = FALSE;
pass_sens = FALSE;
edit_sens = FALSE;
if (FilterWin.SelectedRow)
  {
  if (FilterWin.SelectedRow->Flags & PLUGIN_FILTER)
    {
    if (FilterWin.SelectedRow->Flags & PLUGIN_FILTER_EDIT_HW)
      hw_sens = TRUE;
    if ((FilterWin.SelectedRow->Flags & PLUGIN_FILTER_EDIT_PASS) &&
        (!FilterWin.SelectedRow->HardwareFilter))
      pass_sens = TRUE;
    if (FilterWin.SelectedRow->Flags & PLUGIN_FILTER_EDIT_FILTER)
      edit_sens = TRUE;
    enabled = FALSE;
    }
  else
    {
    if (!FilterWin.SelectedRow->HardwareFilter)
      pass_sens = TRUE;
    hw_sens = TRUE;
    edit_sens = TRUE;
    enabled = TRUE;
    }
  if (FilterWin.SelectedRow != FilGetFirst())
    up_sens = TRUE;
  if (FilterWin.SelectedRow != FilGetLast())
    dn_sens = TRUE;
  if (gtk_combo_box_get_active(GTK_COMBO_BOX(FilterWin.IdModeEdit)) == 1)
    hw_sens = FALSE;
  }
else
  gtk_editable_delete_text(GTK_EDITABLE(FilterWin.NameEdit), 0, -1);
gtk_widget_set_sensitive(FilterWin.DeleteBtn, enabled);
gtk_widget_set_sensitive(FilterWin.NameEdit, enabled);
gtk_widget_set_sensitive(FilterWin.UpBtn, up_sens);
gtk_widget_set_sensitive(FilterWin.DownBtn, dn_sens);

gtk_widget_set_sensitive(FilterWin.HardwareFilterEdit, hw_sens);
gtk_widget_set_sensitive(FilterWin.PassMessageEdit, pass_sens);
gtk_widget_set_sensitive(FilterWin.IdModeEdit, edit_sens);
gtk_widget_set_sensitive(FilterWin.FilterFrame, edit_sens);
}


/**************************************************************/
/* Filter Daten auf Fenster ausgeben                          */
/**************************************************************/
void FlWinSetDataToUi(struct TFilter *filter)
{
if (!filter)
  {
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(FilterWin.HardwareFilterEdit), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(FilterWin.PassMessageEdit), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(FilterWin.EffEdit), FALSE);
  gtk_combo_box_set_active(GTK_COMBO_BOX(FilterWin.IdModeEdit), 0);
  HexEntrySetValue(FilterWin.Id1Edit, 0L);
  HexEntrySetValue(FilterWin.Id2Edit, 0L);
  }
else
  {
  if (filter->HardwareFilter)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(FilterWin.HardwareFilterEdit), TRUE);
  else
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(FilterWin.HardwareFilterEdit), FALSE);
  if (filter->PassMessage)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(FilterWin.PassMessageEdit), TRUE);
  else
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(FilterWin.PassMessageEdit), FALSE);
  if (filter->FormatEff)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(FilterWin.EffEdit), TRUE);
  else
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(FilterWin.EffEdit), FALSE);
  gtk_combo_box_set_active(GTK_COMBO_BOX(FilterWin.IdModeEdit), filter->IdMode);
  HexEntrySetValue(FilterWin.Id1Edit, filter->Id1);
  HexEntrySetValue(FilterWin.Id2Edit, filter->Id2);
  FlWinSetIdMode(filter->IdMode);
  FlWinSetHardwareFilter(filter->HardwareFilter);
  FlWinSetEff(filter->FormatEff);
  }
}


/**************************************************************/
/* Filter Daten vom Fenster einlesen                          */
/**************************************************************/
void FlWinGetDataFromUi(struct TFilter *filter)
{
if (!filter)
  return;
if (GTK_TOGGLE_BUTTON(FilterWin.HardwareFilterEdit)->active == TRUE)
  filter->HardwareFilter = 1;
else
  filter->HardwareFilter = 0;
if (GTK_TOGGLE_BUTTON(FilterWin.PassMessageEdit)->active == TRUE)
  filter->PassMessage = 1;
else
  filter->PassMessage = 0;
if (GTK_TOGGLE_BUTTON(FilterWin.EffEdit)->active == TRUE)
  filter->FormatEff = 1;
else
  filter->FormatEff = 0;
filter->IdMode = (unsigned char)gtk_combo_box_get_active(GTK_COMBO_BOX(FilterWin.IdModeEdit));
filter->Id1 = HexEntryGetValue(FilterWin.Id1Edit);
filter->Id2 = HexEntryGetValue(FilterWin.Id2Edit);
}


/**************************************************************/
/* Filter Id-Mode Edit Callback-Funktion                      */
/**************************************************************/
void FlWinIdModeChangeCB(GtkComboBox *w, gpointer user_data)
{
FlWinSetIdMode(gtk_combo_box_get_active(w));
}


/**************************************************************/
/* Filter "Hardware" Edit Callback-Funktion                   */
/**************************************************************/
void FlWinHardwareFilterChangeCB(GtkToggleButton *w, gpointer user_data)
{
if (w->active == TRUE)
  FlWinSetHardwareFilter(1);
else
  FlWinSetHardwareFilter(0);
}


/**************************************************************/
/* Filter Eff Edit Callback-Funktion                          */
/**************************************************************/
void FlWinEffCheckCB(GtkToggleButton *w, gpointer user_data)
{
if (w->active == TRUE)
  FlWinSetEff(1);
else
  FlWinSetEff(0);
}


void FlWinSetIdMode(unsigned int mode)
{
switch (mode)
  {
  case 0 : {  // Single
           gtk_label_set_text(GTK_LABEL(FilterWin.Id1Label), _("Id:"));
           gtk_label_set_text(GTK_LABEL(FilterWin.Id2Label), _("-:"));
           gtk_widget_set_sensitive(FilterWin.Id2Label, FALSE);
           gtk_widget_set_sensitive(FilterWin.Id2EditBase, FALSE);
           break;
           }
  case 1 : {  // Range
           gtk_label_set_text(GTK_LABEL(FilterWin.Id1Label), _("beginning:"));
           gtk_label_set_text(GTK_LABEL(FilterWin.Id2Label), _("end:"));
           gtk_widget_set_sensitive(FilterWin.Id2Label, TRUE);
           gtk_widget_set_sensitive(FilterWin.Id2EditBase, TRUE);
           gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(FilterWin.HardwareFilterEdit), FALSE);
           break;
           }
  case 2 : {  // Masked
           gtk_label_set_text(GTK_LABEL(FilterWin.Id1Label), _("Id:"));
           gtk_label_set_text(GTK_LABEL(FilterWin.Id2Label), _("maske:"));
           gtk_widget_set_sensitive(FilterWin.Id2Label, TRUE);
           gtk_widget_set_sensitive(FilterWin.Id2EditBase, TRUE);
           break;
           }
  }
FlWinSetSensitivity();
}


void FlWinSetHardwareFilter(unsigned int hardware_filter)
{
if (hardware_filter)
  gtk_widget_set_sensitive(FilterWin.PassMessageEdit, FALSE);
else
  gtk_widget_set_sensitive(FilterWin.PassMessageEdit, TRUE);
}


void FlWinSetEff(unsigned int eff)
{
if (eff)
  {
  HexEntrySetValueWidth(FilterWin.Id1Edit, EDIT_SIZE_32_BIT);
  HexEntrySetValueWidth(FilterWin.Id2Edit, EDIT_SIZE_32_BIT);
  }
else
  {
  HexEntrySetValueWidth(FilterWin.Id1Edit, EDIT_SIZE_12_BIT);
  HexEntrySetValueWidth(FilterWin.Id2Edit, EDIT_SIZE_12_BIT);
  }
}



void ShowFilterDlg(void)
{
GtkWidget *dialog_vbox1;
GtkWidget *widget;
GtkWidget *hbox;
GtkWidget *hbox2;
GtkWidget *vbox1;
GtkWidget *vbox2;
GtkWidget *scrolledwindow1;

FilterWin.MainWin = gtk_dialog_new ();
gtk_window_set_title (GTK_WINDOW (FilterWin.MainWin), _("Filter setup"));
gtk_window_set_position (GTK_WINDOW (FilterWin.MainWin), GTK_WIN_POS_CENTER);
gtk_window_set_type_hint (GTK_WINDOW (FilterWin.MainWin), GDK_WINDOW_TYPE_HINT_DIALOG);

dialog_vbox1 = GTK_DIALOG (FilterWin.MainWin)->vbox;
gtk_widget_show (dialog_vbox1);

vbox1 = gtk_vbox_new (FALSE, 3);
gtk_widget_show (vbox1);
gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);
gtk_container_set_border_width (GTK_CONTAINER (vbox1), 5);

hbox2 = gtk_hbox_new (FALSE, 3);
gtk_widget_show (hbox2);
gtk_box_pack_start (GTK_BOX (vbox1), hbox2, TRUE, TRUE, 0);

vbox2 = gtk_vbox_new (FALSE, 0);
gtk_widget_show (vbox2);
gtk_box_pack_start (GTK_BOX (hbox2), vbox2, FALSE, FALSE, 0);

FilterWin.UpBtn = gtk_button_new ();
gtk_widget_show (FilterWin.UpBtn);
gtk_box_pack_start (GTK_BOX (vbox2), FilterWin.UpBtn, FALSE, FALSE, 0);

widget = gtk_image_new_from_stock ("gtk-go-up", GTK_ICON_SIZE_BUTTON);
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (FilterWin.UpBtn), widget);

FilterWin.DownBtn = gtk_button_new ();
gtk_widget_show (FilterWin.DownBtn);
gtk_box_pack_end (GTK_BOX (vbox2), FilterWin.DownBtn, FALSE, FALSE, 0);

widget = gtk_image_new_from_stock ("gtk-go-down", GTK_ICON_SIZE_BUTTON);
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (FilterWin.DownBtn), widget);

scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
gtk_widget_show (scrolledwindow1);
gtk_box_pack_start (GTK_BOX (hbox2), scrolledwindow1, TRUE, TRUE, 0);
gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_SHADOW_IN);

FilterWin.FlListView = gtk_tree_view_new ();
gtk_widget_show (FilterWin.FlListView);
gtk_container_add (GTK_CONTAINER (scrolledwindow1), FilterWin.FlListView);
gtk_widget_set_size_request (FilterWin.FlListView, 200, 120);
gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (FilterWin.FlListView), FALSE);
gtk_tree_view_set_enable_search (GTK_TREE_VIEW (FilterWin.FlListView), FALSE);

widget = gtk_vbutton_box_new ();
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox2), widget, FALSE, FALSE, 0);

FilterWin.DeleteBtn = gtk_button_new_with_mnemonic (_("Delete"));
gtk_widget_show (FilterWin.DeleteBtn);
gtk_container_add (GTK_CONTAINER (widget), FilterWin.DeleteBtn);
GTK_WIDGET_SET_FLAGS (FilterWin.DeleteBtn, GTK_CAN_DEFAULT);

FilterWin.NewBtn = gtk_button_new_with_mnemonic (_("New"));
gtk_widget_show (FilterWin.NewBtn);
gtk_container_add (GTK_CONTAINER (widget), FilterWin.NewBtn);
GTK_WIDGET_SET_FLAGS (FilterWin.NewBtn, GTK_CAN_DEFAULT);

widget = gtk_hseparator_new ();
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (vbox1), widget, FALSE, FALSE, 2);

hbox = gtk_hbox_new (FALSE, 0);
gtk_widget_show (hbox);
gtk_box_pack_start (GTK_BOX (vbox1), hbox, FALSE, FALSE, 0);

widget = gtk_label_new (_("Name: "));
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

FilterWin.NameEdit = gtk_entry_new ();
gtk_widget_show (FilterWin.NameEdit);
gtk_box_pack_start (GTK_BOX (hbox), FilterWin.NameEdit, FALSE, FALSE, 0);
gtk_entry_set_width_chars (GTK_ENTRY (FilterWin.NameEdit), 40);

hbox = gtk_hbox_new (FALSE, 0);
gtk_widget_show (hbox);
gtk_box_pack_start (GTK_BOX (vbox1), hbox, FALSE, FALSE, 0);

FilterWin.HardwareFilterEdit = gtk_check_button_new_with_mnemonic (_("Hardware filter"));
gtk_widget_show (FilterWin.HardwareFilterEdit);
gtk_box_pack_start (GTK_BOX (hbox), FilterWin.HardwareFilterEdit, FALSE, FALSE, 0);

FilterWin.PassMessageEdit = gtk_check_button_new_with_mnemonic (_("Pass message"));
gtk_widget_show (FilterWin.PassMessageEdit);
gtk_box_pack_start (GTK_BOX (hbox), FilterWin.PassMessageEdit, FALSE, FALSE, 0);

hbox = gtk_hbox_new (FALSE, 0);
gtk_widget_show (hbox);
gtk_box_pack_start (GTK_BOX (vbox1), hbox, FALSE, FALSE, 0);

FilterWin.FilterFrame = gtk_frame_new (NULL);
gtk_widget_show (FilterWin.FilterFrame);
gtk_box_pack_start (GTK_BOX (hbox), FilterWin.FilterFrame, FALSE, FALSE, 0);

widget = gtk_alignment_new (0.5, 0.5, 1, 1);
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (FilterWin.FilterFrame), widget);
gtk_alignment_set_padding (GTK_ALIGNMENT (widget), 5, 5, 7, 5);

hbox = gtk_hbox_new (FALSE, 2);
gtk_widget_show (hbox);
gtk_container_add (GTK_CONTAINER (widget), hbox);

widget = gtk_label_new (_("Format:"));
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
gtk_misc_set_alignment (GTK_MISC (widget), 1, 0.5);

FilterWin.EffEdit = gtk_check_button_new_with_mnemonic (_("EFF"));
gtk_widget_show (FilterWin.EffEdit);
gtk_box_pack_start (GTK_BOX (hbox), FilterWin.EffEdit, FALSE, FALSE, 0);

FilterWin.Id1Label = gtk_label_new (_("Id:"));
gtk_widget_show (FilterWin.Id1Label);
gtk_box_pack_start (GTK_BOX (hbox), FilterWin.Id1Label, FALSE, FALSE, 0);
gtk_label_set_justify (GTK_LABEL (FilterWin.Id1Label), GTK_JUSTIFY_RIGHT);
gtk_misc_set_alignment (GTK_MISC (FilterWin.Id1Label), 1, 0.5);
gtk_label_set_width_chars (GTK_LABEL (FilterWin.Id1Label), 7);

FilterWin.Id1EditBase = gtk_vbox_new (FALSE, 0);
gtk_widget_show (FilterWin.Id1EditBase);
gtk_box_pack_start (GTK_BOX (hbox), FilterWin.Id1EditBase, TRUE, TRUE, 0);

FilterWin.Id2Label = gtk_label_new (_("-:"));
gtk_widget_show (FilterWin.Id2Label);
gtk_box_pack_start (GTK_BOX (hbox), FilterWin.Id2Label, FALSE, FALSE, 0);
gtk_label_set_justify (GTK_LABEL (FilterWin.Id2Label), GTK_JUSTIFY_RIGHT);
gtk_misc_set_alignment (GTK_MISC (FilterWin.Id2Label), 1, 0.5);
gtk_label_set_width_chars (GTK_LABEL (FilterWin.Id2Label), 7);

FilterWin.Id2EditBase = gtk_vbox_new (FALSE, 0);
gtk_widget_show (FilterWin.Id2EditBase);
gtk_box_pack_start (GTK_BOX (hbox), FilterWin.Id2EditBase, TRUE, TRUE, 0);

FilterWin.IdModeEdit = gtk_combo_box_new_text ();
gtk_widget_show (FilterWin.IdModeEdit);
gtk_frame_set_label_widget (GTK_FRAME (FilterWin.FilterFrame), FilterWin.IdModeEdit);
gtk_combo_box_append_text (GTK_COMBO_BOX (FilterWin.IdModeEdit), _("single"));
gtk_combo_box_append_text (GTK_COMBO_BOX (FilterWin.IdModeEdit), _("range"));
gtk_combo_box_append_text (GTK_COMBO_BOX (FilterWin.IdModeEdit), _("masked"));

widget = GTK_DIALOG (FilterWin.MainWin)->action_area;
gtk_widget_show (widget);
gtk_button_box_set_layout (GTK_BUTTON_BOX (widget), GTK_BUTTONBOX_END);

FilterWin.CloseButton = gtk_button_new ();
gtk_widget_show (FilterWin.CloseButton);
gtk_dialog_add_action_widget (GTK_DIALOG (FilterWin.MainWin), FilterWin.CloseButton, GTK_RESPONSE_CLOSE);
GTK_WIDGET_SET_FLAGS (FilterWin.CloseButton, GTK_CAN_DEFAULT);

widget = gtk_alignment_new (0.5, 0.5, 0, 0);
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (FilterWin.CloseButton), widget);

hbox = gtk_hbox_new (FALSE, 2);
gtk_widget_show (hbox);
gtk_container_add (GTK_CONTAINER (widget), hbox);

widget = gtk_image_new_from_stock ("gtk-quit", GTK_ICON_SIZE_BUTTON);
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

widget = gtk_label_new_with_mnemonic (_("_Close"));
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

gtk_widget_show (FilterWin.MainWin);
}
