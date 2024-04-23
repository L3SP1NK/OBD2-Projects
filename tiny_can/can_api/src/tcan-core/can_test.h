#ifndef __CAN_TEST_H__
#define __CAN_TEST_H__

#include "global.h"
#include "com_io.h"
#include "sm_drv.h"
//#include "tcan_drv.h" <*>

#ifdef __cplusplus
  extern "C" {
#endif

int32_t TAR2CanTestExecute(TCanDevice *dev, uint8_t enable);

#ifdef __cplusplus
  }
#endif

#endif