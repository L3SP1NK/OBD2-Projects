#ifndef __MAKRO_WIN_H__
#define __MAKRO_WIN_H__

#include "can_types.h"
#include "makro.h"

enum 
  {
  MAKRO_COLUMN_SHOW = 0,     
  MAKRO_COLUMN_NAME,   
  MAKRO_COLUMN_SHOW_EN,
  MAKRO_COLUMN_MAKRO,  
  MAKRO_COLUMN_NUMBER
  };

/**************************************************************************/
/*                       D A T E N - T Y P E N                            */
/**************************************************************************/
struct TMakroTableWidget
  {
  GtkWidget *Base;
  unsigned int Flags;
  struct TMakro *SelectedRow;
  void (*EventProc)(struct TMakroTableWidget *mtw, int event, struct TMakro *makro);
  };

struct TMakroWin
  {  
  struct TMakroTableWidget *MakListView;      
  int Show;
  // MainWin
  GtkWidget *MainWin;
  // Bottons
  GtkWidget *SendBtn;
  GtkWidget *UpBtn;
  GtkWidget *DownBtn;
  GtkWidget *DeleteBtn;
  GtkWidget *NewBtn;
  GtkWidget *CopyBtn;

  GtkWidget *MakroListView;
  GtkWidget *MakInsertBtn;
  GtkWidget *CanTxWindow;

  GtkWidget *NameEdit;    
  GtkWidget *IntervallFrame;
  GtkWidget *IntervallEnableEdit;
  GtkWidget *IntervallTimeEdit;
  GtkObject *IntervallTimeEdit_adj;
  GtkWidget *MakSetIntervallBtn;
  GtkWidget *MakCloseBtn;

  struct TCanTxWidget *CanMessageWdg;
  };

typedef void (*TMakroTableEventCB)(struct TMakroTableWidget *mtw, int event, struct TMakro *makro);

/**************************************************************************/
/*                         V A R I A B L E N                              */
/**************************************************************************/
extern struct TMakroWin *MakroWin;


/**************************************************************************/
/*                        F U N K T I O N E N                             */
/**************************************************************************/
struct TMakroWin *MakWinCreate(void);
void MakWinDestroy(void);
void MakWinShow(void);
void MakWinHide(void);
void UpdateMakWin(guint select_mode);

struct TMakroTableWidget *MakWinCreateTable(GtkWidget *parent, unsigned int flags, TMakroTableEventCB event_proc);
void MakWinDestroyTable(struct TMakroTableWidget *mtw);
void MakWinRepaintTable(struct TMakroTableWidget *mtw);
void MakSelectMakro(struct TMakroTableWidget *mtw, struct TMakro *makro);
void MakRepaintMakro(struct TMakroTableWidget *mtw, struct TMakro *makro);

#endif
