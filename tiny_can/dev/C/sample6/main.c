/**************************************************************************/
/*               Tiny-CAN API Demoprogramm "Sample9"                      */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : Abfrage und Auswertung der "Hardware Info           */
/*                    Variablen" zur Anpassung des Programms an die       */
/*                    speziellen Funktionen des Tiny-CANs:                */
/*                        - Silent Mode (LOM = Listen only Mode)          */
/*                        - Automatic Retransmission disable              */
/*                        - TX ACK Support                                */
/*                        - Hardware Time Stamp                           */
/*                        - CAN Clock, für benutzerdefinierte CAN Raten   */
/*                        - Anzahl Can Ports                              */
/*                        - Anzahl Intervall Puffer                       */
/*                        - Anzahl Filter                                 */
/*                                                                        */
/*  Version         : 1.20                                                */
/*  Datei Name      : main.c                                              */
/* ---------------------------------------------------------------------- */
/*  Datum           : 16.08.10                                            */
/*  Autor           : Demlehner Klaus, MHS-Elektronik, 94149 Kößlarn      */
/*                    info@mhs-elektronik.de  www.mhs-elektronik.de       */
/* ---------------------------------------------------------------------- */
/*  Compiler        : GNU C Compiler, Microsoft Visual Studio 6           */
/**************************************************************************/
#include "config.h"
#include "global.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef __WIN32__
#include <conio.h>
#endif
#include "util.h"
#include "can_drv.h"


// BTR Werte für 40kBit/s bei 16 u. 24 MHz
#define CAN_40K_16MHZ 0x3E53
#define CAN_40K_24MHZ 0x3E5D


struct TCanModul
  {
  int CanPortCount;
  int IntPufferCount;
  int FilterCount;
  int CanFeaturesFlags;
  int HardwareTimeStamp;
  unsigned long Id;
  int CanClock;
  };


struct TCanModulDesc
  {
  unsigned long HwId;
  int CanClock;
  int HardwareTimeStamp;
  };

static const struct TCanModulDesc CanModulTable[] =
  //   ID    | CAN-Clock |  Hardware
  //             (MHz)     Time Stamp
{{0x43414E01,    24,          0},  // Tiny-CAN I
 {0x43414E81,    16,          0},  // Tiny-CAN I / 10kBit
 {0x43414E41,    24,          0},  // Tiny-CAN IB
 {0x43414EC1,    16,          0},  // Tiny-CAN IB / 10kBit
 {0x43414E02,    24,          0},  // Tiny-CAN II
 {0x43414E82,    16,          0},  // Tiny-CAN II / 10 kBit
 {0x43414E42,    16,          0},  // Tiny-CAN II-XL
 {0x43414EC2,    16,          0},  // Tiny-CAN II-XXL
 {0x43414E03,    24,          1},  // Tiny-CAN III
 {0x43414E83,    16,          0},  // Tiny-CAN III / 10 kBit
 {0x43414E43,    16,          1},  // Tiny-CAN III-XL
 {0x43414E04,    16,          0},  // Tiny-CAN M1
 {0x43414E05,    16,          0},  // Tiny-CAN M232
 {0x43414E06,    16,          1},  // Tiny-CAN IV-XL
 {0x00000000,    0,           }};  // Ende Tabelle

struct TCanModul CanModul;

static void ExtractHardwareInfo(char *str, struct TCanModul *modul)
{
int match;
unsigned long id;
char *tmpstr, *s, *key, *val, *endptr;
struct TCanModulDesc *modul_desc;

tmpstr = mhs_strdup(str);
s = tmpstr;
modul->CanFeaturesFlags = 0;
modul->CanClock = 0;
modul->HardwareTimeStamp = 0;
do
  {
  // Bezeichner auslesen
  key = get_item_as_string(&s, ":=", &match);
  if (match <= 0)
    break;
  // Value auslesen
  val = get_item_as_string(&s, ";", &match);
  if (match < 0)
    break;

  if (!save_strcasecmp(key, "ANZAHL CAN INTERFACES"))
    {
    modul->CanPortCount = strtol(val, (char**)&endptr, 0);
    continue;
    }
  else if (!save_strcasecmp(key, "ANZAHL INTERVAL PUFFER"))
    {
    modul->IntPufferCount = strtol(val, (char**)&endptr, 0);
    continue;
    }
  else if (!save_strcasecmp(key, "ANZAHL FILTER"))
    {
    modul->FilterCount = strtol(val, (char**)&endptr, 0);
    continue;
    }
  else if (!save_strcasecmp(key, "CAN FEATURES FLAGS"))
    {
    modul->CanFeaturesFlags = strtol(val, (char**)&endptr, 0);
    continue;
    }
  else if (!save_strcasecmp(key, "ID"))
    {
    modul->Id = strtoul(val, (char**)&endptr, 0);
    continue;
    }
  else if (!save_strcasecmp(key, "CAN CLOCK"))   // Für die Zukunft, noch nicht verwendet
    {
    modul->CanClock = strtol(val, (char**)&endptr, 0);
    continue;
    }
  }
while(1);
safe_free(tmpstr);
// Hardware Time Stamp & CanClock
for (modul_desc = (struct TCanModulDesc *)&CanModulTable[0]; (id = modul_desc->HwId); modul_desc++)
  {
  if (id == modul->Id)
    {
    if (!modul->CanClock)
      modul->CanClock = modul_desc->CanClock;
    modul->HardwareTimeStamp = modul_desc->HardwareTimeStamp;
    break;    
    }
  }
}


