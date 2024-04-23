#ifndef __ABAUT_DLG_H__
#define __ABAUT_DLG_H__

#include <gtk/gtk.h>

#ifdef __cplusplus
  extern "C" {
#endif

struct TAbautTxt
  {
  char *Programm;
  char *LogoFile;
  char *Version;
  char *Copyright;
  char *Autor;
  char *Email;
  char *Homepage;
  char *ShortLizenz;
  char *LizenzFile;
  };

void ShowAbautDlg(gchar *file_path, const struct TAbautTxt *abaut_txt);

#ifdef __cplusplus
  }
#endif

#endif
