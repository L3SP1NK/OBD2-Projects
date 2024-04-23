/***************************************************************************
                           util.c  -  description
                             -------------------
    begin             : 02.02.2012
    last modify       : 11.04.2022    
    copyright         : (C) 2012 - 2022 by MHS-Elektronik GmbH & Co. KG, Germany
    author            : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/

/**
    Library to talk to Tiny-CAN devices. You find the latest versions at
       http://www.tiny-can.com/
**/
#include "global.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "util.h"


#ifdef __WIN32__
  #define DIR_SEPARATOR '\\'
  #define DIR_SEPARATOR_STR "\\"
#else
  #define DIR_SEPARATOR '/'
  #define DIR_SEPARATOR_STR "/"
#endif


/**************************************************************************/
/*                        F U N K T I O N E N                             */
/**************************************************************************/

#ifndef MEM_DEBUG

void *mhs_malloc(int32_t size)
{
return(malloc(size));
}


void *mhs_calloc(int32_t num, int32_t size)
{
return(calloc(num, size));
}


void mhs_free(void *mem)
{
free(mem);
}

#endif


void *mhs_memdup(void const *mem, int32_t size)
{
void *new_mem;

if (mem)
  {
  new_mem = mhs_malloc(size);
  memcpy(new_mem, mem, size);
  }
else
  new_mem = NULL;

return(new_mem);
}


int32_t safe_strcmp(const char *s1, const char *s2)
{
if ((!s1) && (!s2))
  return(0);
if ((!s1) || (!s2))
  return(-1);
return(strcmp(s1, s2));
}


int32_t safe_strcasecmp(const char *s1, const char *s2)
{
char c1, c2;

if ((!s1) && (!s2))
  return(0);
if ((!s1) || (!s2))
  return(-1);
while ((c1 = *s1++))
  {
  c1 = (char)toupper((int32_t)c1);
  c2 = *s2++;
  c2 = (char)toupper((int32_t)c2);
  if (c1 != c2)
    return(-1);
  }
if (*s2)
  return(-1);
return(0);
//return(strcasecmp(s1, s2));
}


char *str_empty(const char *s)
{
if (s)
  {
// Führende Leerzeichen überspringen
  while (*s == ' ')
    s++;
  if (*s != '\0')
    return((char *)s);
  }
return(NULL);
}


void safe_strcpy(char *dst, uint32_t len, const char *src) 
{
char ch;

if ((len) && (dst))
  {
  if (src)
    {
    while ((ch = *src++) != '\0')
      {
      if (!--len)
        break;
      *dst++ = ch;  
      }
    }
  *dst = '\0';
  }  
}


int32_t find_item(char *s, const char *list, char trenner)
{
int32_t hit, idx;
char c, l;
const char *in;

if ((!s) || (!list))
  return(-1);
idx = 0;
while (*list)
  {
  in = s;
  hit = 1;
  while ((c = *in++))
    {
    c = (char)toupper((int32_t)c);
    l = *list++;
    if (l == trenner)
      {
      if (c)
        hit = 0;
      break;
      }
    l = (char)toupper((int32_t)l);

    if (c != l)
      {
      hit = 0;
      break;
      }
    }
  if (hit)
    return(idx);
  idx++;
  }
return(-1);
}


int32_t find_item_ex(const char **str_list, const char *name)
{
const char *item;
int32_t idx;

if (!str_list)
  return(0);
for (idx = 0; (item = *str_list); str_list++)
  {
  if (!safe_strcasecmp(item, name))
    return(idx);
  idx++;
  }
return(-1);
}


/*
******************** find_upc ********************
Funktion  :

Eingaben  :

Ausgaben  : keine

Call's    :
*/
char *find_upc(char *str, char *search)
{
char *s;
char c;

s = search;
while ((c = *str++))
  {
  c = (char)toupper((int32_t)c);
  if (c == *s++)
    {
    if (!*s)
      return(str);
    }
  else
    s = search;
  }
return(NULL);
}


/*
******************** get_tick ********************
*/
#ifndef __WIN32__
uint32_t get_tick(void)
{
//struct timespec now;
struct timeval now;

//clock_gettime(CLOCK_REALTIME, &now);
//return((now.tv_sec * 1000) + (now.tv_nsec / 1000000));
gettimeofday(&now, NULL);
return((now.tv_sec * 1000) + (now.tv_usec / 1000));
}
#endif


/*
******************** get_unix_time ********************
Funktion  : Unix Zeit unter Windows ermitteln

Eingaben  : p => Buffer für Ergebnis

Ausgaben  : *p => Unix Zeit

Call's    : GetSystemTimeAsFileTime
*/
#ifdef __WIN32__

