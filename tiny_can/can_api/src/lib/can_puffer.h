#ifndef __CAN_PUFFER_H__
#define __CAN_PUFFER_H__

#include "global.h"
#include "can_types.h"
#include "mhs_obj.h"
#include "can_obj.h"

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct _TObjCanPuffer TObjCanPuffer;

struct _TObjCanPuffer
  {
  TObjCan Obj;

  struct TCanFdMsg FdMsg;
  uint32_t Intervall;
  volatile int32_t Update;
  };


TObjCanPuffer *mhs_can_puffer_create_unlocked(uint32_t obj_class, uint32_t index, const char *name, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
TObjCanPuffer *mhs_can_puffer_create(uint32_t obj_class, uint32_t index, const char *name, TMhsObjContext *context) ATTRIBUTE_INTERNAL;

int32_t mhs_can_puffer_clear(TObjCanPuffer *puffer) ATTRIBUTE_INTERNAL;
int32_t mhs_can_puffer_clear_by_index(uint32_t index, TMhsObjContext *context) ATTRIBUTE_INTERNAL;

int32_t mhs_can_puffer_put_fd_unlocked(TObjCanPuffer *puffer, struct TCanFdMsg *msg) ATTRIBUTE_INTERNAL;
int32_t mhs_can_puffer_put_fd(TObjCanPuffer *puffer, struct TCanFdMsg *msg) ATTRIBUTE_INTERNAL;
int32_t mhs_can_puffer_put_fd_by_index(uint32_t index, struct TCanFdMsg *msg, TMhsObjContext *context) ATTRIBUTE_INTERNAL;

int32_t mhs_can_puffer_get_fd(TObjCanPuffer *puffer, struct TCanFdMsg *fd_msg) ATTRIBUTE_INTERNAL;
int32_t mhs_can_puffer_get_fd_by_index(uint32_t index, struct TCanFdMsg *fd_msg, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_can_puffer_get_fd_copy(TObjCanPuffer *puffer, struct TCanFdMsg *fd_msg) ATTRIBUTE_INTERNAL;

int32_t mhs_can_puffer_put_unlocked(TObjCanPuffer *puffer, struct TCanMsg *msg) ATTRIBUTE_INTERNAL;
int32_t mhs_can_puffer_put(TObjCanPuffer *puffer, struct TCanMsg *msg) ATTRIBUTE_INTERNAL;
int32_t mhs_can_puffer_put_by_index(uint32_t index, struct TCanMsg *msg, TMhsObjContext *context) ATTRIBUTE_INTERNAL;

int32_t mhs_can_puffer_get(TObjCanPuffer *puffer, struct TCanMsg *msg) ATTRIBUTE_INTERNAL;
int32_t mhs_can_puffer_get_by_index(uint32_t index, struct TCanMsg *msg, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_can_puffer_get_copy(TObjCanPuffer *puffer, struct TCanMsg *msg) ATTRIBUTE_INTERNAL;

int32_t mhs_can_puffer_count(TObjCanPuffer *puffer) ATTRIBUTE_INTERNAL;
uint32_t mhs_can_puffer_get_intervall(TObjCanPuffer *puffer) ATTRIBUTE_INTERNAL;
void mhs_can_puffer_set_intervall(TObjCanPuffer *puffer, uint32_t intervall) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
