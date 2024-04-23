/***************************************************************************
                           info.c  -  description
                             -------------------
    begin             : 03.11.2010
    last modified     : 23.08.2022    
    copyright         : (C) 2010 - 2022 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include "errors.h"
#include "var_obj.h"  // Value Typen
#include "info.h"


struct TInfoTabItem
  {
  uint32_t Index;
  const char *Bezeichner;
  };

static const struct TInfoTabItem InfoTab[] = {
  {TCAN_INFO_KEY_HW_SNR,          "Hardware Snr"},          
  {TCAN_INFO_KEY_HW_ID_STR,       "Hardware ID String"},    
  {TCAN_INFO_KEY_HW_BIOS_STR,     "Bios ID String"},        
  {TCAN_INFO_KEY_HW_REVISION,     "Hardware Revision"},     
  {TCAN_INFO_KEY_HW_DATE,         "Fertigungsdatum"},       
  {TCAN_INFO_KEY_HW_VARIANT_STR,  "Hardware Variante"},
  
  {TCAN_INFO_KEY_HW_CAN_COUNT,    "Anzahl CAN Interfaces"},  
  {TCAN_INFO_KEY_HW_CAN_DRV,      "Treiber"},               
  {TCAN_INFO_KEY_HW_CAN_OPTO,     "Opto"},                  
  {TCAN_INFO_KEY_HW_CAN_TERM,     "Term"},                  
  {TCAN_INFO_KEY_HW_CAN_HS,       "HighSpeed"},         
  {TCAN_INFO_KEY_HW_I2C_CNT,      "Anzahl I2C Interfaces"}, 
  {TCAN_INFO_KEY_HW_SPI_CNT,      "Anzahl SPI Interfaces"},
                                                       
  {TCAN_INFO_KEY_FW_ID,           "ID"},                    
  {TCAN_INFO_KEY_FW_ID_STR,       "ID String"},             
  {TCAN_INFO_KEY_FW_VERSION,      "Version"},               
  {TCAN_INFO_KEY_FW_VERSION_STR,  "Version String"},        
  {TCAN_INFO_KEY_FW_AUTOR,        "Autor"},                 
  {TCAN_INFO_KEY_FW_OPTIOS,       "Optionen"},              
  {TCAN_INFO_KEY_FW_SNR,          "Snr"},                   
                                                       
  {TCAN_INFO_KEY_FW_CAN_FLAGS,    "CAN Features Flags"},                 
  {TCAN_INFO_KEY_FW_PUFFER_CNT,   "Anzahl Interval Puffer"},
  {TCAN_INFO_KEY_FW_FILTER_CNT,   "Anzahl Filter"},          
                              
  {TCAN_INFO_KEY_OPEN_INDEX,      "Index"},
  {TCAN_INFO_KEY_HARDWARE_ID,     ""},
  {TCAN_INFO_KEY_HARDWARE,        "Hardware"},
  {TCAN_INFO_KEY_VENDOR,          "Hersteller"},
                              
  {TCAN_INFO_KEY_DEVICE_NAME,     "Device Name"},
  {TCAN_INFO_KEY_SERIAL_NUMBER,   "Seriennummer"},
                              
  {TCAN_INFO_KEY_CAN_FEATURES,    "CAN Features"},
  {TCAN_INFO_KEY_CAN_CHANNELS,    "Anzahl CAN Kanäle"},
  {TCAN_INFO_KEY_RX_FILTER_CNT,   "Anzahl Filter"},
  {TCAN_INFO_KEY_TX_BUFFER_CNT,   "Anzahl Tx Buffer"},
  {TCAN_INFO_KEY_CAN_CLOCKS,      "Anzahl CAN Clocks"},
  {TCAN_INFO_KEY_CAN_CLOCK1,      "CAN Clock 1"},
  {TCAN_INFO_KEY_CAN_CLOCK2,      "CAN Clock 2"},
  {TCAN_INFO_KEY_CAN_CLOCK3,      "CAN Clock 3"},
  {TCAN_INFO_KEY_CAN_CLOCK4,      "CAN Clock 4"},
  {TCAN_INFO_KEY_CAN_CLOCK5,      "CAN Clock 5"},
  {TCAN_INFO_KEY_CAN_CLOCK6,      "CAN Clock 6"},
                              
  {TCAN_INFO_KEY_API_VERSION,     "API Version"},
  {TCAN_INFO_KEY_DLL,             "DLL Dateiname"},
  {TCAN_INFO_KEY_CFG_APP,         "Konfig App Dateiname"},
  {0, NULL}};

/*
  {0x1000, "ID"},
  {0x1001, "ID String"},
  {0x1002, "Version"},
  {0x1003, "Version String"},
  {0x1004, "Autor"},
  {0x1005, "Optionen"},
  {0x1006, "Snr"},
  {0x0002, "Bios ID String"},
  {0x0001, "Hardware ID String"},
  {0x0000, "Hardware Snr"},
  {0x0003, "Hardware Revision"},
  {0x0004, "Fertigungsdatum"},
  {0x000A, "Ident-String"},
  {0x8000, "Anzahl CAN Interfaces"},
  {0x8001, "CAN Features Flags"},
  {0x8010, "Treiber"},
  {0x8020, "Opto"},
  {0x8030, "Term"},
  {0x8040, "HighSpeed"},
  {0x8050, "Anzahl Interval Puffer"},
  {0x8060, "Anzahl Filter"},
  {0x8100, "Anzahl I2C Interfaces"},
  {0x8200, "Anzahl SPI Interfaces"}};*/

