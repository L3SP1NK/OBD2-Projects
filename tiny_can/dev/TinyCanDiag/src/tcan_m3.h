#ifndef __TCAN_M3_H__
#define __TCAN_M3_H__

#include <glib.h>
#include "can_types.h"

#ifdef __cplusplus
  extern "C" {
#endif

struct TCanHw *TCanM3Create(void);
void TCanM3Destroy(struct TCanHw *hw);

#ifdef __cplusplus
  }
#endif

#endif