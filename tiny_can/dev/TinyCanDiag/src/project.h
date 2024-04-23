#ifndef __PROJECT_H__
#define __PROJECT_H__

#include <glib.h>
#include "can_types.h"
#include "gui.h"
#include "hw_types.h"

#ifdef __cplusplus
  extern "C" {
#endif

void NewPrj(struct TMainWin *main_win, guint hardware_type);
void DestryPrj(struct TMainWin *main_win);
void LoadPrj(struct TMainWin *main_win, const gchar *filename);
void SavePrj(struct TMainWin *main_win, const gchar *filename);

#ifdef __cplusplus
  }
#endif

#endif
