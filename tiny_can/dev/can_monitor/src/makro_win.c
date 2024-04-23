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
#include "main_can.h"
#include "hex_entry.h"
#include "can_widget.h"
#include "main.h"
#include "dialogs.h"
#include "makro.h"
#include "support.h"
#include "makro_win.h"


#define UP_BUTTON   0
#define DOWN_BUTTON 1

const struct TCanMsg NullCanMsg = {.Id = 0L, .Flags.Long = 0L, .Data.Bytes = {0,0,0,0,0,0,0,0}, .Time = {0L, 0L}};
// const struct TCanMsg NullCanMsg = {/*.Id = */ 0L, /* .Flags.Long = */ 0L, /* .Data.Bytes = */ 0,0,0,0,0,0,0,0, /* .Time = */ 0L, 0L};

/**************************************************************************/
/*                         V A R I A B L E N                              */
/**************************************************************************/
struct TMakroWin *MakroWin = NULL;

/**************************************************************************/
/*                        F U N K T I O N E N                             */
/**************************************************************************/
void MakWinListSelectCB(GtkTreeSelection *sel, gpointer  user_data);
void MakWinShowToggledCB(GtkCellRendererToggle *cell, gchar *path_str, gpointer data);
void MakWinListSelectCB2(struct TMakroTableWidget *mtw, int event, struct TMakro *makro);
void MakWinMainEventCB(gulong signal, gpointer event_data, gpointer user_data);
void MakWinRepaintCB(gulong signal, gpointer event_data, gpointer user_data);
void MakWinSendBtnCB(GtkWidget *w, gpointer data);
void MakWinNewBtnCB(GtkWidget *w, gpointer data);
void MakWinDeleteBtnCB(GtkWidget *w, gpointer data);
void MakWinArrowChangeCB(GtkWidget *w, gpointer user_data);
void MakWinNameEditChangedCB(GtkEditable *te, gpointer data);
void MakWinCanTxWidgetCB(struct TCanTxWidget *ctw, int event, struct TCanMsg *can_msg);
void MakWinIntervallEnEditCB(GtkToggleButton *togglebutton, gpointer user_data);
void MakWinIntervallEditCB(GtkSpinButton *spinbutton, gpointer user_data);

void MakWinInsertBtnCB(GtkButton * button, gpointer user_data);
void MakWinSetIntervallBtnCB(GtkButton * button, gpointer user_data);
void MakWinCloseBtnCB(GtkButton * button, gpointer user_data);

void MakWinSetDataToUi(struct TMakro *makro);
void MakWinGetDataFromUi(struct TMakro *makro);
void MakWinSetSensitivity(void);

struct TMakroWin *ShowMakroDlg(void);


gint Makro_delete_event(GtkWidget *widget, GdkEvent event, gpointer daten)
{
MainVar.MakroWindow = 0;
SetMakroWindow();
return(TRUE);
}


/**************************************************************/
/* Fenster erzeugen                                           */
/**************************************************************/
struct TMakroWin *MakWinCreate(void)
{
struct TMakroWin *makro_win;

makro_win = ShowMakroDlg();

(void)g_signal_connect(makro_win->MainWin, "delete_event", G_CALLBACK(Makro_delete_event), NULL);
(void)g_signal_connect(makro_win->SendBtn, "clicked", G_CALLBACK(MakWinSendBtnCB), NULL);
(void)g_signal_connect(makro_win->NewBtn, "clicked", G_CALLBACK(MakWinNewBtnCB), (gpointer)0);
(void)g_signal_connect(makro_win->CopyBtn, "clicked", G_CALLBACK(MakWinNewBtnCB), (gpointer)1);
(void)g_signal_connect(makro_win->DeleteBtn, "clicked", G_CALLBACK(MakWinDeleteBtnCB), NULL);
(void)g_signal_connect(makro_win->UpBtn, "clicked", G_CALLBACK(MakWinArrowChangeCB), (gpointer)UP_BUTTON);
(void)g_signal_connect(makro_win->DownBtn, "clicked", G_CALLBACK(MakWinArrowChangeCB), (gpointer)DOWN_BUTTON);
(void)g_signal_connect(makro_win->NameEdit, "changed", G_CALLBACK(MakWinNameEditChangedCB), NULL);
(void)g_signal_connect(makro_win->IntervallEnableEdit, "toggled", G_CALLBACK(MakWinIntervallEnEditCB), NULL);
(void)g_signal_connect(makro_win->IntervallTimeEdit, "value_changed", G_CALLBACK(MakWinIntervallEditCB), NULL);
(void)g_signal_connect(makro_win->MakInsertBtn, "clicked", G_CALLBACK(MakWinInsertBtnCB), NULL);
(void)g_signal_connect(makro_win->MakSetIntervallBtn, "clicked", G_CALLBACK(MakWinSetIntervallBtnCB), NULL);
(void)g_signal_connect(makro_win->MakCloseBtn, "clicked", G_CALLBACK(MakWinCloseBtnCB), NULL);

makro_win->Show = 1;
mhs_signal_connect(NULL, SIGC_PUBLIC, SIG_CAN_STATUS_CHANGE, MakWinMainEventCB, NULL);
mhs_signal_connect(NULL, SIGC_PROTECTED, SIG_MAKRO_DLG_REPAINT, MakWinRepaintCB, NULL);
return(makro_win);
}


