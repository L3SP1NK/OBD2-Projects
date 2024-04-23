/*******************************************************************************
                            cview.c  -  description
                             -------------------
    begin             : 23.03.2008
    copyright         : (C) 2008 - 2009 by MHS-Elektronik GmbH & Co. KG, Germany
                               http://www.mhs-elektronik.de
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 *******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <pango/pango.h>
#include "support.h"
#include "util.h"
#include "can_types.h"
#include "cview.h"


static const gchar FORMAT_EFF_RTR_STR[] = {"EFF/RTR"};
static const gchar FORMAT_EFF_STR[]     = {"EFF    "};
static const gchar FORMAT_STD_RTR_STR[] = {"STD/RTR"};
static const gchar FORMAT_STD_STR[]     = {"STD    "};
static const gchar FORMAT_INVALID[]     = {"-"};

static const gchar DIR_RX_STR[]         = {"RX"};
static const gchar DIR_TX_STR[]         = {"TX"};

#define DIR_COL_CHAR_SIZE     3
#define INDEX_COL_CHAR_SIZE   8
#define TIME1_COL_CHAR_SIZE   18
#define TIME2_COL_CHAR_SIZE   15
#define TIME3_COL_CHAR_SIZE   12
#define TYPE_COL_CHAR_SIZE    8
#define ID_COL_CHAR_SIZE      10
#define DLC_COL_CHAR_SIZE     3
#define HEX_COL_CHAR_SIZE     24
#define ASCII_COL_CHAR_SIZE   9
#define DEZIMAL_COL_CHAR_SIZE 32

/* static const char HEX_DEF_STRING[] = {"00 00 00 00 00 00 00 00 "};
static const char ASCII_DEF_STRING[] = {"AAAAAAAA "};
static const char DEZIMAL_DEF_STRING[] = {"255 255 255 255 255 255 255 255 "}; */

static const char ASCII_LABEL_STRING[] = {N_("Data (ASCII)")};
static const char HEX_LABEL_STRING[] = {N_("Data (Hex)")};
static const char DEZIMAL_LABEL_STRING[] = {N_("Data (Dezimal)")};

extern gint CViewConfigureCB(GtkWidget *widget, GdkEventConfigure *configure, gpointer data);
extern gint CViewExposeCB(GtkWidget *widget, GdkEventExpose *expose, gpointer data);
extern void CViewValueChangedCB(GtkAdjustment *adj, gpointer data);

void CViewResizeColumns(struct TCanView *cv);
static void BufferEventCB(struct TCanBuffer *cbuf, gpointer user_data, int last_event);



/*
********************  ********************
Funktion  :

Eingaben  : keine

Ausgaben  : keine

Call's    : keine
*/
struct TCanView *CViewNew(GtkWidget *parent)
{
struct TCanView *cv;
GtkWidget *widget, *widget2, *widget3;

cv = (struct TCanView *)g_malloc0(sizeof(struct TCanView));
if(!cv)
  return(NULL);
cv->AutoScroll = 1;
// ****** Puffer l�schen
cv->Buffer = NULL;
cv->DataHandler = NULL;
// ****** Variablen Initialisieren
cv->FontDesc = NULL;
cv->FontMetrics = NULL;
// ****** Default Farben (schwarz)
cv->DirColor.red = 0;
cv->DirColor.green = 0;
cv->DirColor.blue = 0;
cv->IndexColor.red = 0;
cv->IndexColor.green = 0;
cv->IndexColor.blue = 0;
cv->TimeColor.red = 0;
cv->TimeColor.green = 0;
cv->TimeColor.blue = 0;
cv->FormatColor.red = 0;
cv->FormatColor.green = 0;
cv->FormatColor.blue = 0;
cv->IdColor.red = 0;
cv->IdColor.green = 0;
cv->IdColor.blue = 0;
cv->DlcColor.red = 0;
cv->DlcColor.green = 0;
cv->DlcColor.blue = 0;
cv->Data1Color.red = 0;
cv->Data1Color.green = 0;
cv->Data1Color.blue = 0;
cv->Data2Color.red = 0;
cv->Data2Color.green = 0;
cv->Data2Color.blue = 0;

cv->Showing = SHOW_COL_DIR | SHOW_COL_TIME_SYS | SHOW_COL_TYPE | SHOW_COL_DLC | SHOW_COL_D1_HEX | SHOW_COL_D2_ASCII;
// **** Toplevel GtkVBox
widget = gtk_vbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(parent), widget, TRUE, TRUE, 0);
cv->BaseWidget = widget;
// **** View GtkTable
widget = gtk_table_new(1, 2, 0);
gtk_table_set_row_spacing(GTK_TABLE(widget), 0, 2);
gtk_table_set_col_spacing(GTK_TABLE(widget), 0, 2);
gtk_box_pack_start(GTK_BOX(cv->BaseWidget), widget, TRUE, TRUE, 0);
gtk_widget_show(widget);
cv->TabWidget = widget;
// **** View GtkFrame
widget2 = gtk_frame_new(NULL);
gtk_table_attach(GTK_TABLE(cv->TabWidget), widget2, 0, 1, 0, 1, GTK_EXPAND | GTK_FILL | GTK_SHRINK,
       GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);
