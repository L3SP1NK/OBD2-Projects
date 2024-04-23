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
#include <string.h>
#include "plugin.h"
#include "support.h"
#include "plugin_win.h"


#define PLUGIN_RESPONSE_BTN 98121


static GtkWidget *Expander = NULL;
static GtkWidget *PluginDialog = NULL;
static GtkWidget *PluginInfo = NULL;
static GtkWidget *PrefButton = NULL;




void PluginUpdateList(GtkListStore *ls)
{
GtkTreeIter iter;
struct TPlugin *plugin;
gboolean loaded;
gchar *puf;

gtk_list_store_clear(ls);
for (plugin = Plugins; plugin; plugin = plugin->Next)
  {
  if (plugin->Status >= PLUGIN_STATUS_SCAN)
    {
    if (plugin->Status == PLUGIN_STATUS_LOAD)
      loaded = TRUE;
    else
      loaded = FALSE;
    gtk_list_store_append (ls, &iter);
    puf = g_strdup_printf("<b>%s</b> %s\n%s", plugin->Info.Name,
           plugin->Info.Version, plugin->Info.Summary);
    gtk_list_store_set(ls, &iter,
              0, loaded, 1, puf, 2, plugin, -1);
    g_free(puf);
    }
  }
}


void PrefDialogResponseCB(GtkWidget *d, int response, struct TPlugin *plugin)
{
switch (response)
  {
  case GTK_RESPONSE_CLOSE :
  case GTK_RESPONSE_DELETE_EVENT :
            {
            gtk_widget_destroy(d);
            break;
            }
  }
}



void PluginToggledCB(GtkCellRendererToggle *cell, gchar *path_str, gpointer data)
{
struct TPlugin *plugin;
GtkTreeModel *model;
GtkTreeIter  iter;
GtkTreePath *path;
gboolean aktiv;

model = (GtkTreeModel *)data;
path = gtk_tree_path_new_from_string(path_str);

gtk_tree_model_get_iter(model, &iter, path);
gtk_tree_model_get(model, &iter, 2, &plugin, -1);
aktiv = !gtk_cell_renderer_toggle_get_active(cell);
g_assert(plugin != NULL);
if (aktiv == TRUE)
  {  // Plugin laden
  if (PluginLoad(plugin) < 0)
    aktiv = FALSE;
  }
else
  {  // Plugin entladen
  (void)PluginUnload(plugin);
  }
if ((plugin->Status == PLUGIN_STATUS_LOAD) &&
     (plugin->Data) && (plugin->Data->ExecuteSetupCB))
  gtk_widget_set_sensitive(PrefButton, TRUE);
else
  gtk_widget_set_sensitive(PrefButton, FALSE);
gtk_list_store_set(GTK_LIST_STORE (model), &iter, 0, aktiv, -1);
gtk_tree_path_free(path);
}


// sicherstellen das der ausgewählte Plugin in der Liste angezeigt wird
gboolean PluginEnsureVisible(void *data)
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


// Ein Plugin wurde selektiert
void PluginSelectCB(GtkTreeSelection *sel, GtkTreeModel *model)
{
gchar *buf;
GtkTreeIter  iter;
struct TPlugin *plugin;

if (gtk_tree_selection_get_selected(sel, &model, &iter))
  {
  gtk_widget_set_sensitive(Expander, TRUE);
  gtk_tree_model_get(model, &iter, 2, &plugin, -1);
  if (plugin->Status >= PLUGIN_STATUS_SCAN)
    {
    buf = g_strdup_printf("%s\n\n"
          "<span weight=\"bold\">%s</span>\t%s\n"
          "<span weight=\"bold\">%s</span>\t%s",
        plugin->Info.Description, _("Author:"),
        plugin->Info.Author, _("Homepage:"), plugin->Info.Homepage);
    gtk_label_set_markup(GTK_LABEL(PluginInfo), buf);
    g_free(buf);
    }
  if ((plugin->Status == PLUGIN_STATUS_LOAD) &&
       (plugin->Data) && (plugin->Data->ExecuteSetupCB))
    gtk_widget_set_sensitive(PrefButton, TRUE);
  else
    gtk_widget_set_sensitive(PrefButton, FALSE);
  // Make sure the selected plugin is still visible
  g_idle_add(PluginEnsureVisible, sel);
  }
else
  {
  // Plugin String löschen
  gtk_label_set_markup(GTK_LABEL(PluginInfo), "");
  gtk_widget_set_sensitive(PrefButton, FALSE);

  gtk_expander_set_expanded(GTK_EXPANDER(Expander), FALSE);
  gtk_widget_set_sensitive(Expander, FALSE);
  }
}