const ULONGLONG VALUE_10      = {10};
const ULONGLONG VALUE_1000000 =  {1000000};
const ULONGLONG VALUE_10000000 = {10000000};
const ULONGLONG VALUE_116444736000000000 = {116444736000000000};
/*
const uint64_t VALUE_10      = {10};
const uint64_t VALUE_1000000 =  {1000000};
const uint64_t VALUE_10000000 = {10000000};
const uint64_t VALUE_116444736000000000 = {116444736000000000};*/

void get_unix_time(struct mhs_timeval *p)
{
union
  {
  ULONGLONG ns100; // time since 1 Jan 1601 in 100ns units
  FILETIME ft;
  }
now;

#ifdef WINCE
SYSTEMTIME time;
GetSystemTime(&time);
// if SystemTimeToFileTime() fails, it returns zero.
if (!SystemTimeToFileTime(&time, &(now.ft)))
  return;
#else
GetSystemTimeAsFileTime(&(now.ft));
#endif
p->tv_usec=(uint32_t)((now.ns100 / VALUE_10) % VALUE_1000000);
p->tv_sec= (uint32_t)((now.ns100-(VALUE_116444736000000000))/ VALUE_10000000);
}
#endif


void get_timestamp(struct TTime* time_stamp)
{
struct mhs_timeval t;

get_unix_time(&t);
time_stamp->USec = (uint32_t)t.tv_usec;
time_stamp->Sec = (uint32_t)t.tv_sec;
}


/*
******************** mhs_diff_time ********************
Funktion  : Differenzeit zwischen now und stamp
            berechnen unter berücksichtigung eines
            Überlaufs

Eingaben  : now => Aktuelle Zeit
            stamp => Zeitstempel in der Vergangenheit

Ausgaben  : result => Differenz-Zeit x = now - stamp

Call's    : keine
*/
uint32_t mhs_diff_time(uint32_t now, uint32_t stamp)
{
if (stamp > now)
  return((0xFFFFFFFF - stamp) + now + 1);
else
  return(now - stamp);
}


#define SET_RESULT(result, value) if ((result)) *(result) = (value)

char *get_item_as_string(char **str, char *trenner, uint32_t flags, int32_t *result)
{
int32_t hit, cnt, l;
char *s, *t, *start, *end, *item;
char ch;

if ((!str) || (!trenner))
  {
  SET_RESULT(result, -1);
  return(NULL);
  }
s = *str;
if (!s)
  {
  SET_RESULT(result, -1);
  return(NULL);
  }
if (flags & DELETE_LEADING_SPACE)
  {
  // Führende Leerzeichen überspringen
  while (*s == ' ')
    s++;
  }
if (*s == '\0')
  {
  SET_RESULT(result, -1);
  return(NULL);
  }
SET_RESULT(result, 0);
end = s;
item = s;
start = s;
hit = 0;
for (; (ch = *s); s++)
  {
  if (flags & ENABLE_QUOTATION_MARKS)
    {
    // Eintrag in "xx" Zeichen
    if (ch == '"')
      {
      item = s + 1;
      if ((t = strchr(item, '"')))
        {
        s = t;
        *s++ = '\0';
        end = s;
        if (!(ch = *s))
          break;
        }
      }
    }
  cnt = 0;
  for (t = trenner; *t; t++)
    {
    cnt++;
    if (ch == *t)
      {
      *s++ = '\0';
      end = s;
      SET_RESULT(result, cnt);
      hit = 1;
      break;
      }
    }
  if (hit)
    break;
  }
if (flags & DELETE_TRAILING_SPACE)
  {
  // Abschliesende Leerzeichen löschen
  if ((l = (int32_t)strlen(item)))
    {
    s = item + (l - 1);
    while ((s != item) && ((*s == ' ') || (*s == '\n') || (*s == '\r')))
      s--;
    s++;
    *s = '\0';
    }
  }
if (end == start)
  *str = start + strlen(end);
else
  *str = end;
return(item);
}


uint32_t get_item_as_ulong(char **str, char *trenner, int32_t *result)
{
uint32_t v;
char *s;
int32_t e;
char *endptr;

e = 0;
v = 0L;
s = get_item_as_string(str, trenner, DELETE_LEADING_SPACE, NULL);
if (!s)
  e = -1;
else
  {
  v = strtoul(s, (char**)&endptr, 0);
  if (s == endptr)
    e = -1;
  }
SET_RESULT(result, e);
return(v);
}


