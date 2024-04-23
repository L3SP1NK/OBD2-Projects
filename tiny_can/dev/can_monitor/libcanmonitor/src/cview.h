#ifndef __C_VIEW__
#define __C_VIEW__

#include <gtk/gtk.h>
#include <pango/pango.h>
#include "cbuf.h"

#define DEFAULT_FONT "Courier Medium 12"

#define SHOW_COL_LINE      0x00000001
#define SHOW_COL_INDEX     0x00000002
#define SHOW_COL_TYPE      0x00000004
#define SHOW_COL_DLC       0x00000008
#define SHOW_COL_DIR       0x00000010

// Time Stamp Formate
#define SHOW_COL_TIME_NONE       0x00000000 // 0 = Off
#define SHOW_COL_TIME_SYS        0x00000100 // 1 = System Time (Absolut)
#define SHOW_COL_TIME_SYS_REL    0x00000200 // 2 = System Time (Relative)
#define SHOW_COL_TIME_SYS_MS     0x00000300 // 3 = System Time (Absolut, ms)
#define SHOW_COL_TIME_SYS_REL_MS 0x00000400 // 4 = System Time (Relative, ms)
#define SHOW_COL_TIME_HW         0x00000500 // 5 = HW Time Stamp (Absolute)
#define SHOW_COL_TIME_HW_REL     0x00000600 // 6 = HW Time Stamp (Relative)

#define SHOW_COL_TIME_MASK       0x00000F00

#define SHOW_COL_D1_NONE    0x00000000
#define SHOW_COL_D1_HEX     0x00010000
#define SHOW_COL_D1_ASCII   0x00020000
#define SHOW_COL_D1_DEZIMAL 0x00030000

#define SHOW_COL_D2_NONE    0x00000000
#define SHOW_COL_D2_HEX     0x01000000
#define SHOW_COL_D2_ASCII   0x02000000
#define SHOW_COL_D2_DEZIMAL 0x03000000

#define SHOW_COL_D_NONE     0
#define SHOW_COL_D_HEX      1
#define SHOW_COL_D_ASCII    2
#define SHOW_COL_D_DEZIMAL  3


struct TCanViewShow
  {
  unsigned char Cols;
  unsigned char Time;
  unsigned char Data1;
  unsigned char Data2;
  };


union TCanViewShowing
  {
  unsigned long Long;
  unsigned char Bytes[4];
  struct TCanViewShow View;
  };

typedef struct TCanMsg *(*TGetMsgCB)(unsigned long index);
typedef char *(*TGetNameProc)(unsigned long index);

struct TCViewDataHandler
  {
  unsigned long UsedSize;
  TGetMsgCB GetMsgProc;
  TGetNameProc GetNameProc;
  };


struct TCanView
  {
  GtkWidget *BaseWidget;
  GtkWidget *TabWidget;
  GtkWidget *ToplevelWidget;
  GtkWidget *DirToplevelWidget;
  GtkWidget *IndexToplevelWidget;
  GtkWidget *TimeToplevelWidget;
  GtkWidget *FormatToplevelWidget;
  GtkWidget *IdToplevelWidget;
  GtkWidget *DlcToplevelWidget;
  GtkWidget *Data1ToplevelWidget;
  GtkWidget *Data2ToplevelWidget;

  GtkLabel *DirLabelWidget;
  GtkLabel *IndexLabelWidget;
  GtkLabel *TimeLabelWidget;
  GtkLabel *FormatLabelWidget;
  GtkLabel *IdLabelWidget;
  GtkLabel *DlcLabelWidget;
  GtkLabel *Data1LabelWidget;
  GtkLabel *Data2LabelWidget;

  GdkGC *GC;
  GtkAdjustment *VAdj;
  GtkWidget *VScrollbar;
  // CAN Puffer
  struct TCanBuffer *Buffer;
  struct TCViewDataHandler *DataHandler;
  int AutoScroll;
  // Drawing Area
  PangoFontMetrics *FontMetrics;
  PangoFontDescription *FontDesc;
  PangoLayout *ViewLayout;

  GtkWidget *ViewDa;
  gint CellWidth;
  gint CellHeight;
  // Spalten Positionen
  gint DirX;
  gint IndexX;
  gint TimeX;
  gint FormatX;
  gint IdX;
  gint DlcX;
  gint Data1X;
  gint Data2X;
  // Spalten anzeigen
  unsigned long Showing;
  // Farben
  GdkColor DirColor;
  GdkColor IndexColor;
  GdkColor TimeColor;
  GdkColor FormatColor;
  GdkColor IdColor;
  GdkColor DlcColor;
  GdkColor Data1Color;
  GdkColor Data2Color;
  };


struct TCanView *CViewNew(GtkWidget *parent);
int CViewAttachBuffer(struct TCanView *cv, struct TCanBuffer *cbuf);
struct TCViewDataHandler *CViewDataHandlerCreate(struct TCanView *cv, unsigned long used_size,
  TGetMsgCB get_msg_proc, TGetNameProc get_name_proc);
void CViewDataHandlerDestroy(struct TCanView *cv);
//struct TCanBuffer *CViewBufferCreate(struct TCanView *cv, unsigned int buffer_size);
//void CViewBufferDestroy(struct TCanView *cv);
//int CViewDataAdd(struct TCanView *cv, struct TCanMsg *msgs, unsigned int count, int auto_scroll);
//int CViewDataSet(struct TCanView *cv, unsigned long index, struct TCanMsg *msgs, unsigned int count, int auto_scroll);
//int CViewDataSetInvalid(struct TCanView *cv, unsigned long index, unsigned int count);
int CViewClearAll(struct TCanView *cv);
int CViewGetUsedSize(struct TCanView *cv);
int CViewSetUsedSize(struct TCanView *cv, unsigned long used_size);
int CViewGetBufferSize(struct TCanView *cv);
//char *CViewGetInfoString(struct TCanView *cv);
//void CViewSetInfoString(struct TCanView *cv, char *info_string);
//int CViewLoadFile(struct TCanView *cv, char *file_name);
//int CViewSaveFile(struct TCanView *cv, char *file_name);
void CViewSetScrollTo(struct TCanView *cv, int mode, unsigned int pos);
void CViewSetViewFont(struct TCanView *cv, const gchar *font_name);
void CViewSetColumnsColor(struct TCanView *cv, unsigned int colum, GdkColor *color);
void CViewSetShow(struct TCanView *cv, unsigned long showing);
void CViewDraw(struct TCanView *cv);
void CViewQueueDraw(struct TCanView *cv);

#endif