gtk_frame_set_shadow_type(GTK_FRAME(widget2), GTK_SHADOW_IN);
gtk_widget_show(widget2);
// Columns & View GtkVBox
widget = gtk_vbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(widget2), widget);
gtk_widget_show(widget);
widget3 = widget;
// Column headings GtkHBox
widget = gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(widget3), widget, FALSE, FALSE, 0);
gtk_widget_show(widget);
cv->ToplevelWidget = widget;
// **** Dir
// Frame
widget = gtk_frame_new(NULL);
gtk_widget_set_usize(widget, -1, 22);
gtk_frame_set_shadow_type(GTK_FRAME(widget), GTK_SHADOW_OUT);
gtk_box_pack_start(GTK_BOX(cv->ToplevelWidget), widget, FALSE, FALSE, 0);
gtk_widget_show(widget);
cv->DirToplevelWidget = widget;
// HBox
widget2 = gtk_hbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(widget), widget2);
gtk_widget_show(widget2);
// Label
widget = gtk_label_new("Dir");
gtk_box_pack_start(GTK_BOX(widget2), widget, FALSE, FALSE, 0);
gtk_widget_show(widget);
cv->DirLabelWidget = GTK_LABEL(widget);
// **** Index
// Frame
widget = gtk_frame_new(NULL);
gtk_widget_set_usize(widget, -1, 22);
gtk_frame_set_shadow_type(GTK_FRAME(widget), GTK_SHADOW_OUT);
gtk_box_pack_start(GTK_BOX(cv->ToplevelWidget), widget, FALSE, FALSE, 0);
// gtk_widget_show(widget);   // Index wird nicht angezeigt
cv->IndexToplevelWidget = widget;
// HBox
widget2 = gtk_hbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(widget), widget2);
gtk_widget_show(widget2);
// Label
widget = gtk_label_new("Index");
gtk_box_pack_start(GTK_BOX(widget2), widget, FALSE, FALSE, 0);
gtk_widget_show(widget);
cv->IndexLabelWidget = GTK_LABEL(widget);
// **** Time Stamp
// Frame
widget = gtk_frame_new(NULL);
gtk_widget_set_usize(widget, -1, 22);
gtk_frame_set_shadow_type(GTK_FRAME(widget), GTK_SHADOW_OUT);
gtk_box_pack_start(GTK_BOX(cv->ToplevelWidget), widget, FALSE, FALSE, 0);
gtk_widget_show(widget);
cv->TimeToplevelWidget = widget;
// HBox
widget2 = gtk_hbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(widget), widget2);
gtk_widget_show(widget2);
// Label
widget = gtk_label_new("Time-Stamp");
gtk_box_pack_start(GTK_BOX(widget2), widget, FALSE, FALSE, 0);
gtk_widget_show(widget);
cv->TimeLabelWidget = GTK_LABEL(widget);
// **** Frame Format
// Frame
widget = gtk_frame_new(NULL);
gtk_widget_set_usize(widget, -1, 22);
gtk_frame_set_shadow_type(GTK_FRAME(widget), GTK_SHADOW_OUT);
gtk_box_pack_start(GTK_BOX(cv->ToplevelWidget), widget, FALSE, FALSE, 0);
gtk_widget_show(widget);
cv->FormatToplevelWidget = widget;
// HBox
widget2 = gtk_hbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(widget), widget2);
gtk_widget_show(widget2);
// Label
widget = gtk_label_new("Msg.-Type");
gtk_box_pack_start(GTK_BOX(widget2), widget, FALSE, FALSE, 0);
gtk_widget_show(widget);
cv->FormatLabelWidget = GTK_LABEL(widget);
// **** CAN ID
// Frame
widget = gtk_frame_new(NULL);
gtk_widget_set_usize(widget, -1, 22);
gtk_frame_set_shadow_type(GTK_FRAME(widget), GTK_SHADOW_OUT);
gtk_box_pack_start(GTK_BOX(cv->ToplevelWidget), widget, FALSE, FALSE, 0);
gtk_widget_show(widget);
cv->IdToplevelWidget = widget;
// HBox
widget2 = gtk_hbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(widget), widget2);
gtk_widget_show(widget2);
// Label
widget = gtk_label_new("Id");
gtk_box_pack_start(GTK_BOX(widget2), widget, FALSE, FALSE, 0);
gtk_widget_show(widget);
cv->IdLabelWidget = GTK_LABEL(widget);
// **** Dlc
// Frame
widget = gtk_frame_new(NULL);
gtk_widget_set_usize(widget, -1, 22);
gtk_frame_set_shadow_type(GTK_FRAME(widget), GTK_SHADOW_OUT);
gtk_box_pack_start(GTK_BOX(cv->ToplevelWidget), widget, FALSE, FALSE, 0);
gtk_widget_show(widget);
cv->DlcToplevelWidget = widget;
// HBox
widget2 = gtk_hbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(widget), widget2);
gtk_widget_show(widget2);
// Label
widget = gtk_label_new("DLC");
gtk_box_pack_start(GTK_BOX(widget2), widget, FALSE, FALSE, 0);
gtk_widget_show(widget);
cv->DlcLabelWidget = GTK_LABEL(widget);
// **** CAN Data 1
// Frame
widget = gtk_frame_new(NULL);
gtk_widget_set_usize(widget, -1, 22);
gtk_frame_set_shadow_type(GTK_FRAME(widget), GTK_SHADOW_OUT);
gtk_box_pack_start(GTK_BOX(cv->ToplevelWidget), widget, FALSE, FALSE, 0);
gtk_widget_show(widget);
cv->Data1ToplevelWidget = widget;
// HBox
widget2 = gtk_hbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(widget), widget2);
gtk_widget_show(widget2);
// Label
widget = gtk_label_new("Data (Hex)");
gtk_box_pack_start(GTK_BOX(widget2), widget, FALSE, FALSE, 0);
gtk_widget_show(widget);
cv->Data1LabelWidget = GTK_LABEL(widget);
// **** CAN Data ASCII
// Frame
widget = gtk_frame_new(NULL);
gtk_widget_set_usize(widget, -1, 22);
gtk_frame_set_shadow_type(GTK_FRAME(widget), GTK_SHADOW_OUT);
gtk_box_pack_start(GTK_BOX(cv->ToplevelWidget), widget, FALSE, FALSE, 0);