static const unsigned char InfoVarTypeTranslate[] =
  {0x00,      // 0x00
   VT_BYTE,   // 0x01 BYTE
   VT_UBYTE,  // 0x02 UBYTE
   VT_WORD,   // 0x03 WORD
   VT_UWORD,  // 0x04 UWORD
   VT_LONG,   // 0x05 LONG
   VT_ULONG,  // 0x06 ULONG
   0x00,      // 0x07
   0x00,      // 0x08
   0x00,      // 0x09
   VT_HBYTE,  // 0x0A HBYTE
   0x00,      // 0x0B
   VT_HWORD,  // 0x0C HWORD
   0x00,      // 0x0D
   VT_HLONG,  // 0x0E HLONG
   0x00};     // 0x0F


void InfoVarDestroy(struct TInfoVarList **liste)
{
struct TInfoVarList *l, *h;

if (!liste)
  return;
l = *liste;
while (l)
  {
  h = l->Next;
  safe_free(l->Data);
  safe_free(l);
  l = h;
  }
*liste = NULL;
}


struct TInfoVarList *InfoVarAdd(struct TInfoVarList **liste)
{
struct TInfoVarList *l;

l = *liste;
if (!l)
  {
  // Liste ist leer
  l = (struct TInfoVarList *)mhs_calloc(1, sizeof(struct TInfoVarList));
  *liste = l;
  }
else
  { // Neues Element anhängen
  while (l->Next != NULL) l = l->Next;
  l->Next = (struct TInfoVarList *)mhs_calloc(1, sizeof(struct TInfoVarList));
  l = l->Next;
  if (!l)
    return(NULL);        // Nicht genügend Arbetsspeicher
  }
l->Next = NULL;
return(l);
}


int32_t InfoVarAddEx(TCanDevice *dev, uint32_t key, uint8_t type, char *data, uint8_t size) 
{
struct TInfoVarList *info;

if (!data)
  return(0);
if (!(info = InfoVarAdd(&dev->InfoVars)))
  return(-1);
info->Type = type;
info->Index = key;
info->IndexString = GetIndexString(key);
if (!size)
  size = (uint8_t)strlen(data) + 1;  
info->Data = mhs_malloc0(size + 1);
memcpy(info->Data, data, size);  
info->Size = size;  
return(0);
}


const char *GetIndexString(uint32_t index)
{
const char *bezeichner;
const struct TInfoTabItem *item;

for (item = InfoTab; (bezeichner = item->Bezeichner); item++)
  {  
  if (item->Index == index)
    return(bezeichner);
  }
return(NULL);
}


