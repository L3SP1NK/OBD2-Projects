#ifndef __FD_SPEEDS_H__
#define __FD_SPEEDS_H__

#include "global.h"

#ifdef __cplusplus
  extern "C" {
#endif
  
int32_t GetNbtrDBtrValueFromSpeeds(TCanDevice *dev, uint8_t *clk_idx, uint32_t *nbtr, uint32_t *dbtr, uint16_t speed, uint16_t fd_speed);

#ifdef __cplusplus
  }
#endif

#endif