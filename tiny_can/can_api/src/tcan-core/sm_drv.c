/***************************************************************************
                          sm_drv.c  -  description
                             -------------------
    begin             : 23.03.2008
    last modify       : 19.04.2022    
    copyright         : (C) 2008 - 2022 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include <stdlib.h>
#include <stdio.h>
#include "com_io.h"
#include "util.h"
#ifdef LOG_DEBUG_ENABLE
  #include "log.h"
#endif
#include "sm_drv.h"


/*

Protokoll Aufbau
================

             +---------------------------------------------------+
             |          Kommando an den Mikrocontroller          |
 PC -> uC    |          ===============================          |
             |  - Kommando ohne Parameter, Single Kommando       |
             |  - Kommando mit Parametern, Ext. I / II Kommando  |
             +---------------------------------------------------+
             |           Antworten vom Mikrocontroller           |
             |           =============================           |
 PC <- uC    | - Positive Antwort ohne Parameter (ACK)           |
             | - Negative Antwort mit Fehlercode (NACK)          |
             | - Positive Antwort mit Parameter (ACK)            |
             +---------------------------------------------------+


Kommando ohne Parameter - Single Kommando
=========================================

 Byte |   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0   |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   1  |     0     |                                      Kommando                                     |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   2  |                                      8 Bit CRC Prüfsumme                                      |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+


Kommando mit max. 255 Zeichen Parameter - Ext. I Kommando
=========================================================

 Byte |   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0   |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   1  |     1     |     0     |                                Kommando                               |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   2  |                                        Anzahl Parameter                                       |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   3  |                                             Daten                                             |
      +-----------                                                                         -----------+
   .. |                                            . . . .                                            |
      +-----------                                                                         -----------+
   n  |                                            . . . .                                            |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
  n+1 |                                      8 Bit CRC Prüfsumme                                      |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+


Kommando mit max. 16383 Zeichen Parameter - Ext. II Kommando
============================================================

 Byte |   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0   |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   1  |     1     |     1     |                                Kommando                               |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   2  |     0     |     0     |                Anzahl Parameter High Byte (Bit 13 - 8)                |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   3  |                             Anzahl Parameter Low Byte (Bit 7 - 0)                             |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   4  |                                             Daten                                             |
      +-----------                                                                         -----------+
   .. |                                            . . . .                                            |
      +-----------                                                                         -----------+
   n  |                                            . . . .                                            |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
  n+1 |                                      8 Bit CRC Prüfsumme                                      |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+


Controll Frame Kommando
=======================

 Byte |   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0   |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   1  |     1     |     0     |     0     |     0     |     0     |     0     |     0     |     0     |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   2  |                                       Controll Kommando                                       |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   3  |                                      8 Bit CRC Prüfsumme                                      |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+

Controll Kommando
  0x01 = Retransmit Last ACK

Antworten - ACKs
================

Positiv acknowledge - ACK
=========================

 Byte |   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0   |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   1  |     1     |     0     |     0     |     1     |     0     |     1     |     0     |     1     |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+

Es wird keine Prüfsumme gesendet


Negativ acknowledge - NACK
==========================

 Byte |   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0   |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   1  |     1     |     0     |     1     |                         Fehlercode                        |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+

Es wird keine Prüfsumme gesendet


Positiv acknowledge mit max. 16383 Parametern - ACK
===================================================

 Byte |   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0   |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   1  |     0     |     1     |                Anzahl Parameter High Byte (Bit 13 - 8)                |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   2  |                             Anzahl Parameter Low Byte (Bit 7 - 0)                             |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
   3  |                                             Daten                                             |
      +-----------                                                                         -----------+
   .. |                                            . . . .                                            |
      +-----------                                                                         -----------+
   n  |                                            . . . .                                            |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
  n+1 |                                      8 Bit CRC Prüfsumme                                      |
      +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+

*/

#define SM_ERR_SIO          1
#define SM_ERR_OV           2
#define SM_ERR_CRC          3
#define SM_ERR_PARAM_OV     4
#define SM_ERR_RX_TIMEOUT   5
#define SM_ERR_NO_LAST_ACK  6


#define CONTROL_COMMAND 0x80
#define CTRL_RETRANSMIT_LAST_ACK 0x01

#define NackErrorStrSize 6

