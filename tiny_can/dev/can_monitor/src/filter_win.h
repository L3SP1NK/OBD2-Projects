#ifndef __FILTER_WIN_H__
#define __FILTER_WIN_H__

#include "can_types.h"
#include "filter.h"

/**************************************************************************/
/*                       D A T E N - T Y P E N                            */
/**************************************************************************/
struct TFilterWin
  {  
  GtkWidget *FlListView;
  struct TFilter *SelectedRow;
  //int CurrentRow;
  // MainWin 
  GtkWidget *MainWin;
  // Bottons
  GtkWidget *UpBtn;
  GtkWidget *DownBtn;
  GtkWidget *DeleteBtn;
  GtkWidget *NewBtn;
  GtkWidget *CloseButton;

  GtkWidget *NameEdit;
  //GtkWidget *EnableEdit;
  GtkWidget *HardwareFilterEdit;
  GtkWidget *PassMessageEdit;
  GtkWidget *FilterFrame;
  GtkWidget *IdModeEdit;
  GtkWidget *EffEdit;
  GtkWidget *Id1Label;
  GtkWidget *Id2Label;
  GtkWidget *Id1EditBase;
  GtkWidget *Id2EditBase;
  struct THexEntry *Id1Edit;
  struct THexEntry *Id2Edit;
  };


/**************************************************************************/
/*                         V A R I A B L E N                              */
/**************************************************************************/
extern struct TFilterWin FilterWin;


/**************************************************************************/
/*                        F U N K T I O N E N                             */
/**************************************************************************/
void FlWinExecute(void);
void FlWinCreateTable(void);
void FlWinRepaintTable(void);



#endif
