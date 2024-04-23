/***************************************************************************
                           util.c  -  description
                             -------------------
    begin             : 23.03.2008
    copyright         : (C) 2008 by MHS-Elektronik GmbH & Co. KG, Germany
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "can_types.h"
#include "util.h"


void PrintFdMessages(struct TCanFdMsg *msg, int32_t count, int8_t enable_fd, int8_t enable_source)
{
char *ptr;
unsigned char ch, hex, dev, first, chars;
int32_t cnt, i;
uint32_t us;
const char *type_str, *fd_str, *space_str;
char id_str[10];
char dlc_str[3];
char msg_str[30];

for (;count;count--)
  {
  if (msg->MsgFD)
    {
    if (msg->MsgBRS)
      fd_str = "FD/BRS";
    else
      fd_str = "FD    ";
    }
  else
    fd_str = "      ";
  // Message Format
  if ((msg->MsgRTR) && (msg->MsgEFF))
    type_str = "EFF/RTR";
  else if (msg->MsgEFF)
    type_str = "EFF    ";
  else if (msg->MsgRTR)
    type_str = "STD/RTR";
  else
    type_str = "STD    ";
  // ID
  if (msg->MsgEFF)
   sprintf(id_str, "%08X", msg->Id);
  else
   sprintf(id_str, "     %03X", msg->Id);
  // Dlc
  cnt = msg->MsgLen;
  sprintf(dlc_str, "%2u", (uint32_t)cnt);
  // Daten
  if (msg->MsgRTR)
    cnt = 0;
  if (enable_fd)
    {
    if (enable_source)
      {
      space_str = "                                                            ";
      ch = msg->MsgSource & 0xF;
      dev = (msg->MsgSource >> 4) & 0x0F;
      us = msg->Time.USec / 100;
      printf("Dev:%2u Ch:%2u  %6u.%04u | %s | %s | %s | %s | ",
              dev, ch, msg->Time.Sec, us, fd_str, type_str, id_str, dlc_str);
      }
    else
      {
      space_str = "                                                     ";
      us = msg->Time.USec / 100;
      printf("%6u.%04u | %s | %s | %s | %s | ",
              msg->Time.Sec, us, fd_str, type_str, id_str, dlc_str);
      }
    }
  else
    {
    space_str = " ";
    if (enable_source)
      {
      ch = msg->MsgSource & 0xF;
      dev = (msg->MsgSource >> 4) & 0x0F;
      us = msg->Time.USec / 100;
      printf("Dev:%2u Ch:%2u  %6u.%04u | %s | %s | %s | ",
              dev, ch, msg->Time.Sec, us, type_str, id_str, dlc_str);
      }
    else
      {
      us = msg->Time.USec / 100;
      printf("%6u.%04u | %s | %s | %s | ",
              msg->Time.Sec, us, type_str, id_str, dlc_str);
      }
    }
  first = 1;
  i = 0;
  while (cnt)
    {
    ptr = msg_str;
    for (chars = 0; cnt; cnt--)
      {
      if (++chars > 8)
        break;
      ch = msg->MsgData[i++];
      //ptr += sprintf(ptr, "%02x ", ch);
      hex = ch >> 4;
      if (hex > 9)
        *ptr++ = 55 + hex;
      else
        *ptr++ = '0' + hex;
      hex = ch & 0x0F;
      if (hex > 9)
        *ptr++ = 55 + hex;
      else
        *ptr++ = '0' + hex;
      *ptr++ = ' ';
      }
    *ptr = '\0';
    if (chars)
      {
      if (first)
        {
        first = 0;
        printf("%s\n\r", msg_str);
        }
      else
        printf("%s%s\n\r", space_str, msg_str);
      }
    }
  if (first)
    printf("\n\r");
  msg++;
  }
}


/**************************************************************************/
/*                        F U N K T I O N E N                             */
/**************************************************************************/
int save_strcmp(char *s1, char *s2)
{
if ((!s1) && (!s2))
  return(0);
if ((!s1) || (!s2))
  return(-1);
return(strcmp(s1, s2));
}


int save_strcasecmp(char *s1, char *s2)
{
char c1, c2;

if ((!s1) && (!s2))
  return(0);
if ((!s1) || (!s2))
  return(-1);
while ((c1 = *s1++))
  {
  c1 = (char)toupper((int)c1);
  c2 = *s2++;
  c2 = (char)toupper((int)c2);
  if (c1 != c2)
    return(-1);
  }
if (*s2)
  return(-1);
return(0);
//return(strcasecmp(s1, s2));
}


char *get_item_as_string(char **str, char *trenner, int *result)
{
int cnt;
char t;
char *s, *start, *end, *item;

if (!str)
  {
  if (result)
    *result = -1;
  return(NULL);
  }
s = *str;
if (!s)
  {
  if (result)
    *result = -1;
  return(NULL);
  }
// Führende Lehrzeichen überspringen
while (*s == ' ')
  s++;
if (*s == '\0')
  {
  if (result)
    *result = -1;
  return(NULL);
  }
if (result)
  *result = 0;
end = s;
item = s;
start = s;
// Eintrag in "xx" Zeichen
if ((s = strchr(s, '´')))
  {
  item = s + 1;
  if ((s = strchr(item, '´')))
    {
    *s++ = '\0';
    end = s;
    }
  }

cnt = 0;
while ((t = *trenner++))
  {
  cnt++;
  if ((s = strchr(end, t)))
    {
    *s++ = '\0';
    end = s;
    if (result)
      *result = cnt;
    break;
    }
  }
if (end == start)
  *str = start + strlen(end);
else
  *str = end;
return(item);
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
int len;
char *new_str;

if (str)
  {
  len = strlen(str) + 1;
  new_str = (char *)malloc(len);
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
int l;
va_list args;
char *s, *concat, *ptr;

if (!string1)
  return(NULL);

l = 1 + strlen(string1);
va_start(args, string1);
s = va_arg(args, char*);
while (s)
  {
  l += strlen(s);
  s = va_arg(args, char*);
  }
va_end(args);

concat = (char *)malloc(l);
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
