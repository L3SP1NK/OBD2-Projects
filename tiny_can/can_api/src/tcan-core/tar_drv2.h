#ifndef __TAR2_DRV_H__
#define __TAR2_DRV_H__

#include "global.h"
#include "com_io.h"
#include "sm_drv.h"
//#include "tcan_drv.h" <*>

#ifdef __cplusplus
  extern "C" {
#endif

/***************************************/
/*            Target Status            */
/***************************************/
struct TTar2DevFlags
  {
  unsigned PowerUp:1;
  unsigned Res:6;
  };      
  
struct TTar2CanStatusFlags
  {
  unsigned BusStatus:2;
  unsigned BusFailure:1;
  unsigned RxDPend:1;
  unsigned RxDOv:1;
  unsigned TxDStatus:2;
  unsigned TxDError:1;
  };

union TTar2CanStatus
  {
  unsigned char Byte;
  struct TTar2CanStatusFlags Flag;
  };

union TTar2DevStatus
  {
  unsigned char Byte;
  struct TTar2DevFlags Flag;
  };

struct TTar2Status
  {
  union TTar2DevStatus DevStatus;
  union TTar2CanStatus CanStatus[8];
  }; 

int32_t TAR2SetCANMode(TCanDevice *dev, uint8_t channel, uint8_t can_mode) ATTRIBUTE_INTERNAL;
int32_t TAR2CanSetSpeed(TCanDevice *dev, uint8_t channel, uint32_t speed) ATTRIBUTE_INTERNAL;
int32_t TAR2CanSetBtr(TCanDevice *dev, uint8_t channel, uint32_t btr) ATTRIBUTE_INTERNAL;
//int32_t TAR2CanSetDBtr(TCanDevice *dev, uint8_t channel, uint32_t d_btr) ATTRIBUTE_INTERNAL; <*>
int32_t TAR2CanSetClkNbtrDBtr(TCanDevice *dev, uint8_t channel, uint8_t set_can_clk, uint8_t clk_idx,
    uint32_t nbtr, uint32_t dbtr) ATTRIBUTE_INTERNAL;
int32_t TAR2CanSetFdFlags(TCanDevice *dev, uint8_t channel, uint8_t fd_flags) ATTRIBUTE_INTERNAL;
int32_t TAR2CanSetTxAckEnable(TCanDevice *dev, uint8_t channel, uint8_t enable) ATTRIBUTE_INTERNAL;
int32_t TAR2CanSetErrorLogEnable(TCanDevice *dev, uint8_t channel, uint8_t enable) ATTRIBUTE_INTERNAL;
int32_t TAR2GetStatus(TCanDevice *dev) ATTRIBUTE_INTERNAL;
int32_t TAR2SetFilter(TCanDevice *dev, uint8_t channel, uint32_t index, struct TMsgFilter *msg_filter) ATTRIBUTE_INTERNAL;
int32_t TAR2CanSetIntervall(TCanDevice *dev, uint8_t channel, uint32_t index, uint32_t intervall) ATTRIBUTE_INTERNAL;
int32_t TAR2RxFifoClear(TCanDevice *dev, uint8_t channel, uint32_t index) ATTRIBUTE_INTERNAL;
int32_t TAR2TxFifoClear(TCanDevice *dev, uint8_t channel, uint32_t index) ATTRIBUTE_INTERNAL;
int32_t TAR2SetTimeStamp(TCanDevice *dev, uint8_t enabled) ATTRIBUTE_INTERNAL;
int32_t TAR2SetFifoOvMode(TCanDevice *dev, uint8_t ov_mode) ATTRIBUTE_INTERNAL;

int32_t TAR2SetIoConfig(TCanDevice *dev, uint8_t index, uint32_t config) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
