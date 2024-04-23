#ifndef __API_FD_H__
#define __API_FD_H__

#include "global.h"
#include "can_types.h"

#ifdef __cplusplus
  extern "C" {
#endif

EXPORT_FUNC int32_t CALLBACK_TYPE CanFdTransmit(uint32_t index, struct TCanFdMsg *fd_msg, int32_t count);
EXPORT_FUNC int32_t CALLBACK_TYPE CanFdReceive(uint32_t index, struct TCanFdMsg *fd_msg, int32_t count);

#ifdef __cplusplus
  }
#endif

#endif
