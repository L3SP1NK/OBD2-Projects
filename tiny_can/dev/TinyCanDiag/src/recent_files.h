#ifndef __RECENT_FILES_H__
#define __RECENT_FILES_H__

#include "gui.h"

#ifdef __cplusplus
  extern "C" {
#endif

void ClearRecentFileList(struct TMainWin *main_win);
void UpdateRecentItems(struct TMainWin *main_win);
void RecentFileAddToList(struct TMainWin *main_win, const gchar *filename);
void RecentFileLoadList(struct TMainWin *main_win, ConfigFile *cfgfile);
void RecentFileSaveAndDestroyList(struct TMainWin *main_win, ConfigFile *cfgfile);


#ifdef __cplusplus
  }
#endif

#endif