gtk_widget_show(widget);
cv->Data2ToplevelWidget = widget;
// HBox
widget2 = gtk_hbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(widget), widget2);
gtk_widget_show(widget2);
// Label
widget = gtk_label_new("Data (ASCII)");
gtk_box_pack_start(GTK_BOX(widget2), widget, FALSE, FALSE, 0);
gtk_widget_show(widget);
cv->Data2LabelWidget = GTK_LABEL(widget);
// **** Ende (Open-End )
// Frame
widget = gtk_frame_new(NULL);
gtk_widget_set_usize(widget, -1, 22);
gtk_frame_set_shadow_type(GTK_FRAME(widget), GTK_SHADOW_OUT);
gtk_box_pack_start(GTK_BOX(cv->ToplevelWidget), widget, TRUE, TRUE, 0);
gtk_widget_show(widget);
// View GtkDrawingArea
widget = gtk_drawing_area_new();
// GTK_WIDGET_SET_FLAGS(w, GTK_CAN_FOCUS);
gtk_widget_set_name(widget, "can-view");
gtk_signal_connect_after(GTK_OBJECT(widget), "configure_event", GTK_SIGNAL_FUNC(CViewConfigureCB), cv);
gtk_signal_connect_after(GTK_OBJECT(widget), "expose_event", GTK_SIGNAL_FUNC(CViewExposeCB), cv);
gtk_box_pack_start(GTK_BOX(widget3), widget, TRUE, TRUE, 0);
gtk_widget_realize(widget);
gtk_widget_show(widget);
cv->ViewDa = widget;
// Create graphics context for view drawing area
cv->GC = gdk_gc_new((GdkWindow *)GDK_ROOT_PARENT());
// Create the pango layout for the widget
cv->ViewLayout = gtk_widget_create_pango_layout(cv->ViewDa, "");
// Create adjustment for vertical scrolling
cv->VAdj = (GtkAdjustment *)gtk_adjustment_new(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
gtk_signal_connect(GTK_OBJECT(cv->VAdj), "value_changed", GTK_SIGNAL_FUNC(CViewValueChangedCB), cv);
// Vertical GtkScrollbar
widget = gtk_vscrollbar_new(cv->VAdj);
gtk_table_attach(GTK_TABLE(cv->TabWidget), widget, 1, 2, 0, 1, GTK_FILL, GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0);
gtk_widget_show(widget);
cv->VScrollbar = widget;
CViewSetViewFont(cv, DEFAULT_FONT);

gtk_widget_show(cv->BaseWidget);
return(cv);
}


int CViewAttachBuffer(struct TCanView *cv, struct TCanBuffer *cbuf)
{
if ((!cv) || (!cbuf))
  return(-1);
cv->Buffer = cbuf;
if (CBufferEventConnect(cbuf, BufferEventCB, (gpointer)cv) < 0)
  return(-1);
return(0);
}


struct TCViewDataHandler *CViewDataHandlerCreate(struct TCanView *cv, unsigned long used_size,
  TGetMsgCB get_msg_proc, TGetNameProc get_name_proc)
{
struct TCViewDataHandler *dh;

if (!cv)
  return(NULL);
if (!(dh = cv->DataHandler))
  {
  dh = calloc(sizeof(struct TCViewDataHandler), 1);
  cv->DataHandler = dh;
  }
if (dh)
  {
  dh->UsedSize = used_size;
  dh->GetMsgProc = get_msg_proc;
  dh->GetNameProc = get_name_proc;
  }
return(dh);
}


void CViewDataHandlerDestroy(struct TCanView *cv)
{
struct TCViewDataHandler *dh;

if (!cv)
  return;
if ((dh = cv->DataHandler))
  free(dh);
CViewSetScrollTo(cv, 1, 0);
CViewDraw(cv);
}


/*
********************  ********************
Funktion  :

Eingaben  : keine

Ausgaben  : keine

Call's    : keine
*/
/*int CViewDataSet(struct TCanView *cv, unsigned long index, struct TCanMsg *msgs, unsigned int count, int auto_scroll)
{
struct TCanBuffer *cd;
unsigned long end;

if ((!cv) || (!msgs) || (!count))
  return(-1);
if (!cv->Buffer)
  return(-1);
cd = cv->Buffer;
if (!cd->Data)
  return(-1);
end = index + count;
if (end > cd->BufferSize)
  return(-1);
if (end > cd->UsedSize)
  cd->UsedSize = end;
if (count)
  memcpy(&cd->Data[index], msgs, sizeof(struct TCanMsg) * count);
if (auto_scroll)
  CViewSetScrollTo(cv, 3, index);
else
  CViewSetScrollTo(cv, 0, 0);
CViewDraw(cv);
return(count);
} */


/*
********************  ********************
Funktion  :

Eingaben  : keine

Ausgaben  : keine

Call's    : keine
*/
/*int CViewDataSetInvalid(struct TCanView *cv, unsigned long index, unsigned int count)
{
struct TCanBuffer *cd;
unsigned long end;
struct TCanMsg *msg;

if (!cv)
  return(-1);
if (!cv->Buffer)
  return(-1);
cd = cv->Buffer;
if (!cd->Data)
  return(-1);
if (!count)
  count = cd->UsedSize;
end = index + count;
if (end > cd->BufferSize)
  return(-1);
if (end > cd->UsedSize)
  cd->UsedSize = end;
msg = &cv->Buffer->Data[index];
for (;count; count--)
  {
  msg->Id = 0xFFFFFFFFL;
  msg++;             // n�chste CAN Message
  }
CViewSetScrollTo(cv, 1, 0);
CViewDraw(cv);
return(0);
} */


/*
********************  ********************
Funktion  :

Eingaben  : keine

Ausgaben  : keine

Call's    : keine
*/
int CViewClearAll(struct TCanView *cv)
{
struct TCanBuffer *cbuf;

if (!cv)
  return(-1);
if ((cbuf = cv->Buffer))
  CBufferDataClear(cbuf);
return(0);
}


/*
********************  ********************
Funktion  :

Eingaben  : keine

Ausgaben  : keine

Call's    : keine
*/
int CViewGetUsedSize(struct TCanView *cv)
{
struct TCanBuffer *cbuf;

if (!cv)
  return(-1);
if (cv->DataHandler)
  return(cv->DataHandler->UsedSize);
if ((cbuf = cv->Buffer))
  return(CBufferGetUsedSize(cbuf));
return(0);
}


