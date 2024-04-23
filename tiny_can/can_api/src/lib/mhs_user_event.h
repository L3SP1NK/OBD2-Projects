#ifndef __MHS_USER_EVENT_H__
#define __MHS_USER_EVENT_H__

#include "global.h"
#include "mhs_event.h"

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct _TMhsUserEvent TMhsUserEvent;

struct _TMhsUserEvent
  {
  TMhsEvent Event;
  TMhsUserEvent *Next;
  };


void mhs_user_event_init(void) ATTRIBUTE_INTERNAL;
TMhsEvent *mhs_user_event_create(void) ATTRIBUTE_INTERNAL;
void mhs_user_event_destroy(void) ATTRIBUTE_INTERNAL;
void mhs_user_event_set_all(uint32_t events) ATTRIBUTE_INTERNAL;
int32_t mhs_user_event_valid(TMhsEvent *event_obj) ATTRIBUTE_INTERNAL; 

#ifdef __cplusplus
  }
#endif

#endif
