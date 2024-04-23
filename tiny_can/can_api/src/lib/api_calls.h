#ifndef __API_CALLS_H__
#define __API_CALLS_H__

#include "global.h"

#ifdef __cplusplus
  extern "C" {
#endif

void CanDownDriverInternel(void) ATTRIBUTE_INTERNAL;

EXPORT_FUNC int32_t CALLBACK_TYPE CanInitDriver(char *options);
EXPORT_FUNC void CALLBACK_TYPE CanDownDriver(void);

EXPORT_FUNC int32_t CALLBACK_TYPE CanSetOptions(char *options);
EXPORT_FUNC char * CALLBACK_TYPE CanDrvInfo(void);
EXPORT_FUNC char * CALLBACK_TYPE CanDrvHwInfo(uint32_t index);
EXPORT_FUNC int32_t CALLBACK_TYPE CanGetDeviceStatus(uint32_t index, struct TDeviceStatus *status);
EXPORT_FUNC int32_t CALLBACK_TYPE CanDeviceOpen(uint32_t index, char *parameter);
EXPORT_FUNC int32_t CALLBACK_TYPE CanDeviceClose(uint32_t index);
EXPORT_FUNC int32_t CALLBACK_TYPE CanApplaySettings(uint32_t index);
EXPORT_FUNC int32_t CALLBACK_TYPE CanSetMode(uint32_t index, unsigned char can_op_mode, uint16_t can_command);
EXPORT_FUNC int32_t CALLBACK_TYPE CanSet(uint32_t index, uint16_t obj_index, uint16_t obj_sub_index,
  void *data, int32_t size);
EXPORT_FUNC int32_t CALLBACK_TYPE CanGet(uint32_t index, uint16_t obj_index, uint16_t obj_sub_index,
  void *data, int32_t size);
EXPORT_FUNC int32_t CALLBACK_TYPE CanTransmit(uint32_t index, struct TCanMsg *msg, int32_t count);
EXPORT_FUNC void CALLBACK_TYPE CanTransmitClear(uint32_t index);
EXPORT_FUNC uint32_t CALLBACK_TYPE CanTransmitGetCount(uint32_t index);
EXPORT_FUNC int32_t CALLBACK_TYPE CanTransmitSet(uint32_t index, uint16_t cmd,
  uint32_t time);
EXPORT_FUNC int32_t CALLBACK_TYPE CanReceive(uint32_t index, struct TCanMsg *msg, int32_t count);
EXPORT_FUNC void CALLBACK_TYPE CanReceiveClear(uint32_t index);
EXPORT_FUNC uint32_t CALLBACK_TYPE CanReceiveGetCount(uint32_t index);
EXPORT_FUNC int32_t CALLBACK_TYPE CanSetSpeed(uint32_t index, uint16_t speed);
EXPORT_FUNC int32_t CALLBACK_TYPE CanSetSpeedUser(uint32_t index, uint32_t value);
EXPORT_FUNC int32_t CALLBACK_TYPE CanSetFilter(uint32_t index, struct TMsgFilter *msg_filter);

#ifdef __cplusplus
  }
#endif

#endif
