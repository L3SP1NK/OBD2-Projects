#ifndef __PNP_H__
#define __PNP_H__

#include "global.h"
#include "mhs_thread.h"
#include "mhs_event_list.h"

#ifdef __cplusplus
  extern "C" {
#endif


struct TMhsPnP
  {
  TMhsThread *Thread;
  struct TFileEvent *FileEvent;
  TMhsEventList *EventList;
  };

extern struct TMhsPnP *MhsPnP ATTRIBUTE_INTERNAL;

/***************************************************************/
/*  Funktionen                                                 */
/***************************************************************/
// Aus pnp_inc.c
void PnPInit(void) ATTRIBUTE_INTERNAL;
void PnPLock(void) ATTRIBUTE_INTERNAL;
void PnPUnlock(void) ATTRIBUTE_INTERNAL;
int32_t GetDevicesInfo(struct TCanInfoVarList **devices_list, int32_t flags) ATTRIBUTE_INTERNAL;

int32_t PnPCreate(void) ATTRIBUTE_INTERNAL;
void PnPDestroy(void) ATTRIBUTE_INTERNAL;

void UpdateTCanUsbList(void) ATTRIBUTE_INTERNAL;
void PnPEventSetup(TMhsEvent *event, uint32_t events) ATTRIBUTE_INTERNAL;
void PnPSetDeviceStatus(uint32_t index, char *snr, int32_t status, int32_t disable_pnp_event) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