/**************************************************************/
/* Fenster löschen                                            */
/**************************************************************/
void MakWinDestroy(void)
{
if (MakroWin)
  {
  if (MakroWin->MainWin)
    gtk_widget_destroy(MakroWin->MainWin);
  CanTxWidgetDestroy(MakroWin->CanMessageWdg);
  g_free(MakroWin);
  MakroWin = NULL;
  }
mhs_signal_disconnect(SIGC_PUBLIC, SIG_CAN_STATUS_CHANGE, MakWinMainEventCB);
mhs_signal_disconnect(SIGC_PROTECTED, SIG_MAKRO_DLG_REPAINT, MakWinRepaintCB);
}


/**************************************************************/
/* Fenster anzeigen                                           */
/**************************************************************/
void MakWinShow(void)
{
if (!MakroWin)
  MakroWin = MakWinCreate();
else
  gtk_widget_show_all(MakroWin->MainWin);
MakroWin->Show = 1;
MakWinRepaintTable(MakroWin->MakListView);
MakWinSetDataToUi(Makro);
MakWinSetSensitivity();
}


/**************************************************************/
/* Fenster verbergen                                          */
/**************************************************************/
void MakWinHide(void)
{
if (MakroWin)
  gtk_widget_hide_all(MakroWin->MainWin);
MakroWin->Show = 0;
}


/**************************************************************/
/*                                                            */
/**************************************************************/
void MakWinListSelectCB(GtkTreeSelection *sel, gpointer user_data)
{
struct TMakroTableWidget *mtw;
struct TMakro *makro;
GtkTreeModel *model;
GtkTreeIter iter;

mtw = (struct TMakroTableWidget *)user_data;
if(!mtw)
  return;
model = gtk_tree_view_get_model(GTK_TREE_VIEW(mtw->Base));
if (gtk_tree_selection_get_selected(sel, &model, &iter))
  {
  gtk_tree_model_get(model, &iter, MAKRO_COLUMN_MAKRO, &makro, -1);
  g_assert(makro != NULL);
  /* if ((mtw->SelectedRow != makro) && (mtw->SelectedRow) && (mtw->EventProc))
    (mtw->EventProc)(mtw, 0, mtw->SelectedRow);    */
  mtw->SelectedRow = makro;
  if (mtw->EventProc)
    (mtw->EventProc)(mtw, 1, mtw->SelectedRow);
  }
else
  {
  mtw->SelectedRow = NULL;
  /* if (mtw->EventProc)
    (mtw->EventProc)(mtw, 1, mtw->SelectedRow); */
  }
}


/**************************************************************/
/* Filter "Show" Toggle Callback-Funktion                     */
/**************************************************************/
void MakWinShowToggledCB(GtkCellRendererToggle *cell, gchar *path_str, gpointer data)
{
struct TMakro *makro;
GtkTreeModel *model;
GtkTreeIter  iter;
GtkTreePath *path;
gboolean show;
struct TMakroTableWidget *mtw;

mtw = (struct TMakroTableWidget *)data;
model = gtk_tree_view_get_model(GTK_TREE_VIEW(mtw->Base));
path = gtk_tree_path_new_from_string (path_str);

gtk_tree_model_get_iter(model, &iter, path);
gtk_tree_model_get(model, &iter, MAKRO_COLUMN_MAKRO, &makro, -1);
g_assert(makro != NULL);
if ((!(makro->Flags & PLUGIN_MAKRO)) || (makro->Flags & PLUGIN_MAKRO_EDIT_SHOW))
  {
  show = !gtk_cell_renderer_toggle_get_active(cell);
  if (show)
    makro->Flags |= MAKRO_SHOW;
  else
    makro->Flags &= (~MAKRO_SHOW);
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, MAKRO_COLUMN_SHOW, show, -1);
  if (mtw->EventProc)
    (mtw->EventProc)(mtw, 3, makro);
  }
gtk_tree_path_free(path);
}


struct TMakroTableWidget *MakWinCreateTable(GtkWidget *parent, unsigned int flags, TMakroTableEventCB event_proc)
{
GtkListStore *store;
GtkCellRenderer *renderer;
GtkTreeViewColumn *column;
GtkTreeSelection *sel;
struct TMakroTableWidget *mtw;

if (!parent)
  return(NULL);
mtw = (struct TMakroTableWidget *)g_malloc0(sizeof(struct TMakroTableWidget));
if(!mtw)
  return(NULL);
mtw->Flags = flags;
mtw->Base = parent;
mtw->SelectedRow = NULL;
mtw->EventProc = event_proc;

store = gtk_list_store_new(MAKRO_COLUMN_NUMBER, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_POINTER);
gtk_tree_view_set_model(GTK_TREE_VIEW(mtw->Base), GTK_TREE_MODEL(store));

gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(mtw->Base), TRUE);
gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(mtw->Base), FALSE);
if (flags)
  {
  renderer = gtk_cell_renderer_toggle_new();
  g_signal_connect (G_OBJECT(renderer), "toggled", G_CALLBACK(MakWinShowToggledCB), mtw);
  column = gtk_tree_view_column_new_with_attributes(_("Show"), renderer,
                       "active", MAKRO_COLUMN_SHOW, "activatable", MAKRO_COLUMN_SHOW_EN, NULL);
  gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_append_column (GTK_TREE_VIEW(mtw->Base), column);
  }

renderer = gtk_cell_renderer_text_new();
column = gtk_tree_view_column_new_with_attributes(_("Macro name"), renderer, "markup", 1,
                                                  "text", MAKRO_COLUMN_NAME, NULL);
gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
gtk_tree_view_append_column(GTK_TREE_VIEW(mtw->Base), column);

sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(mtw->Base));
gtk_tree_selection_set_mode(sel, GTK_SELECTION_SINGLE);

g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(MakWinListSelectCB), mtw);

g_object_unref(G_OBJECT(store));
return(mtw);
}


void MakWinDestroyTable(struct TMakroTableWidget *mtw)
{
if (mtw)
  g_free(mtw);
}


/**************************************************************/
/* Makro Tabelle neu Zeichnen                                */
/**************************************************************/
void MakWinRepaintTable(struct TMakroTableWidget *mtw)
{
GtkListStore *store;
struct TMakro *makro;
GtkTreeIter iter;
GtkTreeIter first_iter;
gint row;
GtkTreeSelection *sel;
gchar puf[40];
gboolean show, edit_show;

if (!mtw)
  return;
row = 0;
store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(mtw->Base)));
gtk_list_store_clear(store);
sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(mtw->Base));
mtw->SelectedRow = Makro;
for (makro = Makro; makro; makro = makro->Next)
  {
  if (mtw->Flags)
    {
    gtk_list_store_append(store, &iter);
    if (makro->Flags & MAKRO_SHOW)
      show = TRUE; else show = FALSE;
    if (makro->Flags & PLUGIN_MAKRO)
      {
      if (makro->Flags & PLUGIN_MAKRO_EDIT_SHOW)
        edit_show = TRUE; else edit_show = FALSE;
      g_snprintf(puf, 40, "<i>%s</i>", makro->Name);
      gtk_list_store_set(store, &iter, MAKRO_COLUMN_SHOW, show, MAKRO_COLUMN_NAME, puf, MAKRO_COLUMN_MAKRO, makro, MAKRO_COLUMN_SHOW_EN, edit_show, -1);
      }
    else
      gtk_list_store_set(store, &iter, MAKRO_COLUMN_SHOW, show, MAKRO_COLUMN_NAME, makro->Name, MAKRO_COLUMN_MAKRO, makro, MAKRO_COLUMN_SHOW_EN, TRUE, -1);
    if (!row++)
      first_iter = iter;
    }
  else if (makro->Flags & MAKRO_SHOW)
    {
    gtk_list_store_append(store, &iter);
    if (makro->Flags & PLUGIN_MAKRO)
      {
      g_snprintf(puf, 40, "<i>%s</i>", makro->Name);
      gtk_list_store_set(store, &iter, MAKRO_COLUMN_NAME, puf, MAKRO_COLUMN_MAKRO, makro, -1);
      }
    else
      gtk_list_store_set(store, &iter, MAKRO_COLUMN_NAME, makro->Name, MAKRO_COLUMN_MAKRO, makro, -1);
    if (!row++)
      first_iter = iter;
    }
  }
// 1. Zeile markieren
if (row)
  gtk_tree_selection_select_iter(sel, &first_iter);
}


void MakSelectMakro(struct TMakroTableWidget *mtw, struct TMakro *makro)
{
GtkTreeModel *model;
GtkTreeIter iter;
GtkTreeSelection *sel;
struct TMakro *mak;

if ((!mtw) || (!makro))
  return;
model = gtk_tree_view_get_model(GTK_TREE_VIEW(mtw->Base));
sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(mtw->Base));
if (gtk_tree_model_get_iter_first(model, &iter))
  {
  do
    {
    gtk_tree_model_get(model, &iter, MAKRO_COLUMN_MAKRO, &mak, -1);
    if (mak != makro)
      continue;
    gtk_tree_selection_select_iter(sel, &iter);
    break;
    }
  while (gtk_tree_model_iter_next(model, &iter));
  }
}


void MakRepaintMakro(struct TMakroTableWidget *mtw, struct TMakro *makro)
{
GtkTreeModel *model;
GtkTreeIter iter;
struct TMakro *mak;
gchar puf[40];
gboolean show, edit_show;

if ((!mtw) || (!makro))
  return;
model = gtk_tree_view_get_model(GTK_TREE_VIEW(mtw->Base));
if (gtk_tree_model_get_iter_first(model, &iter))
  {
  do
    {
    gtk_tree_model_get(model, &iter, MAKRO_COLUMN_MAKRO, &mak, -1);
    if (mak != makro)
      continue;
    if (mtw->Flags)
      {
      if (makro->Flags & MAKRO_SHOW)
        show = TRUE; else show = FALSE;
      if (makro->Flags & PLUGIN_MAKRO)
        {
        if (makro->Flags & PLUGIN_MAKRO_EDIT_SHOW)
          edit_show = TRUE; else edit_show = FALSE;
        g_snprintf(puf, 40, "<i>%s</i>", makro->Name);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, MAKRO_COLUMN_SHOW, show, MAKRO_COLUMN_NAME, puf, MAKRO_COLUMN_SHOW_EN, edit_show, -1);
        }
      else
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, MAKRO_COLUMN_SHOW, show, MAKRO_COLUMN_NAME, makro->Name, MAKRO_COLUMN_SHOW_EN, TRUE, -1);
      }
    else if (makro->Flags & MAKRO_SHOW)
      {
      if (mak->Flags & PLUGIN_MAKRO)
        {
        g_snprintf(puf, 40, "<i>%s</i>", mak->Name);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, MAKRO_COLUMN_NAME, puf, -1);
        }
      else
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, MAKRO_COLUMN_NAME, mak->Name, -1);
      }
    break;
    }
  while (gtk_tree_model_iter_next(model, &iter));
  }
}


