/***************************************************************************
                          win_reg.c  -  description
                             -------------------
    begin             : 25.03.2009
    copyright         : (C) 2009 by MHS-Elektronik GmbH & Co. KG, Germany
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/
#include <glib.h>
#include <windows.h>
#include "util.h"
#include "win_reg.h"


/**************************************************************************/
/*                        C O N S T A N T E N                             */
/**************************************************************************/
const char REG_TINY_CAN_API_PATH[] = {"Software\\Tiny-CAN\\API"};
const char REG_TINY_CAN_API_ENTRY[] = {"PATH"};

/**************************************************************************/
/*                        F U N K T I O N E N                             */
/**************************************************************************/


char *RegGetString(HKEY key, const char *value_name)
{
DWORD type, data_size;
char *str;

type = 0;
data_size = 0;
if ((!key) || (!value_name))
  return(NULL);
// Länge des Strings ermitteln
if (RegQueryValueEx(key, value_name, 0, &type, NULL, &data_size) != ERROR_SUCCESS)
  return(NULL);
// Wert ist String ?
if (type != REG_SZ)
  return(NULL);
str = g_malloc(data_size+1);
if (!str)
  return(NULL);
// String lesen
if (RegQueryValueEx(key, value_name, 0, &type, (LPBYTE)str, &data_size) != ERROR_SUCCESS)
  {
  g_free(str);
  return(NULL);
  }
str[data_size] = '\0';
return(str);
}


/*
******************** RegReadApiPath ********************
Funktion  : Tiny-CAN API Path aus der Registrierungsdatenbank auslesen

Eingaben  : keine

Ausgaben  : Path, NULL = Fehler


Call's    : RegOpenKeyEx, RegGetString, RegCloseKey
*/
char *RegReadApiPath(void)
{
HKEY key;
char *value;

value = NULL;
if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_TINY_CAN_API_PATH, 0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS) // KEY_READ
  {
  value = RegGetString(key, REG_TINY_CAN_API_ENTRY);
  RegCloseKey(key);
  }
return(value);
}


/*
******************** RegWriteApiPath ********************
Funktion  : Tiny-CAN API Path in die Registrieungsdatenbank schreiben

Eingaben  : path => Path String

Ausgaben  : 0 = Path erfolgreich in Registrierungsdatenbank gespeichert
            -1 = Fehler

Call's    : RegCreateKeyEx, RegSetValueEx, RegCloseKey
*/
int RegWriteApiPath(char *path)
{
HKEY key;
DWORD dw;
int res;

res = -1;
if (RegCreateKeyEx(HKEY_CURRENT_USER, REG_TINY_CAN_API_PATH, 0, REG_NONE, REG_OPTION_NON_VOLATILE,
     KEY_WRITE | KEY_READ, NULL, &key, &dw) == ERROR_SUCCESS)
  {
  if (key)
    {
    if (RegSetValueEx(key, REG_TINY_CAN_API_ENTRY, 0, REG_SZ, (LPBYTE)path, strlen(path)+1) == ERROR_SUCCESS)
      res = 0;
    }
  RegCloseKey(key);
  }
return(res);
}
