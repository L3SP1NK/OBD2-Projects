#ifndef __SYS_FS_H__
#define __SYS_FS_H__

#include "global.h"

#ifdef __cplusplus
  extern "C" {
#endif

int32_t mhs_remove_trailing_slash(char *path) ATTRIBUTE_INTERNAL;
char *sysfs_get_mnt_path(void) ATTRIBUTE_INTERNAL;
char *mhs_get_link(const char *path) ATTRIBUTE_INTERNAL;
int32_t mhs_path_is_dir(const char *path) ATTRIBUTE_INTERNAL;
int32_t mhs_path_is_link(const char *path) ATTRIBUTE_INTERNAL;
int32_t mhs_path_is_file(const char *path) ATTRIBUTE_INTERNAL;
int32_t mhs_sys_read_value(char *dir, char *filename, char *data, int32_t max_size) ATTRIBUTE_INTERNAL;
uint32_t mhs_sys_read_as_ulong(char *dir, char *filename, int32_t base, int32_t *error) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