/*
********************  ********************
Funktion  :

Eingaben  : keine

Ausgaben  : keine

Call's    : keine
*/
int CViewSetUsedSize(struct TCanView *cv, unsigned long used_size)
{
if (!cv)
  return(-1);
if (cv->DataHandler)
  {
  cv->DataHandler->UsedSize = used_size;
  return(0);
  }
return(0);
}


/*
********************  ********************
Funktion  :

Eingaben  : keine

Ausgaben  : keine

Call's    : keine
*/
void CViewSetScrollTo(struct TCanView *cv, int mode, unsigned int pos)
{
unsigned int buf_size;
GtkAdjustment *adj;
GtkWidget *w;
gint height;

if (!cv)
  return;
adj = cv->VAdj;
if(!adj)
  return;
/* Update adjustment */
w = cv->ViewDa;

if (cv->Buffer)
  buf_size = CBufferGetUsedSize(cv->Buffer);
else if (cv->DataHandler)
  buf_size = cv->DataHandler->UsedSize;
else
  buf_size = 0;

if ((w != NULL) && (cv->CellHeight > 0))
  {
  height = w->allocation.height;

  adj->lower = 0.0f;
  adj->upper = (gfloat)((buf_size+1) * cv->CellHeight);
  adj->page_size = (gfloat)height;
  adj->step_increment = (gfloat)cv->CellHeight;
  adj->page_increment = adj->page_size / 2.0f;

  switch (mode)
    {
    case 0 : {  // Scroll Position nicht �ndern
             break;
             }
    case 1 : {  // Scroll Anfang
             adj->value = 0.0f;
             break;
             }
    case 2 : {  // Scroll Ende
             adj->value = adj->upper - adj->page_size;
             break;
             }
    case 3 : {
             if (!pos)
               break;
             if (pos >= buf_size)
               pos = buf_size;
             adj->value = (gfloat)((pos-1) * cv->CellHeight);
             }
    }
  if (adj->value > (adj->upper - adj->page_size))
    adj->value = adj->upper - adj->page_size;
  if (adj->value < adj->lower)
    adj->value = adj->lower;
  gtk_signal_emit_by_name(GTK_OBJECT(adj), "value_changed");
  gtk_signal_emit_by_name(GTK_OBJECT(adj), "changed");
  }
}


/*
********************  ********************
Funktion  :

Eingaben  : keine

Ausgaben  : keine

Call's    : keine
*/
void CViewSetViewFont(struct TCanView *cv, const gchar *font_name)
{
PangoFontDescription *font_desc;
PangoContext *context;
PangoFont *new_font;

if(!cv)
  return;
if (font_name != NULL)
  {
  // **** Pango
  if (cv->FontDesc)
    pango_font_description_free(cv->FontDesc);
  cv->FontDesc = NULL;
  if (cv->FontMetrics)
    pango_font_metrics_unref(cv->FontMetrics);
  cv->FontMetrics = NULL;
  font_desc = pango_font_description_from_string(font_name);
  if (!font_desc)
    return;
  cv->FontDesc = font_desc;
  // Modify the font for the widget
  gtk_widget_modify_font(cv->ViewDa, font_desc);
  context = gdk_pango_context_get();
  /* FIXME - Should get the locale language here */
  pango_context_set_language (context, gtk_get_default_language());
  new_font = pango_context_load_font(context, font_desc);
  if (new_font)
    {
    cv->FontMetrics = pango_font_get_metrics (new_font, pango_context_get_language (context));
    g_object_unref(G_OBJECT(new_font));
    }
  g_object_unref(G_OBJECT(context));
  if ((!font_desc) || (!cv->FontMetrics))
    return;
  CViewResizeColumns(cv);
  }
CViewQueueDraw(cv);
}


/*
********************  ********************
Funktion  :

Eingaben  : keine

Ausgaben  : keine

Call's    : keine
*/
void CViewSetColumnsColor(struct TCanView *cv, unsigned int colum, GdkColor *color)
{
if (!cv)
  return;
switch (colum)
  {
  case 0 : {
           memcpy(&cv->DirColor, color, sizeof(GdkColor));
           break;
           }
  case 1 : {
           memcpy(&cv->IndexColor, color, sizeof(GdkColor));
           break;
           }
  case 2 : {
           memcpy(&cv->TimeColor, color, sizeof(GdkColor));
           break;
           }
  case 3 : {
           memcpy(&cv->FormatColor, color, sizeof(GdkColor));
           break;
           }
  case 4 : {
           memcpy(&cv->IdColor, color, sizeof(GdkColor));
           break;
           }
  case 5 : {
           memcpy(&cv->DlcColor, color, sizeof(GdkColor));
           break;
           }
  case 6 : {
           memcpy(&cv->Data1Color, color, sizeof(GdkColor));
           break;
           }
  case 7 : {
           memcpy(&cv->Data2Color, color, sizeof(GdkColor));
           break;
           }
  }
gtk_widget_queue_draw(cv->ViewDa);
}


