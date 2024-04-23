/***************************************************************************
                          pnp_inc.c  -  description
                             -------------------
    begin             : 18.10.2012
    last modify       : 23.07.2021
    copyright         : (C) 2008 - 2021 by MHS-Elektronik GmbH & Co. KG, Germany
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
struct UsbChipIdent
  {
  unsigned short idVendor;
  unsigned short idProduct;
  };

static const struct UsbChipIdent UsbChipList[] =
  // Vendor-Id, Product-Id
  {{  0x0403,    0x6001},
   {  0x0403,    0x6010},
   {  0x0403,    0x6015},
   {  0x0000,    0x0000}};


struct TMhsPnP *MhsPnP = NULL;

#define DEVICES_INFO_SIZE 11

/*
******************** PnPInit **********************
*/
void PnPInit(void)
{
MhsPnP = NULL;
}


/*
******************** PnPLock **********************
*/
void PnPLock(void)
{
if (!MhsPnP)
  return;
mhs_enter_critical(MhsPnP->Thread);
}


/*
******************** PnPUnlock *******************
*/
void PnPUnlock(void)
{
if (!MhsPnP)
  return;
mhs_leave_critical(MhsPnP->Thread);
}


int32_t GetDeviceCount(int32_t flags)
{
struct TTCanUsbList *usb_list;
uint32_t num_devs;

num_devs = 0;
PnPLock();
// Anzahl der Einträge ermitteln
for (usb_list = TCanUsbList; usb_list; usb_list = usb_list->Next)
  {
  if ((!(flags & DEV_LIST_SHOW_UNCONNECT)) && (usb_list->Status == PNP_DEVICE_UNPLUGED))
    continue;
  if ((flags & DEV_LIST_SHOW_TCAN_ONLY) && (!usb_list->Modul))
    continue;
  num_devs++;
  }
PnPUnlock();
return(num_devs);
}


static void CreateDeviceInfoVars(struct TCanInfoVar *info, struct TTCanUsbList *item)
{
// *** Device Index
info[0].Key = TCAN_INFO_KEY_OPEN_INDEX;
info[0].Type = VT_ULONG; 
info[0].Size = 4;
*((uint32_t *)(info[0].Data)) = item->TCanIdx;
// *** Hardware ID
info[1].Key = TCAN_INFO_KEY_HARDWARE_ID;
info[1].Type = VT_ULONG; 
info[1].Size = 4;
if (item->Modul)
  *((uint32_t *)(info[1].Data)) = item->Modul->HwId;
else
  *((uint32_t *)(info[1].Data)) = 0;
// *** Hardware (Description) 
info[2].Key = TCAN_INFO_KEY_HARDWARE;
info[2].Type = VT_STRING; 
info[2].Size = 0;
if (item->Description)
  safe_strcpy(info[2].Data, DESCRIPTION_MAX_SIZE, item->Description);
else
  info[2].Data[0] = '\0';   
// **** Device Name
info[3].Key = TCAN_INFO_KEY_DEVICE_NAME;
info[3].Type = VT_STRING; 
info[3].Size = 0;    
if (item->DeviceName[0])
  safe_strcpy(info[3].Data, DEVICE_NAME_MAX_SIZE, item->DeviceName[0]);
else    
  info[3].Data[0] = '\0';
// *** Seriennummer 
info[4].Key = TCAN_INFO_KEY_SERIAL_NUMBER;
info[4].Type = VT_STRING; 
info[4].Size = 0; 
if (item->Serial)
  safe_strcpy(info[4].Data, SERIAL_NUMBER_MAX_SIZE, item->Serial);
else    
  info[4].Data[0] = '\0';
// *** Features
info[5].Key = TCAN_INFO_KEY_CAN_FEATURES;
info[5].Type = VT_ULONG; 
info[5].Size = 4;
// *** CAN Channels
info[6].Key = TCAN_INFO_KEY_CAN_CHANNELS;
info[6].Type = VT_ULONG; 
info[6].Size = 4; 
// *** RX Filter Count
info[7].Key = TCAN_INFO_KEY_RX_FILTER_CNT;
info[7].Type = VT_ULONG; 
info[7].Size = 4; 
// *** Tx Puffer Cunt
info[8].Key = TCAN_INFO_KEY_TX_BUFFER_CNT;
info[8].Type = VT_ULONG; 
info[8].Size = 4; 
// *** CAN Clocks
info[9].Key = TCAN_INFO_KEY_CAN_CLOCKS;
info[9].Type = VT_ULONG; 
info[9].Size = 4;
*((uint32_t *)(info[1].Data)) = 1;
// *** CAN Clock 1
info[10].Key = TCAN_INFO_KEY_CAN_CLOCK1;
info[10].Type = VT_ULONG; 
info[10].Size = 4; 
   
if (item->Modul)
  {
  // *** Features
  *((uint32_t *)(info[5].Data)) = item->Modul->CanFeaturesFlags;   
  // *** CAN Channels          
  *((uint32_t *)(info[6].Data)) = item->Modul->CanChannels;
  // *** RX Filter Count  
  *((uint32_t *)(info[7].Data)) = item->Modul->HwRxFilterCount;
  // *** Tx Puffer Cunt  
  *((uint32_t *)(info[8].Data)) = item->Modul->HwTxPufferCount;
  // *** CAN Clock 1  
  *((uint32_t *)(info[10].Data)) = item->Modul->CanClock; // <*> ändern
  }
else
  {
  // *** Features
  *((uint32_t *)(info[6].Data)) = 0;
  // *** CAN Channels          
  *((uint32_t *)(info[6].Data)) = 1;
  // *** RX Filter Count  
  *((uint32_t *)(info[7].Data)) = 4;
  // *** Tx Puffer Cunt  
  *((uint32_t *)(info[8].Data)) = 4;
  // *** CAN Clock 1  
  *((uint32_t *)(info[10].Data)) = 24;                
  }
}


