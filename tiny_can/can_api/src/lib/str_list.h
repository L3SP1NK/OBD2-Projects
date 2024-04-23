#ifndef __STR_LIST_H__
#define __STR_LIST_H__

#include "os_core.h"

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct _TStrList TStrList;

struct _TStrList
  {
  TStrList *Next;
  char String[1];
  //char *String;
  };

TStrList *StrListAdd(TStrList **list, const char *str) ATTRIBUTE_INTERNAL;
void StrListDestroy(TStrList **list) ATTRIBUTE_INTERNAL;
int32_t StrListFind(TStrList *list, const char *str) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
