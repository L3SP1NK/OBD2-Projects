#ifndef __MHS_EVENT_LIST_H__
#define __MHS_EVENT_LIST_H__

#include "global.h"
#include "mhs_event.h"

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct _TMhsEventItem TMhsEventItem;

struct _TMhsEventItem
  {
  TMhsEventItem *Next;
  TMhsEvent *Event;
  uint32_t Events;
  };
  
typedef struct _TMhsEventList TMhsEventList;


struct _TMhsEventList
  {
  TMhsEventItem *Items;
  DRV_LOCK_TYPE *Lock;
  };  


TMhsEventList *MhsEventListCreate(void) ATTRIBUTE_INTERNAL;
void MhsEventListDestroy(TMhsEventList **event_list) ATTRIBUTE_INTERNAL;

void MhsEventListAdd(TMhsEventList *event_list, TMhsEvent *event, uint32_t events) ATTRIBUTE_INTERNAL;
void MhsEventListDelete(TMhsEventList *event_list, TMhsEvent *event) ATTRIBUTE_INTERNAL;
void MhsEventListAddDelete(TMhsEventList *event_list, TMhsEvent *event, uint32_t events) ATTRIBUTE_INTERNAL;
void MhsEventListExecute(TMhsEventList *event_list) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