/*
********************  ********************
Funktion  :

Eingaben  : keine

Ausgaben  : keine

Call's    : keine
*/
void CViewSetShow(struct TCanView *cv, unsigned long showing)
{
const char *ptr;
union TCanViewShowing show;

if (!cv)
  return;
show.Long = showing;
if (show.View.Data2 == show.View.Data1)
  show.View.Data2 = 0;
cv->Showing = show.Long;
if (show.View.Cols & SHOW_COL_DIR)
  gtk_widget_show(cv->DirToplevelWidget);
else
  gtk_widget_hide(cv->DirToplevelWidget);
if (show.View.Cols & SHOW_COL_INDEX)
  gtk_widget_show(cv->IndexToplevelWidget);
else
  gtk_widget_hide(cv->IndexToplevelWidget);
if (show.View.Time)
  gtk_widget_show(cv->TimeToplevelWidget);
else
  gtk_widget_hide(cv->TimeToplevelWidget);
if (show.View.Cols & SHOW_COL_TYPE)
  gtk_widget_show(cv->FormatToplevelWidget);
else
  gtk_widget_hide(cv->FormatToplevelWidget);
if (show.View.Cols & SHOW_COL_DLC)
  gtk_widget_show(cv->DlcToplevelWidget);
else
  gtk_widget_hide(cv->DlcToplevelWidget);
if (show.View.Data1)
  {
  if (show.View.Data1 == SHOW_COL_D_ASCII)
    ptr = _(ASCII_LABEL_STRING);
  else if (show.View.Data1 == SHOW_COL_D_DEZIMAL)
    ptr = _(DEZIMAL_LABEL_STRING);
  else
    ptr = _(HEX_LABEL_STRING);
  gtk_label_set_text(cv->Data1LabelWidget, ptr);
  gtk_widget_show(cv->Data1ToplevelWidget);
  }
else
  gtk_widget_hide(cv->Data1ToplevelWidget);
if (show.View.Data2)
  {
  if (show.View.Data2 == SHOW_COL_D_ASCII)
    ptr = _(ASCII_LABEL_STRING);
  else if (show.View.Data2 == SHOW_COL_D_DEZIMAL)
    ptr = _(DEZIMAL_LABEL_STRING);
  else
    ptr = _(HEX_LABEL_STRING);
  gtk_label_set_text(cv->Data2LabelWidget, ptr);
  gtk_widget_show(cv->Data2ToplevelWidget);
  }
else
  gtk_widget_hide(cv->Data2ToplevelWidget);

CViewResizeColumns(cv);
CViewExposeCB(NULL, NULL, cv);
}


/*
********************  ********************
Funktion  :

Eingaben  : keine

Ausgaben  : keine

Call's    : keine
*/
void CViewDraw(struct TCanView *cv)
{
CViewExposeCB(NULL, NULL, cv);
}


/*
********************  ********************
Funktion  : Queues a draw

Eingaben  : keine

Ausgaben  : keine

Call's    : keine
*/
void CViewQueueDraw(struct TCanView *cv)
{
if(!cv)
  return;
gtk_widget_queue_draw(cv->ViewDa);
}



/* jaKa: this causes trouble to pango */
#if 0
#  define is_displayable(c) ((char_widths[(guchar)c])?1:0)
#else
#  define is_displayable(c) (((c) >= 0x20) && ((c) < 0x7f))
#endif /* 0 */

void CViewGetMaxCharWidth(struct TCanView *cv)
{
guint i;
guint maxwidth;
PangoRectangle logical_rect;
PangoLayout *layout;
gchar str[2];

layout = gtk_widget_create_pango_layout(cv->ViewDa, "");
pango_layout_set_font_description (layout, cv->FontDesc);
cv->CellHeight = PANGO_PIXELS(pango_font_metrics_get_ascent (cv->FontMetrics)) +
                 PANGO_PIXELS(pango_font_metrics_get_descent (cv->FontMetrics));
maxwidth = 0;
for(i = 1; i < 0x100; i++)
  {
  logical_rect.width = 0;
  /* Check if the char is displayable. Caused trouble to pango */
  if (is_displayable((guchar)i))
    {
    sprintf (str, "%c", (gchar)i);
    pango_layout_set_text(layout, str, -1);
    pango_layout_get_pixel_extents(layout, NULL, &logical_rect);
    }
  if ((guint)logical_rect.width > maxwidth)
    maxwidth = logical_rect.width;
  }
g_object_unref (G_OBJECT (layout));
cv->CellWidth = maxwidth;
}



/*
********************  ********************
Funktion  :

Eingaben  : keine

Ausgaben  : keine

Call's    : keine
*/
void CViewResizeColumns(struct TCanView *cv)
{
gint x_ofs;
gint column_width[8];
gint maxwidth, w;
union TCanViewShowing showing;

if (!cv)
  return;
CViewGetMaxCharWidth(cv);
showing.Long = cv->Showing;
maxwidth = cv->CellWidth;
x_ofs = 0;
// **** Dir
if (showing.View.Cols & SHOW_COL_DIR)
  {
  cv->DirX = x_ofs;
  w = maxwidth * DIR_COL_CHAR_SIZE;
  column_width[0] = w;
  x_ofs += w;
  }
// **** Index
if (showing.View.Cols & SHOW_COL_INDEX)
  {
  cv->IndexX = x_ofs;
  w = maxwidth * INDEX_COL_CHAR_SIZE;
  column_width[1] = w;
  x_ofs += w;
  }
// **** Time
if (showing.View.Time)
  {
  cv->TimeX = x_ofs;
  switch (showing.View.Time)
    {
    case 1 : { // System Time (Absolut)
             w = maxwidth * TIME1_COL_CHAR_SIZE;
             break;
             }
    case 2 : { // System Time (Relative)
             w = maxwidth * TIME1_COL_CHAR_SIZE;
             break;
             }
    case 3 : { // System Time (Absolut, ms)
             w = maxwidth * TIME2_COL_CHAR_SIZE;
             break;
             }
    case 4 : { // System Time (Relative, ms)
             w = maxwidth * TIME2_COL_CHAR_SIZE;
             break;
             }
    case 5 : { // HW Time Stamp (Absolute)
             w = maxwidth * TIME3_COL_CHAR_SIZE;
             break;
             }
    case 6 : { // HW Time Stamp (Relative)
             w = maxwidth * TIME3_COL_CHAR_SIZE;
             break;
             }
    default : w = maxwidth * TIME1_COL_CHAR_SIZE;
    }
  column_width[2] = w;
  x_ofs += w;
  }
// **** Format
if (showing.View.Cols & SHOW_COL_TYPE)
  {
  cv->FormatX = x_ofs;
  w = maxwidth * TYPE_COL_CHAR_SIZE;
  column_width[3] = w;
  x_ofs += w;
  }
// **** ID
cv->IdX = x_ofs;
w = maxwidth * ID_COL_CHAR_SIZE;
column_width[4] = w;
x_ofs += w;
// **** Dlc
if (showing.View.Cols & SHOW_COL_DLC)
  {
  cv->DlcX = x_ofs;
  w = maxwidth * DLC_COL_CHAR_SIZE;
  column_width[5] = w;
  x_ofs += w;
  }
if (showing.View.Data2 == showing.View.Data1)
  showing.View.Data2 = 0;
w = 0;
if (showing.View.Data1 == SHOW_COL_D_HEX)
  w = maxwidth * HEX_COL_CHAR_SIZE;
else if (showing.View.Data1 == SHOW_COL_D_ASCII)
  w = maxwidth * ASCII_COL_CHAR_SIZE;
else if (showing.View.Data1 == SHOW_COL_D_DEZIMAL)
  w = maxwidth * DEZIMAL_COL_CHAR_SIZE;
if (w)
  {
  // **** Hex Daten
  cv->Data1X = x_ofs;
  column_width[6] = w;
  x_ofs += w;
  }
w = 0;
if (showing.View.Data2 == SHOW_COL_D_HEX)
  w = maxwidth * HEX_COL_CHAR_SIZE;
else if (showing.View.Data2 == SHOW_COL_D_ASCII)
  w = maxwidth * ASCII_COL_CHAR_SIZE;
else if (showing.View.Data2 == SHOW_COL_D_DEZIMAL)
  w = maxwidth * DEZIMAL_COL_CHAR_SIZE;
if (w)
  {
  // **** Ascii/Dezimal Daten
  cv->Data2X = x_ofs;
  column_width[7] = w;
  x_ofs += w;
  }
// Update the column heading sizes
if (showing.View.Cols & SHOW_COL_DIR)
  gtk_widget_set_usize(cv->DirToplevelWidget, column_width[0], 22);
if (showing.View.Cols & SHOW_COL_INDEX)
  gtk_widget_set_usize(cv->IndexToplevelWidget, column_width[1], 22);
if (showing.View.Time)
  gtk_widget_set_usize(cv->TimeToplevelWidget, column_width[2], 22);
if (showing.View.Cols & SHOW_COL_TYPE)
  gtk_widget_set_usize(cv->FormatToplevelWidget, column_width[3], 22);
gtk_widget_set_usize(cv->IdToplevelWidget, column_width[4], 22);
if (showing.View.Cols & SHOW_COL_DLC)
  gtk_widget_set_usize(cv->DlcToplevelWidget, column_width[5], 22);
if (showing.View.Data1)
  gtk_widget_set_usize(cv->Data1ToplevelWidget, column_width[6], 22);
if (showing.View.Data2)
  gtk_widget_set_usize(cv->Data2ToplevelWidget, column_width[7], 22);
gtk_widget_queue_resize(cv->ToplevelWidget);
}



