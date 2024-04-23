#ifndef __OPEN_EXT_H__
#define __OPEN_EXT_H__

#include <glib.h>
#include <gtk/gtk.h>
#include "can_types.h"

#ifdef __cplusplus
  extern "C" {
#endif

gboolean OpenExternel(const gchar *url_file);

#ifdef __cplusplus
  }
#endif

#endif