int32_t GetDevicesInfo(struct TCanInfoVarList **devices_list, int32_t flags)
{
struct TCanInfoVarList *list;
struct TCanInfoVar *dev_list;
struct TTCanUsbList *usb_list;
uint32_t header_size, var_size;
int32_t num_devs, i;

if (!devices_list)
  return(ERR_PARAM);
*devices_list = NULL;  
if (!MhsPnP)
  return(0);
num_devs = 0;
PnPLock();
// Anzahl der Einträge ermitteln
for (usb_list = TCanUsbList; usb_list; usb_list = usb_list->Next)
  {
  if ((!(flags & DEV_LIST_SHOW_UNCONNECT)) && (usb_list->Status == PNP_DEVICE_UNPLUGED))
    continue;
  if ((flags & DEV_LIST_SHOW_TCAN_ONLY) && (!usb_list->Modul))
    continue;
  num_devs++;
  }
if (num_devs)
  {
  header_size = sizeof(struct TCanInfoVarList) * (num_devs + 1);
  var_size = (DEVICES_INFO_SIZE * sizeof(struct TCanInfoVar)) * num_devs;
  list = (struct TCanInfoVarList *)mhs_malloc(header_size + var_size);
  list[num_devs].List = NULL;
  list[num_devs].Size = 0;
  i = 0;
  dev_list = (struct TCanInfoVar *)((uint8_t *)list + header_size); 
  for (usb_list = TCanUsbList; usb_list; usb_list = usb_list->Next)
    {
    if ((!(flags & DEV_LIST_SHOW_UNCONNECT)) && (usb_list->Status == PNP_DEVICE_UNPLUGED))
      continue;
    if ((flags & DEV_LIST_SHOW_TCAN_ONLY) && (!usb_list->Modul))
      continue;
    list[i].List = dev_list;
    list[i].Size = DEVICES_INFO_SIZE;
    CreateDeviceInfoVars(dev_list, usb_list);
    dev_list += DEVICES_INFO_SIZE;
    i++;
    }
  }
else
  list = NULL;
PnPUnlock();
*devices_list = list;
return(num_devs);
}


