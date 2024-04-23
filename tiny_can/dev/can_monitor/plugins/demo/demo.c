#include <glib.h>
#include <gtk/gtk.h>
#include <gmodule.h>
#include "plugin_base.h"
#include "dialogs.h"
#include "global.h"
#include <stdlib.h>
#include <string.h>
#include "giw.h"

static GtkWidget *MainForm = NULL;
static GtkWidget *MenuItem = NULL;
static gpointer Self;

static void MenueCreate(GtkWidget *menuitem);
static void ExecuteSetup(void);
static void MenueEventCB(GtkObject *obj, gpointer user_data);
static void FilterEditFinishEventCB(gulong signal, gpointer event_data, gpointer user_data);
static void Filter1HitEventCB(gulong signal, gpointer event_data, gpointer user_data);
static void Filter2HitEventCB(gulong signal, gpointer event_data, gpointer user_data);
static void Filter3HitEventCB(gulong signal, gpointer event_data, gpointer user_data);

static void ShowMainForm(void);
static void DestroyMainForm(void);


char *PluginPath;
static struct TDeviceStatus *CanDevStatus;
TSendMessageProc SendCanMessage;
TMakroSetProc MakroSet;
TMakroGetProc MakroGet;
TMakroClearProc MakroClear;
TMakroSendProc MakroSend;
TFilterSetProc FilterSet;
TFilterGetProc FilterGet;
TFilterClearProc FilterClear;
TFilterEditingFinish FilterEditingFinish;

GtkWidget *Leds[8];
GtkWidget *Drehzahl;
GtkWidget *Spannung;
GtkWidget *Strom;
GtkWidget *Fuelstand;
GtkWidget *Temp1;
GtkWidget *Temp2;

const char FILTER1_NAME[] = {"Msg 1"};
const char FILTER2_NAME[] = {"Msg 2"};
const char FILTER3_NAME[] = {"Msg 3"};
const unsigned long FILTER1_ID = 0x100;
const unsigned long FILTER2_ID = 0x101;
const unsigned long FILTER3_ID = 0x102;
   

G_MODULE_EXPORT const struct TPluginInfo PluginInfo =
  {  
  "demo-plugin",                     // Id
  "Demo",                            // Name
  "1.0",                             // Version
  "Demo Plugin",                     // Summary
  "Das ist ein kleiner Demo Plugin", // Description
  "MHS-Elektronik",                  // Author
  "www.mhs-elektronik.de"            // Homepage
  };

G_MODULE_EXPORT const struct TPluginData PluginData =
  {
  "Demo",         // Menue Label
  &MenueCreate,   // Menue Create Funktion
  &ExecuteSetup   // Setup Funktion
  };



G_MODULE_EXPORT int PluginLoadCB(gpointer plugin, char *plugin_path, struct TPluginMainCalls *main_calls)
{
struct TFilter *filter;
int i;
//struct TCanMsg msg;

Self = plugin;
PluginPath = plugin_path;
CanDevStatus = main_calls->PlCanDevStatus;
mhs_signal_init(main_calls->PlSignals);
SendCanMessage = main_calls->SendMessageProc;
MakroSet = main_calls->MakroSetProc;
MakroGet = main_calls->MakroGetProc;
MakroClear = main_calls->MakroClearProc;
MakroSend = main_calls->MakroSendProc;
FilterSet = main_calls->FilterSetProc;
FilterGet = main_calls->FilterGetProc;
FilterClear = main_calls->FilterClearProc;
FilterEditingFinish = main_calls->FilterEditingFinishProc;

MainForm = NULL;
MenuItem = NULL;

for (i = 0; i < 8; i++)
  Leds[i] = NULL;
Drehzahl = NULL;
Spannung = NULL;
Strom = NULL;
Fuelstand = NULL;
Temp1 = NULL;
Temp2 = NULL;

mhs_signal_connect(Self, SIGC_PUBLIC, SIG_FILTER_EDIT_FINISH, FilterEditFinishEventCB, NULL);

filter = (struct TFilter *)g_malloc0(sizeof(struct TFilter));

filter->HardwareFilter = 0;
filter->PassMessage = 0;
filter->IdMode = 0;
filter->FormatEff = 0;
filter->Id1 = FILTER1_ID;
filter->Id2 = 0;
filter->Flags = FILTER_ENABLED | FILTER_SHOW;      
strcpy(filter->Name, FILTER1_NAME);
FilterSet(Self, filter->Name, filter);

filter->Id1 = FILTER2_ID;
strcpy(filter->Name, FILTER2_NAME);
FilterSet(Self, filter->Name, filter);

filter->Id1 = FILTER3_ID;
strcpy(filter->Name, FILTER3_NAME);
FilterSet(Self, filter->Name, filter);

g_free(filter);
FilterEditingFinish(Self);
return(0);
}


