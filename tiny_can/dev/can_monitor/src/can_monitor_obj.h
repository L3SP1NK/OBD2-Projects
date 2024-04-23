#ifndef __CAN_MONITOR_OBJ__
#define __CAN_MONITOR_OBJ__

#include <glib.h>
#include "can_monitor.h"
#include "mhs_obj.h"

struct TObjTinyCanStatus
  {
  struct TMhsObj Obj;
  struct TDeviceStatus DeviceStatus;
  struct TDeviceStatus DeviceStatusOut;
  };

struct TObjTinyCanPnP
  {
  struct TMhsObj Obj;
  int PnPStatus;
  int PnPStatusOut;
  };

typedef void(*TTinyCanStatusCB)(struct TMhsObj *obj, struct TDeviceStatus *device_status, gpointer user_data);
typedef void(*TTinyCanPnPCB)(struct TMhsObj *obj, int status, gpointer user_data);

extern struct TObjTinyCanStatus *ObjTinyCanStatus;
extern struct TObjTinyCanPnP *ObjTinyCanPnP;
extern struct TMhsObj *ObjTinyCanBufferFull;
extern struct TMhsObj *ObjRxDFilter;

int CanViewObjInit(void);


//void CALLBACK_TYPE CanPnPEvent(guint32 index, gint32 status);
//void CALLBACK_TYPE CanStatusEvent(guint32 index, struct TDeviceStatus *DeviceStatus);

#endif
