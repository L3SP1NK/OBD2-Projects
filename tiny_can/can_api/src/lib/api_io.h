#ifndef __API_IO_H__
#define __API_IO_H__

#include "global.h"
//#include "mhs_user_event.h"

#ifdef __cplusplus
  extern "C" {
#endif

EXPORT_FUNC int32_t CALLBACK_TYPE CanExIoOpen(uint32_t index, const char *name, uint32_t *io_id);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExIoClose(uint32_t io_id);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExIoWrite(uint32_t io_id, void *data, uint32_t *size, uint32_t timeout);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExIoRead(uint32_t io_id, void *data, uint32_t *size, uint32_t timeout);

#ifdef __cplusplus
  }
#endif

#endif