int32_t GetDeviceList(struct TCanDevicesList **devices_list, int32_t flags)
{
struct TCanDevicesList *list;
struct TTCanUsbList *usb_list;
uint32_t num_devs, i;

if (!devices_list)
  return(ERR_PARAM);
*devices_list = NULL;
if (!MhsPnP)
  return(0);
PnPLock();
list = NULL;
num_devs = 0;
// Anzahl der Einträge ermitteln
for (usb_list = TCanUsbList; usb_list; usb_list = usb_list->Next)
  {
  if ((!(flags & DEV_LIST_SHOW_UNCONNECT)) && (usb_list->Status == PNP_DEVICE_UNPLUGED))
    continue;
  if ((flags & DEV_LIST_SHOW_TCAN_ONLY) && (!usb_list->Modul))
    continue;
  num_devs++;
  }
if (num_devs)
  {
  list = (struct TCanDevicesList*)mhs_calloc(num_devs, sizeof(struct TCanDevicesList));
  i = 0;
  for (usb_list = TCanUsbList; usb_list; usb_list = usb_list->Next)
    {
    if ((!(flags & DEV_LIST_SHOW_UNCONNECT)) && (usb_list->Status == PNP_DEVICE_UNPLUGED))
      continue;
    if ((flags & DEV_LIST_SHOW_TCAN_ONLY) && (!usb_list->Modul))
      continue;
    if (usb_list->DeviceName[0])  // [0] ergänzt
      safe_strcpy(list[i].DeviceName, DEVICE_NAME_MAX_SIZE, usb_list->DeviceName[0]);
    if (usb_list->Description)
      safe_strcpy(list[i].Description, DESCRIPTION_MAX_SIZE, usb_list->Description);
    if (usb_list->Serial)
      safe_strcpy(list[i].SerialNumber, SERIAL_NUMBER_MAX_SIZE, usb_list->Serial);
    if (usb_list->Modul)
      {
      list[i].ModulFeatures.CanClock = usb_list->Modul->CanClock;
      list[i].ModulFeatures.Flags = usb_list->Modul->CanFeaturesFlags;
      list[i].ModulFeatures.CanChannelsCount = usb_list->Modul->CanChannels;
      list[i].ModulFeatures.HwRxFilterCount = usb_list->Modul->HwRxFilterCount;
      list[i].ModulFeatures.HwTxPufferCount = usb_list->Modul->HwTxPufferCount;
      list[i].HwId = usb_list->Modul->HwId;
      }
    else
      {
      list[i].ModulFeatures.CanClock = 24;
      list[i].ModulFeatures.Flags = 0;
      list[i].ModulFeatures.CanChannelsCount = 1;
      list[i].ModulFeatures.HwRxFilterCount = 4;
      list[i].ModulFeatures.HwTxPufferCount = 4;
      list[i].HwId = 0;
      }
    list[i].TCanIdx = usb_list->TCanIdx;
    i++;
    }
  }
PnPUnlock();
*devices_list = list;
return(num_devs);
}


#ifndef __WIN32__

static int32_t CheckScanComplete(TUsbDevList *usb_dev, uint32_t num)
{
uint32_t cnt, i;

cnt = 0;
for (i = 0; i < 4; i++)
  {
  if ((usb_dev->DeviceName[i]) && (strlen(usb_dev->DeviceName[i])))
    cnt++;
  }
if (cnt >= num)
  return(0);
else
  return(-1);
}

#endif


#ifndef __APPLE__
/*
******************** UpdateListAndSetEvent *******************
*/
static void UpdateListAndSetEvent(uint32_t only_event_check)
{
struct TTCanUsbList *usb_list;
uint32_t change;

if (!MhsPnP)
  return;
change = 0;
PnPLock();
if (!only_event_check)
  UpdateTCanUsbList();
for (usb_list = TCanUsbList; usb_list; usb_list = usb_list->Next)
  {
  if (usb_list->LastSignaledStatus != usb_list->Status)
    {
    usb_list->LastSignaledStatus = usb_list->Status;
    change = 1;
    }
  }
PnPUnlock();
if (change)
  MhsEventListExecute(MhsPnP->EventList);
}