/**************************************************************/
/* Makro in Tabelle auswählen Callback                        */
/**************************************************************/
void MakWinListSelectCB2(struct TMakroTableWidget *mtw, int event, struct TMakro *makro)
{
if (!MakroWin)
  return;
if (makro)
  {
  /*if (!event)
    MakWinGetDataFromUi(makro);
  else */
  if (event == 3)
    {
    MakChangeShowing(makro);
    }
  else if (event)
    {
    MakWinSetDataToUi(makro);
    gtk_entry_set_text(GTK_ENTRY(MakroWin->NameEdit), makro->Name);
    gtk_editable_select_region(GTK_EDITABLE(MakroWin->NameEdit), 0, -1);
    gtk_widget_grab_focus(MakroWin->NameEdit);
    }
  }
MakWinSetSensitivity();
}


void MakWinMainEventCB(gulong signal, gpointer event_data, gpointer user_data)
{
MakWinSetSensitivity();
}


void MakWinRepaintCB(gulong signal, gpointer event_data, gpointer user_data)
{
if ((MakroWin) && (MakroWin->MakListView))
  MakWinRepaintTable(MakroWin->MakListView);
}

/**************************************************************/
/* Makro Senden-Button Callback                               */
/**************************************************************/
void MakWinSendBtnCB(GtkWidget *w, gpointer data)
{
struct TMakro *makro;
GtkTreeModel *model;
GtkTreeIter iter;
GtkTreeSelection *sel;

if (!MakroWin)
  return;
sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(MakroWin->MakListView->Base));
if (gtk_tree_selection_get_selected(sel, &model, &iter))
  {
  gtk_tree_model_get(model, &iter, MAKRO_COLUMN_MAKRO, &makro, -1);
  g_assert(makro != NULL);
  if (CanTransmit(0, &makro->CanMsg, 1) < 0)
    msg_box(MSG_TYPE_ERROR, _("Error"), _("Error transmit CAN message"));
  }
}


/**************************************************************/
/* Makro Neu Callback                                         */
/**************************************************************/
void MakWinNewBtnCB(GtkWidget *w, gpointer data)
{
struct TMakro *makro;
GtkTreeModel *model;
GtkTreeIter iter;
int aktion;

if (!MakroWin)
  return;
aktion = (int)data;
/* if (MakroWin->MakListView->SelectedRow)
  MakWinGetDataFromUi(MakroWin->MakListView->SelectedRow); */
makro = MakCreateNew();
if (!makro)
  {
  msg_box(MSG_TYPE_ERROR, _("Error"), _("Error create macro"));
  return;
  }
if ((aktion) && (MakroWin->MakListView->SelectedRow))
  {
  memcpy(&makro->CanMsg, &MakroWin->MakListView->SelectedRow->CanMsg, sizeof(struct TCanMsg));
  g_snprintf(makro->Name, 40, _("Copy from %s"), MakroWin->MakListView->SelectedRow->Name);
  }
else
  strcpy(makro->Name, _("unnamed")); // "Unbenannt"));
makro->Flags = STANDART_MAKRO | MAKRO_SHOW;
MakroWin->MakListView->SelectedRow = makro;
model = gtk_tree_view_get_model(GTK_TREE_VIEW(MakroWin->MakListView->Base));
gtk_list_store_append(GTK_LIST_STORE(model), &iter);
gtk_list_store_set(GTK_LIST_STORE(model), &iter, MAKRO_COLUMN_NAME, makro->Name, MAKRO_COLUMN_MAKRO, makro,
  MAKRO_COLUMN_SHOW, TRUE, MAKRO_COLUMN_SHOW_EN, TRUE, -1);
gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(MakroWin->MakListView->Base)), &iter);
MakCreateNewEnd(makro);
}


/**************************************************************/
/* Makro löschen Button Callback-Funktion                    */
/**************************************************************/
void MakWinDeleteBtnCB(GtkWidget *w, gpointer data)
{
struct TMakro *makro;
GtkTreeSelection *sel;
GtkTreeModel *model;
GtkTreeIter iter;

if (!MakroWin)
  return;
sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(MakroWin->MakListView->Base));
if (gtk_tree_selection_get_selected(sel, &model, &iter))
  {
  gtk_tree_model_get(model, &iter, MAKRO_COLUMN_MAKRO, &makro, -1);
  g_assert(makro != NULL);
  if (makro->Flags & PLUGIN_MAKRO)
    return;
  MakRemove(makro);
  gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
  }
MakroWin->MakListView->SelectedRow = NULL;
MakWinSetSensitivity();
}


/**************************************************************/
/* Makro Up/Down Button Callback-Funktion                    */
/**************************************************************/
void MakWinArrowChangeCB(GtkWidget *w, gpointer user_data)
{
GtkTreeSelection *sel;
GtkTreeModel *model;
struct TMakro *makro;
GtkTreeIter iter;
GtkTreeIter iter_next;
GtkTreeIter prev_iter;
GtkTreePath *path;
int aktion;

if (!MakroWin)
  return;
aktion = (int)user_data;
sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(MakroWin->MakListView->Base));
if (gtk_tree_selection_get_selected(sel, &model, &iter))
  {
  gtk_tree_model_get(model, &iter, MAKRO_COLUMN_MAKRO, &makro, -1);
  if (aktion == UP_BUTTON)
    {
    path = gtk_tree_model_get_path(model, &iter);
    if (gtk_tree_path_prev(path))
      {

      if (gtk_tree_model_get_iter(model, &prev_iter, path))
        {
        gtk_list_store_move_before(GTK_LIST_STORE(model), &iter, &prev_iter);
        MakMoveBefore(makro);
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
      MakMoveAfter(makro);
      }
    }
  MakroWin->MakListView->SelectedRow = makro;
  }
MakWinSetSensitivity();
}


