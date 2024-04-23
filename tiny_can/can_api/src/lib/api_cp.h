#ifndef __API_CP_H__
#define __API_CP_H__

#include "global.h"
#include "mhs_thread.h"

#ifdef __cplusplus
  extern "C" {
#endif


void ApiCpInit(void) ATTRIBUTE_INTERNAL;

EXPORT_FUNC void CALLBACK_TYPE CanSetPnPEventCallback(void (CALLBACK_TYPE *event)(uint32_t index, int32_t status));
EXPORT_FUNC void CALLBACK_TYPE CanSetStatusEventCallback(void (CALLBACK_TYPE *event)
  (uint32_t index, struct TDeviceStatus *DeviceStatus));
EXPORT_FUNC void CALLBACK_TYPE CanSetRxEventCallback(void (CALLBACK_TYPE *event)
  (uint32_t index, struct TCanMsg *msg, int32_t count));
EXPORT_FUNC void CALLBACK_TYPE CanSetEvents(uint16_t events);
EXPORT_FUNC uint32_t CALLBACK_TYPE CanEventStatus(void);

TMhsThread *GetApiCpTheread(void) ATTRIBUTE_INTERNAL;

TMhsThread *CreateApiCp(uint32_t priority, uint32_t min_sleep_time) ATTRIBUTE_INTERNAL;
int32_t RunApiCp(TMhsThread *thread, TCanDevice *dev) ATTRIBUTE_INTERNAL;
TMhsThread *ExCreateApiCp(uint32_t priority, uint32_t min_sleep_time) ATTRIBUTE_INTERNAL;
void DestoryApiCp(void) ATTRIBUTE_INTERNAL;
int32_t ApiCpCreateRxDBuffer(uint32_t size) ATTRIBUTE_INTERNAL;


#ifdef __cplusplus
  }
#endif

#endif
