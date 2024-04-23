/***************************************************************************
                           util.c  -  description
                             -------------------
    begin             : 08.04.2008
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
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <glib.h>
#include "util.h"

// Die Funktion g_strcmp0 ist nicht in allen
// Version der GLib enthalten, deshalbt der
// Ersatz mhs_strcmp
int mhs_strcmp0(const char *str1, const char *str2)
{
if (!str1)
  return -(str1 != str2);
if (!str2)
  return str1 != str2;
return strcmp(str1, str2);
}

/*
******************** strlwc ********************
Funktion  : Einen String in Kleinbuchstaben umwandeln

Eingaben  : str => Zu wandelnder String

Ausgaben  : keine

Call's    : tolower
*/
/*void strlwc(char *str)
{
while (*str)
  {
  *str = (char)tolower((int)*str);
  str++;
  }
}*/


/*
******************** strupc ********************
Funktion  : Einen String in Großbuchstaben umwandeln

Eingaben  : str => Zu wandelnder String

Ausgaben  : keine

Call's    : toupper
*/
/*void strupc(char *str)
{
while (*str)
  {
  *str = (char)toupper((int)*str);
  str++;
  }
}*/


/*
******************** strskp ********************
Funktion  : Führende Leerzeichen des Strings überspringen

Eingaben  : str => Input String

Ausgaben  : keine

Call's    : isspace
*/
/*void strskp(char **str)
{
char *s;

if (!str)
  return;
s = *str;
while (isspace((int)*s))
  s++;
*str = s;
}*/


/*
******************** strcrop ********************
Funktion  : Abschliesende Leerzeichen des Strings löschen

Eingaben  : str => Zu bearbeitender String

Ausgaben  : keine

Call's    : strlen, isspace
*/
/*void strcrop(char *str)
{
char *last;
int len;

if (!str)
  return;
len = strlen(str);
if (len == 0) return;
last = str + (len-1);
while ((last != str) && (isspace((int)*last)))
  last--;
last++;
*last=0;
}*/


/*
******************** strstrip ********************
Funktion  : Führende Leerzeichen des Strings überspringen
            und abschliesende Leerzeichen löschen

Eingaben  : str => Input String

Ausgaben  : keine

Call's    : isspace
*/
/*void strstrip(char **str)
{
char *last;
int len;

last = *str;
// Leerzeichen werden ignoriert
while (isspace((int)*last))
  last++;
*str = last;
len = strlen(last);
if (len)
  {
  last += (len-1);
  while ((len--) && (isspace((int)*last)))
    last--;
  last++;
  *last = 0;
  }
}*/


/*
******************** hex_to_long ********************
Funktion  :

Eingaben  :

Ausgaben  :

Call's    : keine
*/
/* unsigned long hex_to_long(char **str)
{
char *s;
unsigned char ch;
unsigned long ahex_res;

ahex_res = 0;
s = *str;
while ((ch = (unsigned char)*s))
  {
  // Klein in Großbuchstaben konvertieren
  if (ch >= 'a' && ch <= 'z')
      ch &= 0xDF;

  if (ch >= '0' && ch <= '9')
    {
    ahex_res <<= 4;
    ch = ch - 0x30;
    ahex_res |= ch;
    }
  else if (ch >= 'A' && ch <= 'F')
    {
    ahex_res <<= 4;
    ch = ch - 0x37;
    ahex_res |= ch;
    }
  else
    break;
  s++;
  }
*str = s;
return(ahex_res);
} */


/*
******************** scan ********************
Funktion  : Eine Quellstring nach einer Liste von Zeichen
            durchsuchen, den Quellstring bis zu diesen
            Zeichen in den Zielstring kopieren

Eingaben  : str => Zu bearbeitender String
            keys => Zeichentabelle
            outstr => Zielstring
            max => Max. Zeichen des Zielstring Puffers

Ausgaben  : result =>

Call's    : clrlastspk
*/
/*int scan(char **str, char *keys, char *outstr, int max)
{
int res;
char *key;
char *outptr;
char *s;
char c;

res = 0;
if ((!str) || (!keys))
  return(-2);
if ((outstr) && (max < 2))
  return(-2);
max--;          // Stringende "0" berücksichtigen
strskp(str);      // Führende Leerzeichen löschen
s = *str;
outptr = outstr;
while ((c = *s))
  {
  s++;
  res=0;
  key = keys;
  while (*key)
    {
    res++;
    if (c == *key++)
      {
      if (outptr)
        {
        *outptr = (char)0;
        strcrop(outstr);  // abschließende Leerzeichen löschen
        }
      *str = s;
      return(res);
      }
    }
  if (outptr)
    *outptr++ = c;
  if (!max--)
    {
    res = -1;
    break;
    }
  }
if (outptr)
  {
  *outptr = (char)0;
  strcrop(outstr);        // abschlieáende Leerzeichen l”schen
  }
*str = s;
return(0);
}*/