// Ein Plugin wurde ausgewählt
static void PluginDialogResponseCB(GtkWidget *d, int response, GtkTreeSelection *sel)
{
struct TPlugin *plugin;
//GtkWidget *dialog, *box;
GtkTreeModel *model;
GtkTreeIter iter;

switch (response)
  {
  case GTK_RESPONSE_CLOSE :
  case GTK_RESPONSE_DELETE_EVENT :
              {
              // purple_signals_disconnect_by_handle(PluginDialog);
              gtk_widget_destroy(d);
              PluginDialog = NULL;
              break;
              }
  case PLUGIN_RESPONSE_BTN:
              {
              if (gtk_tree_selection_get_selected (sel, &model, &iter))
                {
                gtk_tree_model_get(model, &iter, 2, &plugin, -1);
                if (!plugin)
                  break;
                if ((plugin->Data) && (plugin->Data->ExecuteSetupCB))
                  (plugin->Data->ExecuteSetupCB)();
                /* box = pidgin_plugin_get_config_frame(plug);
                if (box == NULL)
                  break;

                dialog = gtk_dialog_new_with_buttons(plugin->Info.Name, GTK_WINDOW(d),
                      GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR | GTK_DIALOG_DESTROY_WITH_PARENT,
                      GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                      NULL);
                g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(PrefDialogResponseCB), plugin);
                gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), box);
                gtk_widget_show_all(dialog);  */
                }
              break;
              }
  }
}


// Info zum Plugin anzeigen
static void ShowPluginPrefsCB(GtkTreeView *view, GtkTreePath *path, GtkTreeViewColumn *column, GtkWidget *dialog)
{
GtkTreeSelection *sel;
GtkTreeIter iter;
struct TPlugin *plugin;
GtkTreeModel *model;

sel = gtk_tree_view_get_selection(view);
if (gtk_tree_selection_get_selected(sel, &model, &iter))
  {
  gtk_tree_model_get(model, &iter, 2, &plugin, -1);
  // Plugin geladen ?
  if ((plugin) && (plugin->Status == PLUGIN_STATUS_LOAD))
    PluginDialogResponseCB(dialog, PLUGIN_RESPONSE_BTN, sel);
  }
}


void PluginDialogShow(void)
{
GtkWidget *sw;
GtkWidget *event_view;
GtkListStore *ls;
GtkCellRenderer *rend, *rendt;
GtkTreeViewColumn *col;
GtkTreeSelection *sel;

if (PluginDialog != NULL)
  {
  gtk_window_present(GTK_WINDOW(PluginDialog));
  return;
  }
// **** Dialog mit Close und Config Button erzeugen
PluginDialog = gtk_dialog_new_with_buttons(_("Plugins"), NULL, GTK_DIALOG_NO_SEPARATOR, NULL);
PrefButton = gtk_dialog_add_button(GTK_DIALOG(PluginDialog), _("Setup plugin"), PLUGIN_RESPONSE_BTN);
gtk_dialog_add_button(GTK_DIALOG(PluginDialog), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
gtk_widget_set_sensitive(PrefButton, FALSE);
// **** Scroll Window
sw = gtk_scrolled_window_new(NULL,NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_IN);
gtk_box_pack_start(GTK_BOX(GTK_DIALOG(PluginDialog)->vbox), sw, TRUE, TRUE, 0);
// **** Tabelle erzeugen
ls = gtk_list_store_new(3, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_POINTER);
// **** Plugin Liste updaten
PluginUpdateList(ls);

event_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(ls));
gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(event_view), TRUE);
g_signal_connect(G_OBJECT(event_view), "row-activated",
         G_CALLBACK(ShowPluginPrefsCB), PluginDialog);
// **** Spalte "Enabled"
rend = gtk_cell_renderer_toggle_new();
sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (event_view));
col = gtk_tree_view_column_new_with_attributes (_("Enabled"), rend, "active", 0, NULL);
g_signal_connect(G_OBJECT(rend), "toggled", G_CALLBACK(PluginToggledCB), ls);
gtk_tree_view_append_column (GTK_TREE_VIEW(event_view), col);
// **** Spalte "Name"
rendt = gtk_cell_renderer_text_new();
//g_object_set(rendt, "foreground", "#C0C0C0", NULL);
col = gtk_tree_view_column_new_with_attributes(_("Name"), rendt, "markup", 1, NULL); // "foreground-set", 3, NULL);
gtk_tree_view_column_set_expand(col, TRUE);
g_object_set(rendt, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
gtk_tree_view_append_column (GTK_TREE_VIEW(event_view), col);
g_object_unref(G_OBJECT(ls));
gtk_container_add(GTK_CONTAINER(sw), event_view);
// **** Plugin Info
Expander = gtk_expander_new(_("<b>About plugin</b>"));
gtk_expander_set_use_markup(GTK_EXPANDER(Expander), TRUE);
PluginInfo = gtk_label_new(NULL);
gtk_label_set_line_wrap(GTK_LABEL(PluginInfo), TRUE);
gtk_container_add(GTK_CONTAINER(Expander), PluginInfo);
gtk_widget_set_sensitive(Expander, FALSE);
gtk_box_pack_start(GTK_BOX(GTK_DIALOG(PluginDialog)->vbox), Expander, FALSE, FALSE, 0);

g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK (PluginSelectCB), NULL);
g_signal_connect(G_OBJECT(PluginDialog), "response", G_CALLBACK(PluginDialogResponseCB), sel);
gtk_window_set_default_size(GTK_WINDOW(PluginDialog), 430, 430);
gtk_widget_show_all(PluginDialog);
}
