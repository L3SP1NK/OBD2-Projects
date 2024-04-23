/**************************************************************************/
/*               Tiny-CAN API Demoprogramm "ex_sample8"                   */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : - Beispiel für "CanExGetDeviceInfo" und             */
/*                        "CanSetSpeedUser"                               */
/*                                                                        */
/*  Version         : 1.00                                                */
/*  Datei Name      : main.c                                              */
/* ---------------------------------------------------------------------- */
/*  Datum           : 21.05.15                                            */
/*  Autor           : Demlehner Klaus, MHS-Elektronik, 94149 Kößlarn      */
/*                    info@mhs-elektronik.de  www.mhs-elektronik.de       */
/**************************************************************************/
#include "config.h"
#include "global.h"
#include <string.h>
#include <stdio.h>
#ifdef __WIN32__
#include <conio.h>
#endif
#include "util.h"
#include "can_drv.h"

// BTR Werte für 40kBit/s bei 16 u. 24 MHz
#define CAN_40K_16MHZ 0x3E53
#define CAN_40K_24MHZ 0x3E5D


#define VALUE_TYPE_DESC_SIZE 22

struct TValueTypeDesc
  {
  uint32_t Type; 
  const char *Bezeichner;
  };

static const struct TValueTypeDesc ValueTypeDesc[VALUE_TYPE_DESC_SIZE] = {
  {VT_BYTE  ,   "BYTE"},
  {VT_UBYTE ,   "UBYTE"},
  {VT_WORD  ,   "WORD"},
  {VT_UWORD ,   "UWORD"},
  {VT_LONG  ,   "LONG"},
  {VT_ULONG ,   "ULONG"},
  {VT_HBYTE ,   "HBYTE"},
  {VT_HWORD ,   "HWORD"},
  {VT_HLONG ,   "HLONG"},
  {VT_REVISION, "REVISION"},       
  {VT_DATE,     "DATE"},
  {VT_STRING,   "STRING"}};


static const char *GetValueTypeString(uint32_t type)
{
int32_t i;
const struct TValueTypeDesc *item;

for (i = 0; i < VALUE_TYPE_DESC_SIZE; i++)
  {
  item = &ValueTypeDesc[i];
  if (item->Type == type)
    return(item->Bezeichner);
  }
return(NULL);
}


static uint32_t mhs_pow10(uint32_t n)
{
uint32_t out;

out = 1;
for (; n; n--)
  {
  out *= 10;
  }
return(out);
}


static uint32_t GetBCD(uint32_t in, uint32_t len)
{
uint32_t out, exp;

out = 0;
exp = 0;
for (;len; len--)
  {
  out += ((in & 0x0F) * mhs_pow10(exp));
  exp++;
  in = in >> 4; 
  }
return(out);  
}


static void PrintHwInfo(void)
{
struct TCanInfoVar info;
uint32_t value, tag, monat, jahr, major,  minor, revision;

printf(" Key   |   Type   | Size | Value\n\r");
printf("-------+----------+------+-------------------------------\n\r");
while (CanExGetDeviceInfoGet(&info) > 0)
  {
  printf("0x%04X | %-8s |  %-3u | ", info.Key, GetValueTypeString(info.Type), info.Size);

  switch (info.Type)
    {
    case VT_BYTE   : {
                     printf("%d\n\r", *((signed char*)(info.Data)));
                     break;
                     }
    case VT_UBYTE  : {
                     printf("%u\n\r", *((unsigned char *)(info.Data)));
                     break;
                     }
    case VT_HBYTE  : {
                     printf("0x%02X\n\r", *((unsigned char *)(info.Data)));
                     break;
                     }
    case VT_WORD   : {
                     printf("%d\n\r", *((signed short *)(info.Data)));
                     break;
                     }
    case VT_UWORD  : {
                     printf("%u\n\r", *((uint16_t *)(info.Data)));
                     break;
                     }
    case VT_HWORD  : {
                     printf("0x%04X\n\r", *((uint16_t *)(info.Data)));
                     break;
                     }
    case VT_LONG   : {
                     printf("%d\n\r", *((int32_t *)(info.Data)));
                     break;
                     }
    case VT_ULONG  : {
                     printf("%u\n\r", *((uint32_t *)(info.Data)));
                     break;
                     }
    case VT_HLONG  : {
                     printf("0x%08X\n\r", *((uint32_t *)(info.Data)));
                     break;
                     }
    case VT_REVISION :
                     {
                     value = *((uint32_t *)(info.Data));
                     major = value / 10000;
                     value = value % 10000;  
                     minor = value / 100;                   
                     revision = value % 100;                   
                     printf("%u.%u.%u\n\r", major, minor, revision); 
                     break; 
                     }
    case VT_DATE   : {
                     value = *((uint32_t *)(info.Data));
                     tag = GetBCD(value & 0xFF, 2);
                     monat = GetBCD((value >> 8) & 0xFF, 2);  
                     jahr = GetBCD((value >> 16) & 0xFFFF, 4);
                     printf("%02u.%02u.%4u\n\r", tag, monat, jahr);
                     break; 
                     }                     
    case VT_STRING : {                    
                     printf("%s\n\r", info.Data);
                     break;
                     }
    }
  }
printf("-------+----------+------+-------------------------------\n\r\n\r");
}