/*
******************** get_value ********************
Funktion  :

Eingaben  :

Ausgaben  : keine

Call's    :
*/
/* int get_value(char *instr, char *bezeichner, char *value, int max)
{
int match;
char puf[50];

while (1)
  {
  // Bezeichner auslesen
  match = scan(&instr, "=;", puf, 49);
  if (!strlen(puf))
    break;
  strupc(puf);  // String in Großbuchstaben umwandeln
  if (strcmp(puf, bezeichner) == 0)
    {
    if (match == 1)
      {
      // Value auslesen
      (void)scan(&instr, ";", value, max);
      return(0);
      }
    else
      {
      *value = '\0';  // Value nicht gesetzt
      return(1);
      }
    }
  else
    (void)scan(&instr, ";", NULL, 500);
  }
return(-1);
} */


/*
******************** find_string ********************
Funktion  : Einen Quellstring mit einer Tabelle
            von Strings vergleichen

Eingaben  : instr => Quellstring
            tab => Tabelle von Strings

Ausgaben  : int => Index des Tabellen eintrags mit dem
                   der Quellstring übereinstimmt
                   -1 Vergleich erfolglos

Call's    : StrUpr, strcmp
*/
/* int find_string(char *instr, const char *tab[])
{
unsigned int i = 0;

strupc(instr);  // String in Groábuchstaben umwandeln
while (tab[i] != NULL)
  {
  if (strcmp(instr, tab[i]) == 0) return(i);  // String vergleich
  i++;
  }
return(-1);
} */

void diff_timestamp(struct TTime *t_diff, struct TTime *t1, struct TTime *t2)
{
if (t1->Sec == t2->Sec)
  {
  t_diff->Sec = 0L;
  t_diff->USec = t1->USec - t2->USec;
  }
else if (t1->Sec <= t2->Sec)
  {
  t_diff->Sec = t1->Sec - t2->Sec;
  if (t2->USec >= t1->USec)
    t_diff->USec = t1->USec - t2->USec;
  else
    {
    t_diff->USec = (t1->USec - 1000000) - t2->USec;
    (t_diff->Sec)++;
    }
  }
else
  {
  t_diff->Sec = t1->Sec - t2->Sec;
  if (t2->USec <= t1->USec)
    t_diff->USec = t1->USec - t2->USec;
  else
    {
    t_diff->USec = (t1->USec + 1000000) - t2->USec;
    (t_diff->Sec)--;
    }
  }
}



/*
******************** diff_time ********************
Funktion  : Differenzeit berechnen

Eingaben  :

Ausgaben  : keine

Call's    :
*/
unsigned long diff_time(unsigned long now, unsigned long stamp)
{
if (stamp > now)
  return((0xFFFFFFFF - stamp) + now + 1);
else
  return(now - stamp);
}


/* char *GetItemAsString(char **str, char trenner)
{
int l;
char *s, *s1, *s2, *item;

item = NULL;     // kein Rückgabestring
s = *str;
// Führende Lehrzeichen überspringen
while (*s = ' ')
  s++;
// Eintrag in "xx" Zeichen
if ((s1 = strchr(s, '"')))
  {
  s1++;
  if ((s2 = strchr(s1, '"')))
    {
    l = s2 - s1;
    item = (char *)g_malloc(l+1);
    memcpy(item, s1, l);
    s1[l] = '\0';
    s = s2+1;
    }
  }

if ((s1 = strchr(s, trenner)))
  {
  l = s1 - s;
  if ((!item) && (l))
    {
    item = (char *)g_malloc(l+1);
    memcpy(item, s, l);
    s[l] = '\0';
    }
  s = s1+1;
  }
if ((!item) && ((l = strlen(s))))
  {
  item = g_strdup(s);
  s = s + l;
  }
*str = s;
return(item);
} */



char *GetItemAsString(char **str, char *trenner, int *result)
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

/*
char *GetItemAsString(char **str, char trenner)
{
char *s, *start, *end, *item;

if (!str)
  return(NULL);
s = *str;
if (!s)
  return(NULL);
// Führende Lehrzeichen überspringen
while (*s == ' ')
  s++;
if (*s == '\0')
  return(NULL);
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

if ((s = strchr(end, trenner)))
  {
  *s++ = '\0';
  end = s;
  }

if (end == start)
  *str = start + strlen(end);
else
  *str = end;
return(item);
}
*/


unsigned long GetItemAsULong(char **str, char *trenner, int base, int *result)
{
unsigned long value;
char *s, *endptr;

s = GetItemAsString(str, trenner, result);
if (!s)
  return(0L);
//return(strtoul(s, NULL, base));
value = strtoul(s, (char**)&endptr, base);
if (s == endptr)
  {
  if (result)
    *result = -1;
  }
return(value);
}

/*
******************** get_unix_time ********************
Funktion  :

Eingaben  :

Ausgaben  : keine

Call's    :
*/
/*#ifdef __WIN32__

const ULONGLONG VALUE_10      = {10};
const ULONGLONG VALUE_1000000 =  {1000000};
const ULONGLONG VALUE_10000000 = {10000000};
const ULONGLONG VALUE_116444736000000000 = {116444736000000000};

void get_unix_time(struct timeval* p)
{
union
  {
  ULONGLONG ns100; // time since 1 Jan 1601 in 100ns units
  FILETIME ft;
  }
now;

GetSystemTimeAsFileTime( &(now.ft) );
p->tv_usec=(long)((now.ns100 / VALUE_10) % VALUE_1000000);
p->tv_sec= (long)((now.ns100-(VALUE_116444736000000000))/ VALUE_10000000);
}
#endif*/
