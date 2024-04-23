#ifndef __USB_SCAN_H__
#define __USB_SCAN_H__

//#include <global.h>

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct _TUsbDevList TUsbDevList;

struct _TUsbDevList
  {
  TUsbDevList *Next;
  unsigned short idProduct;
  unsigned short idVendor;
  unsigned short bcdDevice;
  char *DeviceName[4];
  char *Manufacturer;
  char *Product;
  char *SerialNumber;
  uint32_t User;
  };


void UsbDevListDestroy(TUsbDevList **list) ATTRIBUTE_INTERNAL;
TUsbDevList *UsbScan(void) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
