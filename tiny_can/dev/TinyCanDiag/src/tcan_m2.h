#ifndef __TCAN_M2_H__
#define __TCAN_M2_H__

#include <glib.h>
#include "can_types.h"

#ifdef __cplusplus
  extern "C" {
#endif

struct TCanHw *TCanM2Create(void);
void TCanM2Destroy(struct TCanHw *hw);

#ifdef __cplusplus
  }
#endif

#endif