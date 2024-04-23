#ifndef __MAIN_SAMPLES_H__
#define __MAIN_SAMPLES_H__

#include <glib.h>
#include "can_types.h"
#include "gui.h"

#ifdef __cplusplus
  extern "C" {
#endif

void fill_sample_projects(struct TMainWin *main_win, GtkWidget *box);
void sample_projects_destroy(void);

#ifdef __cplusplus
  }
#endif

#endif
