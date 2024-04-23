#ifndef __USER_MEM_H__
#define __USER_MEM_H__

#include "global.h"

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct _TMemList TMemList;

struct _TMemList
  {
  TMemList *Next;
  char *Description;
  void *Ref;
  };

extern TMemList *UserMemList ATTRIBUTE_INTERNAL;

void UserMemInit(void) ATTRIBUTE_INTERNAL;
TMemList *MemAddRef(TMemList **list, const char *str, void *ref) ATTRIBUTE_INTERNAL;
void MemDestroy(TMemList **list, uint32_t ref_free) ATTRIBUTE_INTERNAL;
int32_t MemDestroyEntry(TMemList **list, TMemList *entry, uint32_t ref_free) ATTRIBUTE_INTERNAL;
int32_t MemDestroyEntryByRef(TMemList **list, void *ref, uint32_t ref_free) ATTRIBUTE_INTERNAL;
TMemList *GetEntryByRef(TMemList *list, void *ref) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
