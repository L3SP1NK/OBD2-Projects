#ifndef __INDEX_H__
#define __INDEX_H__

#include <global.h>
//#include "tcan_drv.h" <*>

#ifdef __cplusplus
  extern "C" {
#endif

#define get_can_channel(x)           (((x) >> 16) & 0x0000000F)

#define convert_idx_to_device_idx(x) ((x) << 20) & 0xF00000

#define convert_device_idx_to_idx(x) ((x) >> 20) & 0x00000F

#define get_device(x) DeviceList[((x) >> 20) & 0x00000F]

#define clear_device_from_list(x) DeviceList[((x) >> 20) & 0x00000F] = NULL

extern TCanDevice *DeviceList[16] ATTRIBUTE_INTERNAL;

void index_init(void) ATTRIBUTE_INTERNAL;
void index_create(void) ATTRIBUTE_INTERNAL;
void index_destroy(void) ATTRIBUTE_INTERNAL;
uint32_t create_new_device_index(TCanDevice *device) ATTRIBUTE_INTERNAL;

TCanDevice *get_device_and_ref(uint32_t index) ATTRIBUTE_INTERNAL;
void device_unref(TCanDevice *dev) ATTRIBUTE_INTERNAL;
TCanDevice *device_create(void) ATTRIBUTE_INTERNAL;
int32_t device_destroy(uint32_t index, uint32_t block) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