G_MODULE_EXPORT int PluginUnloadCB(void)
{
DestroyMainForm();
return(0);
}


void MenueCreate(GtkWidget *menuitem)
{
g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(MenueEventCB), NULL);
MenuItem = menuitem;
}


void ExecuteSetup(void)
{
}


/**************************************************************/
/*  Gtk Main Callbacks                                        */
/**************************************************************/
gint delete_event(GtkWidget *widget, GdkEvent event, gpointer daten)
{
return(FALSE);
}


void ende(GtkWidget *widget, gpointer daten)
{
DestroyMainForm();
}


void MenueEventCB(GtkObject *obj, gpointer user_data)
{
ShowMainForm();
}


static void ShowMainForm(void)
{
GtkWidget *vbox1, *vbox2;
GtkWidget *hbox1, *hbox2;
GtkWidget *widget;
GtkWidget *table1, *table2, *table4;

if (MainForm)
  {
  gtk_window_present(GTK_WINDOW(MainForm));
  return;
  }

MainForm = gtk_window_new (GTK_WINDOW_TOPLEVEL);
gtk_window_set_title (GTK_WINDOW (MainForm), "CAN-Monitor Demo Plugin");
gtk_window_set_position (GTK_WINDOW (MainForm), GTK_WIN_POS_CENTER);

vbox1 = gtk_vbox_new (FALSE, 0);
gtk_widget_show (vbox1);
gtk_container_add (GTK_CONTAINER (MainForm), vbox1);
gtk_container_set_border_width (GTK_CONTAINER (vbox1), 5);

hbox1 = gtk_hbox_new (FALSE, 0);
gtk_widget_show (hbox1);
gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

vbox2 = gtk_vbox_new (FALSE, 2);
gtk_widget_show (vbox2);
gtk_box_pack_start (GTK_BOX (hbox1), vbox2, TRUE, TRUE, 0);

widget = gtk_label_new ("Drehzahl");
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (vbox2), widget, FALSE, FALSE, 0);
// **** Drehzahl
widget = giw_dial_new_with_adjusment(0.0, 0.0, 200.0);
gtk_widget_show (widget);
gtk_box_pack_start (GTK_BOX (vbox2), widget, TRUE, TRUE, 0);
Drehzahl = widget;

table1 = gtk_table_new (8, 2, FALSE);
gtk_widget_show (table1);
gtk_box_pack_start (GTK_BOX (hbox1), table1, FALSE, FALSE, 0);
gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
gtk_table_set_col_spacings (GTK_TABLE (table1), 5);

widget = gtk_label_new ("LED 1");
gtk_widget_show (widget);
gtk_table_attach (GTK_TABLE (table1), widget, 1, 2, 0, 1,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);

widget = gtk_label_new ("LED 2");
gtk_widget_show (widget);
gtk_table_attach (GTK_TABLE (table1), widget, 1, 2, 1, 2,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);

widget = gtk_label_new ("LED 3");
gtk_widget_show (widget);
gtk_table_attach (GTK_TABLE (table1), widget, 1, 2, 2, 3,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);

widget = gtk_label_new ("LED 4");
gtk_widget_show (widget);
gtk_table_attach (GTK_TABLE (table1), widget, 1, 2, 3, 4,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);

widget = gtk_label_new ("LED 5");
gtk_widget_show (widget);
gtk_table_attach (GTK_TABLE (table1), widget, 1, 2, 4, 5,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);

widget = gtk_label_new ("LED 6");
gtk_widget_show (widget);
gtk_table_attach (GTK_TABLE (table1), widget, 1, 2, 5, 6,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);
    
widget = gtk_label_new ("LED 7");
gtk_widget_show (widget);
gtk_table_attach (GTK_TABLE (table1), widget, 1, 2, 6, 7,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);

widget = gtk_label_new ("LED 8");
gtk_widget_show (widget);
gtk_table_attach (GTK_TABLE (table1), widget, 1, 2, 7, 8,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);
  
// **** LED 1
widget = giw_led_new();
gtk_widget_show(widget);
gtk_table_attach (GTK_TABLE (table1), widget, 0, 1, 0, 1,
                  (GtkAttachOptions) (0),
                  (GtkAttachOptions) (GTK_FILL), 0, 0);
