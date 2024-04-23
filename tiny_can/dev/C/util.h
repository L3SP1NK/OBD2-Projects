#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdlib.h>
#include "can_types.h"

#ifdef __cplusplus
  extern "C" {
#endif


#define lo(x)  (x & 0xFF)
#define hi(x)  ((x >> 8) & 0xFF)

#define l_lo(x) (unsigned char)(x)
#define l_m1(x) (unsigned char)((x) >> 8)
#define l_m2(x) (unsigned char)((x) >> 16)
#define l_hi(x) (unsigned char)((x) >> 24)

#define safe_free(d) do { \
  if ((d)) \
    { \
    free((d)); \
    (d) = NULL; \
    } \
  } while(0)


void PrintFdMessages(struct TCanFdMsg *msg, int32_t count, int8_t enable_fd, int8_t enable_source);

int save_strcmp(char *s1, char *s2);
int save_strcasecmp(char *s1, char *s2);
char *get_item_as_string(char **str, char *trenner, int *result);
char *mhs_stpcpy(char *dest, const char *src);
char *mhs_strdup(const char *str);
char *mhs_strconcat(const char *string1, ...);
char *create_file_name(char *dir, char *file_name);
char *get_file_name(char *file_name);
char *path_get_dirname(char *file_name);

#ifdef __cplusplus
  }
#endif

#endif
