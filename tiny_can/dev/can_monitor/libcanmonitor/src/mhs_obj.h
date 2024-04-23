#ifndef __MHS_OBJ__
#define __MHS_OBJ__

#include <glib.h>

#define MHS_LOCK_INIT g_mutex_new
#define MHS_LOCK_DESTROY g_mutex_free
#define MHS_LOCK_ENTER g_mutex_lock
#define MHS_LOCK_LEAVE g_mutex_unlock

typedef struct TMhsObj _TMhsObj;
typedef struct TMhsObjHandler _TMhsObjHandler;

struct TObjFuncs
  {
  void(*Event)(_TMhsObj *obj);
  void(*UserCB)(_TMhsObj *obj, _TMhsObjHandler *handler);
  };


struct TMhsObj
  {
  struct TMhsObj *Next;
  struct TMhsObj *MarkedNext;
  struct TMhsObjContext *Context;
  int MarkedFlag;
  int EventCount;
  int CbEnable;
  const struct TObjFuncs *Funcs;
  struct TMhsObjHandler *HandlerList;
  };

struct TMhsObjContext
  {
  struct TMhsObjContext *Next;
  struct TMhsObj *Items;
  struct TMhsObj *MarkedFirst;
  struct TMhsObj *MarkedLast;
  int DeleteItems;
  gint MhsEventId;
  GMutex *Lock;
  };

typedef void(*TMhsObjectCB)(struct TMhsObj *obj, gpointer user_data);

struct TMhsObjHandler
  {
  struct TMhsObjHandler *Next;
  TMhsObjectCB Proc;
  gpointer UserData;
  };

//typedef void(*TMhsUserEventCB)(struct TMhsObj *obj, struct TMhsObjHandler *handler);

void mhs_object_init(void);
void mhs_object_destroy(void);
struct TMhsObjContext *mhs_obj_context_new(void);
struct TMhsObjContext *mhs_obj_context_default(void);
struct TMhsObj *mhs_object_new(const struct TObjFuncs *funcs, int struct_size, struct TMhsObjContext *context);
int mhs_object_remove(struct TMhsObj *obj);
int mhs_object_valid(struct TMhsObj *obj);
int mhs_object_valid_unlocked(struct TMhsObj *obj);
int mhs_object_set_event_unlocked(struct TMhsObj *obj);
int mhs_object_set_event(struct TMhsObj *obj);
void mhs_object_reset_event(struct TMhsObj *obj);
int mhs_object_get_event_count(struct TMhsObj *obj);
int mhs_object_event_connect(struct TMhsObj *obj, TMhsObjectCB proc, gpointer user_data);


#endif
