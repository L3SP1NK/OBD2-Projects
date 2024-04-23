/***************************************************************************
                           util.c  -  description
                             -------------------
    begin             : 02.02.2012
    copyright         : (C) 2012 by MHS-Elektronik GmbH & Co. KG, Germany
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
/*#include <windows.h>
#include <basetyps.h>
#include <winioctl.h>
#include <tchar.h>*/
#include "mhs_list.h"
#include "util.h"

#ifdef MEM_DEBUG


//*****************************************************************************
// G L O B A L S
//*****************************************************************************

static TMhsList AllocListHead = {&AllocListHead, &AllocListHead};


//*****************************************************************************
//
// mhs_malloc_()
//
//*****************************************************************************
void *mhs_malloc_(const char *file, uint32_t line, uint32_t size)
{
TMemAllocHeader *header;

if (size)
  {
  header = (TMemAllocHeader *)malloc(size + sizeof(TMemAllocHeader));
  if (header)
    {
    mhs_insert_tail_list(&AllocListHead, header);
    header->File = file;
    header->Line = line;
    return(header + 1);
    }
  }
return(NULL);
}


void *mhs_calloc_(const char *file, uint32_t line, uint32_t nmemb, uint32_t size)
{
TMemAllocHeader *header;

if (size)
  {
  size = sizeof(TMemAllocHeader) + (nmemb*size);
  header = (TMemAllocHeader *)calloc(size, 1);
  if (header)
    {
    mhs_insert_tail_list(&AllocListHead, header);
    header->File = file;
    header->Line = line;
    return(header + 1);
    }
  }
return(NULL);
}


void mhs_free(void *mem)
{
TMemAllocHeader *header;

if (mem)
  {
  header = (TMemAllocHeader *)mem;
  header--;
  mhs_remove_entry_list(header);
  free(header);
  }
}


const char *mhs_check_for_leaks(int32_t *line, char **mem)
{
TMemAllocHeader *header;
const char *file;

if (!line)
  return(NULL);
while (!mhs_is_list_empty(&AllocListHead))
  {
  header = (TMemAllocHeader *)mhs_remove_head_list(&AllocListHead);
  if (line)
    *line = header->Line;
  file = header->File;
  header++;
  if (mem)
    *mem = (char *)header;
  return(file);
  }
return(NULL);
}

#endif
