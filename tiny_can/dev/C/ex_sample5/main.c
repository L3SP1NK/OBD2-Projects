/**************************************************************************/
/*                Tiny-CAN API Demoprogramm "ex_sample5"                  */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : - Laden einer Treiber DLL und Initialisierung       */
/*                      im EX-Modus                                       */
/*                    - Anzahl verbundener Devices anzeigen               */
/*                        Funktion: CanExGetDeviceCount                   */
/*                    - Device Liste auf dem Bildschim ausgeben           */
/*                        Funktion: CanExGetDeviceList                    */
/*                                                                        */
/*  Version         : 1.00                                                */
/*  Datei Name      : main.c                                              */
/* ---------------------------------------------------------------------- */
/*  Datum           : 09.08.11                                            */
/*  Autor           : Demlehner Klaus, MHS-Elektronik, 94149 Kößlarn      */
/*                    info@mhs-elektronik.de  www.mhs-elektronik.de       */
/**************************************************************************/
#include "config.h"
#include "global.h"
#include <string.h>
#include <stdio.h>
#include "util.h"
#include "can_drv.h"


void PrintDevices(void)
{
int32_t i, num_devs;
uint32_t idx;
char *str;
char str_puf[100];
struct TCanDevicesList *l;

if ((num_devs = CanExGetDeviceList(&l, 0)) > 0)  // Device Liste lesen
  {
  for (i = 0; i < num_devs; i++)  // Alle Devices durchlaufen
    {
#ifndef __WIN32__
// Der "DeviceName" ist nur unter Linux gesetzt
    printf("Dev:%s ->", l[i].DeviceName); 
#endif
    idx = l[i].TCanIdx;    
    if (idx == INDEX_INVALID)  // Device geöffnet ?
      {                        // -> Nein
      printf("%s [%s]\n\r", l[i].Description, l[i].SerialNumber);  
      }
    else
      {                        // -> Ja, Device Index mit anzeigen
      printf("%s [%s] Open: 0x%08X\n\r", l[i].Description, l[i].SerialNumber, idx);
      }
    printf("   Id               : 0x%08X\n\r", l[i].HwId);
    if (l[i].HwId)             // Nachfolgende Werte nur gültig wenn HwId > 0
      {
      printf("   CanClock         : %u\n\r", l[i].ModulFeatures.CanClock);
      str = str_puf;
      if (l[i].ModulFeatures.Flags & CAN_FEATURE_LOM) // Silent Mode (LOM = Listen only Mode)
        str = mhs_stpcpy(str, "LOM ");
      if (l[i].ModulFeatures.Flags & CAN_FEATURE_ARD) // Automatic Retransmission disable
        str = mhs_stpcpy(str, "ARD ");
      if (l[i].ModulFeatures.Flags & CAN_FEATURE_TX_ACK)  
        str = mhs_stpcpy(str, "TX_ACK ");             // TX ACK
      if (l[i].ModulFeatures.Flags & CAN_FEATURE_ERROR_MSGS)
        str = mhs_stpcpy(str, "ERROR_MSGS ");         // Error Messages Support
      if (l[i].ModulFeatures.Flags & CAN_FEATURE_FD_HARDWARE)
        str = mhs_stpcpy(str, "FD_HARDWARE ");        // CAN-FD Hardware
      if (l[i].ModulFeatures.Flags & CAN_FEATURE_FIFO_OV_MODE)
        str = mhs_stpcpy(str, "FIFO_OV_MODE ");       // FIFO OV Mode (Auto Clear, OV CAN Messages)   
      if (l[i].ModulFeatures.Flags & CAN_FEATURE_HW_TIMESTAMP)
        str = mhs_stpcpy(str, "HW_TIMESTAMP ");       // Hardware Time-Stamps
      printf("   Features-Flags   : %s\n\r", str_puf);
      printf("   CanChannelsCount : %u\n\r", l[i].ModulFeatures.CanChannelsCount);
      printf("   HwRxFilterCount  : %u\n\r", l[i].ModulFeatures.HwRxFilterCount);
      printf("   HwTxPufferCount  : %u\n\r", l[i].ModulFeatures.HwTxPufferCount);
      }
    printf("\n\r");
    }  
  }
else
  printf("keine Devices gefunden.\n\r");
// WICHTIG: Den allocierten Speicher mit CanExDataFree wieder freigeben
CanExDataFree((void **)&l);
}


/**************************************************************/
/*                        M A I N                             */
/**************************************************************/
int main(int argc, char **argv)
{
int32_t err;

// **** Treiber DLL laden
if ((err = LoadDriver(TREIBER_NAME)) < 0)
  {
  printf("LoadDriver Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Treiber DLL im extended Mode initialisieren
if ((err = CanExInitDriver(NULL)) < 0)
  {
  printf("CanExInitDrv Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Anzahl verbundener Devices abfragen
if ((err = CanExGetDeviceCount(0)) < 0)
  {
  printf("CanExGetDeviceCount Error-Code:%d\n\r", err);
  goto ende;
  }
else
  printf("Anzahl Devices: %d\n\r\n\r", err);
// ****  Device Liste erzeugen & ausgeben
PrintDevices();
// **** DLL entladen
ende :
UnloadDriver();
return(0);
}