static int32_t CheckFtdi(TUsbDevList *usb_dev, uint32_t *type)
{
int32_t hit, i;

hit = 0;
for (i = 0; UsbChipList[i].idVendor; i++)
  {
  if ((UsbChipList[i].idVendor == usb_dev->idVendor) &&
      (UsbChipList[i].idProduct == usb_dev->idProduct))
    {
    hit = 1;
    break;
    }
  }
if ((hit) && (type))
  {
  switch (usb_dev->bcdDevice)
    {
    case 0x0900 : {
                  *type = FT_DEVICE_232H;
                  break;
                  }
    case 0x0800 : {  // FT4232H
                  *type = FT_DEVICE_4232H;
#ifndef __WIN32__
                  if (CheckScanComplete(usb_dev, 4))
                    hit = 0;
#endif
                  break;
                  }
    case 0x0700 : {  // FT2232H
                  *type = FT_DEVICE_2232H;
#ifndef __WIN32__
                  if (CheckScanComplete(usb_dev, 2))
                    hit = 0;
#endif
                  break;
                  }
    case 0x0600 : {  // FT232R
                  *type = FT_DEVICE_232R;
                  break;
                  }
    case 0x0500 : {  // FT2232C
                  *type = FT_DEVICE_2232C;
#ifndef __WIN32__
                  if (CheckScanComplete(usb_dev, 2))
                    hit = 0;
#endif
                  break;
                  }
//#ifdef __WIN32__ <*>
    case 0x0200 : 
//#else
    case 0x1000 : {
//#endif
                  *type = FT_DEVICE_X_SERIES;
                  break;
                  }
    default     : {
                  *type = FT_DEVICE_BM;
                  }
    }
  }
return(hit);
}
#endif



#ifdef __APPLE__
void UpdateTCanUsbList(void)
{
const struct TCanModulDesc *info_db;
const char *description;
char *product, *snr;
char **device_name;
TUsbDevList *usb_dev_list, *usb_dev;
struct TTCanUsbList *tcan_list;
uint32_t type;

usb_dev_list = UsbScan();
for (usb_dev = usb_dev_list; usb_dev; usb_dev = usb_dev->Next)
  {
  //product = usb_dev->Product;
  //snr = usb_dev->SerialNumber;
  usb_dev->User = 1; // Hit -> Tiny-CAN Device erkannt
  }
for (tcan_list = TCanUsbList; tcan_list; tcan_list = tcan_list->Next)
  {
  for (usb_dev = usb_dev_list; usb_dev; usb_dev = usb_dev->Next)
    {
    if ((usb_dev->User == 0) || (usb_dev->User == 2))
      continue;
    product = usb_dev->Product;
    snr = usb_dev->SerialNumber;
    if ((!safe_strcmp(tcan_list->Description, product)) &&
        (!safe_strcmp(tcan_list->Serial, snr)))
      {
      usb_dev->User = 2;
      if (tcan_list->Status < PNP_DEVICE_PLUGED)
        tcan_list->Status = PNP_DEVICE_PLUGED;
      break;
      }
    }
  if (!usb_dev)
    tcan_list->Status = PNP_DEVICE_UNPLUGED;
  }
for (usb_dev = usb_dev_list; usb_dev; usb_dev = usb_dev->Next)
  {
  if (usb_dev->User == 1)
    {
    product = usb_dev->Product;
    snr = usb_dev->SerialNumber;
    device_name = usb_dev->DeviceName;
    switch (usb_dev->bcdDevice)
      {
      case 0x0900 : {
                    type = FT_DEVICE_232H;
                    break;
                    }
      case 0x0800 : {  // FT4232H
                    type = FT_DEVICE_4232H;
                    break;
                    }
      case 0x0700 : {  // FT2232H
                    type = FT_DEVICE_2232H;
                    break;
                    }
      case 0x0600 : {  // FT232R
                    type = FT_DEVICE_232R;
                    break;
                    }
      case 0x0500 : {  // FT2232C
                    type = FT_DEVICE_2232C;
                    break;
                    }
      case 0x1000 : { //0x0200 : {
                    type = FT_DEVICE_X_SERIES;
                    break;
                    }
      default     : {
                    type = FT_DEVICE_BM;
                    }
      }
    for (info_db = &ExCanModulTable[0]; info_db->HwId; info_db++)
      {
      if (!(description = info_db->Description))
        continue;
      if (!safe_strcmp(description, product))
        break;
      }
    if (!info_db->HwId)
      info_db = NULL;
    (void)AddTCanUsbDevice(&TCanUsbList, device_name, product, snr, type, INDEX_INVALID, info_db);
    }
  }
UsbDevListDestroy(&usb_dev_list);
}
#else


