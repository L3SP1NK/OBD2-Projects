#ifndef __PATHS_H__
#define __PATHS_H__

#include <glib.h>

struct TPaths
  {
  gchar *base_dir;
  gchar *setup_dir;
  gchar *doc_dir;
  gchar *samples_dir;
  };

extern struct TPaths Paths;

gchar *CreateDocFileName(const gchar *doc_file);
void PathsInit(char *prog);
void PathsFree(void);


#endif
