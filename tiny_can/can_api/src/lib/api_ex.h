#ifndef __API_EX_H__
#define __API_EX_H__

#include "global.h"
#include "mhs_user_event.h"

#ifdef __cplusplus
  extern "C" {
#endif

extern struct TCanDevicesList *LocalDeviceList ATTRIBUTE_INTERNAL;
extern struct TCanInfoVar *LocalHwInfo ATTRIBUTE_INTERNAL;
extern struct TCanInfoVarList *LocalDevicesInfo ATTRIBUTE_INTERNAL;

EXPORT_FUNC int32_t CALLBACK_TYPE CanExInitDriver(char *options);

EXPORT_FUNC int32_t CALLBACK_TYPE CanExCreateDevice(uint32_t *index, char *options);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExDestroyDevice(uint32_t *index);

EXPORT_FUNC int32_t CALLBACK_TYPE CanExCreateFifo(uint32_t index, uint32_t size, TMhsEvent *event_obj, uint32_t event, uint32_t channels);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExBindFifo(uint32_t fifo_index, uint32_t device_index, uint32_t bind);

EXPORT_FUNC TMhsEvent * CALLBACK_TYPE CanExCreateEvent(void);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExSetObjEvent(uint32_t index, uint32_t source, TMhsEvent *event_obj, uint32_t event);
EXPORT_FUNC void CALLBACK_TYPE CanExSetEvent(TMhsEvent *event_obj, uint32_t event);
EXPORT_FUNC void CALLBACK_TYPE CanExSetEventAll(uint32_t event);
EXPORT_FUNC void CALLBACK_TYPE CanExResetEvent(TMhsEvent *event_obj, uint32_t event);
EXPORT_FUNC uint32_t CALLBACK_TYPE CanExWaitForEvent(TMhsEvent *event_obj, uint32_t timeout);

EXPORT_FUNC int32_t CALLBACK_TYPE CanExGetDeviceCount(int32_t flags);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExGetDeviceList(struct TCanDevicesList **devices_list, int32_t flags);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExGetDeviceListPerform(int32_t flags);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExGetDeviceListGet(struct TCanDevicesList *item);

EXPORT_FUNC int32_t CALLBACK_TYPE CanExGetDeviceInfo(uint32_t index, struct TCanDeviceInfo *device_info,
  struct TCanInfoVar **hw_info, uint32_t *hw_info_size);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExGetDeviceInfoPerform(uint32_t index, struct TCanDeviceInfo *device_info);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExGetDeviceInfoGet(struct TCanInfoVar *item);  
EXPORT_FUNC void CALLBACK_TYPE CanExDataFree(void **data);

EXPORT_FUNC int32_t CALLBACK_TYPE CanExSetOptions(uint32_t index, const char *options);

EXPORT_FUNC int32_t CALLBACK_TYPE CanExSetAsByte(uint32_t index, const char *name, char value);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExSetAsWord(uint32_t index, const char *name, int16_t value);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExSetAsLong(uint32_t index, const char *name, int32_t value);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExSetAsUByte(uint32_t index, const char *name, unsigned char value);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExSetAsUWord(uint32_t index, const char *name, uint16_t value);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExSetAsULong(uint32_t index, const char *name, uint32_t value);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExSetAsString(uint32_t index, const char *name, char *value);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExGetAsByte(uint32_t index, const char *name, char *value);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExGetAsWord(uint32_t index, const char *name, int16_t *value);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExGetAsLong(uint32_t index, const char *name, int32_t *value);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExGetAsUByte(uint32_t index, const char *name, unsigned char *value);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExGetAsUWord(uint32_t index, const char *name, uint16_t *value);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExGetAsULong(uint32_t index, const char *name, uint32_t *value);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExGetAsString(uint32_t index, const char *name, char **str);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExGetAsStringCopy(uint32_t index, const char *name, char *dest, uint32_t *dest_size);

EXPORT_FUNC int32_t CALLBACK_TYPE CanExSetAsData(uint32_t index, const char *name, void *data, uint32_t size);  // <*> neu
EXPORT_FUNC int32_t CALLBACK_TYPE CanExSetDataPtr(uint32_t index, const char *name, void *data_ptr, uint32_t size);  // <*> neu
EXPORT_FUNC int32_t CALLBACK_TYPE CanExSetDataBlockCB(uint32_t index, TCanGetDataBlockCB func, void *user_data);  // <*> neu

EXPORT_FUNC int32_t CALLBACK_TYPE CanExGetInfoList(uint32_t dev_idx, const char *name, struct TCanInfoVarList **devices_info, int32_t flags); // <*> neu
EXPORT_FUNC int32_t CALLBACK_TYPE CanExGetInfoListPerform(uint32_t dev_idx,  const char *name, int32_t flags);
EXPORT_FUNC int32_t CALLBACK_TYPE CanExGetInfoListGet(uint32_t list_idx, struct TCanInfoVar *item);

#ifdef HAVE_TX_ACK_BYPASS
void TxAckBypassInit(void);
#endif

#ifdef __cplusplus
  }
#endif

#endif
