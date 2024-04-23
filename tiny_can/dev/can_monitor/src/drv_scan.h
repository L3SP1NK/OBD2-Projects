#ifndef __DRV_SCAN_H__
#define __DRV_SCAN_H__

#include <glib.h>
#include <gtk/gtk.h>


struct TDriverList
  {
  struct TDriverList *Next;

  gchar *Filename;
  gchar *Name;
  gchar *Version;
  gchar *Summary;
  gchar *Description;
  gchar *Info;
  };

#define INTERFACE_TYPE_USB    0x0001
#define INTERFACE_TYPE_SERIAL 0x0002

struct TDriverInfo
  {
  unsigned int InterfaceType;
  };

//extern struct TDriverList *DriverList;

void DriverListDestroy(struct TDriverList **driver_list);
struct TDriverList *DriverScan(char *dir_name);

struct TDriverInfo *GetDriverInfo(char *filename);
void DriverInfoFree(struct TDriverInfo **info);

int IsDriverDir(char *dir_name);

#endif