static const char *NackErrorStr[NackErrorStrSize] = {
     "SIO",
     "OV",
     "CRC",
     "PARAM OV",
     "RX TIMEOUT",
     "NO LAST ACK"};

struct TIndexStrList
  {
  int Index;
  const char *Str;
  };
  
  
static const struct TIndexStrList SmDrvErrorList[] = { 
  {ERR_SM_PORT_NOT_OPEN,        "Port not open"}, 
  {ERR_SM_TX_CMD,               "Tx Command"}, 
  {ERR_SM_RX_ACK,               "Rx ACK"}, 
  {ERR_SM_TX_DATA,              "Tx Data"}, 
  {ERR_SM_RX_DATA,              "Rx Data"}, 
  {ERR_SM_RX_ACK_TIMEOUT,       "ACK Timeout"}, 
  {ERR_SM_RX_PARAM_CNT,         "Rx param count"}, 
  {ERR_SM_RX_PARAM_CNT_TIMEOUT, "Rx param count timeout"},  
  {ERR_SM_RX_PARAM,             "Rx param"},  
  {ERR_SM_RX_PARAM_TIMEOUT,     "Rx param timeout"},  
  {ERR_SM_RX_PARAM_COUNT,       "Rx param ov"},  
  {ERR_SM_RX_CRC,               "Rx CRC"},  
  {ERR_SM_RX_CRC_TIMEOUT,       "Rx CRC Timeout"},  
  {ERR_SM_WRONG_CRC,            "WRONG CRC"},  
  {ERR_SM_WRONG_ACK,            "WRONG ACK"},
  {ERR_SM_NACK,                 "NACK"},
  {0, NULL}};


static uint8_t CRCTabelle[] = {
    0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83,
    0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41,
    0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e,   // 10
    0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc,
    0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0,   // 20
    0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,
    0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d,   // 30
    0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff,
    0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5,   // 40
    0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07,
    0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58,   // 50
    0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,
    0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6,   // 60
    0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24,
    0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b,   // 70
    0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9,
    0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f,   // 80
    0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd,
    0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92,   // 90
    0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50,
    0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c,   // A0
    0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee,
    0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1,   // B0
    0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73,
    0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49,   // C0
    0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b,
    0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4,   // D0
    0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16,
    0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a,   // E0
    0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,
    0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7,   // F0
    0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35};


#define updcrc8(cp, crc) (CRCTabelle[crc ^ cp])


static const char *SmDrvGetErrorStr(int err)
{
const struct TIndexStrList *err_tab;
const char *s;

for (err_tab = SmDrvErrorList; (s = err_tab->Str); err_tab++)
  {
  if (err_tab->Index == err)
    return(s);    
  }       
return("?");  
}


int32_t SMCheckRtsCtsClear(TCanDevice *dev)
{
int32_t res, ok;
uint32_t i, status, rts;

res = 0;
ok = 1;
if (ComIsOpen(dev->Io) < 1)          // Com Port geöffnet ?
  return(ERR_SM_PORT_NOT_OPEN);
for (i = 0; i < 2; i++)
  {  
  rts = i & 0x01;
  if ((res = ComWriteRTS(dev->Io, 0, rts)) < 0)
    break;
  mhs_sleep(100);
  status = ComGetLineStatus(dev->Io, 0);
  if (status & ST_CTS)
    {
    if (!rts)
      {
      ok = 0;
      break;
      }
    }
  else
    {
    if (rts)
      {
      ok = 0;
      break;
      }
    }    
  }
if (res)
  return(res);
else    
  return(ok);
}


int32_t SMDrvClear(TCanDevice *dev)
{
int i;
uint8_t c;

if (ComIsOpen(dev->Io) < 1)          // Com Port geöffnet ?
  return(ERR_SM_PORT_NOT_OPEN);
ComFlushBuffer(dev->Io);             // Sende und Empfangspuffer löschen
for (i = 10; i; i--)
  {
  mhs_sleep(10);
  if (ComReadByte(dev->Io, &c) < 1)
    return(0);
  ComFlushBuffer(dev->Io);           // Sende und Empfangspuffer löschen
  }
return(-1);
}