Leds[0] = widget;
// **** LED 2
widget = giw_led_new();
gtk_widget_show(widget);
gtk_table_attach (GTK_TABLE (table1), widget, 0, 1, 1, 2,
                  (GtkAttachOptions) (0),
                  (GtkAttachOptions) (GTK_FILL), 0, 0);
Leds[1] = widget;
// **** LED 3
widget = giw_led_new();
gtk_widget_show(widget);
gtk_table_attach (GTK_TABLE (table1), widget, 0, 1, 2, 3,
                  (GtkAttachOptions) (0),
                  (GtkAttachOptions) (GTK_FILL), 0, 0);
Leds[2] = widget;
// **** LED 4
widget = giw_led_new();
gtk_widget_show(widget);
gtk_table_attach (GTK_TABLE (table1), widget, 0, 1, 3, 4,
                  (GtkAttachOptions) (0),
                  (GtkAttachOptions) (GTK_FILL), 0, 0);
Leds[3] = widget;
// **** LED 5
widget = giw_led_new();
gtk_widget_show(widget);
gtk_table_attach (GTK_TABLE (table1), widget, 0, 1, 4, 5,
                  (GtkAttachOptions) (0),
                  (GtkAttachOptions) (GTK_FILL), 0, 0);
Leds[4] = widget;
// **** LED 6
widget = giw_led_new();
gtk_widget_show(widget);
gtk_table_attach (GTK_TABLE (table1), widget, 0, 1, 5, 6,
                  (GtkAttachOptions) (0),
                  (GtkAttachOptions) (GTK_FILL), 0, 0);
Leds[5] = widget;
// **** LED 7
widget = giw_led_new();
gtk_widget_show(widget);
gtk_table_attach (GTK_TABLE (table1), widget, 0, 1, 6, 7,
                  (GtkAttachOptions) (0),
                  (GtkAttachOptions) (GTK_FILL), 0, 0);
Leds[6] = widget;
// **** LED 8
widget = giw_led_new();
gtk_widget_show(widget);
gtk_table_attach (GTK_TABLE (table1), widget, 0, 1, 7, 8,
                  (GtkAttachOptions) (0),
                  (GtkAttachOptions) (GTK_FILL), 0, 0);
Leds[7] = widget;

hbox2 = gtk_hbox_new (FALSE, 0);
gtk_widget_show (hbox2);
gtk_box_pack_start (GTK_BOX (vbox1), hbox2, TRUE, TRUE, 0);

table2 = gtk_table_new (2, 2, FALSE);
gtk_widget_show (table2);
gtk_box_pack_start (GTK_BOX (hbox2), table2, TRUE, TRUE, 0);
gtk_table_set_row_spacings (GTK_TABLE (table2), 2);
gtk_table_set_col_spacings (GTK_TABLE (table2), 5);

widget = gtk_label_new ("Spannug");
gtk_widget_show (widget);
gtk_table_attach (GTK_TABLE (table2), widget, 0, 1, 0, 1,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_CENTER);