/**************************************************************/
/* Makro Name Edit Callback-Funktion                         */
/**************************************************************/
void MakWinNameEditChangedCB(GtkEditable *te, gpointer data)
{
struct TMakro *makro;
GtkTreeSelection *sel;
GtkTreeModel *model;
GtkTreeIter iter;

if (!MakroWin)
  return;
sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(MakroWin->MakListView->Base));
if (gtk_tree_selection_get_selected(sel, &model, &iter))
  {
  gtk_tree_model_get(model, &iter, MAKRO_COLUMN_MAKRO, &makro, -1);
  g_assert(makro != NULL);
  if (makro->Flags & PLUGIN_MAKRO)
    return;
  g_strlcpy(makro->Name, gtk_entry_get_text(GTK_ENTRY(te)), 40);
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, MAKRO_COLUMN_NAME, makro->Name, -1);
  mhs_signal_emit(SIGC_PROTECTED, SIG_MAIN_CHANGE_MAKRO_NAME, makro);
  }
}


void MakWinCanTxWidgetCB(struct TCanTxWidget *ctw, int event, struct TCanMsg *can_msg)
{
struct TMakro *makro;


if (!MakroWin)
  return;
if ((makro = MakroWin->MakListView->SelectedRow))
  memcpy(&makro->CanMsg, can_msg, sizeof(struct TCanMsg));
}


void MakWinIntervallEnEditCB(GtkToggleButton *togglebutton, gpointer user_data)
{
struct TMakro *makro;

if (!MakroWin)
  return;
if ((makro = MakroWin->MakListView->SelectedRow))
  {
  if ((!(makro->Flags & PLUGIN_MAKRO)) || (makro->Flags & PLUGIN_MAKRO_EDIT_INT_EN))
    {
    if (GTK_TOGGLE_BUTTON(MakroWin->IntervallEnableEdit)->active == TRUE)
      makro->IntervallEnabled = 1;
    else
      makro->IntervallEnabled = 0;
    }
  }
}


void MakWinIntervallEditCB(GtkSpinButton *spinbutton, gpointer user_data)
{
struct TMakro *makro;

if (!MakroWin)
  return;
if ((makro = MakroWin->MakListView->SelectedRow))
  {
  if ((!(makro->Flags & PLUGIN_MAKRO)) || (makro->Flags & PLUGIN_MAKRO_EDIT_INT_TIME))
    {
    //gtk_spin_button_update(GTK_SPIN_BUTTON(MakroWin->IntervallTimeEdit));
    makro->IntervallTime = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(MakroWin->IntervallTimeEdit));
    }
  }
}


void MakWinInsertBtnCB(GtkButton * button, gpointer user_data)
{
struct TMakro *makro;

if (!MakroWin)
  return;
CanTxWidgetSetMsg(MakroWin->CanMessageWdg, &LastTransmit);
if ((makro = MakroWin->MakListView->SelectedRow))
  memcpy(&makro->CanMsg, &LastTransmit, sizeof(struct TCanMsg));
}


void MakWinSetIntervallBtnCB(GtkButton * button, gpointer user_data)
{
if (!MakroWin)
  return;
MakSetIntervall();
}


void MakWinCloseBtnCB(GtkButton * button, gpointer user_data)
{
if (!MakroWin)
  return;
MainVar.MakroWindow = 0;
SetMakroWindow();
}


/**************************************************************/
/* Widget Sensitive setzen                                    */
/**************************************************************/
void MakWinSetSensitivity(void)
{
struct TMakro *makro;
gint up_sens, dn_sens;
gboolean enabled, tx_enabled, int_en_sens, int_time_sens, msg_sens;

if (!MakroWin)
  return;
if (!MakroWin->Show)
  return;
up_sens = FALSE;
dn_sens = FALSE;
tx_enabled = FALSE;
int_en_sens = FALSE;
int_time_sens = FALSE;
msg_sens = FALSE;
if ((makro = MakroWin->MakListView->SelectedRow))
  {
  if (makro->Flags & PLUGIN_MAKRO)
    {
    if (makro->Flags & PLUGIN_MAKRO_EDIT_INT_EN)
      int_en_sens = TRUE;
    if (makro->Flags & PLUGIN_MAKRO_EDIT_INT_TIME)
      int_time_sens = TRUE;
    if (makro->Flags & PLUGIN_MAKRO_EDIT_MSG)
      msg_sens = TRUE;
    }
  else
    {
    int_en_sens = TRUE;
    int_time_sens = TRUE;
    msg_sens = TRUE;
    }
  enabled = TRUE;
  if (makro != MakGetFirst())
    up_sens = TRUE;
  if (makro != MakGetLast())
    dn_sens = TRUE;
  if (CanModul.DeviceStatus.DrvStatus >= DRV_STATUS_CAN_RUN)
    tx_enabled = TRUE;
  }
else
  {
  gtk_editable_delete_text(GTK_EDITABLE(MakroWin->NameEdit), 0, -1);
  CanTxWidgetSetMsg(MakroWin->CanMessageWdg, (struct TCanMsg *)&NullCanMsg);
  enabled = FALSE;
  }
gtk_widget_set_sensitive(MakroWin->SendBtn, tx_enabled);
gtk_widget_set_sensitive(MakroWin->CopyBtn, enabled);
gtk_widget_set_sensitive(MakroWin->DeleteBtn, enabled);
gtk_widget_set_sensitive(MakroWin->NameEdit, enabled);
//gtk_widget_set_sensitive(MakroWin->IntervallFrame, enabled);
gtk_widget_set_sensitive(MakroWin->UpBtn, up_sens);
gtk_widget_set_sensitive(MakroWin->DownBtn, dn_sens);
gtk_widget_set_sensitive(MakroWin->IntervallEnableEdit, int_en_sens);
gtk_widget_set_sensitive(MakroWin->IntervallTimeEdit, int_time_sens);
gtk_widget_set_sensitive(MakroWin->CanMessageWdg->BaseWdg, msg_sens);
}


