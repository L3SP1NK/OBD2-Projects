#ifndef __CAN_WIDGET_H__
#define __CAN_WIDGET_H__

#include "can_types.h"          
   
#define CAN_TX_WDG_END_SEPERATROR     0x0001
#define CAN_TX_WDG_SEND_TOOLBAR       0x0002
#define CAN_TX_WDG_SEND_BTN           0x0004

#define CAN_TX_WDG_DISPLAY_ENABLE     0x0010
#define CAN_TX_WDG_EDIT_MODE          0x1000
#define CAN_TX_WDG_ENABLE_RETURN_SEND 0x2000
#define CAN_TX_EDIT_FINISH_EVENT      0x4000

// Events
#define CAN_TX_ENTRY_SEND_BTN    0x0001
#define CAN_TX_ENTRY_RETURN      0x0002
#define CAN_TX_ENTRY_EDIT_FINISH 0x8000

struct TCanTxWidget
  {
  struct TCanMsg CanMsg;  
  char CanMakroName[100];
  int EditMode;
  int EnableReturnSend;
  int EditFinishEvent;

  gulong EffCheckSig;
  gulong RtrCheckSig;
  gulong DlcEntrySig;

  GtkWidget *BaseWdg;
  GtkWidget *MainDisplayFrame;
  GtkWidget *MainEditFrame;
  GtkWidget *EditBtn;
  GtkWidget *DisplayFrame;  
  GtkWidget *BezeichnerLabel;
  GtkWidget *DisplayLabel;
  GtkWidget *DispSendBtn;

  GtkWidget *EffCheckBtn;
  GtkWidget *RtrCheckBtn;
  GtkWidget *IdLabel;
  struct THexEntry *IdEntry;
  GtkWidget *DlcLabel;
  GtkObject *DlcEntryAdj;
  GtkWidget *DlcEntry;
  GtkWidget *DatenLabel;
  struct THexEntry *CanDataEntry[8];
  GtkWidget *Toolbar;
  GtkWidget *SendBtn;
  void (*EventProc)(struct TCanTxWidget *ctw, int event, struct TCanMsg *can_msg);
  //TCanTxWidgetEventCB EventProc;
  };


typedef void (*TCanTxWidgetEventCB)(struct TCanTxWidget *ctw, int event, struct TCanMsg *can_msg);


struct TCanTxWidget *CanTxWidgetNew(GtkWidget *parent, unsigned int flags, TCanTxWidgetEventCB event_proc);
void CanTxWidgetDestroy(struct TCanTxWidget *ctw);
void CanTxWidgetSetMsg(struct TCanTxWidget *ctw, struct TCanMsg *can_msg);
void CanTxWidgetGetMsg(struct TCanTxWidget *ctw, struct TCanMsg *can_msg);
void CanTxSetEditMode(struct TCanTxWidget *ctw, int edit_mode);
void CanTxSetReturnAutoSend(struct TCanTxWidget *ctw, int mode);

#endif
