#ifndef __SPLASH_H__
#define __SPLASH_H__

#include <glib.h>

struct TSplashData
  {
  gchar *Grafik;
  //gchar *StartText;
  unsigned long ShowTime;
  };

void SplashStart(gchar *file_path, const struct TSplashData *splash_data, char *message);
void SplashUpdate(char *message);
void SplashStop(void);

#endif