/**************************************************************/
/* Makro Daten auf Fenster ausgeben                          */
/**************************************************************/
void MakWinSetDataToUi(struct TMakro *makro)
{
if (!MakroWin)
  return;
if (!makro)
  {
  gtk_editable_delete_text(GTK_EDITABLE(MakroWin->NameEdit), 0, -1);
  CanTxWidgetSetMsg(MakroWin->CanMessageWdg, (struct TCanMsg *)&NullCanMsg);
  }
else
  {
  CanTxWidgetSetMsg(MakroWin->CanMessageWdg, &makro->CanMsg);
  if (makro->IntervallEnabled)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(MakroWin->IntervallEnableEdit), TRUE);
  else
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(MakroWin->IntervallEnableEdit), FALSE);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(MakroWin->IntervallTimeEdit), makro->IntervallTime);
  }
}


/**************************************************************/
/* Makro Daten vom Fenster einlesen                          */
/**************************************************************/
/* void MakWinGetDataFromUi(struct TMakro *makro)
{
if (!MakroWin)
  return;
if (!makro)
  return;
CanTxWidgetGetMsg(MakroWin->CanMessageWdg, &makro->CanMsg);
if (GTK_TOGGLE_BUTTON(MakroWin->IntervallEnableEdit)->active == TRUE)
  makro->IntervallEnabled = 1;
else
  makro->IntervallEnabled = 0;
gtk_spin_button_update(GTK_SPIN_BUTTON(MakroWin->IntervallTimeEdit));
makro->IntervallTime = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(MakroWin->IntervallTimeEdit));
}  */


struct TMakroWin *ShowMakroDlg(void)
{
struct TMakroWin *makro_win;
GtkWidget *widget;
GtkWidget *hbox;
GtkWidget *hbox2;
GtkWidget *dialog_vbox1;
GtkWidget *vbox1;
GtkWidget *vbox2;
GtkWidget *vbox3;
GtkWidget *vbox4;
GtkWidget *scrolledwindow1;
GtkWidget *vbuttonbox2;
GtkWidget *hbuttonbox1;

makro_win = (struct TMakroWin *)g_malloc0(sizeof(struct TMakroWin));
if (!makro_win)
  return(NULL);

makro_win->MainWin = gtk_dialog_new ();
gtk_widget_set_size_request (makro_win->MainWin, -1, 600);
gtk_window_set_title (GTK_WINDOW (makro_win->MainWin), _("Macros ..."));
gtk_window_set_type_hint (GTK_WINDOW (makro_win->MainWin), GDK_WINDOW_TYPE_HINT_DIALOG);

dialog_vbox1 = GTK_DIALOG (makro_win->MainWin)->vbox;
gtk_widget_show (dialog_vbox1);

vbox1 = gtk_vbox_new (FALSE, 0);
gtk_widget_show (vbox1);
gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);
gtk_container_set_border_width (GTK_CONTAINER (vbox1), 5);

hbox2 = gtk_hbox_new (FALSE, 0);
gtk_widget_show (hbox2);
gtk_box_pack_start (GTK_BOX (vbox1), hbox2, TRUE, TRUE, 0);

vbox3 = gtk_vbox_new (FALSE, 0);
gtk_widget_show (vbox3);
gtk_box_pack_start (GTK_BOX (hbox2), vbox3, FALSE, FALSE, 6);
// Up Button
makro_win->UpBtn = gtk_button_new ();
gtk_widget_show (makro_win->UpBtn);
gtk_box_pack_start (GTK_BOX (vbox3), makro_win->UpBtn, FALSE, FALSE, 0);

widget = gtk_image_new_from_stock ("gtk-go-up", GTK_ICON_SIZE_BUTTON);
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (makro_win->UpBtn), widget);
// Down Button
makro_win->DownBtn = gtk_button_new ();
gtk_widget_show (makro_win->DownBtn);
gtk_box_pack_end (GTK_BOX (vbox3), makro_win->DownBtn, FALSE, FALSE, 0);

widget = gtk_image_new_from_stock ("gtk-go-down", GTK_ICON_SIZE_BUTTON);
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (makro_win->DownBtn), widget);
// Makro List View
scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
gtk_widget_show (scrolledwindow1);
gtk_box_pack_start (GTK_BOX (hbox2), scrolledwindow1, TRUE, TRUE, 0);
gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_SHADOW_IN);

makro_win->MakroListView = gtk_tree_view_new ();
gtk_widget_show (makro_win->MakroListView);
gtk_container_add (GTK_CONTAINER (scrolledwindow1), makro_win->MakroListView);
gtk_widget_set_size_request (makro_win->MakroListView, 200, 175);
gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (makro_win->MakroListView), FALSE);
gtk_tree_view_set_enable_search (GTK_TREE_VIEW (makro_win->MakroListView), FALSE);

vbox2 = gtk_vbox_new (FALSE, 0);
gtk_widget_show (vbox2);
gtk_box_pack_start (GTK_BOX (hbox2), vbox2, FALSE, FALSE, 6);
// Senden Button
makro_win->SendBtn = gtk_button_new ();
gtk_widget_show (makro_win->SendBtn);
gtk_box_pack_start (GTK_BOX (vbox2), makro_win->SendBtn, FALSE, FALSE, 0);

widget = gtk_alignment_new (0.5, 0.5, 0, 0);
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (makro_win->SendBtn), widget);

hbox = gtk_hbox_new (FALSE, 2);
gtk_widget_show (hbox);
gtk_container_add (GTK_CONTAINER (widget), hbox);

widget = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_BUTTON);
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

widget = gtk_label_new_with_mnemonic (_("Transmit"));
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
// Delete Button
makro_win->DeleteBtn = gtk_button_new ();
gtk_widget_show (makro_win->DeleteBtn);
gtk_box_pack_start (GTK_BOX (vbox2), makro_win->DeleteBtn, TRUE, FALSE, 0);

widget = gtk_alignment_new (0.5, 0.5, 0, 0);
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (makro_win->DeleteBtn), widget);

hbox = gtk_hbox_new (FALSE, 2);
gtk_widget_show (hbox);
gtk_container_add (GTK_CONTAINER (widget), hbox);

widget = gtk_image_new_from_stock ("gtk-clear", GTK_ICON_SIZE_BUTTON);
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

widget = gtk_label_new_with_mnemonic (_("Delete"));
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

vbuttonbox2 = gtk_vbutton_box_new ();
gtk_widget_show (vbuttonbox2);
gtk_box_pack_end (GTK_BOX (vbox2), vbuttonbox2, FALSE, FALSE, 0);
// Copy Button
makro_win->CopyBtn = gtk_button_new ();
gtk_widget_show (makro_win->CopyBtn);
gtk_container_add (GTK_CONTAINER (vbuttonbox2), makro_win->CopyBtn);
GTK_WIDGET_SET_FLAGS (makro_win->CopyBtn, GTK_CAN_DEFAULT);

widget = gtk_alignment_new (0.5, 0.5, 0, 0);
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (makro_win->CopyBtn), widget);

hbox = gtk_hbox_new (FALSE, 2);
gtk_widget_show (hbox);
gtk_container_add (GTK_CONTAINER (widget), hbox);

widget = gtk_image_new_from_stock ("gtk-copy", GTK_ICON_SIZE_BUTTON);
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

widget = gtk_label_new_with_mnemonic (_("Copy"));
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
// New Button
makro_win->NewBtn = gtk_button_new ();
gtk_widget_show (makro_win->NewBtn);
gtk_container_add (GTK_CONTAINER (vbuttonbox2), makro_win->NewBtn);
GTK_WIDGET_SET_FLAGS (makro_win->NewBtn, GTK_CAN_DEFAULT);

widget = gtk_alignment_new (0.5, 0.5, 0, 0);
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (makro_win->NewBtn), widget);

hbox = gtk_hbox_new (FALSE, 2);
gtk_widget_show (hbox);
gtk_container_add (GTK_CONTAINER (widget), hbox);

widget = gtk_image_new_from_stock ("gtk-new", GTK_ICON_SIZE_BUTTON);
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

widget = gtk_label_new_with_mnemonic (_("Add"));
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

widget = gtk_hseparator_new ();
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (vbox1), widget, FALSE, TRUE, 4);

vbox4 = gtk_vbox_new (FALSE, 5);
gtk_widget_show (vbox4);
gtk_box_pack_start (GTK_BOX (vbox1), vbox4, FALSE, TRUE, 0);
// Name Edit
hbox = gtk_hbox_new (FALSE, 0);
gtk_widget_show (hbox);
gtk_box_pack_start (GTK_BOX (vbox4), hbox, TRUE, TRUE, 0);

widget = gtk_label_new (_("  Name : "));
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
gtk_misc_set_alignment (GTK_MISC (widget), 1, 0.5);

makro_win->NameEdit = gtk_entry_new ();
gtk_widget_show (makro_win->NameEdit);
gtk_box_pack_start (GTK_BOX (hbox), makro_win->NameEdit, FALSE, FALSE, 0);
gtk_entry_set_max_length (GTK_ENTRY (makro_win->NameEdit), 30);
gtk_entry_set_width_chars (GTK_ENTRY (makro_win->NameEdit), 40);

hbuttonbox1 = gtk_hbutton_box_new ();
gtk_widget_show (hbuttonbox1);
gtk_box_pack_start (GTK_BOX (vbox4), hbuttonbox1, FALSE, TRUE, 0);
gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox1), GTK_BUTTONBOX_END);
// Einfügen Button
makro_win->MakInsertBtn = gtk_button_new ();
gtk_widget_show (makro_win->MakInsertBtn);
gtk_container_add (GTK_CONTAINER (hbuttonbox1), makro_win->MakInsertBtn);
GTK_WIDGET_SET_FLAGS (makro_win->MakInsertBtn, GTK_CAN_DEFAULT);

widget = gtk_alignment_new (0.5, 0.5, 0, 0);
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (makro_win->MakInsertBtn), widget);

