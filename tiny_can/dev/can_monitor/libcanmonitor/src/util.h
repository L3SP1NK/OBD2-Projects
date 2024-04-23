#ifndef __UTIL_H__
#define __UTIL_H__

#define lo(x)  (x & 0xFF)
#define hi(x)  ((x >> 8) & 0xFF)

#include <stdio.h>
#include "can_types.h"

#define save_free(d) do { \
  if ((d)) \
    { \
    g_free((d)); \
    (d) = NULL; \
    } \
  } while(0)

int mhs_strcmp0(const char *str1, const char *str2);
/* void strlwc(char *str);
void strupc(char *str);
void strskp(char **str);
void strcrop(char *str);
void strstrip(char **str); */
//unsigned long hex_to_long(char **str);
//int scan(char **str, char *keys, char *outstr, int max);
//int get_value(char *instr, char *bezeichner, char *value, int max);
//int find_string(char *instr,const char *tab[]);
void diff_timestamp(struct TTime *t_diff, struct TTime *t1, struct TTime *t2);
unsigned long diff_time(unsigned long now, unsigned long stamp);

char *GetItemAsString(char **str, char *trenner, int *result);
unsigned long GetItemAsULong(char **str, char *trenner, int base, int *result);

#ifdef __WIN32__
// ****** Windows
#include <windows.h>
#define CALLBACK_TYPE CALLBACK
#define mhs_sleep(x) Sleep(x)
#else
// ****** Linux>
#include <unistd.h>
#define CALLBACK_TYPE
#define mhs_sleep(x) usleep((x) * 1000)
#endif

#endif
