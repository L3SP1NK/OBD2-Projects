/**************************************************************************/
/*               Tiny-CAN API Demoprogramm "Sample5"                      */
/* ---------------------------------------------------------------------- */
/*  Beschreibung    : - Laden einer Treiber DLL                           */
/*                    - Initialisierung des CAN-Buses                     */
/*                    - Versand einer CAN-Message                         */
/*                    - Empfang von CAN-Messages                          */
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
#include <stdlib.h>
#ifdef __WIN32__
#include <conio.h>
#endif
#include "util.h"
#include "can_drv.h"

#define TX_FIFO_SIZE_STR "10000"
#define TX_FIFO_SIZE 5000

struct TCanMsg *MsgPuffer = NULL;
unsigned long MsgCounter;

struct TCanMsg *MsgPufferPtr;
unsigned long MsgPufferSize;

void CreateMsgData(void);

int CreateMsgPuffer(void)
{
MsgPuffer = calloc(TX_FIFO_SIZE, sizeof(struct TCanMsg));
if (!MsgPuffer)
  {
  printf("Zu wenig Speicher");
  return(-1);
  }
MsgCounter = 0;
CreateMsgData();
return(0);
}


void DestroyMsgPuffer(void)
{
safe_free(MsgPuffer);
}


void CreateMsgData(void)
{
int i;
struct TCanMsg *msg;

msg = MsgPuffer;
for (i = 0; i < TX_FIFO_SIZE; i++)
  {
  msg->Id = 0x123;
  msg->MsgLen = 8;
  msg->Data.Longs[0] = MsgCounter;
  msg->Data.Longs[1] = 0x00;
  msg++;
  MsgCounter++;  
  }
MsgPufferPtr = MsgPuffer;
MsgPufferSize = TX_FIFO_SIZE;
}


int TxCanMessages(struct TCanMsg *msgs, int size)
{
int err, tx_max;

tx_max = TX_FIFO_SIZE - CanTransmitGetCount(0);
if ((tx_max <= 0) || (size <= 0))
  return(0);
if (size > tx_max)
  size = tx_max;
if ((err = CanTransmit(0, msgs, size)) < 0)
  {
  printf("CanTransmit Error-Code:%d\n\r", err);
  return(-1);
  }
return(err);
}


int TxEventHandler(void)
{
int true_tx;
 
if (!MsgPufferSize)
  CreateMsgData();
if ((true_tx = TxCanMessages(MsgPufferPtr, MsgPufferSize)) < 0)
  return(-1);
if (!true_tx)
  return(0);    
if (MsgPufferSize >= (unsigned long)true_tx)
  {  
  MsgPufferPtr += true_tx;
  MsgPufferSize -= true_tx;
  }
else
  {
  CreateMsgData();
  if ((true_tx = TxCanMessages(MsgPufferPtr, MsgPufferSize)) < 0)
    return(-1);
  MsgPufferPtr += true_tx;
  MsgPufferSize -= true_tx;
  }
return(0);
}


/**************************************************************/
/*                        M A I N                             */
/**************************************************************/
int main(int argc, char **argv)
{
int err;
struct TDeviceStatus status;   // Status

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
if ((err = CanInitDriver("CanTxDFifoSize="TX_FIFO_SIZE_STR)) < 0)
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
// **** Übertragungsgeschwindigkeit einstellen
CanSetSpeed(0, CAN_SPEED);

// **** CAN Bus Start
CanSetMode(0, OP_CAN_START, CAN_CMD_ALL_CLEAR);

printf("CAN-Message Tx Demo\n\r");

CreateMsgPuffer();

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
      CanSetMode(0, OP_CAN_RESET, CAN_CMD_ALL_CLEAR);
      }
    }
  else
    {
    printf("CAN Device nicht geöffnet\n\r");
    goto ende;
    }
  /******************************/
  /*  Message versenden         */
  /******************************/
  if (TxEventHandler() < 0)        
    goto ende;
    
  Sleep(5);  // 5 ms schlafen
  }

/******************************/
/*  Treiber beenden           */
/******************************/
ende :

DestroyMsgPuffer();
// **** DLL entladen
UnloadDriver();

return(0);
}
