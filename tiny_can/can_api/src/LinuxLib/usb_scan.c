/***************************************************************************
                            usb_hlp.c  -  description
                             -------------------
    begin             : 03.11.2010
    last modify       : 27.04.2019
    copyright         : (C) 2010 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "util.h"
#include "sysfs.h"
/*#ifdef ENABLE_LOG_SUPPORT
#include "log.h"
#endif*/
//#include "tcan_drv.h" <*>
#include "global.h"
//#include "usb_hlp.h"
#include "usb_scan.h"

//struct TTCanUsbList *TCanUsbList;

static const char USB_SERIAL_DEVICES_PATH[] = {"bus/usb-serial/devices"};


void UsbDevListDestroy(TUsbDevList **list)
{
TUsbDevList *ptr, *l;
int i;

if (!list)
  return;
l = *list;
while (l)
  {
  ptr = l->Next;
  for (i = 0; i < 4; i++)
    safe_free(l->DeviceName[i]);
  safe_free(l->Manufacturer);
  safe_free(l->Product);
  safe_free(l->SerialNumber);
  mhs_free(l);
  l = ptr;
  }
*list = NULL;
}


static TUsbDevList *UsbDevAdd(TUsbDevList **list, char *device_name, unsigned short id_product,
                            unsigned short id_vendor, unsigned short bcd_device,
                            char *manufacturer, char *product, char *serial, uint32_t interface_idx, uint32_t interface_count)
{
TUsbDevList *ptr, *neu;

if (!list)
  return(NULL);
if (interface_count > 1)
  {
  if (interface_idx >= 4)
    return(NULL);
  }
else
  interface_idx = 0;
neu = (TUsbDevList *)mhs_calloc(1, sizeof(TUsbDevList));
if (!neu)
  return(NULL);        // Nicht genügend Arbetsspeicher
neu->Next = NULL;
neu->idProduct = id_product;
neu->idVendor = id_vendor;
neu->bcdDevice = bcd_device;
neu->Manufacturer = mhs_strdup(manufacturer);
neu->Product = mhs_strdup(product);
neu->SerialNumber = mhs_strdup(serial);
neu->DeviceName[interface_idx] = mhs_strdup(device_name);
neu->User = 0;
ptr = *list;
if (!ptr)
  // Liste ist leer
  *list = neu;
else
  { // Neues Element anhängen
  while (ptr->Next != NULL) ptr = ptr->Next;
    ptr->Next = neu;
  }
return(neu);
}


static TUsbDevList *UsbDevAutoAdd(TUsbDevList **list, char *device_name, unsigned short id_product,
                            unsigned short id_vendor, unsigned short bcd_device,
                            char *manufacturer, char *product, char *serial, uint32_t interface_idx, uint32_t interface_count)
{
TUsbDevList *item;

if (!list)
  return(NULL);
if (interface_count > 1)
  {
  if (interface_idx >= 4)
    return(NULL);
  }
else
  interface_idx = 0;
for (item = *list; item; item = item->Next)
  {
  if ((item->idProduct != id_product) || (item->idVendor != id_vendor) || (item->bcdDevice != bcd_device))
    continue;
  if (strcmp(item->Product, product))
    continue;
  if (strcmp(item->SerialNumber, serial))
    continue;
  break;
  }
if (!item)
  {
  item = UsbDevAdd(list, device_name, id_product, id_vendor, bcd_device,
                   manufacturer, product, serial, interface_idx, interface_count);
  }
else
  {
  safe_free(item->DeviceName[interface_idx]);
  item->DeviceName[interface_idx] = mhs_strdup(device_name);
  }
return(item);
}



TUsbDevList *UsbScan(void)
{
//int32_t num_devs;
int32_t err;
char *path, *link, *s, *devicename;
DIR *dir;
struct dirent *entry;
char description[MAX_LINE_SIZE];
char serial[MAX_LINE_SIZE];
char manufacturer[MAX_LINE_SIZE];
unsigned short id_product, id_vendor, bcd_device;
uint32_t interface_idx, interface_count;
TUsbDevList *list;

id_product = 0;
id_vendor = 0;
bcd_device = 0;
list = NULL;
devicename = NULL;
link = NULL;
if (!(s = sysfs_get_mnt_path()))
  return(NULL);
path = mhs_strconcat(s, "/", USB_SERIAL_DEVICES_PATH, NULL);
safe_free(s);
if (!(dir = opendir(path)))
  {
  safe_free(path);
  return(NULL);
  }
//num_devs = 0;
while ((entry = readdir(dir)) != NULL)
  {
  interface_count = 0;
  // . u. .. überspringen
  if (entry->d_name[0] == '.')
    continue;
  s = mhs_strconcat(path, "/", entry->d_name, NULL);
  link = mhs_get_link(s);
  safe_free(s);
  if (!link)
    continue;
  // **** Device Name
  devicename = mhs_strconcat("/dev/", entry->d_name, NULL);
  if ((s = strrchr(link, '/')))
    *s = '\0';
  interface_idx = mhs_sys_read_as_ulong(link, "bInterfaceNumber", 0, &err);     // Port Index, zählt ab 0
  if (!err)
    {
    if ((s = strrchr(link, '/')))
      *s = '\0';
    interface_count = mhs_sys_read_as_ulong(link, "bNumInterfaces", 0, &err);   // Anzahl der Ports
    }
  // **** Description
  if (!err)
    {
    if (mhs_sys_read_value(link, "product", description, MAX_LINE_SIZE) <= 0)
      err = -1;
    }
  if (!err)
    {
    // **** Serial
    if (mhs_sys_read_value(link, "serial", serial, MAX_LINE_SIZE) <= 0)
      err = -1;
    }
  if (!err)
    {
    // Die Variable "manufacturer" ist möglicherweise nicht vorhanden, Fehler ignorieren
    (void)mhs_sys_read_value(link, "manufacturer", manufacturer, MAX_LINE_SIZE);
    }
  if (!err)
    {
    id_product = (unsigned short) mhs_sys_read_as_ulong(link, "idProduct", 16, &err);
    if (!err)
      {
      id_vendor = (unsigned short) mhs_sys_read_as_ulong(link, "idVendor", 16, &err);
      if (!err)
        bcd_device = (unsigned short) mhs_sys_read_as_ulong(link, "bcdDevice", 16, &err);
      }
    }
  if (!err)
    (void)UsbDevAutoAdd(&list, devicename, id_product, id_vendor, bcd_device, manufacturer, description,
      serial, interface_idx, interface_count);
  safe_free(devicename);
  safe_free(link);
  }
closedir(dir);
safe_free(link);
safe_free(path);
safe_free(devicename);
return(list);
}
