/**************************************************************************/
/*               Tiny-CAN API Demoprogramm "Sample8"                      */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : - Laden einer Treiber DLL                           */
/*                    - Initialisierung des CAN-Buses                     */
/*                    - Versand einer CAN-Message                         */
/*                    - Empfang von CAN-Messages nur einer ausgewählten ID*/
/*                    - Bus-Status abfragen und BusOff löschen            */
/*                                                                        */
/*  Version         : 1.10                                                */
/*  Datei Name      : main.c                                              */
/* ---------------------------------------------------------------------- */
/*  Datum           : 07.03.07                                            */
/*  Autor           : Demlehner Klaus, MHS-Elektronik, 94149 Kößlarn      */
/*                    info@mhs-elektronik.de  www.mhs-elektronik.de       */
/* ---------------------------------------------------------------------- */
/*  Compiler        : GNU C Compiler                                      */
/**************************************************************************/
#include "config.h"
#include "global.h"
#include <string.h>
#include <stdio.h>
#ifdef __WIN32__
  #include <conio.h>
#endif
#include "can_drv.h"


int SetPassFilter(uint32_t id, uint32_t mask)
{
struct TMsgFilter msg_filter;

msg_filter.Flags.Long = 0L;
msg_filter.FilIdMode = 3;   // 3 = Maske & Code Pass Mode
msg_filter.FilEnable = 1;   // Filter freigeben
msg_filter.Code = id;    // Start
msg_filter.Maske = mask;   // Stop
return(CanSetFilter(1L | INDEX_SOFT_FLAG, &msg_filter));
}


/**************************************************************/
/*                        M A I N                             */
/**************************************************************/
int main(int argc, char **argv)
{
char ch;
int err;
unsigned long i;
struct TDeviceStatus status;   // Status
struct TCanMsg msg;
struct TCanMsg tx_msg;

/******************************/
/*  Initialisierung           */
/******************************/

// **** Initialisierung Utility Funktionen
UtilInit();
// **** Treiber DLL laden
if ((err = LoadDriver(TREIBER_NAME)) < 0)
  {
  printf("LoadDriver Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Treiber DLL initialisieren
if ((err = CanInitDriver(TREIBER_INIT)) < 0)
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

/******************************/
/*  CAN Speed einstellen      */
/******************************/
// **** Übertragungsgeschwindigkeit auf 125kBit/s einstellen
CanSetSpeed(0, 250); //CAN_SPEED);

// **** CAN Bus Start
CanSetMode(0, OP_CAN_START, CAN_CMD_ALL_CLEAR);

// msg Variable Initialisieren
tx_msg.MsgFlags = 0L; // Alle Flags löschen, Stanadrt Frame Format,
                   // keine RTR, Datenlänge auf 0

//msg.MsgRTR = 1;    // Nachricht als RTR Frame versenden
tx_msg.MsgEFF = 1;    // Nachricht im EFF (Ext. Frame Format) versenden

tx_msg.Id = 0x12345671;    // Message Id auf 100 Hex
tx_msg.MsgLen = 5;    // Datenlänge auf 5
memcpy(tx_msg.MsgData, "HALLO", 5);

printf("Tiny-CAN API Demoprogramm\n\r");
printf("=========================\n\r\n\r");
printf("Empfangene CAN-Messages :\n\r");

if ((err = CanExSetAsUByte(0, "Can1TxAckEnable", 1)) < 0)
    printf("Set \"Can1TxAckEnable\" Fehler: %d\n\r", err);

if ((err = SetPassFilter(0x12345678, 0x1FFFFFF0)) < 0)  // Ihre Id ist 0x100, Standart Format
  {
  printf("SetPassFilter Error-Code:%d\n\r", err);
  goto ende;
  }

do
  {
  if (KeyHit())
    {
    ch = getch();
    switch (ch)
      {
      case 't' : {
                 if ((err = CanTransmit(0, &tx_msg, 1)) < 0)
                   {
                   printf("CanTransmit Error-Code:%d\n\r", err);
                   goto ende;
                   }
                 break;
                 }
      }
    }
  /******************************/
  /*  Status abfragen           */
  /******************************/
  CanGetDeviceStatus(0, &status);

  if (status.DrvStatus >= DRV_STATUS_CAN_OPEN)
    {
    if (status.CanStatus == CAN_STATUS_BUS_OFF)
      {
      printf("CAN Status BusOff\n\r");
      CanSetMode(0, OP_CAN_RESET, 0);
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
while (ch != 'q');
/******************************/
/*  Treiber beenden           */
/******************************/
ende :

CanDownDriver();
// **** DLL entladen
UnloadDriver();

return(0);
}
