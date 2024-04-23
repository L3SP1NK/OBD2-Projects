/***************************************************************************
                           drv_win.c  -  description
                             -------------------
    begin             : 08.09.2008
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
#include "dialogs.c"
#include "drv_scan.h"
#include "drv_win.h"

//#define PLUGIN_RESPONSE_BTN 98121

static void DriverSelectCB(GtkTreeSelection *sel, gpointer user_data);


void DriverUpdateList(struct TDriverSetupWin *driver_win)
{
GtkTreeIter iter;
GtkTreeSelection *sel;
GtkListStore *store;
struct TDriverList *driver;
gchar *puf;

store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(driver_win->TreeView)));
sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(driver_win->TreeView));
gtk_list_store_clear(store);
DriverListDestroy(&driver_win->DriverList);
driver_win->DriverList = DriverScan(driver_win->FilePath);
if (driver_win->Flags & DRV_WIN_SHOW_ERRORS)
  {
  if (!driver_win->DriverList)
    msg_box(MSG_TYPE_ERROR, _("Error"), _("No Tiny-CAN API drivers found in dirctory \"%s\""), driver_win->FilePath);
  }
for (driver = driver_win->DriverList; driver; driver = driver->Next)
  {
  gtk_list_store_append (store, &iter);
  puf = g_strdup_printf("<b>%s</b> (Version: %s)\n<i>%s</i>", driver->Name,
         driver->Version, driver->Summary);
  gtk_list_store_set(store, &iter, 0, puf, 1, driver, -1);
  if (driver_win->SelectedName)
    {
    if (!strcmp(driver_win->SelectedName, driver->Name))
      gtk_tree_selection_select_iter(sel, &iter);
    }
  g_free(puf);
  }
DriverSelectCB(sel, driver_win);
}


// sicherstellen das der ausgewählte Driver in der Liste angezeigt wird
static gboolean DriverEnsureVisible(void *data)
{
GtkTreeSelection *sel;
GtkTreeView *tv;
GtkTreeModel *model;
GtkTreePath *path;
GtkTreeIter iter;

sel = GTK_TREE_SELECTION(data);
tv = gtk_tree_selection_get_tree_view(sel);
model = gtk_tree_view_get_model(tv);
if (gtk_tree_selection_get_selected (sel, &model, &iter))
  {
  path = gtk_tree_model_get_path(model, &iter);
  gtk_tree_view_scroll_to_cell(gtk_tree_selection_get_tree_view(sel), path, NULL, FALSE, 0, 0);
  gtk_tree_path_free(path);
  }
return(FALSE);
}


// Ein Driver wurde selektiert
static void DriverSelectCB(GtkTreeSelection *sel, gpointer user_data)
{
int event;
gchar *buf;
GtkTreeIter  iter;
struct TDriverList *driver;
GtkTreeModel *model;
struct TDriverSetupWin *driver_win;

driver_win = (struct TDriverSetupWin *)user_data;
if(!driver_win)
  return;
save_free(driver_win->SelectedName);
model = gtk_tree_view_get_model(GTK_TREE_VIEW(driver_win->TreeView));
if (gtk_tree_selection_get_selected(sel, &model, &iter))
  {
  gtk_widget_set_sensitive(driver_win->Expander, TRUE);
  gtk_tree_model_get(model, &iter, 1, &driver, -1);

  if (driver->Description)
    {
    buf = g_strdup_printf("%s\n\n%s", driver->Description, driver->Info);
    gtk_label_set_markup(GTK_LABEL(driver_win->Info), buf);
    g_free(buf);
    }
  else
    gtk_label_set_markup(GTK_LABEL(driver_win->Info), driver->Info);

  driver_win->SelectedName = g_strdup(driver->Name);
  //gtk_widget_set_sensitive(driver_win->PrefButton, TRUE);
  // Make sure the selected driver is still visible
  g_idle_add(DriverEnsureVisible, sel);
  event = 1;
  }
else
  {
  // Driver String löschen
  gtk_label_set_markup(GTK_LABEL(driver_win->Info), "");
  //gtk_widget_set_sensitive(PrefButton, FALSE);

  gtk_expander_set_expanded(GTK_EXPANDER(driver_win->Expander), FALSE);
  gtk_widget_set_sensitive(driver_win->Expander, FALSE);
  event = 0;
  }
if (driver_win->EventCallback)
  (driver_win->EventCallback)(driver_win, event);
}


// Info zum Driver anzeigen
/* static void ShowDriverPrefsCB(GtkTreeView *view, GtkTreePath *path, GtkTreeViewColumn *column, GtkWidget *dialog)
{
GtkTreeSelection *sel;
GtkTreeIter iter;
struct TDriverList *driver;
GtkTreeModel *model;

sel = gtk_tree_view_get_selection(view);
if (gtk_tree_selection_get_selected(sel, &model, &iter))
  {
  gtk_tree_model_get(model, &iter, 2, &driver, -1);
  // Driver geladen ?
  //if (driver)
  //  DriverDialogResponseCB(dialog, PLUGIN_RESPONSE_BTN, sel);
  }
} */


