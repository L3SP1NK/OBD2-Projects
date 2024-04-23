#ifndef __CAN_RX_OBJ_H__
#define __CAN_RX_OBJ_H__

#include "global.h"
#include "can_types.h"
#include "mhs_obj.h"

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct _TObjCan TObjCan;

struct _TObjCan
  {
  TMhsObj Obj;
  uint32_t Channels;
  struct TMsgFilter Filter;
  };


uint32_t index_to_channels(uint32_t index) ATTRIBUTE_INTERNAL;
int32_t process_rx_msg(uint32_t index, struct TCanFdMsg *fd_msg) ATTRIBUTE_INTERNAL;

int32_t can_put_msgs_api(uint32_t index, struct TCanMsg *msg, int32_t count) ATTRIBUTE_INTERNAL;
int32_t can_get_msgs_api(uint32_t index, struct TCanMsg *msg, int32_t count) ATTRIBUTE_INTERNAL;
int32_t can_clear_msgs(uint32_t index) ATTRIBUTE_INTERNAL;
int32_t can_get_msg_count(uint32_t index) ATTRIBUTE_INTERNAL;
int32_t can_set_filter(uint32_t index, struct TMsgFilter *msg_filter) ATTRIBUTE_INTERNAL;
void can_sw_filter_remove_all(void) ATTRIBUTE_INTERNAL;

int32_t can_put_fd_msgs_api(uint32_t index, struct TCanFdMsg *fd_msg, int32_t count) ATTRIBUTE_INTERNAL;
int32_t can_get_fd_msgs_api(uint32_t index, struct TCanFdMsg *fd_msg, int32_t count) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif

