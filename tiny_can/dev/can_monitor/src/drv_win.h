#ifndef __DRV_WIN_H__
#define __DRV_WIN_H__

#include <glib.h>
#include <gtk/gtk.h>


#define DRV_WIN_SHOW_PATH    0x0001
#define DRV_WIN_SHOW_ERRORS  0x0002

struct TDriverSetupWin
  {
  char *FilePath;
  char *SelectedName;
  struct TDriverList *DriverList;
  void(*EventCallback)(struct TDriverSetupWin *driver_win, int event);
  int Flags;
  GtkWidget *Base;
  GtkWidget *Label;
  GtkWidget *TreeView;
  GtkWidget *Expander;
  GtkWidget *Info;
  GtkWidget *PrefButton;
  };


struct TDriverSetupWin *DriverWinNew(char *drv_path, char *drv_filename, int flags);
void DriverWinDestroy(struct TDriverSetupWin *driver_win);

void SetDriverWinEventCallback(struct TDriverSetupWin *driver_win, void(*event)(struct TDriverSetupWin *driver_win, int event));
void DriverUpdateList(struct TDriverSetupWin *driver_win);

void DriverWinSetPathFile(struct TDriverSetupWin *driver_win, char *drv_path, char *drv_filename);
gchar *DriverWinGetFile(struct TDriverSetupWin *driver_win);
gchar *DriverWinGetPath(struct TDriverSetupWin *driver_win);

#endif
