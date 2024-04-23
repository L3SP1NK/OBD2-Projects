#ifndef __CAN_FIFO_H__
#define __CAN_FIFO_H__

#include "global.h"
#include "can_types.h"
#include "can_core.h"
#include "can_obj.h"
#include "mhs_obj.h"

#ifdef __cplusplus
  extern "C" {
#endif

#define CANMSG_BUF_EMPTY   0x00  /* fifo is empty */
#define CANMSG_BUF_PEND    0x01  /* message(s) pending */
#define CANMSG_BUF_FULL    0x02  /* fifo is full */
#define CANMSG_BUF_OVR     0x04  /* fifo overrun */


typedef struct _TObjCanFifo TObjCanFifo;

struct _TObjCanFifo
  {
  TObjCan Obj;

  struct TCanFdMsg *fd_base;
  struct TCanMsg *base;
  uint32_t bufsize;
  uint32_t head;
  uint32_t tail;
  volatile uint32_t status;
  volatile uint32_t overrun;
  };


#define mhs_can_fifo_out_fd_finish_by_index   mhs_can_fifo_out_finish_by_index
#define mhs_can_fifo_get_obj_by_index(i) (TObjCanFifo *)(mhs_object_get_by_index((i), can_core_get_context())) 

TObjCanFifo *mhs_can_fifo_create_unlocked(uint32_t index, const char *name, int32_t size, uint8_t fd, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
TObjCanFifo *mhs_can_fifo_create(uint32_t index, const char *name, int32_t size, uint8_t fd) ATTRIBUTE_INTERNAL;
void mhs_can_fifo_destry(uint32_t index) ATTRIBUTE_INTERNAL;

int32_t mhs_can_fifo_clear_unlocked(TObjCanFifo *fifo) ATTRIBUTE_INTERNAL;
int32_t mhs_can_fifo_clear(TObjCanFifo *fifo) ATTRIBUTE_INTERNAL;
int32_t mhs_can_fifo_clear_by_index(uint32_t index) ATTRIBUTE_INTERNAL;

void mhs_can_fifo_ovverrun_clear(TObjCanFifo *fifo) ATTRIBUTE_INTERNAL;
void mhs_can_fifo_overrun_clear_by_index(uint32_t index) ATTRIBUTE_INTERNAL;
void mhs_can_fifo_overrun_clear(TObjCanFifo *fifo) ATTRIBUTE_INTERNAL;

int32_t mhs_can_fifo_count(TObjCanFifo *fifo) ATTRIBUTE_INTERNAL;
int32_t mhs_can_fifo_count_by_index(uint32_t index) ATTRIBUTE_INTERNAL;

int32_t mhs_can_fifo_get(TObjCanFifo *fifo, struct TCanMsg *msg_buf, int32_t count) ATTRIBUTE_INTERNAL;
int32_t mhs_can_fifo_get_by_index(uint32_t index, struct TCanMsg *msg_buf, int32_t count) ATTRIBUTE_INTERNAL;

int32_t mhs_can_fifo_put_unlocked(TObjCanFifo *fifo, struct TCanMsg *msg_buf, int32_t count) ATTRIBUTE_INTERNAL;
int32_t mhs_can_fifo_put(TObjCanFifo *fifo, struct TCanMsg *msg_buf, int32_t count) ATTRIBUTE_INTERNAL;

// *** CAN-FD
int32_t mhs_can_fifo_get_fd(TObjCanFifo *fifo, struct TCanFdMsg *msg_buf, int32_t count) ATTRIBUTE_INTERNAL;
int32_t mhs_can_fifo_get_fd_by_index(uint32_t index, struct TCanFdMsg *msg_buf, int32_t count) ATTRIBUTE_INTERNAL;

int32_t mhs_can_fifo_put_fd_unlocked(TObjCanFifo *fifo, struct TCanFdMsg *msg_buf, int32_t count) ATTRIBUTE_INTERNAL;
int32_t mhs_can_fifo_put_fd(TObjCanFifo *fifo, struct TCanFdMsg *msg_buf, int32_t count) ATTRIBUTE_INTERNAL;

// Nachfolgende Funktionen werden von socketcan verwendent
int32_t mhs_can_fifo_out_ref_by_index(uint32_t index, struct TCanMsg **msg_ref) ATTRIBUTE_INTERNAL;
int32_t mhs_can_fifo_out_finish_by_index(uint32_t index) ATTRIBUTE_INTERNAL;

int32_t mhs_can_fifo_out_fd_ref_by_index(uint32_t index, struct TCanFdMsg **msg_ref) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