static void ApiDirChangeCB(GtkButton *button, gpointer user_data)
{
char *file_name;
struct TDriverSetupWin *driver_win;

driver_win = (struct TDriverSetupWin *)user_data;
if(!driver_win)
  return;
file_name = NULL;
if (SelectFileDlg(_("Change Tiny-CAN API directory"), FILE_SELECTION_PATH, &file_name) > 0)
  {
  save_free(driver_win->FilePath);
  driver_win->FilePath = g_strdup(file_name);
  gtk_label_set_text(GTK_LABEL(driver_win->Label), driver_win->FilePath);
  DriverUpdateList(driver_win);
  }
save_free(file_name);
}


struct TDriverSetupWin *DriverWinNew(char *drv_path, char *drv_filename, int flags)
{
struct TDriverSetupWin *driver_win;
GtkWidget *widget;
GtkWidget *hbox;
GtkWidget *sw;
GtkListStore *ls;
GtkCellRenderer *rendt;
GtkTreeViewColumn *col;
GtkTreeSelection *sel;
char *full_filename;

driver_win = (struct TDriverSetupWin *)g_malloc0(sizeof(struct TDriverSetupWin));
if (!driver_win)
  return(NULL);
driver_win->EventCallback = NULL;
driver_win->Flags = flags;
// Driver Path festlegen
if (drv_path)
  {
  if (g_file_test(drv_path, G_FILE_TEST_IS_DIR))
    driver_win->FilePath = g_strdup(drv_path);
  else
    driver_win->FilePath = g_strdup("."G_DIR_SEPARATOR_S);
  }
else
  driver_win->FilePath = g_strdup("."G_DIR_SEPARATOR_S);
// Driver File
if (drv_filename)
  {
  full_filename = g_build_filename(driver_win->FilePath, drv_filename, NULL);
  if (g_file_test(full_filename, G_FILE_TEST_IS_REGULAR))
    driver_win->SelectedName = g_strdup(drv_filename);
  save_free(full_filename);
  }

driver_win->Base = gtk_vbox_new(FALSE, 0);

if (flags & DRV_WIN_SHOW_PATH)
  {
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(driver_win->Base), hbox, FALSE, FALSE, 0);

  if (driver_win->FilePath)
    driver_win->Label = gtk_label_new(driver_win->FilePath);
  else
    driver_win->Label = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(hbox), driver_win->Label, TRUE, TRUE, 0);

  widget = gtk_button_new_with_label("...");
  gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
  (void)g_signal_connect(widget, "clicked", G_CALLBACK(ApiDirChangeCB), driver_win);
  }
//PrefButton = gtk_dialog_add_button(GTK_DIALOG(DriverDialog), _("Setup driver"), PLUGIN_RESPONSE_BTN);
//gtk_widget_set_sensitive(PrefButton, FALSE);