void UpdateTCanUsbList(void)
{
const struct TCanModulDesc *info_db;
const char *description;
char *product, *snr;
TUsbDevList *usb_dev_list, *usb_dev;
struct TTCanUsbList *tcan_list;
uint32_t type;

usb_dev_list = UsbScan();

for (usb_dev = usb_dev_list; usb_dev; usb_dev = usb_dev->Next)
  {
  // *** Nur FTDI Chips
  if (CheckFtdi(usb_dev, &type))
    {
    if (!(product = usb_dev->Product))
      continue;
    for (info_db = &ExCanModulTable[0]; info_db->HwId; info_db++)
      {
      if (!(description = info_db->Description))
        continue;
      if (!safe_strcmp(description, product))
        {
        usb_dev->User = 1; // Hit -> Tiny-CAN Device erkannt
        break;
        }
      }
    }
  }

for (tcan_list = TCanUsbList; tcan_list; tcan_list = tcan_list->Next)
  {
  for (usb_dev = usb_dev_list; usb_dev; usb_dev = usb_dev->Next)
    {
    if ((usb_dev->User == 0) || (usb_dev->User == 2))
      continue;
    product = usb_dev->Product;
    snr = usb_dev->SerialNumber;
    if ((!safe_strcmp(tcan_list->Description, product)) &&
        (!safe_strcmp(tcan_list->Serial, snr)))
      {
      usb_dev->User = 2;
      if (tcan_list->Status < PNP_DEVICE_PLUGED)
        tcan_list->Status = PNP_DEVICE_PLUGED;
      break;
      }
    }
  if (!usb_dev)
    tcan_list->Status = PNP_DEVICE_UNPLUGED;
  }
for (usb_dev = usb_dev_list; usb_dev; usb_dev = usb_dev->Next)
  {
  if (usb_dev->User == 1)
    {
    (void)CheckFtdi(usb_dev, &type);
    snr = usb_dev->SerialNumber;
    if ((product = usb_dev->Product))
      {
      for (info_db = &ExCanModulTable[0]; info_db->HwId; info_db++)
        {
        if (!(description = info_db->Description))
          continue;
        if (!strcmp(description, product))
          break;
        }
      }
    else
      continue;
#ifdef __WIN32__
    (void)AddTCanUsbDevice(&TCanUsbList, NULL, product, snr, type, INDEX_INVALID, info_db);
#else
    (void)AddTCanUsbDevice(&TCanUsbList, usb_dev->DeviceName, product, snr, type, INDEX_INVALID, info_db);
#endif
    }
  }
UsbDevListDestroy(&usb_dev_list);
}
#endif


void PnPEventSetup(TMhsEvent *event, uint32_t events)
{
if (!MhsPnP)
  return;
MhsEventListAddDelete(MhsPnP->EventList, event, events);
}


void PnPSetDeviceStatus(uint32_t index, char *snr, int32_t status, int32_t disable_pnp_event)
{
int hit;
struct TTCanUsbList *usb_dev;

if (!MhsPnP)
  return;
hit = 0;
if ((index == INDEX_INVALID) && (!snr))
  return;
PnPLock();
for (usb_dev = TCanUsbList; usb_dev; usb_dev = usb_dev->Next)
  {
  if (snr)  // Device mit einer bestimmten Snr suchen u. index setzen
    {
    if (!safe_strcmp(usb_dev->Serial, snr))
      {
      usb_dev->TCanIdx = index;
      break;
      }
    }
  else
    {      // Device nach index suchen
    if (usb_dev->TCanIdx == index)
      break;
    }
  }
if (usb_dev)
  {
  if (status <= PNP_DEVICE_PLUGED)
    {
    if (usb_dev->Status > status)
      usb_dev->Status = status;
    usb_dev->TCanIdx = INDEX_INVALID;
    hit = 1;
    }
  else if (usb_dev->Status != status)
    {
    usb_dev->Status = status;
    hit = 1;
    }
  if ((hit) && (!disable_pnp_event)) // <*> && (!disable_pnp_event) neu
    {
    usb_dev->LastSignaledStatus = usb_dev->Status;
    /*if (disable_pnp_event)  // <*> geändert
      usb_dev->LastSignaledStatus = usb_dev->Status; */
#ifdef __WIN32__
    if (MhsPnP->WinHandle)
      PostMessage(MhsPnP->WinHandle, WM_USER_PNP, 0, 0);
#elif defined(__APPLE__)
     // <*> fehlt
#else
    (void)set_file_event(MhsPnP->FileEvent, 0x01);
#endif
    }
  }
PnPUnlock();
}