static void PrintDeviceInfo(struct TCanDeviceInfo *info)
{
char *str;
char str_puf[100];
uint32_t ver, ver2;

printf("Device:%s [Snr:%s]\n\r", info->Description, info->SerialNumber);
printf("   Hardware Id      : 0x%08X\n\r", info->HwId);
ver = info->FirmwareVersion / 1000;
ver2 = info->FirmwareVersion % 1000;
printf("   Firmware Version : %u.%03u\n\r", ver, ver2);
printf("   CanClock         : %u\n\r", info->ModulFeatures.CanClock);
str = str_puf;
if (info->ModulFeatures.Flags & CAN_FEATURE_LOM)            // Silent Mode (LOM = Listen only Mode)
  str = mhs_stpcpy(str, "LOM ");
if (info->ModulFeatures.Flags & CAN_FEATURE_ARD)            // Automatic Retransmission disable
  str = mhs_stpcpy(str, "ARD ");
if (info->ModulFeatures.Flags & CAN_FEATURE_TX_ACK)         // TX ACK
  str = mhs_stpcpy(str, "TX_ACK ");
if (info->ModulFeatures.Flags & CAN_FEATURE_ERROR_MSGS)     // Error Messages Support
  str = mhs_stpcpy(str, "ERROR_MSGS ");
if (info->ModulFeatures.Flags & CAN_FEATURE_FD_HARDWARE)    // CAN-FD Hardware
  str = mhs_stpcpy(str, "FD_HARDWARE ");
if (info->ModulFeatures.Flags & CAN_FEATURE_FIFO_OV_MODE)   // FIFO OV Mode (Auto Clear, OV CAN Messages)
  str = mhs_stpcpy(str, "FIFO_OV_MODE ");  
if (info->ModulFeatures.Flags & CAN_FEATURE_HW_TIMESTAMP)
  str = mhs_stpcpy(str, "HW_TIMESTAMP ");
printf("   Features-Flags   : %s\n\r", str_puf);
printf("   CanChannelsCount : %u\n\r", info->ModulFeatures.CanChannelsCount);
printf("   HwRxFilterCount  : %u\n\r", info->ModulFeatures.HwRxFilterCount);
printf("   HwTxPufferCount  : %u\n\r", info->ModulFeatures.HwTxPufferCount);
printf("\n\r");
}


