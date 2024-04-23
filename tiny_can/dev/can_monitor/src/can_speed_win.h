#ifndef __CAN_SPEED_H__
#define __CAN_SPEED_H__

#include <glib.h>
#include <gtk/gtk.h>

struct TCanSpeed
  {
  int Mode;
  unsigned long CanSpeed;
  unsigned long UserCanSpeed;
  int CanOpMode;
  };

struct TCanSpeedWin
  {
  GtkWidget *Base;
  GtkWidget *ComboBox;
  int Mode;
  unsigned long CanSpeed;
  unsigned long UserCanSpeed;
  GSList *DatenListe_group;
  GtkWidget *DatenListe[3];
  };


struct TCanSpeedWin *CanSpeedWinNew(struct TCanSpeed *can_speed);
void CanSpeedWinGet(struct TCanSpeedWin *can_speed_win, struct TCanSpeed *can_speed);
void CanSpeedWinDestroy(struct TCanSpeedWin *can_speed_win);

#endif
