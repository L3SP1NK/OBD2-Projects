#ifndef __CAN_CORE_H__
#define __CAN_CORE_H__

#include "global.h"
#include "mhs_obj.h"

#ifdef __cplusplus
  extern "C" {
#endif  

extern const uint8_t DlcLenTab[] ATTRIBUTE_INTERNAL;

void can_core_init(void) ATTRIBUTE_INTERNAL;
int32_t can_core_create(void) ATTRIBUTE_INTERNAL;
void can_core_destroy(void) ATTRIBUTE_INTERNAL;
TMhsObjContext *can_core_get_context(void) ATTRIBUTE_INTERNAL;
TMhsObjContext *can_filter_get_context(void) ATTRIBUTE_INTERNAL;
TMhsObjContext *can_main_get_context(void) ATTRIBUTE_INTERNAL;
TMhsObjContext *can_get_context(uint32_t index) ATTRIBUTE_INTERNAL;

uint8_t LenToDlc(uint8_t len) ATTRIBUTE_INTERNAL;
int CanFdCanCmp(struct TCanFdMsg *fd_msg, const struct TCanMsg *msg) ATTRIBUTE_INTERNAL; 

#ifdef __cplusplus
  }
#endif

#endif
