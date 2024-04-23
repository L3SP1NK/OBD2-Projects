#ifndef __TCAN_COM_H__
#define __TCAN_COM_H__

#include "global.h"
//#include "tcan_drv.h" <*>
#include "tar_drv.h"

#ifdef __cplusplus
  extern "C" {
#endif

void SetTarStatusToDev(TCanDevice *dev, struct TTarStatus tar_status) ATTRIBUTE_INTERNAL;

int32_t MainCommProc(TCanDevice *dev) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
