#ifndef __PATHS_H__
#define __PATHS_H__

#include <glib.h>

struct TPaths
  {
  char *base_dir;
  char *setup_dir;
  char *plugin_dir;
  char *locale_dir;
  };

extern struct TPaths Paths;


void PathsInit(char *prog);
void PathsFree(void);


#endif
