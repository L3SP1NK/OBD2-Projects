#ifndef __MHS_OBJ__
#define __MHS_OBJ__

#include "global.h"
#include "mhs_event.h"
#include "mhs_class.h"

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct _TMhsObjContext TMhsObjContext;
typedef struct _TMhsObj TMhsObj;
typedef struct _TMhsObjHandler TMhsObjHandler;

struct TObjFuncs
  {
  void(*DestroyCB)(TMhsObj *obj);
  void(*Event)(TMhsObj *obj);
  void(*UserCB)(TMhsObj *obj, TMhsObjHandler *handler);
  };


struct _TMhsObj
  {
  TMhsObj *Next;
  TMhsObj *MarkedNext;
#ifdef MHS_OBJ_USE_INDEX
  uint32_t Index;
#endif
  char *Name;
  char *Alias;
  uint32_t Class;
  TMhsObjContext *Context;
  int32_t MarkedFlag;
  int32_t EventCount;
  int32_t CbEnable;
  const struct TObjFuncs *Funcs;
  TMhsObjHandler *SyncHandlerList;
  TMhsObjHandler *HandlerList;
#ifdef MHS_OBJ_USE_EVENTS
  TMhsEvent *Event;
  uint32_t EventFlags;
  // ** User Data
  void *UserPtr;
  uint32_t UserUInt;
#endif
  // Commando Events
  TMhsObj *CmdMarkedNext;
  int32_t CmdMarkedFlag;
  int32_t CmdEventCount;
  TMhsObjHandler *CmdSyncHandlerList;
  TMhsObjHandler *CmdHandlerList;
  };

struct _TMhsObjContext
  {
  TMhsObjContext *Next;
  char *Name;
  TMhsObj *Items;
  TMhsObj *MarkedFirst;
  TMhsObj *MarkedLast;
  int32_t DeleteItems;
  TMhsEvent *Event;
  // Commando Events
  TMhsObj *CmdMarkedFirst;
  TMhsObj *CmdMarkedLast;
  int32_t CmdDeleteItems;
  };

typedef void(*TMhsObjectCB)(TMhsObj *obj, void *user_data);

struct _TMhsObjHandler
  {
  TMhsObjHandler *Next;
  TMhsObjectCB Proc;
  void *UserData;
  };

//typedef void(*TMhsUserEventCB)(struct TMhsObj *obj, struct TMhsObjHandler *handler);

void mhs_object_init(void) ATTRIBUTE_INTERNAL;
#ifdef MHS_OBJ_USE_CONTEXT_LIST
void mhs_object_destroy(void) ATTRIBUTE_INTERNAL;
#else
//void mhs_object_destroy(TMhsObjContext *context) ATTRIBUTE_INTERNAL;
void mhs_obj_context_destroy(TMhsObjContext **context) ATTRIBUTE_INTERNAL;
#endif

#ifdef MHS_OBJ_USE_CONTEXT_LIST
TMhsObjContext *mhs_obj_context_auto_new(const char *name, TMhsEvent *event) ATTRIBUTE_INTERNAL;
TMhsObjContext *mhs_obj_context_get_by_name(const char *name) ATTRIBUTE_INTERNAL;
#endif
TMhsObjContext *mhs_obj_context_new(const char *name, TMhsEvent *event) ATTRIBUTE_INTERNAL;
TMhsObjContext *mhs_obj_context_default(void) ATTRIBUTE_INTERNAL;

#ifdef MHS_OBJ_USE_INDEX
TMhsObj *mhs_object_new_unlocked(uint32_t obj_class, uint32_t index, const char *name, const struct TObjFuncs *funcs, int32_t struct_size, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
TMhsObj *mhs_object_new(uint32_t obj_class, uint32_t index, const char *name, const struct TObjFuncs *funcs, int32_t struct_size, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
TMhsObj *mhs_object_get_by_index_unlocked(uint32_t index, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
TMhsObj *mhs_object_get_by_index(uint32_t index, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
#else
TMhsObj *mhs_object_new_unlocked(uint32_t obj_class, const char *name, const struct TObjFuncs *funcs, int32_t struct_size, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
TMhsObj *mhs_object_new(uint32_t obj_class, const char *name, const struct TObjFuncs *funcs, int32_t struct_size, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
#endif

TMhsObj *mhs_object_get_by_name_unlocked(const char *name, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
TMhsObj *mhs_object_get_by_name(const char *name, TMhsObjContext *context) ATTRIBUTE_INTERNAL;

void mhs_all_objects_destroy(TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_object_remove(TMhsObj *obj) ATTRIBUTE_INTERNAL;
#ifdef MHS_OBJ_USE_INDEX
int32_t mhs_obj_remove_by_index(uint32_t, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
#endif
int32_t mhs_obj_remove_by_name(const char *name, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_object_valid(TMhsObj *obj) ATTRIBUTE_INTERNAL;
int32_t mhs_object_valid_unlocked(TMhsObj *obj) ATTRIBUTE_INTERNAL;
int32_t mhs_object_set_event_unlocked(TMhsObj *obj) ATTRIBUTE_INTERNAL;
int32_t mhs_object_set_event(TMhsObj *obj) ATTRIBUTE_INTERNAL;
int32_t mhs_object_event_block(TMhsObj *obj) ATTRIBUTE_INTERNAL;
int32_t mhs_object_event_unblock(TMhsObj *obj) ATTRIBUTE_INTERNAL;
int32_t mhs_object_set_user_data(TMhsObj *obj, void *user_ptr, uint32_t user_uint) ATTRIBUTE_INTERNAL;
void mhs_object_reset_event(TMhsObj *obj) ATTRIBUTE_INTERNAL;
int32_t mhs_object_get_event_count(TMhsObj *obj) ATTRIBUTE_INTERNAL;
#ifdef MHS_OBJ_USE_EVENTS
int32_t mhs_object_events_set(TMhsObj *obj, TMhsEvent *event, uint32_t events) ATTRIBUTE_INTERNAL;
#endif
int32_t mhs_object_event_connect(TMhsObj *obj, int32_t type, TMhsObjectCB proc, void *user_data) ATTRIBUTE_INTERNAL;
int32_t mhs_object_event_disconnect(TMhsObj *obj, TMhsObjectCB proc) ATTRIBUTE_INTERNAL;

TMhsObj *mhs_object_get_marked(TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_process_events(TMhsObjContext *context) ATTRIBUTE_INTERNAL;

int32_t mhs_object_set_cmd_event(TMhsObj *obj) ATTRIBUTE_INTERNAL;
int32_t mhs_object_cmd_event_connect(TMhsObj *obj, int32_t type, TMhsObjectCB proc, void *user_data) ATTRIBUTE_INTERNAL;
int32_t mhs_object_cmd_event_disconnect(TMhsObj *obj, TMhsObjectCB proc) ATTRIBUTE_INTERNAL;
TMhsObj *mhs_object_get_cmd_marked(TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_object_get_cmd_event_count(TMhsObj *obj, uint32_t clear) ATTRIBUTE_INTERNAL;
int32_t mhs_process_cmd_events(TMhsObjContext *context) ATTRIBUTE_INTERNAL;

void mhs_object_set_alias(TMhsObj *obj, const char *alias);
char *mhs_object_get_alias(TMhsObj *obj);

#ifdef __cplusplus
  }
#endif

#endif
