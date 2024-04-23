#ifndef __ABOUT_DLG_H__
#define __ABOUT_DLG_H__

#include <gtk/gtk.h>

struct TAboutTxt
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

void ShowAboutDlg(gchar *file_path, const struct TAboutTxt *about_txt);

#endif
