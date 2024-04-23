#ifndef __USB_HLP_H__
#define __USB_HLP_H__

#include "global.h"  // Definition von ATTRIBUTE_INTERNAL
//#include "tcan_drv.h" <*>

#ifdef __cplusplus
  extern "C" {
#endif


#define PNP_DEVICE_UNPLUGED 0
#define PNP_DEVICE_PLUGED   1
#define PNP_DEVICE_OFFLINE  2
#define PNP_DEVICE_ONLINE   3

struct TTCanUsbList
  {
  struct TTCanUsbList *Next;
  char *DeviceName[4];
  char *Description;
  char *Serial;
  uint32_t DeviceType;
  uint32_t TCanIdx;
  const struct TCanModulDesc *Modul;
  int32_t Status;
  int32_t LastSignaledStatus;
  };

extern struct TTCanUsbList *TCanUsbList ATTRIBUTE_INTERNAL;


void UsbHlpInit(void) ATTRIBUTE_INTERNAL;

void DestroyTCanUsbList(struct TTCanUsbList **list) ATTRIBUTE_INTERNAL;


struct TTCanUsbList *AddTCanUsbDevice(struct TTCanUsbList **list, char *devicename[], char *description, char *serial, uint32_t type, uint32_t idx, const struct TCanModulDesc *modul) ATTRIBUTE_INTERNAL;

struct TTCanUsbList *GetUsbDevice(char *snr) ATTRIBUTE_INTERNAL;

int32_t GetDeviceCount(int32_t flags) ATTRIBUTE_INTERNAL;

int32_t GetDeviceList(struct TCanDevicesList **devices_list, int32_t flags) ATTRIBUTE_INTERNAL;

int32_t GetDevicePnPStatus(void) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
