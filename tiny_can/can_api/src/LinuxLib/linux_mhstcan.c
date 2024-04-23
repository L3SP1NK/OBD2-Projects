/***************************************************************************
                        linux_mhstcan.c  -  description
                             -------------------
    begin             : 14.07.2012
    last modified     : 21.01.2021    
    copyright         : (C) 2012 - 2021 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include "api_calls.h"
#include "api_cp.h"
#if (defined(ENABLE_CONFIG_FILE)) || (defined(ENABLE_CONFIG_FILE))
#include "paths.h"
#endif
#include "can_core.h"
#include "index.h"
#include "log.h"
#ifndef DISABLE_PNP_SUPPORT
  #if defined(MHSIPCAN_DRV)
    #include "net_pnp.h"
  #elif defined(MHSPASSTHRU_DRV)
    #include "passthru_pnp.h"  
  #else
    #include "usb_pnp.h"
    #include "usb_hlp.h"
  #endif  
#endif

#include "user_mem.h"
#include "mhs_user_event.h"
#include "com_io.h"
#include "os_mhstcan.h"

int32_t DriverInit = TCAN_DRV_NOT_INIT;


static void my_init(void) __attribute__ ((constructor));
static void my_fini(void) __attribute__ ((destructor));

/**************************************************************************/
/*                        F U N K T I O N E N                             */
/**************************************************************************/

/***************************************************************/
/*  Treiber Initialisieren                                     */
/***************************************************************/
static void CanLoadDrv(void)
{
ComIoDrvCreate();
#ifndef DISABLE_PNP_SUPPORT
PnPInit();
#endif
ApiCpInit();
#if (defined(ENABLE_CONFIG_FILE)) || (defined(ENABLE_CONFIG_FILE))
PathsInit();
#endif
can_core_init();
index_init();
LogInit();
#ifndef DISABLE_PNP_SUPPORT
  #ifndef MHSIPCAN_DRV
UsbHlpInit();
  #endif
#endif
UserMemInit();
mhs_user_event_init();
DriverInit = TCAN_DRV_NOT_INIT;
}



//__attribute__((constructor)) void _init(void)  APPLE
//void __attribute__ ((constructor)) my_init(void)
static void my_init(void)
{
CanLoadDrv();
}


//__attribute__((destructor)) void _fini(void)  APPLE

//void __attribute__ ((destructor)) my_fini(void)
static void my_fini(void)
{
CanDownDriverInternel();
//CanDownDriver();
}
