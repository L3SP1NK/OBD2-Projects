#ifndef __MHS_CMD_H__
#define __MHS_CMD_H__

#include "global.h"
//#include "tcan_drv.h" <*>

#ifdef __cplusplus
  extern "C" {
#endif

TMhsCmd *mhs_cmd_create(void) ATTRIBUTE_INTERNAL;
void mhs_cmd_destroy(TMhsCmd **cmd) ATTRIBUTE_INTERNAL;

int32_t mhs_cmd_execute(TCanDevice *dev, uint32_t command, void *param, uint32_t param_size, int32_t timeout) ATTRIBUTE_INTERNAL;
void mhs_cmd_finish(TCanDevice *dev, int32_t result) ATTRIBUTE_INTERNAL;
void mhs_cmd_exit_thread(TCanDevice *dev) ATTRIBUTE_INTERNAL;
void mhs_cmd_clear(TCanDevice *dev) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
