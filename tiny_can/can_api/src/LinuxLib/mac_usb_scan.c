/***************************************************************************
                         mac_usb_hlp.c  -  description
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
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/IOBSD.h>
#include <string.h>
#include <stdio.h>
#include "util.h"
#include "usb_scan.h"


struct THardwareList
  {
  const char *Description;
  unsigned short bcdDevice;
  unsigned short idVendor;
  unsigned short idProduct;
  };

static const struct THardwareList HardwareList[] =
  // Description,     BcdDevice, Vendor-Id, Product-Id
  {{"Tiny-CAN II-XL",  0x0600,    0x0403,    0x6001},  // FT232RL
   {"Tiny-CAN M1",     0x0600,    0x0403,    0x6001},  // FT232RL
   {"Tiny-CAN M2",     0x1000,    0x0403,    0x6015},  // X_SERIES
   {"Tiny-CAN IV-XL",  0x0700,    0x0403,    0x6010},  // FT2232H
   {"Tiny-CAN M3",     0x0700,    0x0403,    0x6010},  // FT2232H
   {"Tiny-CAN I-XL",   0x1000,    0x0403,    0x6015},  // X_SERIES
   {NULL,              0x0000,    0x0000,    0x0000}};

static const char FTDI_DEVICE_STR[] = {"/dev/cu.usbserial-"};


void UsbDevListDestroy(TUsbDevList **list)
{
TUsbDevList *ptr, *l;

if (!list)
  return;
l = *list;
while (l)
  {
  ptr = l->Next;
  safe_free(l->DeviceName);
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
                            char *manufacturer, char *product, char *serial)
{
TUsbDevList *ptr, *neu;

if (!list)
  return(NULL);
neu = (TUsbDevList *)mhs_calloc(1, sizeof(TUsbDevList));
if (!neu)
  return(NULL);        // Nicht genügend Arbetsspeicher
neu->Next = NULL;
neu->idProduct = id_product;
neu->idVendor = id_vendor;
neu->bcdDevice = bcd_device;
neu->Manufacturer = manufacturer;
neu->Product = product;
neu->SerialNumber = serial;
neu->DeviceName = device_name;
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


static char *get_str_from_cf_string(CFTypeRef cf_str)
{
const char *s;
char *out_str;
int32_t size;

if (!cf_str)
  return(NULL);
if ((s = CFStringGetCStringPtr((CFStringRef)cf_str, kCFStringEncodingUTF8)))
  out_str = mhs_strdup(s);
else
  {
  size = CFStringGetLength((CFStringRef)cf_str) * sizeof(uint16_t);
  if ((out_str = mhs_malloc(size + 1)))
    {
    out_str[size] = '\0';
    if (!CFStringGetCString((CFStringRef)cf_str, out_str, size, kCFStringEncodingUTF8))
      mhs_free(out_str);
    }
  }
CFRelease(cf_str);
return(out_str);
}


TUsbDevList *UsbScan(void)
{
io_object_t service;
io_iterator_t iterator;
CFMutableDictionaryRef dictionary;
CFTypeRef cf_ref;
io_registry_entry_t parent;
int32_t i;
const struct THardwareList *hw;
char *description, *serial, *manufacturer, *devicename, *s;
const char *desc;
TUsbDevList *list;

description = NULL;
serial = NULL;
manufacturer = NULL;
devicename = NULL;
iterator = 0;
list = NULL;
// create a dictionary that looks for all BSD modems
if (!(dictionary = IOServiceMatching(kIOSerialBSDServiceValue)))
  return(NULL);
CFDictionarySetValue(dictionary, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDAllTypes));
// then create the iterator with all the matching devices
if(IOServiceGetMatchingServices(kIOMasterPortDefault, dictionary, &iterator ) != KERN_SUCCESS)
  return(NULL);
// Iterate through all modems found. In this example, we bail after finding the first modem
while ((service = IOIteratorNext(iterator)))
  {
  // ** Device Name
  cf_ref = IORegistryEntrySearchCFProperty(service, kIOServicePlane, CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, 0);
  if (!(devicename = get_str_from_cf_string(cf_ref)))
    {
    IOObjectRelease(service);
    continue;
    }
  // then, because the callout device could be any old thing, and because the reference to the modem returned by the
  // iterator doesn't include much device specific info, look at its parent, and check the product name
  desc = NULL;
  if (IORegistryEntryGetParentEntry(service, kIOServicePlane, &parent ) == KERN_SUCCESS)
    {
    cf_ref = IORegistryEntrySearchCFProperty(parent, kIOServicePlane, CFSTR("Product Name"), kCFAllocatorDefault, 0);

    if ((description = get_str_from_cf_string(cf_ref)))
      {
      for (hw = HardwareList; (desc = hw->Description); hw++)
        {
        if (!strcmp(desc, description))
          break;
        }
      }
    IOObjectRelease(parent);
    }

  if (desc)
    {
    if ((s = strstr(devicename, FTDI_DEVICE_STR)))
      {
      s += strlen(FTDI_DEVICE_STR);
      serial = mhs_strdup(s);
      if (hw->bcdDevice == 0x0700)  // FT2232H
        {
        i = strlen(serial) - 1;
        if (serial[i] == 'A')
          manufacturer = mhs_strdup("FTDI A");
        else if (serial[i] == 'B')
          manufacturer = mhs_strdup("FTDI B");
        else
          manufacturer = mhs_strdup("FTDI");
        }
      else
        manufacturer = mhs_strdup("FTDI");

      (void)UsbDevAdd(&list, devicename, hw->idProduct, hw->idVendor,
           hw->bcdDevice, manufacturer, description, serial);
      devicename = NULL;
      manufacturer = NULL;
      description = NULL;
      serial = NULL;
      }
    }
  safe_free(description);
  safe_free(serial);
  safe_free(manufacturer);
  safe_free(devicename);

  IOObjectRelease(service);
  }
IOObjectRelease(iterator);
return(list);
}