/**************************************************************/
/* Callback Funktionen                                        */
/**************************************************************/

static void BufferEventCB(struct TCanBuffer *cbuf, gpointer user_data, int last_event)
{
struct TCanView *cv;

if (!(cv = (struct TCanView *)user_data))
  return;
switch (last_event)
  {
  case CBUF_EVENT_CLEAR       :
  case CBUF_EVENT_LOAD        :
  case CBUF_EVENT_CHANGE_DATA : {
                                CViewSetScrollTo(cv, 1, 0);
                                break;
                                }
  case CBUF_EVENT_ADD_DATA    : {
                                if (cv->AutoScroll)
                                  CViewSetScrollTo(cv, 2, 0);
                                else
                                  CViewSetScrollTo(cv, 0, 0);
                                break;
                                }
  }
}

/*
 *	Hex View view GtkDrawingaArea "configure_event" signal callback.
 */
gint CViewConfigureCB(GtkWidget *widget, GdkEventConfigure *configure, gpointer data)
{
gint width, height;
unsigned int buf_size;
GtkAdjustment *adj;
GtkWidget *w;
struct TCanView *cv;

cv = (struct TCanView *)data;
if((widget == NULL) || (configure == NULL) || (cv == NULL))
    return(FALSE);
w = cv->ViewDa;
if(w == NULL)
  return(FALSE);
if (cv->Buffer)
  buf_size = CBufferGetUsedSize(cv->Buffer);
else if (cv->DataHandler)
  buf_size = cv->DataHandler->UsedSize;
else
  buf_size = 0;
width = configure->width;
height = configure->height;
// Update adjustments
adj = cv->VAdj;
if(adj != NULL)
  {
  adj->lower = 0.0f;
  adj->upper = (gfloat)((buf_size+1) * cv->CellHeight);
  adj->page_size = (gfloat)height;
  adj->step_increment = (gfloat)cv->CellHeight;
  adj->page_increment = adj->page_size / 2.0f;
  gtk_signal_emit_by_name(GTK_OBJECT(adj), "changed");

  if(adj->value > (adj->upper - adj->page_size))
    {
    adj->value = adj->upper - adj->page_size;
    if (adj->value < adj->lower)
      adj->value = adj->lower;
    gtk_signal_emit_by_name(GTK_OBJECT(adj), "value_changed");
    }
  }
return(TRUE);
}


/*
 *	Hex View view GtkDrawingArea "expose_event" callback.
 */
