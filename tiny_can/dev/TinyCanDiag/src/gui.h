#ifndef __GUI_H__
#define __GUI_H__

#include <glib.h>
#include "can_types.h"
#include "hw_types.h"

#ifdef __cplusplus
  extern "C" {
#endif

#define PROG_INIT         0
#define PROG_WELCOMME     1
#define PROG_OFFLINE      2 // Verbindung noch nicht hergestellt
#define PROG_ERROR        3 // Tiny-CAN kann nicht geöffnet werden
#define PROG_GO_ONLINE    4 // Verbindung wird aufgebaut
#define PROG_RUN          5 // System RUN

#define START_PAGE_INDEX  0
#define APP_PAGE_INDEX    1

#define WINDOW_REFRESH_TIME 50

struct TMainWin
  {
  gchar *PrjFileName;
  guint RefreshTimerId;
  gint ProgMode;

  GtkWidget *MNeu;
  GtkWidget *MSave;
  GtkWidget *MDevice;
  GtkWidget *MConfig;
  GtkWidget *RecentFilesMenu;
  guint MConfigSigId;
  GtkWidget *Statusbar;

  GtkWidget *NeuButton;
  GtkWidget *FileOpenButton;
  GtkWidget *FileSaveButton;
  GtkWidget *ConnectButton;
  guint ConnectButtonSigId;
  GtkWidget *InfoButton;
  GtkWidget *BeendenButton;
  GtkWidget *MainWin;
  GtkWidget *MainToolbar;

  GtkWidget *Notebook;
  GtkWidget *AppContainer;
  GtkWidget *AppPage;

  GtkWidget *FileBox;
  };


void OpenProjectFile(struct TMainWin *main_win);
void DestroyMainWinData(struct TMainWin *main_win);
void UpdateMain(struct TMainWin *main_win);
void SetProgMode(struct TMainWin *main_win, gint prog_mode);
struct TMainWin *CreateMainWin(void);

#ifdef __cplusplus
  }
#endif

#endif
