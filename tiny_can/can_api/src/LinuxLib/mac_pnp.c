/***************************************************************************
                          mac_pnp.c  -  description
                             -------------------
    begin             : 27.07.2012
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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "errors.h"
#include "util.h"
#include "com_io.h"
#include "index.h"
#include "usb_hlp.h"
#include "usb_scan.h"
#include "usb_pnp.h"
#include "pnp_inc.c"



/*
******************** PnPCreate ********************
*/
int32_t PnPCreate(void)
{
if (MhsPnP)
  return(0);
if (!(MhsPnP = (struct TMhsPnP *)mhs_calloc(1, sizeof(struct TMhsPnP))))
  return(-1);
UpdateTCanUsbList();
mhs_event_set_event_mask((TMhsEvent *)MhsPnP->Thread, MHS_ALL_EVENTS);
return(0);
}


/*
******************** PnPDestroy *******************
*/
void PnPDestroy(void)
{
if (MhsPnP)
  {
  DestroyTCanUsbList(&TCanUsbList);
  safe_free(MhsPnP);
  }
}