hbox = gtk_hbox_new (FALSE, 2);
gtk_widget_show (hbox);
gtk_container_add (GTK_CONTAINER (widget), hbox);

widget = gtk_image_new_from_stock ("gtk-paste", GTK_ICON_SIZE_BUTTON);
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

widget = gtk_label_new_with_mnemonic (_("Insert"));
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
// CAN Tx View
makro_win->CanTxWindow = gtk_vbox_new (FALSE, 0);
gtk_widget_show (makro_win->CanTxWindow);
gtk_box_pack_start (GTK_BOX (vbox4), makro_win->CanTxWindow, TRUE, TRUE, 0);

hbox = gtk_hbox_new (FALSE, 0);
gtk_widget_show (hbox);
gtk_box_pack_start (GTK_BOX (vbox4), hbox, TRUE, TRUE, 0);

makro_win->IntervallFrame = gtk_frame_new (NULL);
gtk_widget_show (makro_win->IntervallFrame);
gtk_box_pack_start (GTK_BOX (hbox), makro_win->IntervallFrame, FALSE, FALSE, 0);
// Intervall
widget = gtk_alignment_new (0.5, 0.5, 1, 1);
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (makro_win->IntervallFrame), widget);
gtk_alignment_set_padding (GTK_ALIGNMENT (widget), 5, 5, 7, 5);

hbox = gtk_hbox_new (FALSE, 3);
gtk_widget_show (hbox);
gtk_container_add (GTK_CONTAINER (widget), hbox);

makro_win->IntervallEnableEdit = gtk_check_button_new_with_mnemonic (_("Enable"));
gtk_widget_show (makro_win->IntervallEnableEdit);
gtk_box_pack_start (GTK_BOX (hbox), makro_win->IntervallEnableEdit, FALSE, FALSE, 0);

widget = gtk_label_new (_(" Time:"));
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

makro_win->IntervallTimeEdit_adj = gtk_adjustment_new (1, 1, 60000, 1, 10, 0);
makro_win->IntervallTimeEdit = gtk_spin_button_new (GTK_ADJUSTMENT (makro_win->IntervallTimeEdit_adj), 1, 0);
gtk_widget_show (makro_win->IntervallTimeEdit);
gtk_box_pack_start (GTK_BOX (hbox), makro_win->IntervallTimeEdit, FALSE, FALSE, 0);
gtk_widget_set_size_request (makro_win->IntervallTimeEdit, 70, -1);
gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (makro_win->IntervallTimeEdit), TRUE);
gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (makro_win->IntervallTimeEdit), GTK_UPDATE_IF_VALID);

widget = gtk_label_new (_("ms"));
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

widget = gtk_label_new (_(" Interval timer "));
gtk_widget_show (widget);
gtk_frame_set_label_widget (GTK_FRAME (makro_win->IntervallFrame), widget);
gtk_label_set_use_markup (GTK_LABEL (widget), TRUE);

widget = GTK_DIALOG (makro_win->MainWin)->action_area;
gtk_widget_show (widget);
gtk_button_box_set_layout (GTK_BUTTON_BOX (widget), GTK_BUTTONBOX_EDGE);
// Makro Intervall Button
makro_win->MakSetIntervallBtn = gtk_button_new ();
gtk_widget_show (makro_win->MakSetIntervallBtn);
gtk_dialog_add_action_widget (GTK_DIALOG (makro_win->MainWin), makro_win->MakSetIntervallBtn, GTK_RESPONSE_CLOSE);
GTK_WIDGET_SET_FLAGS (makro_win->MakSetIntervallBtn, GTK_CAN_DEFAULT);

widget = gtk_alignment_new (0.5, 0.5, 0, 0);
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (makro_win->MakSetIntervallBtn), widget);

hbox = gtk_hbox_new (FALSE, 2);
gtk_widget_show (hbox);
gtk_container_add (GTK_CONTAINER (widget), hbox);

widget = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_BUTTON);
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

widget = gtk_label_new_with_mnemonic (_("Update interval timers"));
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

// Schließen Button
makro_win->MakCloseBtn = gtk_button_new ();
gtk_widget_show (makro_win->MakCloseBtn);
gtk_dialog_add_action_widget (GTK_DIALOG (makro_win->MainWin), makro_win->MakCloseBtn, GTK_RESPONSE_CLOSE);
GTK_WIDGET_SET_FLAGS (makro_win->MakCloseBtn, GTK_CAN_DEFAULT);

widget = gtk_alignment_new (0.5, 0.5, 0, 0);
gtk_widget_show (widget);
gtk_container_add (GTK_CONTAINER (makro_win->MakCloseBtn), widget);

hbox = gtk_hbox_new (FALSE, 2);
gtk_widget_show (hbox);
gtk_container_add (GTK_CONTAINER (widget), hbox);

widget = gtk_image_new_from_stock ("gtk-quit", GTK_ICON_SIZE_BUTTON);
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

widget = gtk_label_new_with_mnemonic (_("_Close"));
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

makro_win->MakListView = MakWinCreateTable(makro_win->MakroListView, 1, MakWinListSelectCB2);
makro_win->CanMessageWdg = CanTxWidgetNew(makro_win->CanTxWindow, CAN_TX_WDG_EDIT_MODE | CAN_TX_EDIT_FINISH_EVENT, MakWinCanTxWidgetCB);

gtk_widget_show(makro_win->MainWin);

return(makro_win);
}