// **** Kommando senden
static int32_t SMDrvCommandoTx(TCanDevice *dev)
{
uint8_t c, crc;
uint16_t i, param_cnt;
uint32_t tx_cnt;
uint8_t *tx_puf;

tx_puf = &dev->TxDDataPuffer[0];
c = dev->Kommando;
*tx_puf++ = c;
crc = CRCTabelle[c];
if (c == CONTROL_COMMAND)
  {
  param_cnt = 1;
  tx_cnt = 3;
  }
else
  {  
  c &= 0xC0;                       // Commando Type -> Single, Ext I oder II
  // **** Parameter länge senden
  if (c == 0x80)                   // Ext. I
    {
    param_cnt = (uint16_t)dev->TxDParameter.Count;
    tx_cnt = param_cnt + 3;
    c = (uint8_t)param_cnt;
    crc = updcrc8(c, crc);
    *tx_puf++ = c;
    }
  else if (c == 0xC0)              // Ext. II
    {
    param_cnt = (uint16_t)dev->TxDParameter.Count;
    tx_cnt = param_cnt + 4;
    c = (uint8_t)((param_cnt >> 8) & 0x3F);
    crc = updcrc8(c, crc);
    *tx_puf++ = c;
    c = (uint8_t)param_cnt;
    crc = updcrc8(c, crc);
    *tx_puf++ = c;
    }
  else
    {
    param_cnt = 0;
    tx_cnt = 2;
    }
  }  
// **** Parameter senden
for (i = 0; i < param_cnt; i++)
  {
  c = dev->TxDParameter.Data[i];
  crc = updcrc8(c, crc);
  *tx_puf++ = c;
  }
// **** CRC senden
crc = CRCTabelle[CRCTabelle[crc]];
*tx_puf = crc;
if (ComWriteData(dev->Io ,(char *)dev->TxDDataPuffer, tx_cnt) < 0)
  {
  ComClose(dev->Io);
  return(ERR_SM_TX_CMD);
  }
return(0);  
}


// **** ACK Empfangen
static int32_t SMDrvCommandoRx(TCanDevice *dev)
{
uint8_t c, crc, read_crc;
uint16_t i, param_cnt;
int32_t res;

dev->RxDParameter.Count = 0;
if ((res = ComReadByte(dev->Io, &c)) < 0)  // ACK Byte Empfangen
  {
  ComClose(dev->Io);
  return(ERR_SM_RX_ACK);
  }
if (res == 0)
  return(ERR_SM_RX_ACK_TIMEOUT);
dev->ACKByte = c;
crc = CRCTabelle[c];
if ((c & 0xC0) == 0x40)          // noch Parameter Empf. ? (ACK Bit 7 = 0 Bit 6 = 1)
  {                              // Ja =>
  param_cnt = ((uint16_t)c << 8) & 0x3F00;   // Anzahl Parameter (High Byte)
  if ((res = ComReadByte(dev->Io, &c)) < 0)  // Anzahl Parameter (Low Byte) lesen
    {
    ComClose(dev->Io);
    return(ERR_SM_RX_PARAM_CNT);
    }
  if (res == 0)
    return(ERR_SM_RX_PARAM_CNT_TIMEOUT);
  crc = updcrc8(c, crc);
  param_cnt |= c;
  dev->RxDParameter.Count = param_cnt;
  // Parameter lesen
  if ((res = ComReadData(dev->Io, (char *)dev->RxDParameter.Data, param_cnt)) < 0)
    {
    ComClose(dev->Io);
    return(ERR_SM_RX_PARAM);
    }
  if ((uint32_t)res != param_cnt)
    {
#ifdef LOG_DEBUG_ENABLE    
    LogPrintf(LOG_DEBUG, "[DEBUG] SMDrvCommandoRx %u von %u Parameter Bytes gelesen", res , param_cnt);
#endif
    if (res > 0)    
      return(ERR_SM_RX_PARAM_COUNT);
    else
      return(ERR_SM_RX_PARAM_TIMEOUT);
    }
  //if ((uint32_t)res != param_cnt)   // Anzahl Empfangener Parameter stimmt nicht <*>
  //  return(ERR_SM_RX_PARAM_COUNT);
  for (i = 0; i < param_cnt; i++)
    {
    c = dev->RxDParameter.Data[i];
    crc = updcrc8(c, crc);
    }
  // CRC lesen
  if ((res = ComReadByte(dev->Io, &read_crc)) < 0)
    {
    ComClose(dev->Io);
    return(ERR_SM_RX_CRC);
    }
  if (res == 0)
    return(ERR_SM_RX_CRC_TIMEOUT);
  crc = CRCTabelle[CRCTabelle[crc]];
  if (crc != read_crc)           // CRC prüfen
    return(ERR_SM_WRONG_CRC);
  }
else
  {
  if ((c & 0xE0) == 0xA0)              
    return(ERR_SM_NACK);                  // NACK      
  else if (c != 0x95)
    return(ERR_SM_WRONG_ACK);             // Fehler beim ACK Empfang
  }
return(0);
}