int32_t get_item_as_long(char **str, char *trenner, int32_t *result)
{
int32_t v;
char *s;
int32_t e;
char *endptr;

e = 0;
v = 0L;
s = get_item_as_string(str, trenner, DELETE_LEADING_SPACE, NULL);
if (!s)
  e = -1;
else
  {
  v = strtol(s, (char**)&endptr, 0);
  if (s == endptr)
    e = -1;
  }
SET_RESULT(result, e);
return(v);
}


uint32_t str_to_ulong(char *str, int32_t *result)
{
uint32_t v;
int32_t e;
char *endptr;

e = 0;
v = 0UL;
if (!str)
  e = -1;
else
  {
  v = strtoul(str, (char**)&endptr, 0);
  if (str == endptr)
    e = -1;
  }
SET_RESULT(result, e);
return(v);
}


char *mhs_stpcpy(char *dest, const char *src)
{
register char *d = dest;
register const char *s = src;

do
  *d++ = *s;
while (*s++);

return(d - 1);
}


char *mhs_strdup(const char *str)
{
int32_t len;
char *new_str;

if (str)
  {
	len = (int32_t)strlen(str) + 1;
  new_str = (char *)mhs_malloc(len);
  if (!new_str)
    return(NULL);
  memcpy(new_str, str, len);
  return(new_str);
  }
else
  return(NULL);
}


char *mhs_strconcat(const char *string1, ...)
{
int32_t l;
va_list args;
char *s, *concat, *ptr;

if (!string1)
  return(NULL);

l = 1 + (int32_t)strlen(string1);
va_start(args, string1);
s = va_arg(args, char*);
while (s)
  {
	l += (int32_t)strlen(s);
  s = va_arg(args, char*);
  }
va_end(args);

concat = (char *)mhs_malloc(l);
if (!concat)
  return(NULL);
ptr = concat;

ptr = mhs_stpcpy(ptr, string1);
va_start (args, string1);
s = va_arg (args, char*);
while (s)
  {
  ptr = mhs_stpcpy(ptr, s);
  s = va_arg(args, char*);
  }
va_end (args);

return(concat);
}


char *get_file_name(const char *file_name)
{
char *p;

if (!file_name)
  return(NULL);
if ((p = strrchr(file_name, DIR_SEPARATOR)))
  return(p+1);
else
  return((char *)file_name);
}


char *create_file_name(const char *dir, const char *file_name)
{
if (!dir)
  return(mhs_strdup(file_name));
if (!file_name)
  return(NULL);
if (strchr(file_name, DIR_SEPARATOR))
  return(mhs_strdup(file_name));
else
  {
  if (dir[strlen(dir)-1] == DIR_SEPARATOR)
    return(mhs_strconcat(dir, file_name, NULL));
  else
    return(mhs_strconcat(dir, DIR_SEPARATOR_STR, file_name, NULL));
  }
}


char *path_get_dirname(const char *file_name)
{
int32_t len;
char *base;

if (!file_name)
  return(NULL);
base = strrchr(file_name, DIR_SEPARATOR);
if (base)
  {
  while ((base > file_name) && (*base == DIR_SEPARATOR))
    base--;
  len = (int32_t)(base - file_name + 1);

  base = (char *)mhs_malloc(len + 1);
  if (!base)
    return(NULL);
  memcpy(base, file_name, len);
  base[len] = 0;
  return(base);
  }
else
  return(mhs_strdup("."));
}


char *change_file_ext(const char *file_name, const char *ext)
{
char *last_period, *tmp, *new_filename;

if (!file_name)
  return(NULL);
tmp = mhs_strdup(file_name);
last_period = strrchr(tmp, '.');
if (last_period)
  *last_period = '\0';
new_filename = mhs_strconcat(tmp, ".", ext, NULL);
safe_free(tmp);
return(new_filename);
}


char *check_string(const char *str)
{
if (!str)
  return(NULL);
while (*str == ' ')
  str++;
if (*str == '\0')
  return(NULL);
return((char *)str);    
}


uint32_t mhs_pow10(uint32_t n)
{
uint32_t out;

out = 1;
for (; n; n--)
  {
  out *= 10;
  }
return(out);
}


uint32_t GetBCD(uint32_t in, uint32_t len)
{
uint32_t out, exp;

out = 0;
exp = 0;
for (;len; len--)
  {
  out += ((in & 0x0F) * mhs_pow10(exp));
  exp++;
  in = in >> 4; 
  }
return(out);  
}