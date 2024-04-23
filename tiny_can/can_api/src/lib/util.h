#ifndef __UTIL_H__
#define __UTIL_H__

#include "global.h"  // definition von ATTRIBUTE_INTERNAL

#ifdef __cplusplus
  extern "C" {
#endif


#ifdef __WIN32__
struct mhs_timeval
  {
  uint32_t tv_sec;
  uint32_t tv_usec;
  };
#else  
  #define mhs_timeval timeval
#endif  

#define lo(x)  (unsigned char)(x & 0xFF)
#define hi(x)  (unsigned char)((x >> 8) & 0xFF)

#define l_lo(x) (unsigned char)(x)
#define l_m1(x) (unsigned char)((x) >> 8)
#define l_m2(x) (unsigned char)((x) >> 16)
#define l_hi(x) (unsigned char)((x) >> 24)


#define DELETE_LEADING_SPACE    0x01
#define ENABLE_QUOTATION_MARKS  0x02
#define DELETE_TRAILING_SPACE   0x04
#define GET_ITEM_DEF_FLAGS (DELETE_LEADING_SPACE | ENABLE_QUOTATION_MARKS | DELETE_TRAILING_SPACE)

#define safe_free(d) do { \
  if ((d)) \
    { \
    mhs_free((d)); \
    (d) = NULL; \
    } \
  } while(0)

#define mhs_malloc0(s)  mhs_calloc(1, (s))

#define mhs_memcpy  memcpy
#define mhs_strcpy  strcpy

#ifdef MEM_DEBUG

#pragma message("WARNING: Memory Debug feature is enabled!")

#define mhs_malloc(size) mhs_malloc_(__FILE__, __LINE__, (size))
#define mhs_calloc(num, size)mhs_calloc_(__FILE__, __LINE__, (num), (size))

void *mhs_malloc_(const char *file, uint32_t line, uint32_t size) ATTRIBUTE_INTERNAL;
void *mhs_calloc_(const char *file, uint32_t line, uint32_t nmemb, uint32_t size) ATTRIBUTE_INTERNAL;
const char *mhs_check_for_leaks(int32_t *line, char **mem) ATTRIBUTE_INTERNAL;

#else
void *mhs_malloc(int32_t size) ATTRIBUTE_INTERNAL;
void *mhs_calloc(int32_t num, int32_t size) ATTRIBUTE_INTERNAL;
#define mhs_check_for_leaks(line)
#endif

void mhs_free(void *mem) ATTRIBUTE_INTERNAL;
void *mhs_memdup (void const *mem, int32_t size) ATTRIBUTE_INTERNAL;

int32_t safe_strcmp(const char *s1, const char *s2) ATTRIBUTE_INTERNAL;
int32_t safe_strcasecmp(const char *s1, const char *s2) ATTRIBUTE_INTERNAL;
char *str_empty(const char *s) ATTRIBUTE_INTERNAL;
void safe_strcpy(char *dst, uint32_t len, const char *src) ATTRIBUTE_INTERNAL; 

int32_t find_item(char *s, const char *list, char trenner) ATTRIBUTE_INTERNAL;
int32_t find_item_ex(const char **str_list, const char *name) ATTRIBUTE_INTERNAL;
char *find_upc(char *str, char *search) ATTRIBUTE_INTERNAL;

#ifndef __WIN32__
uint32_t get_tick(void) ATTRIBUTE_INTERNAL;
#endif

#ifdef __WIN32__
void get_unix_time(struct mhs_timeval* p) ATTRIBUTE_INTERNAL;
#else
#define get_unix_time(t) gettimeofday((t), NULL)
#endif

void get_timestamp(struct TTime* time_stamp) ATTRIBUTE_INTERNAL;
uint32_t mhs_diff_time(uint32_t now, uint32_t stamp) ATTRIBUTE_INTERNAL;

char *get_item_as_string(char **str, char *trenner, uint32_t flags, int32_t *result) ATTRIBUTE_INTERNAL;
uint32_t get_item_as_ulong(char **str, char *trenner, int32_t *result) ATTRIBUTE_INTERNAL;
int32_t get_item_as_long(char **str, char *trenner, int32_t *result) ATTRIBUTE_INTERNAL;
uint32_t str_to_ulong(char *str, int32_t *result) ATTRIBUTE_INTERNAL;

char *mhs_stpcpy(char *dest, const char *src) ATTRIBUTE_INTERNAL;
char *mhs_strdup(const char *str) ATTRIBUTE_INTERNAL;
char *mhs_strconcat(const char *string1, ...) ATTRIBUTE_INTERNAL;
char *get_file_name(const char *file_name) ATTRIBUTE_INTERNAL;
char *create_file_name(const char *dir, const char *file_name) ATTRIBUTE_INTERNAL;
char *path_get_dirname(const char *file_name) ATTRIBUTE_INTERNAL;
char *change_file_ext(const char *file_name, const char *ext) ATTRIBUTE_INTERNAL;

char *check_string(const char *str) ATTRIBUTE_INTERNAL;

uint32_t mhs_pow10(uint32_t n) ATTRIBUTE_INTERNAL;
uint32_t GetBCD(uint32_t in, uint32_t len) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