void PrintModul(struct TCanModul *modul)
{
printf("Anzahl Can Ports: %d\n\r", modul->CanPortCount);
printf("Anzahl Intervall Puffer: %d\n\r", modul->IntPufferCount);
printf("Anzahl Filter: %d\n\r", modul->FilterCount);
printf("Can Features Flags: %#04X\n\r", modul->CanFeaturesFlags);
if (modul->CanFeaturesFlags & CAN_FEATURE_LOM)
  printf("     Silent Mode (LOM = Listen only Mode)\n\r");
if (modul->CanFeaturesFlags & CAN_FEATURE_ARD)
  printf("     Automatic Retransmission disable\n\r");
if (modul->CanFeaturesFlags & CAN_FEATURE_TX_ACK)
  printf("     TX ACK Support\n\r");
if (modul->CanFeaturesFlags & CAN_FEATURE_ERROR_MSGS) 
  printf("     Error Messages Support\n\r");
if (modul->CanFeaturesFlags & CAN_FEATURE_FD_HARDWARE) 
  printf("     CAN-FD Hardware\n\r");
if (modul->CanFeaturesFlags & CAN_FEATURE_FIFO_OV_MODE) 
  printf("     FIFO OV Mode (Auto Clear, OV CAN Messages)\n\r");  
if (modul->HardwareTimeStamp)
  printf("Hardware Time Stamps\n\r");
else
  printf("keine Hardware Time Stamps\n\r");
printf("Modul ID: %#08lX\n\r", modul->Id);
printf("CAN-Clock: %d\n\r", modul->CanClock);
}


/**************************************************************/
/*                        M A I N                             */
/**************************************************************/
int main(int argc, char **argv)
{
int err;
char *str;
unsigned long i;
struct TDeviceStatus status;   // Status
struct TCanMsg msg;

/******************************/
/*  Initialisierung           */
/******************************/

// **** Initialisierung Utility Funktionen
UtilInit();

printf("Tiny-CAN API Demoprogramm\n\r");
printf("=========================\n\r\n\r");

// **** Treiber DLL laden
if ((err = LoadDriver(TREIBER_NAME)) < 0)
  {
  printf("LoadDriver Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Treiber DLL initialisieren
// Größe des Sende-Fifos auf 10 und des Empfangs-Fifos auf 100
if ((err = CanInitDriver(NULL)) < 0)
  {
  printf("CanInitDrv Error-Code:%d\n\r", err);
  goto ende;
  }

// **** Schnittstelle PC <-> USB-Tiny öffnen
// COM Port 1 auswählen
if ((err = CanDeviceOpen(0, DEVICE_OPEN)) < 0)
  {
  printf("CanDeviceOpen Error-Code:%d\n\r", err);
  goto ende;
  }

if (!(str = CanDrvHwInfo(0L)))
  {
  printf("CanDrvHwInfo Error\n\r");
  goto ende;
  }
                                      
ExtractHardwareInfo(str, &CanModul);
PrintModul(&CanModul);                     

printf("\n\r");

/******************************/
/*  CAN Speed einstellen      */
/******************************/
// **** Übertragungsgeschwindigkeit auf 40kBit/s einstellen
if (CanModul.CanClock == 16)
  CanSetSpeedUser(0, CAN_40K_16MHZ);
else
  CanSetSpeedUser(0, CAN_40K_24MHZ);

// **** CAN Bus Start, alle FIFOs, Filter, Puffer und Fehler löschen
CanSetMode(0, OP_CAN_START, CAN_CMD_ALL_CLEAR);

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
if ((err = CanTransmit(0, &msg, 1)) < 0)
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
  CanGetDeviceStatus(0, &status);

  if (status.DrvStatus >= DRV_STATUS_CAN_OPEN)
    {
    if (status.CanStatus == CAN_STATUS_BUS_OFF)
      {
      printf("CAN Status BusOff\n\r");
      CanSetMode(0, OP_CAN_RESET, CAN_CMD_NONE);
      }
    }
  else
    {
    printf("CAN Device nicht geöffnet\n\r");
    goto ende;
    }

  if (CanReceive(0, &msg, 1) > 0)
    {
    printf("%10u.%10u ", msg.Time.Sec, msg.Time.USec);
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

CanDownDriver();
// **** DLL entladen
UnloadDriver();

return(0);
}