char *CreateInfoString(struct TInfoVarList *info)
{
int32_t size, n;
uint32_t value, tag, monat, jahr, major,  minor, revision;
char *str, *p;
struct TInfoVarList *i;

size = 0;  // <*> geändert alt 1
for (i = info; i; i = i->Next)
  {
  if (i->IndexString)
    size += ((int32_t)strlen(i->IndexString) + 1);
  else
    size += 7;
  switch (i->Type)
    {
    case IT_BYTE   : {
                     size += 5;
                     break;
                     }
    case IT_UBYTE  : {
                     size += 5;
                     break;
                     }
    case IT_HBYTE  : {
                     size += 5;
                     break;
                     }
    case IT_WORD   : {
                     size += 7;
                     break;
                     }
    case IT_UWORD  : {
                     size += 6;
                     break;
                     }
    case IT_HWORD  : {
                     size += 7;
                     break;
                     }
    case IT_LONG   : {
                     size += 12;
                     break;
                     }
    case IT_ULONG  : {
                     size += 11;
                     break;
                     }
    case IT_HLONG  : {
                     size += 11;
                     break;
                     }
    case IT_REVISION :      // Software Version
                     {
                     size += 13;
                     break;
                     }
    case IT_DATE   : {      // Datum
                     size += 13;
                     break;
                     }                     
    case IT_STRING : {
                     i->Data[i->Size] = '\0';
                     size += ((int32_t)strlen(i->Data) + 1);
                     break;
                     }
    }
  }

if (!size)
  return(NULL);
str = (char *)mhs_malloc(size + 1);  // <*> neu +1
str[size] = '\0'; // <*> alt size-1
p = str;

for (; info; info = info->Next)
  {
  if (info->IndexString)
    n = safe_sprintf(p, size, "%s=", info->IndexString);
  else
    n = safe_sprintf(p, size, "%#04X=", info->Index);
  p += n;
  size -= n;
  switch (info->Type)
    {
    case IT_BYTE   : {
                     n = safe_sprintf(p, size, "%d;", *((signed char*)(info->Data)));
                     break;
                     }
    case IT_UBYTE  : {
                     n = safe_sprintf(p, size, "%u;", *((unsigned char *)(info->Data)));
                     break;
                     }
    case IT_HBYTE  : {
                     n = safe_sprintf(p, size, "%#02X;", *((unsigned char *)(info->Data)));
                     break;
                     }
    case IT_WORD   : {
                     n = safe_sprintf(p, size, "%d;", *((signed short *)(info->Data)));
                     break;
                     }
    case IT_UWORD  : {
                     n = safe_sprintf(p, size, "%u;", *((uint16_t *)(info->Data)));
                     break;
                     }
    case IT_HWORD  : {
                     n = safe_sprintf(p, size, "%#04X;", *((uint16_t *)(info->Data)));
                     break;
                     }
    case IT_LONG   : {
                     n = safe_sprintf(p, size, "%d;", *((int32_t *)(info->Data)));
                     break;
                     }
    case IT_ULONG  : {
                     n = safe_sprintf(p, size, "%u;", *((uint32_t *)(info->Data)));
                     break;
                     }
    case IT_HLONG  : {
                     n = safe_sprintf(p, size, "%#08X;", *((uint32_t *)(info->Data)));
                     break;
                     }
    case IT_REVISION :
                     {
                     value = *((uint32_t *)(info->Data));
                     major = value / 10000;
                     value = value % 10000;  
                     minor = value / 100;                   
                     revision = value % 100;                   
                     n = safe_sprintf(p, size, "%u.%u.%u;", major, minor, revision); 
                     break; 
                     }
    case IT_DATE   : {
                     value = *((uint32_t *)(info->Data));
                     tag = GetBCD(value & 0xFF, 2);
                     monat = GetBCD((value >> 8) & 0xFF, 2);  
                     jahr = GetBCD((value >> 16) & 0xFFFF, 4);
                     n = safe_sprintf(p, size, "%02u.%02u.%4u;", tag, monat, jahr);
                     break; 
                     }                     
    case IT_STRING : {
                     info->Data[info->Size] = '\0';
                     n = safe_sprintf(p, size, "%s;", info->Data);
                     break;
                     }
    default        : n = 0;
    }
  p += n;
  size -= n;
  }
return(str);
}


struct TInfoVarList *GetInfoByIndex(TCanDevice *dev, uint32_t index)
{
struct TInfoVarList *info;

for (info = dev->InfoVars; info; info = info->Next)
  {
  if (info->Index == index)
    return(info);
  }
return(NULL);
}


uint32_t GetInfoValueAsULong(struct TInfoVarList *info)
{
uint32_t value;

value = 0;
if (!info)
  return(value);
switch (info->Type)
  {
  case IT_BYTE   :
  case IT_UBYTE  :
  case IT_HBYTE  : {
                   value = *((unsigned char *)(info->Data));
                   break;
                   }
  case IT_WORD   :
  case IT_UWORD  :
  case IT_HWORD  : {
                   value = *((uint16_t *)(info->Data));
                   break;
                   }
  case IT_LONG   :
  case IT_ULONG  :
  case IT_HLONG  : {
                   value = *((uint32_t *)(info->Data));
                   break;
                   }
  }

return(value);
}


char *GetInfoValueAsString(struct TInfoVarList *info)
{
if (!info)
  return(NULL);
if (info->Type != IT_STRING)
  return(NULL);
info->Data[info->Size] = '\0';
return(info->Data);
}


int32_t GetHardwareInfo(TCanDevice *dev, struct TCanInfoVar **info_var)
{
struct TInfoVarList *info;
struct TCanInfoVar *out;
uint32_t size, i;
int32_t cnt;

if (!info_var)
  return(ERR_PARAM);
//*info_var = NULL;
out = NULL;
// Anzahl der Einträge ermitteln
cnt = 0;
for (info = dev->InfoVars; info; info = info->Next)
  cnt++;
if (cnt > 0)
  {
  i = 0;
  out = (struct TCanInfoVar*)mhs_calloc(cnt, sizeof(struct TCanInfoVar));
  for (info = dev->InfoVars; info; info = info->Next)
    {
    out[i].Key = info->Index;    
    if (info->Type == IT_STRING)
      out[i].Type = VT_STRING;
    else if (info->Type == IT_REVISION)      // Software Version
      out[i].Type = VT_REVISION;
    else if (info->Type == IT_DATE)          // Datum
      out[i].Type = VT_DATE;
    else
      out[i].Type = InfoVarTypeTranslate[(info->Type & 0x000F)];
    size = info->Size;
    out[i].Size = size;
    if ((size) && (size <= 254))
      {
      memcpy(out[i].Data, info->Data, size);
      if (info->Type == IT_STRING)
        out[i].Data[size] = '\0';
      }
    i++;
    }
  }
*info_var = out;
return(cnt);
}
