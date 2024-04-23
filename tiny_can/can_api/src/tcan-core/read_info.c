/***************************************************************************
                           read_info.c  -  description
                             -------------------
    begin             : 15.01.2021
    last modified     : 15.01.2021    
    copyright         : (C) 2021 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include <string.h>
#include <stdio.h>
#include "util.h"
#include "tar_drv.h"
#include "info.h"
#include "read_info.h"


int32_t HwReadInfoVar(TCanDevice *dev)
{
int32_t res;
struct TInfoVarList *info;
struct TTarInfoVar info_tmp;

safe_free(dev->DrvInfoHwStr);
InfoVarDestroy(&dev->InfoVars);
if (!(res = TARInfoStartRead(dev)))
  {
  while ((res = TARInfoGetNext(dev, &info_tmp)) > 0)
    {
    if (!(info = InfoVarAdd(&dev->InfoVars)))
      {
      res = -1;
      break;
      }
    info->Size = info_tmp.Size;
    info->Type = info_tmp.Type;
    info->Index = info_tmp.Index;
    info->IndexString = GetIndexString(info_tmp.Index);
    if (info_tmp.Size)
      {
      info->Data = mhs_calloc(1, (info_tmp.Size + 1));
      if (!info->Data)
        {
        res = -1;
        break;
        }
      memcpy(info->Data, info_tmp.Data, info_tmp.Size);
      }
    }
  }
if (res < 0)   // Fehler
  InfoVarDestroy(&dev->InfoVars);
else
  dev->DrvInfoHwStr = CreateInfoString(dev->InfoVars);
return(res);
}
