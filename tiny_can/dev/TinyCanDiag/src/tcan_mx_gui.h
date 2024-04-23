#ifndef __TCAN_MX_GUI_H__
#define __TCAN_MX_GUI_H__

#include <glib.h>
#include "can_types.h"
#include "hw_types.h"

#ifdef __cplusplus
  extern "C" {
#endif

GtkWidget *TCanMxGuiCreate(guint hardware_type);
//void TCanMxGuiDestroy(GtkWidget *widget);
void AddKeyToList(struct TCanHw *hw, uint32_t key);

#ifdef __cplusplus
  }
#endif

#endif