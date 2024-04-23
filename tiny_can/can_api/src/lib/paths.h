#ifndef __PATHS_H__
#define __PATHS_H__

#include "global.h"

#ifdef __cplusplus
  extern "C" {
#endif

#define MAX_PATH_LEN 251

extern char ConfigPath[MAX_PATH_LEN] ATTRIBUTE_INTERNAL;
extern char LogPath[MAX_PATH_LEN] ATTRIBUTE_INTERNAL;


void PathsInit(void) ATTRIBUTE_INTERNAL;
int32_t PathsCreate(void) ATTRIBUTE_INTERNAL;
void PathsDestroy(void) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
