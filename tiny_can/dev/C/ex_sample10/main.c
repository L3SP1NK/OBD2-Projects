/**************************************************************************/
/*               Tiny-CAN API Demoprogramm "ex_sample1"                   */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : - Laden einer Treiber DLL und Initialisierung       */
/*                      im EX-Modus                                       */
/*                    - Initialisierung des CAN-Buses                     */
/*                    - Versand einer CAN-Message                         */
/*                    - Empfang von CAN-Messages                          */
/*                    - Bus-Status abfragen und BusOff löschen            */
/*                                                                        */
/*  Version         : 1.00                                                */
/*  Datei Name      : main.c                                              */
/* ---------------------------------------------------------------------- */
/*  Datum           : 29.09.11                                            */
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
#include "can_drv.h"


static const char *CanBusStatusStr[] = {"Bus Ok      ",
                                        "Error Warn. ",
                                        "Error Passiv",
                                        "Bus Off!    "};
                                           
static const char *CanErrorsStr[] = {"Stuff Error",
                                     "Form Error",
                                     "Ack Error",
                                     "Bit1 Error",
                                     "Bit0 Error",
                                     "CRC Error"};


static void PrintCanMessage(struct TCanMsg *msg)
{
unsigned long i;
unsigned char err_nr, bus_stat, rx_err_cnt, tx_err_cnt;

//printf("%10lu.%10lu ", msg.Time.Sec, msg.Time.USec); 
if (msg->MsgErr)
  {
  err_nr = msg->MsgData[0] - 1;
  bus_stat = msg->MsgData[1] & 0x0F;
  rx_err_cnt = msg->MsgData[2]; 
  tx_err_cnt = msg->MsgData[3]; 
  if ((err_nr >= 0) && (err_nr < 7) && (bus_stat < 4))
    printf("[%s] (Rx-ErrCnt:%3u, Tx-ErrCnt:%3u): %s", CanBusStatusStr[bus_stat], rx_err_cnt, 
        tx_err_cnt, CanErrorsStr[err_nr]);
  else
    printf("Unbek. Fehler");    
  }
else
  {  
  printf("id:0x%03X dlc:%01d data:", msg->Id, msg->MsgLen);
  if (msg->MsgLen)
    {
    for (i = 0; i < msg->MsgLen; i++)
      printf("%02X ", msg->MsgData[i]);
    }
  else
    printf(" keine");
  }  
printf("\n\r");
}

/**************************************************************/
/*                        M A I N                             */
/**************************************************************/
int main(int argc, char **argv)
{
unsigned char ch;
int err;
struct TDeviceStatus status;   // Status
struct TCanMsg msg, rx_msg;
uint32_t device_index;

device_index = INDEX_INVALID;
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
// Keinen Callback Thread erzeugen, die Callback Funktionen stehen 
// nicht zur verfügung
if ((err = CanExInitDriver("CanCallThread=0")) < 0)
  {
  printf("CanInitDrv Error-Code:%d\n\r", err);
  goto ende;
  }
// **** Device u. Empfangs-FIFO für das Device erzeugen
if ((err = CanExCreateDevice(&device_index, "CanRxDFifoSize=16384")) < 0)
  {
  printf("CanExCreateDevice Error-Code:%d\n\r", err);
  goto ende;
  }
// **** CAN Bus Fehler Nachrichten erzeugen
if ((err = CanExSetAsUByte(device_index, "CanErrorMsgsEnable", 1)) < 0)  
  {
  printf("CanExSetAsUByte Error-Code:%d\n\r", err);
  goto ende;
  } 
// **** Schnittstelle PC <-> Tiny-CAN öffnen
if ((err = CanDeviceOpen(device_index, NULL)) < 0)
  {
  printf("CanDeviceOpen Error-Code:%d\n\r", err);
  goto ende;
  }
/*****************************************/
/*  CAN Speed einstellen & Bus starten   */
/*****************************************/
// **** Übertragungsgeschwindigkeit einstellen
CanSetSpeed(device_index, CAN_SPEED);

// Achtung: Um Fehler auf dem Bus zu vermeiden ist die Übertragungsgeschwindigkeit
//          vor dem starten des Busses einzustellen.

// **** CAN Bus Start, alle FIFOs, Filter, Puffer und Fehler löschen
CanSetMode(device_index, OP_CAN_START, CAN_CMD_ALL_CLEAR);  // OP_CAN_START_LOM

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
/*if ((err = CanTransmit(device_index, &msg, 1)) < 0)
  {
  printf("CanTransmit Error-Code:%d\n\r", err);
  goto ende;
  }*/

printf("Tiny-CAN API Demoprogramm\n\r");
printf("=========================\n\r\n\r");
printf("Empfangene CAN-Messages :\n\r");

ch = '\0';
do
  {
  if (KeyHit())
    {
    ch = getch();
    switch (ch)
      {
      case '1' : {
                 if ((err = CanTransmit(device_index, &msg, 1)) < 0)
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
  CanGetDeviceStatus(device_index, &status);

  /*if (status.DrvStatus >= DRV_STATUS_CAN_OPEN)
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
    }*/

  if (CanReceive(device_index, &rx_msg, 1) > 0)
    {
    PrintCanMessage(&rx_msg);
    }
  }
while (ch != 'q');  

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