gint CViewExposeCB(GtkWidget *widget, GdkEventExpose *expose, gpointer data)
{
struct TCanView *cv;
struct TCanMsg *msg;
unsigned long msg_len;
gchar *ascii_ptr;
gchar *hex_ptr;
gchar *dez_ptr;
const char *tmp_ptr;
gchar buf1[50];
gchar buf2[50];
gint size;
gint width, height;
gint cell_width, cell_height;
GdkDrawable *window;
GdkGC *gc;
GtkAdjustment *adj;
GtkStateType state;
GtkStyle *style;
GtkWidget *w;
unsigned long idx;
gint i, x, y;
unsigned char hex;
unsigned char ch;
struct TTime t_diff, start_time;
guint32 t;
int valid;
union TCanViewShowing showing;
TGetMsgCB get_msg_proc;
TGetNameProc get_name_proc;

get_msg_proc = NULL;
get_name_proc = NULL;
cv = (struct TCanView *)data;
if (!cv)
  return(FALSE);
if (cv->Buffer)
  {
  msg = cv->Buffer->Data;
  size = cv->Buffer->UsedSize;
  }
else if (cv->DataHandler)
  {
  get_msg_proc = cv->DataHandler->GetMsgProc;
  get_name_proc = cv->DataHandler->GetNameProc;
  msg = NULL;
  size = cv->DataHandler->UsedSize;
  }
else
  {
  msg = NULL;
  size = 0;
  }
showing.Long = cv->Showing;
if ((!((msg) || (get_msg_proc))) || (!size))
  return(FALSE);
cell_width = cv->CellWidth;
cell_height = cv->CellHeight;
if((cell_width <= 0) || (cell_height <= 0))
  return(FALSE);
gc = cv->GC;
adj = cv->VAdj;
w = cv->ViewDa;
if((gc == NULL) || (adj == NULL) || (w == NULL))
  return(FALSE);
if(!GTK_WIDGET_VISIBLE(w))
  return(FALSE);
gdk_gc_set_function(gc, GDK_COPY);
gdk_gc_set_fill(gc, GDK_SOLID);

state = GTK_WIDGET_STATE(w);
style = gtk_widget_get_style(w);
window = w->window;
if((style == NULL) || (window == NULL))
  return(FALSE);

gdk_window_get_size(window, &width, &height);
if((width <= 0) || (height <= 0))
  return(FALSE);
// Draw base
// gdk_draw_rectangle(window, style->base_gc[state], TRUE, 0, 0, width, height);
// Draw text
if (cv->FontDesc)
  {
  idx = (gint)adj->value / cell_height;
  y = 0;
  // Set the starting positions
  if (idx < 0)
    idx = 0;
  if (idx >= (unsigned long)size-1)
    idx = size-1;
  if (get_msg_proc)
    {
    start_time.Sec = 0L;
    start_time.USec = 0L;
    msg = (get_msg_proc)(idx);
    }
  else
    {
    start_time.Sec = msg[0].Time.Sec;
    start_time.USec = msg[0].Time.USec;
    msg = &msg[idx];
    }
  if (!msg)
    return(FALSE);
  // Draw each line
  while((idx < (unsigned long)size) && (y < height))
    {
    if (msg->Id == 0xFFFFFFFFL)
      valid = 0;
    else
      valid = 1;
    msg_len = msg->MsgLen;
    if (showing.View.Cols & SHOW_COL_DIR)
      {
      // **** Index/Name schreiben
      x = cv->DirX;
      if (state == GTK_STATE_INSENSITIVE)
        gdk_gc_set_foreground(gc, &style->text[GTK_STATE_INSENSITIVE]);
      else
        gdk_gc_set_rgb_fg_color(gc, &cv->DlcColor);
      if (msg->MsgTxD)
        tmp_ptr = DIR_TX_STR;
      else
        tmp_ptr = DIR_RX_STR;
      pango_layout_set_text(cv->ViewLayout, tmp_ptr, -1);
      gdk_draw_layout(window, gc, x, y, cv->ViewLayout);
      }
    if (showing.View.Cols & SHOW_COL_INDEX)
      {
      // **** Index/Name schreiben
      x = cv->IndexX;
      if (state == GTK_STATE_INSENSITIVE)
        gdk_gc_set_foreground(gc, &style->text[GTK_STATE_INSENSITIVE]);
      else
        gdk_gc_set_rgb_fg_color(gc, &cv->IndexColor);
      ascii_ptr = NULL;
      if (get_name_proc)
        {
        ascii_ptr = (get_name_proc)(idx);
        if (ascii_ptr)
          pango_layout_set_text (cv->ViewLayout, ascii_ptr, -1);
        }
      else
        {
        g_snprintf(buf1, sizeof(buf1), "%07lu", idx);
        //g_snprintf(buf1, sizeof(buf1), "%07u", idx);
        pango_layout_set_text (cv->ViewLayout, buf1, -1);
        }
      gdk_draw_layout (window, gc, x, y, cv->ViewLayout);
      }
    if (showing.View.Time)
      {
      // **** Time-Stamp schreiben
      if (valid)
        {
        switch (showing.View.Time)
          {
          case 1 : { // System Time (Absolut)
                   g_snprintf(buf1, sizeof(buf1), "%10u.%-6u", (unsigned int)msg->Time.Sec, (unsigned int)msg->Time.USec);
                   break;
                   }
          case 2 : { // System Time (Relative)
                   diff_timestamp(&t_diff, &msg->Time, &start_time);
                   g_snprintf(buf1, sizeof(buf1), "%10u.%-6u", (unsigned int)t_diff.Sec, (unsigned int)t_diff.USec);
                   break;
                   }
          case 3 : { // System Time (Absolut, ms)
                   g_snprintf(buf1, sizeof(buf1), "%10u.%03u", (unsigned int)msg->Time.Sec, (unsigned int)(msg->Time.USec / 1000));
                   break;
                   }
          case 4 : { // System Time (Relative, ms)
                   diff_timestamp(&t_diff, &msg->Time, &start_time);
                   g_snprintf(buf1, sizeof(buf1), "%10u.%03u", (unsigned int)t_diff.Sec, (unsigned int)(t_diff.USec / 1000));
                   break;
                   }
          case 5 : { // HW Time Stamp (Absolute)
                   t = msg->Time.USec;
                   g_snprintf(buf1, sizeof(buf1), "%9u.%u", (t / 10), (t % 10));
                   break;
                   }
          case 6 : { // HW Time Stamp (Relative)
                   t = msg->Time.USec;
                   t = diff_time(t, start_time.USec);
                   g_snprintf(buf1, sizeof(buf1), "%9u.%u", (t / 10), (t % 10));
                   break;
                   }
          }
        }
      else
        strcpy(buf1, "-");
      x = cv->TimeX;
      if (state == GTK_STATE_INSENSITIVE)
        gdk_gc_set_foreground(gc, &style->text[GTK_STATE_INSENSITIVE]);
      else
        gdk_gc_set_rgb_fg_color(gc, &cv->TimeColor);
      pango_layout_set_text (cv->ViewLayout, buf1, -1);
      gdk_draw_layout (window, gc, x, y, cv->ViewLayout);
      }
    if (showing.View.Cols & SHOW_COL_TYPE)
      {
      if (valid)
        {
        // **** Message Format
        if ((msg->MsgRTR) && (msg->MsgEFF))
          tmp_ptr = FORMAT_EFF_RTR_STR;
        else if (msg->MsgEFF)
          tmp_ptr = FORMAT_EFF_STR;
        else if (msg->MsgRTR)
          tmp_ptr = FORMAT_STD_RTR_STR;
        else
          tmp_ptr = FORMAT_STD_STR;
        }
      else
        tmp_ptr = FORMAT_INVALID;
      x = cv->FormatX;
      if (state == GTK_STATE_INSENSITIVE)
        gdk_gc_set_foreground(gc, &style->text[GTK_STATE_INSENSITIVE]);
      else
        gdk_gc_set_rgb_fg_color(gc, &cv->FormatColor);

      pango_layout_set_text (cv->ViewLayout, tmp_ptr, -1);
      gdk_draw_layout(window, gc, x, y, cv->ViewLayout);
      }
    // **** CAN Id schreiben
    if (valid)
      {
      if (msg->MsgEFF)
        g_snprintf(buf1, sizeof(buf1), "%.8X", (unsigned int)msg->Id);
      else
        g_snprintf(buf1, sizeof(buf1), "     %.3X", (unsigned int)msg->Id);
      }
    else
      strcpy(buf1, "-");
    x = cv->IdX;
    if (state == GTK_STATE_INSENSITIVE)
      gdk_gc_set_foreground(gc, &style->text[GTK_STATE_INSENSITIVE]);
    else
      gdk_gc_set_rgb_fg_color(gc, &cv->IdColor);
    pango_layout_set_text(cv->ViewLayout, buf1, -1);
    gdk_draw_layout (window, gc, x, y, cv->ViewLayout);
    if (showing.View.Cols & SHOW_COL_DLC)
      {
      if (valid)
        g_snprintf(buf1, sizeof(buf1), "%1u", (unsigned int)msg_len);
      else
        strcpy(buf1, "-");
      // **** CAN Dlc schreiben
      x = cv->DlcX;
      if (state == GTK_STATE_INSENSITIVE)
        gdk_gc_set_foreground(gc, &style->text[GTK_STATE_INSENSITIVE]);
      else
        gdk_gc_set_rgb_fg_color(gc, &cv->DlcColor);
      pango_layout_set_text (cv->ViewLayout, buf1, -1);
      gdk_draw_layout (window, gc, x, y, cv->ViewLayout);
      }
    // **** CAN Daten schreiben
    if (msg->MsgRTR)
      msg_len = 0;

    ascii_ptr = NULL;
    hex_ptr = NULL;
    dez_ptr = NULL;
    if (showing.View.Data2 == showing.View.Data1)
      showing.View.Data2 = 0;
    if (showing.View.Data1 == SHOW_COL_D_HEX)
      hex_ptr = buf1;
    else if (showing.View.Data1 == SHOW_COL_D_ASCII)
      ascii_ptr = buf1;
    else if (showing.View.Data1 == SHOW_COL_D_DEZIMAL)
      dez_ptr = buf1;
    if (showing.View.Data2 == SHOW_COL_D_HEX)
      hex_ptr = buf2;
    else if (showing.View.Data2 == SHOW_COL_D_ASCII)
      ascii_ptr = buf2;
    else if (showing.View.Data2 == SHOW_COL_D_DEZIMAL)
      dez_ptr = buf2;
    for (i = 0; i < (int)msg_len; i++)
      {
      ch = msg->MsgData[i];
      if (ascii_ptr)
        {
        if ((ch <= 32) || (ch >= 126))
          *ascii_ptr++ = '.';
        else
          *ascii_ptr++ = (char)ch;
        }
      if (hex_ptr)
        {
        hex = ch >> 4;
        if (hex > 9)
          *hex_ptr++ = 55 + hex;
        else
          *hex_ptr++ = '0' + hex;
        hex = ch & 0x0F;
        if (hex > 9)
          *hex_ptr++ = 55 + hex;
        else
          *hex_ptr++ = '0' + hex;
        *hex_ptr++ = ' ';
        }
      if (dez_ptr)
        dez_ptr += g_snprintf(dez_ptr, 5, "%3u ", ch);
      }
    if (ascii_ptr)
      *ascii_ptr = '\0';
    if (hex_ptr)
      *hex_ptr = '\0';
    if (dez_ptr)
      *dez_ptr = '\0';
    if (showing.View.Data1)
      {
      if (!valid)
        strcpy(buf1, "-");
      x = cv->Data1X;
      if (state == GTK_STATE_INSENSITIVE)
        gdk_gc_set_foreground(gc, &style->text[GTK_STATE_INSENSITIVE]);
      else
        gdk_gc_set_rgb_fg_color(gc, &cv->Data1Color);
      pango_layout_set_text (cv->ViewLayout, buf1, -1);
      gdk_draw_layout (window, gc, x, y, cv->ViewLayout);
      }
    if (showing.View.Data2)
      {
      if (!valid)
        strcpy(buf2, "-");
      x = cv->Data2X;
      if (state == GTK_STATE_INSENSITIVE)
        gdk_gc_set_foreground(gc, &style->text[GTK_STATE_INSENSITIVE]);
      else
        gdk_gc_set_rgb_fg_color(gc, &cv->Data2Color);
      pango_layout_set_text (cv->ViewLayout, buf2, -1);
      gdk_draw_layout (window, gc, x, y, cv->ViewLayout);
      }
    y += cell_height;  // n�chste Zeile
    idx++;
    if (get_msg_proc)
      {
      msg = (get_msg_proc)(idx);
      if (!msg)
        break;
      }
    else
      msg++;             // n�chste CAN Message
    }
  }
return(TRUE);
}


/*
 *	Hex View GtkAdjustment "value_changed" callback.
 */
void CViewValueChangedCB(GtkAdjustment *adj, gpointer data)
{
CViewQueueDraw((struct TCanView *)data);
}