// **** Scroll Window
sw = gtk_scrolled_window_new(NULL, NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_IN);
gtk_box_pack_start(GTK_BOX(driver_win->Base), sw, TRUE, TRUE, 0);
// **** Tabelle erzeugen
ls = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);

driver_win->TreeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(ls));
gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(driver_win->TreeView), TRUE);
/* g_signal_connect(G_OBJECT(driver_win->TreeView), "row-activated",
         G_CALLBACK(ShowDriverPrefsCB), driver_win); */
// **** Spalte "Name"
sel = gtk_tree_view_get_selection (GTK_TREE_VIEW(driver_win->TreeView));
rendt = gtk_cell_renderer_text_new();
col = gtk_tree_view_column_new_with_attributes(_("Selecting a driver:"), rendt, "markup", 0, NULL);
gtk_tree_view_column_set_expand(col, TRUE);
g_object_set(rendt, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
gtk_tree_view_append_column (GTK_TREE_VIEW(driver_win->TreeView), col);
g_object_unref(G_OBJECT(ls));
gtk_container_add(GTK_CONTAINER(sw), driver_win->TreeView);
// **** Driver Info
driver_win->Expander = gtk_expander_new(_("<b>About driver</b>"));
gtk_expander_set_use_markup(GTK_EXPANDER(driver_win->Expander), TRUE);
driver_win->Info = gtk_label_new(NULL);
gtk_label_set_line_wrap(GTK_LABEL(driver_win->Info), TRUE);
gtk_container_add(GTK_CONTAINER(driver_win->Expander), driver_win->Info);
gtk_widget_set_sensitive(driver_win->Expander, FALSE);
gtk_box_pack_start(GTK_BOX(driver_win->Base), driver_win->Expander, FALSE, FALSE, 0);
// **** Driver Liste updaten
DriverUpdateList(driver_win);

g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK (DriverSelectCB), driver_win);
//g_signal_connect(G_OBJECT(driver_win->Base), "response", G_CALLBACK(DriverDialogResponseCB), driver_win);

gtk_widget_show_all(driver_win->Base);

return(driver_win);
}


void SetDriverWinEventCallback(struct TDriverSetupWin *driver_win, void(*event)(struct TDriverSetupWin *driver_win, int event))
{
if (driver_win)
  driver_win->EventCallback = event;
}


void DriverWinDestroy(struct TDriverSetupWin *driver_win)
{
if (driver_win)
  {
  DriverListDestroy(&driver_win->DriverList);
  save_free(driver_win->FilePath);
  g_free(driver_win);
  }
}


void DriverWinSetPathFile(struct TDriverSetupWin *driver_win, char *drv_path, char *drv_filename)
{
char *full_filename;

if (!driver_win)
  return;
// Driver Path festlegen
if (drv_path)
  {
  if (g_file_test(drv_path, G_FILE_TEST_IS_DIR))
    driver_win->FilePath = g_strdup(drv_path);
  else
    driver_win->FilePath = g_strdup("."G_DIR_SEPARATOR_S);
  }
else
  driver_win->FilePath = g_strdup("."G_DIR_SEPARATOR_S);
// Driver File
if (drv_filename)
  {
  full_filename = g_build_filename(driver_win->FilePath, drv_filename, NULL);
  if (g_file_test(full_filename, G_FILE_TEST_IS_REGULAR))
    driver_win->SelectedName = g_strdup(drv_filename);
  save_free(full_filename);
  }
DriverUpdateList(driver_win);
}



gchar *DriverWinGetFile(struct TDriverSetupWin *driver_win)
{
if (!driver_win)
  return(NULL);
if (!driver_win->SelectedName)
  return(NULL);
return(g_strdup(driver_win->SelectedName));
}


gchar *DriverWinGetPath(struct TDriverSetupWin *driver_win)
{
if (!driver_win)
  return(NULL);
if (!driver_win->FilePath)
  return(NULL);
return(g_strdup(driver_win->FilePath));
}