int32_t SMDrvCommando(TCanDevice *dev, uint32_t sleep_time)
{
int32_t res;
uint8_t try_count, nack;

if (ComIsOpen(dev->Io) < 1)          // Com Port geöffnet ?
  return(ERR_SM_PORT_NOT_OPEN);
res = 0;
for (try_count = 1; try_count <= dev->CommTryCount; try_count++)
  {  
  // **** Kommando senden
  if ((res = SMDrvCommandoTx(dev) < 0))
    {
#ifdef LOG_DEBUG_ENABLE    
         LogPrintf(LOG_DEBUG, "[DEBUG] SMDrvCommando Fehler: %s", SmDrvGetErrorStr((int)res));
#endif  
    return(res);
    }
  if (sleep_time)  
    mhs_sleep(sleep_time);  
  // **** ACK Empfangen
  res = SMDrvCommandoRx(dev);
  if (res >= 0)
    break;
  switch (res)
    {
    // Fehler Schnittstelle
    case ERR_SM_RX_ACK       :
    case ERR_SM_RX_PARAM_CNT :
    case ERR_SM_RX_PARAM     :
    case ERR_SM_RX_CRC       : 
         {
#ifdef LOG_DEBUG_ENABLE    
         LogPrintf(LOG_DEBUG, "[DEBUG] SMDrvCommando Fehler: %s", SmDrvGetErrorStr((int)res));
#endif                               
         return(res);
         }    
    // RX Timeout Fehler                                 
    case ERR_SM_RX_ACK_TIMEOUT       :
    case ERR_SM_RX_PARAM_CNT_TIMEOUT :
    case ERR_SM_RX_PARAM_TIMEOUT     :
    case ERR_SM_RX_CRC_TIMEOUT       :  
    
    case ERR_SM_RX_PARAM_COUNT       :
    
    case ERR_SM_WRONG_CRC            : 
         {
#ifdef LOG_DEBUG_ENABLE    
         LogPrintf(LOG_DEBUG, "[DEBUG] SMDrvCommando (%u/%u) %s",  try_count, dev->CommTryCount, SmDrvGetErrorStr((int)res));
#endif                                      
         dev->Kommando = CONTROL_COMMAND;
         dev->TxDParameter.Data[0] = CTRL_RETRANSMIT_LAST_ACK;
         break;  
         }
    
    case ERR_SM_NACK                 : 
         {
         nack = dev->ACKByte & 0x1F;
#ifdef LOG_DEBUG_ENABLE                                       
         if ((!nack) || (nack > NackErrorStrSize))
           LogPrintf(LOG_DEBUG, "[DEBUG] SMDrvCommando (%u/%u) NACK: 0x%02X Cmd:%02X", try_count,
               dev->CommTryCount, nack, dev->Kommando);     
         else  
           LogPrintf(LOG_DEBUG, "[DEBUG] SMDrvCommando (%u/%u) NACK: %s Cmd:%02X", try_count, 
               dev->CommTryCount, NackErrorStr[nack-1], dev->Kommando);                                       
#endif                                        
/* Fehlercodes:    
     SM_ERR_SIO         1
     SM_ERR_OV          2
     SM_ERR_CRC         3
     SM_ERR_PARAM_OV    4
     SM_ERR_RX_TIMEOUT  5
     SM_ERR_NO_LAST_ACK 6 */
         if (nack == SM_ERR_NO_LAST_ACK)
           return(res);
         break;
         }
    case ERR_SM_WRONG_ACK    :
         {
#ifdef LOG_DEBUG_ENABLE    
         LogPrintf(LOG_DEBUG, "[DEBUG] SMDrvCommando Fehler: %s (ACK: 0x%02X)", SmDrvGetErrorStr((int)res), dev->ACKByte);
#endif                               
         return(res);
         } 
    }
  }
return(res);
}