widget = gtk_label_new ("Strom");
gtk_widget_show (widget);
gtk_table_attach (GTK_TABLE (table2), widget, 1, 2, 0, 1,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_CENTER);
// **** Spannung
widget = giw_show_new_with_adjusment(100.0, 100.0, 200.0);
gtk_widget_show (widget);
gtk_table_attach (GTK_TABLE (table2), widget, 0, 1, 1, 2,
                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
Spannung = widget;
// **** Strom
widget = giw_show_new_with_adjusment(100.0, 100.0, 200.0);
gtk_widget_show (widget);
gtk_table_attach (GTK_TABLE (table2), widget, 1, 2, 1, 2,
                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
Strom = widget;
table4 = gtk_table_new (2, 4, FALSE);
gtk_widget_show (table4);
gtk_box_pack_start (GTK_BOX (vbox1), table4, TRUE, TRUE, 0);
gtk_table_set_row_spacings (GTK_TABLE (table4), 2);
gtk_table_set_col_spacings (GTK_TABLE (table4), 5);

widget = gtk_label_new ("F\303\274llstand");
gtk_widget_show (widget);
gtk_table_attach (GTK_TABLE (table4), widget, 0, 1, 0, 1,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_CENTER);

widget = gtk_label_new ("Temp 1");
gtk_widget_show (widget);
gtk_table_attach (GTK_TABLE (table4), widget, 1, 2, 0, 1,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_CENTER);

widget = gtk_label_new ("Temp 2");
gtk_widget_show (widget);
gtk_table_attach (GTK_TABLE (table4), widget, 2, 3, 0, 1,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_CENTER);

widget = gtk_label_new ("Power");
gtk_widget_show (widget);
gtk_table_attach (GTK_TABLE (table4), widget, 3, 4, 0, 1,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (0), 0, 0);
gtk_label_set_justify (GTK_LABEL (widget), GTK_JUSTIFY_CENTER);
// **** Füllstand
widget = giw_tank_new_with_adjustment(0, 0, 100);
gtk_widget_show(widget);
gtk_table_attach (GTK_TABLE (table4), widget, 0, 1, 1, 2,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (GTK_FILL), 0, 0);  
Fuelstand = widget;
// **** Temp 1
widget = giw_thermo_new_with_adjustment(0, 0, 100);
gtk_widget_show(widget);
gtk_table_attach (GTK_TABLE (table4), widget, 1, 2, 1, 2,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (GTK_FILL), 0, 0);
Temp1 = widget;
// **** Temp 2
widget = giw_thermo_new_with_adjustment(0, 0, 100);
gtk_widget_show(widget);
gtk_table_attach (GTK_TABLE (table4), widget, 2, 3, 1, 2,
                  (GtkAttachOptions) (GTK_FILL),
                  (GtkAttachOptions) (GTK_FILL), 0, 0);
Temp2 = widget;
// **** Power                          
widget = giw_knob_new_with_adjustment(0.0, 0.0, 200.0);
gtk_widget_show(widget);
gtk_table_attach (GTK_TABLE (table4), widget, 3, 4, 1, 2,
                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);  

gtk_widget_show(MainForm);

g_signal_connect(MainForm, "delete_event", G_CALLBACK(delete_event), NULL);
g_signal_connect(MainForm, "destroy", G_CALLBACK(ende), NULL);
}


static void DestroyMainForm(void)
{
int i;

if (MainForm)
  {
  for (i = 0; i < 8; i++)
    Leds[i] = NULL;
  Drehzahl = NULL;
  Spannung = NULL;
  Strom = NULL;
  Fuelstand = NULL;
  Temp1 = NULL;
  Temp2 = NULL;  
  gtk_widget_destroy(GTK_WIDGET(MainForm));
  MainForm = NULL;
  }
}


void FilterEditFinishEventCB(gulong signal, gpointer event_data, gpointer user_data)
{
struct TFilter *filter;

filter = (struct TFilter *)g_malloc0(sizeof(struct TFilter));

if (FilterGet(Self, (char *)FILTER1_NAME, filter) == 1)
  mhs_signal_connect(Self, SIGC_CAN, filter->Index, Filter1HitEventCB, NULL);
if (FilterGet(Self, (char *)FILTER2_NAME, filter) == 1)
  mhs_signal_connect(Self, SIGC_CAN, filter->Index, Filter2HitEventCB, NULL);
if (FilterGet(Self, (char *)FILTER3_NAME, filter) == 1)
  mhs_signal_connect(Self, SIGC_CAN, filter->Index, Filter3HitEventCB, NULL);

g_free(filter);
}


void Filter1HitEventCB(gulong signal, gpointer event_data, gpointer user_data)
{
struct TCanMsg *msg;
int i;
unsigned char b;

if (!Leds[0])
  return;
msg = (struct TCanMsg *)event_data;
b = msg->MsgData[0];
for (i = 0; i < 8; i++)
  {
  if (b & 0x01)
    giw_led_set_mode(GIW_LED(Leds[i]), 1);
  else
    giw_led_set_mode(GIW_LED(Leds[i]), 0);
  b = b >> 1;
  }
b = msg->MsgData[1];
giw_dial_set_value(GIW_DIAL(Drehzahl), (float)b);
}


void Filter2HitEventCB(gulong signal, gpointer event_data, gpointer user_data)
{
struct TCanMsg *msg;

if ((!Spannung) || (!Strom))
  return;
msg = (struct TCanMsg *)event_data;
giw_show_set_value(GIW_SHOW(Spannung), msg->MsgData[0]);
giw_show_set_value(GIW_SHOW(Strom), msg->MsgData[1]);
}


void Filter3HitEventCB(gulong signal, gpointer event_data, gpointer user_data)
{
struct TCanMsg *msg;

if ((!Fuelstand) || (!Temp1) || (!Temp2))
  return;
msg = (struct TCanMsg *)event_data;
giw_tank_set_value(GIW_TANK(Fuelstand), msg->MsgData[0]);
giw_thermo_set_value(GIW_THERMO(Temp1), msg->MsgData[1]);
giw_thermo_set_value(GIW_THERMO(Temp2), msg->MsgData[2]);
}
