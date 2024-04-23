/***************************************************************************
                           paths.c  -  description
                             -------------------
    begin             : 23.03.2008
    last modify       : 08.07.2022    
    copyright         : (C) 2008 - 2022 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include "config.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include "paths.h"

#ifdef __WIN32__
// ****** Windows
#define FILE_SEPERATOR '\\'
#else
// ****** Linux
#define FILE_SEPERATOR '/'
#endif

/**************************************************************************/
/*                         V A R I A B L E N                              */
/**************************************************************************/
char ConfigPath[MAX_PATH_LEN];
char LogPath[MAX_PATH_LEN];


/**************************************************************************/
/*                        F U N K T I O N E N                             */
/**************************************************************************/


/*
******************** PathsInit ********************
*/
void PathsInit(void)
{
ConfigPath[0] = '\0';
LogPath[0] = '\0';
}


/*
******************** PathsCreate ********************
*/
int32_t PathsCreate(void)
{
#ifdef __WIN32__
// ****** Windows
char file_name[MAX_PATH_LEN];
char *ptr;

// ****** Windows
// Programm Path ermitteln
GetModuleFileName(GetModuleHandle(NULL), file_name, MAX_PATH_LEN);
GetFullPathName(file_name, sizeof(file_name), LogPath, &ptr);
LogPath[ptr-LogPath] = '\0';
// Treiber Path ermitteln
GetModuleFileName(GetModuleHandle(DLL_NAME), file_name, MAX_PATH_LEN);
GetFullPathName(file_name, sizeof(file_name), ConfigPath, &ptr);
ConfigPath[ptr-ConfigPath] = '\0';
#else
// ****** Linux
safe_strcpy(ConfigPath, MAX_PATH_LEN, "/etc/tiny_can/");
safe_strcpy(LogPath, MAX_PATH_LEN, "/var/log/tiny_can/");
#endif
return(0);
}


/*
******************** PathsDestroy ********************
*/
void PathsDestroy(void)
{
ConfigPath[0] = '\0';
LogPath[0] = '\0';
}