/**************************************************************/
/*                        M A I N                             */
/**************************************************************/
int main(int argc, char **argv)
{
int err;
unsigned long i;
struct TDeviceStatus status;   // Status
struct TCanMsg msg;
uint32_t device_index;
struct TCanDeviceInfo device_info;

device_index = INDEX_INVALID;
/******************************/
/*  Initialisierung           */
/******************************/

printf("Demo fuer \"CanExGetDeviceInfoPerform\" und \"CanExGetDeviceInfoGet\"\n\r");
printf("================================================================\n\r\n\r");
// **** Initialisierung Utility Funktionen
UtilInit();
// **** Treiber DLL laden
if ((err = LoadDriver(TREIBER_NAME)) < 0)
  {
  printf("LoadDriver Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Treiber DLL initialisieren
// Keinen Callback Thread erzeugen, die Callback Funktionen stehen 
// nicht zur verfügung
if ((err = CanExInitDriver("CanCallThread=0")) < 0)
  {
  printf("CanInitDrv Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Device u. Empfangs-FIFO erzeugen für das Device erzeugen
if ((err = CanExCreateDevice(&device_index, "CanRxDFifoSize=16384")) < 0)
  {
  printf("CanExCreateDevice Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Schnittstelle PC <-> Tiny-CAN öffnen
if ((err = CanDeviceOpen(device_index, DEVICE_OPEN)) < 0)
  {
  printf("CanDeviceOpen Error-Code:%d\n\r", err);
  goto ende;
  }
/*************************************************/
/*  Device u. Hardware Info Variablen abfragen   */
/*************************************************/
if ((err = CanExGetDeviceInfoPerform(device_index, &device_info)) < 0)
  {
  printf("CanExGetDeviceInfo Error-Code:%d\n\r", err);
  goto ende;
  } 
PrintDeviceInfo(&device_info);
if (err > 0) 
  PrintHwInfo();
/*****************************************/
/*  CAN Speed einstellen & Bus starten   */
/*****************************************/
// **** Übertragungsgeschwindigkeit auf 40kBit/s einstellen
if (device_info.ModulFeatures.CanClock == 16)
  CanSetSpeedUser(device_index, CAN_40K_16MHZ);
else
  CanSetSpeedUser(device_index, CAN_40K_24MHZ);

// Achtung: Um Fehler auf dem Bus zu vermeiden ist die Übertragungsgeschwindigkeit
//          vor dem starten des Busses einzustellen.

// **** CAN Bus Start, alle FIFOs, Filter, Puffer und Fehler löschen
CanSetMode(device_index, OP_CAN_START, CAN_CMD_ALL_CLEAR);

/******************************/
/*  Message versenden         */
/******************************/

// msg Variable Initialisieren
msg.MsgFlags = 0L; // Alle Flags löschen, Stanadrt Frame Format,
                   // keine RTR, Datenlänge auf 0

//msg.MsgRTR = 1;    // Nachricht als RTR Frame versenden
//msg.MsgEFF = 1;    // Nachricht im EFF (Ext. Frame Format) versenden

msg.Id = 0x100;    // Message Id auf 100 Hex
msg.MsgLen = 5;    // Datenlänge auf 5
memcpy(msg.MsgData, "HALLO", 5);
if ((err = CanTransmit(device_index, &msg, 1)) < 0)
  {
  printf("CanTransmit Error-Code:%d\n\r", err);
  goto ende;
  }

printf("Empfangene CAN-Messages :\n\r");

while (!KeyHit())
  {
  /******************************/
  /*  Status abfragen           */
  /******************************/
  CanGetDeviceStatus(device_index, &status);

  if (status.DrvStatus >= DRV_STATUS_CAN_OPEN)
    {
    if (status.CanStatus == CAN_STATUS_BUS_OFF)
      {
      printf("CAN Status BusOff\n\r");
      CanSetMode(device_index, OP_CAN_RESET, CAN_CMD_NONE);
      }
    }
  else
    {
    printf("CAN Device nicht geöffnet\n\r");
    goto ende;
    }

  if (CanReceive(device_index, &msg, 1) > 0)
    {
    //printf("%10lu.%10lu ", msg.Time.Sec, msg.Time.USec);
    printf("id:%03X dlc:%01d data:", msg.Id, msg.MsgLen);
    if (msg.MsgLen)
      {
      for (i = 0; i < msg.MsgLen; i++)
        printf("%02X ", msg.MsgData[i]);
      }
    else
      printf(" keine");
    printf("\n\r");
    }
  }

/******************************/
/*  Treiber beenden           */
/******************************/
ende :

// Device schließen
(void)CanDeviceClose(device_index);
// Device löschen
(void)CanExDestroyDevice(&device_index);
// CanDownDriver wird auch automatisch von UnloadDriver aufgerufen,
// der separate Aufruf ist nicht zwingend notwendig
CanDownDriver();
// **** DLL entladen
UnloadDriver();

return(0);
}
