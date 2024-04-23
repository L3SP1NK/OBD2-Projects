/***************************************************************************
                         log.c  -  description
                             -------------------
    begin             : 23.03.2008
    last modified     : 20.04.2023    
    copyright         : (C) 2008 - 2023 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "global.h"
#include "can_types.h"
#ifndef DISABLE_PNP_SUPPORT
  #ifdef MHSIPCAN_DRV
    #include "net_pnp.h"
  #else
    #include "usb_pnp.h"
  #endif  
#endif
#include "util.h"
#include "log.h"


/**************************************************************************/
/*                         V A R I A B L E N                              */
/**************************************************************************/
static FILE *LogFile;
static DRV_LOCK_TYPE *LogLock = NULL;
static uint32_t LogFlags;

uint32_t LogEnable = 0;


/**************************************************************************/
/*                        C O N S T A N T E N                             */
/**************************************************************************/
static const char *LogSpaceLinesTab[] = {
// 0        1         2         3         4         5         6
// 123456789012345678901234567890123456789012345678901234567890   
  "             ",
  "              ",
  "               ",
  "                ",
  "                 ",
  "                  ",
  "                   ",
  "                    ",  
  "                     ",
  "                      ",
  "                       ",
  "                        ",
  "                         ",
  "                          ",  
  "                           ",
  "                            ",
  "                             ",
  "                              ",
  "                               ",
  "                                ",
  "                                 ",  
  "                                  ",
  "                                   ",
  "                                    ",
  "                                     ",
  "                                      ",  
  "                                       ",
  "                                        ",
  "                                         ",
  "                                          ",
  "                                           ",
  "                                            ",
  "                                             ",
  "                                              ",  
  "                                               ",
  "                                                ",
  "                                                 ",
  "                                                  ",
  "                                                   ",
  "                                                    ",  
  "                                                     ",
  "                                                      ",
  "                                                       ",
  "                                                        ",  
  "                                                         ",
  "                                                          ",
  "                                                           "};


#define DRV_STATUS_MAX 9

static const char *DrvStatusStrings[] = {
  "NOT_LOAD",       // 0  Die Treiber DLL wurde noch nicht geladen
  "NOT_INIT",       // 1  Treiber noch nicht Initialisiert (Funktion "CanInitDrv" noch nicht aufgerufen)
  "INIT",           // 2  Treiber erfolgrich Initialisiert
  "PORT_NOT_OPEN",  // 3  Die Schnittstelle wurde nicht geöffnet
  "PORT_OPEN",      // 4  Die Schnittstelle wurde geöffnet
  "DEVICE_FOUND",   // 5  Verbindung zur Hardware wurde Hergestellt
  "CAN_OPEN",       // 6  Device wurde geöffnet und erfolgreich Initialisiert
  "CAN_RUN_TX",     // 7  CAN Bus RUN nur Transmitter (wird nicht verwendet !)
  "CAN_RUN",        // 8  CAN Bus RUN
  "UNBEKANNT"};

#define CAN_STATUS_MAX 5

static const char *CanStatusStrings[] = {
  "OK",          // 0  Ok
  "ERROR",       // 1  CAN Error
  "WARNING",     // 2  Error warning
  "PASSIV",      // 3  Error passiv
  "BUS_OFF",     // 4  Bus Off
  "UNBEKANNT"};  // 5  Status Unbekannt

#define FIFO_STATUS_MAX 4

static const char *FifoStatusStrings[] = {
  "OK",                 // 0  Fifo-Status: Ok
  "HW_OVERRUN",         // 1  Fifo-Status: Hardware Fifo Überlauf
  "SW_OVERRUN",         // 2  Fifo-Status: Software Fifo Überlauf
  "HW_SW_OVERRUN",      // 3  Fifo-Status: Hardware & Software Fifo Überlauf
  "UNBEKANNT"};         // 4  Fifo-Status: Unbekannt

static const char *CanMsgBusStatusStrings[] = {
  "Bus Ok",
  "Error Warn.",
  "Error Passiv",
  "Bus Off!"};
                   
static const char *CanMsgBusErrorStrings[] = {
  "-",                                           
  "Stuff Error",
  "Form Error",
  "Ack Error",
  "Bit1 Error",
  "Bit0 Error",
  "CRC Error"};


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
  
  
/**************************************************************************/
/*                        F U N K T I O N E N                             */
/**************************************************************************/

static void GetLogTimeStamp(char *buffer)
{
time_t time_now;
#if _MSC_VER >= 1500
struct tm time_info;
#else
struct tm *time_info;
#endif

time(&time_now);
#if _MSC_VER >= 1500
localtime_s(&time_info, &time_now);
strftime(buffer, 30, "%H:%M:%S %d.%m.%Y", &time_info);
#else
time_info = localtime(&time_now);
strftime(buffer, 30, "%H:%M:%S %d.%m.%Y", time_info);
#endif
}


static void LogTimeStamp(void) 
{
char buffer[30];

if ((LogFlags & LOG_WITH_TIME)) 
  {
  GetLogTimeStamp((char *)&buffer);
  fprintf(LogFile, "%s  ", buffer);
  }
}

/*
******************** LogInit ********************
Funktion  : Initialisiert den Treiber
*/
void LogInit(void)
{
LogLock = NULL;
LogFile = NULL;
LogEnable = 0;
}


/*
******************** LogDestroy ********************
Funktion  : Log File schließen

Eingaben  : keine

Ausgaben  : keine
*/
void LogDestroy(void)
{
LogEnable = 0;
if (LogFile)
  {
  DRV_LOCK_ENTER(LogLock);
  fflush(LogFile);
  (void)fclose(LogFile);
  LogFile = NULL;
  DRV_LOCK_LEAVE(LogLock);
  DRV_LOCK_DESTROY(LogLock);
  }
}


/*
******************** LogCreate ********************
Funktion  : Log File öffnen

Eingaben  : filename => Log Filename

Ausgaben  : result => 0 = kein Fehler
                      -1 = Fehler beim öffnen/anlegen des Log Files
*/
int32_t LogCreate(char *filename, uint32_t flags)
{
//LogClose(); <*>
DRV_LOCK_INIT(LogLock);
DRV_LOCK_ENTER(LogLock);
LogFlags = flags;
//LogFile = fopen(filename, "a"); //O_WRONLY|O_APPEND|O_CREAT);
#if _MSC_VER >= 1500
if (fopen_s(&LogFile, filename, "w+"))
  LogFile = NULL;
#else
LogFile = fopen(filename, "w+");
#endif
DRV_LOCK_LEAVE(LogLock);
if (LogFile)
  {
  LogEnable = 1;
  return(0);
  }
else
  return(-1);
}


/*
******************** LogPrintf ********************
Funktion  : Einen "printf" formatierten String in das Log File schreiben

Eingaben  : log_type =>
            fmt => printf Format String
            ... => Printf Parameter

Ausgaben  : keine
*/
static void LogVPrintf(const char *fmt, va_list ap)
{
if (!LogFile)
  return;
LogTimeStamp();  
vfprintf(LogFile, fmt, ap);
fprintf(LogFile, "\n");
#ifndef LOG_FILE_MODE_BUFFERED
if (!(LogFlags & LOG_DISABLE_SYNC))
  fflush(LogFile);
#endif
}


void LogPrintf(uint32_t log_type, const char *fmt, ...)
{
va_list ap;

if ((LogEnable) && (log_type & LogFlags))
  {  
  DRV_LOCK_ENTER(LogLock);
  va_start(ap, fmt);
  LogVPrintf(fmt, ap);
  va_end(ap);
  DRV_LOCK_LEAVE(LogLock);
  }
}


void LogPrintfUnlocked(const char *fmt, ...)
{
va_list ap;

va_start(ap, fmt);
LogVPrintf(fmt, ap);
va_end(ap);
}


static void LogWriteFdErrMsg(struct TCanFdMsg *msg, uint32_t flags)
{
uint8_t source, err_nr, bus_stat;
uint32_t lost_msgs;
const char *can_drv_stat;

can_drv_stat = "";
source = msg->MsgSource;
if (msg->MsgErr)
  {  
  err_nr = msg->MsgData[0];
  bus_stat = msg->MsgData[1] & 0x0F;
  if (msg->MsgData[1] & 0x10)
    can_drv_stat = " BUS-FAILURE";
  }
else if (msg->MsgOV)
  {
  err_nr = msg->MsgData[0];
  lost_msgs = msg->MsgData[1] & ((uint32_t)msg->MsgData[2] << 8);          
  }      
else
  return;
if (flags & LOG_FLAG_SOURCE)
  {   
  if (msg->MsgErr)
    {     
    if ((err_nr > 0) && (err_nr < 7) && (bus_stat < 4))
      fprintf(LogFile, "Source:0x%02X  ERROR [%s REC:%u TEC:%u%s] %s\n", msg->MsgSource, CanMsgBusStatusStrings[bus_stat],
      msg->MsgData[2], msg->MsgData[3], can_drv_stat, CanMsgBusErrorStrings[err_nr]);
    else  
      fprintf(LogFile, "Source:0x%02X  ERROR Unbek. Fehler\n", source);
    }
  else
    {
    if ((err_nr > 0) && (err_nr < 4))
      fprintf(LogFile, "Source:0x%02X  OV [%u] Messages-Lost: %u\n", msg->MsgSource, err_nr, lost_msgs);
    else
      fprintf(LogFile, "Source:0x%02X  OV Unbek. Fehler\n", source);
    }    
  }
else
  {
  if (msg->MsgErr)
    {     
    if ((err_nr > 0) && (err_nr < 7) && (bus_stat < 4))
      fprintf(LogFile, "      ERROR [%s REC:%u TEC:%u%s] %s\n",  CanMsgBusStatusStrings[bus_stat], msg->MsgData[2], 
          msg->MsgData[3], can_drv_stat, CanMsgBusErrorStrings[err_nr]);
    else  
      fprintf(LogFile, "      ERROR Unbek. Fehler\n");
    }
  else
    {
    if ((err_nr > 0) && (err_nr < 4))
      fprintf(LogFile, "      OV [%u] Messages-Lost: %u\n", err_nr, lost_msgs);
    else
      fprintf(LogFile, "      OV Unbek. Fehler\n");
    }    
  }  
} 


static void LogWriteMessage(struct TCanFdMsg *msg, uint32_t flags)
{
char *ptr;
unsigned char ch, hex, chars, first;
int32_t cnt, i, space_len;
const char *type_str, *fd_str, *space_str;
char id_str[10];
char dlc_str[8];
char msg_str[30];

if ((msg->MsgErr) || (msg->MsgOV))
  {
  LogWriteFdErrMsg(msg, flags);
  return;
  }
space_len = 13;  // CAN ID = 8 + DLC = 1
fd_str = "";
type_str = "";
if ((flags & (LOG_FLAG_FD | LOG_FLAG_TYPE)) == (LOG_FLAG_FD | LOG_FLAG_TYPE))
  {
  if (msg->MsgFD)
    {
    if (msg->MsgBRS)
      fd_str = "FD/BRS";
    else
      fd_str = "FD    "; 
    }
  else
    fd_str = "      ";
  space_len += 9;  
  }    
// Message Format
if (flags & LOG_FLAG_TYPE)
  {
  if ((msg->MsgRTR) && (msg->MsgEFF))
    type_str = "EFF/RTR";
  else if (msg->MsgEFF)
    type_str = "EFF    ";
  else if (msg->MsgRTR)
    type_str = "STD/RTR";
  else
    type_str = "STD    ";
  space_len += 10;  
  }  
// ID
if (msg->MsgEFF)
  safe_sprintf(id_str, 10, "%08X", msg->Id);
else
  safe_sprintf(id_str, 10, "     %03X", msg->Id);
// Dlc
cnt = msg->MsgLen;
if (flags & LOG_FLAG_FD)
  {
  safe_sprintf(dlc_str, 8, "%2u", (uint32_t)cnt);
  space_len += 1;
  }
else  
  safe_sprintf(dlc_str, 8, "%1u", (uint32_t)cnt);
if (flags & LOG_FLAG_TIMESTAMP)
  space_len += 19;
if (flags & LOG_FLAG_SOURCE)
  space_len += 4;  
// ** Timestamp + Source enabled  
if (flags == (LOG_FLAG_TIMESTAMP | LOG_FLAG_SOURCE | LOG_FLAG_FD | LOG_FLAG_TYPE))   
  fprintf(LogFile, "%8u.%8u | %02X | %s %s | %s | %s ", msg->Time.Sec, msg->Time.USec, msg->MsgSource, fd_str, type_str, id_str, dlc_str);
else if (flags == (LOG_FLAG_TIMESTAMP | LOG_FLAG_SOURCE | LOG_FLAG_TYPE))   
  fprintf(LogFile, "%8u.%8u | %02X | %s | %s | %s ", msg->Time.Sec, msg->Time.USec, msg->MsgSource, type_str, id_str, dlc_str);   
else if (flags == (LOG_FLAG_TIMESTAMP | LOG_FLAG_SOURCE))   
  fprintf(LogFile, "%8u.%8u | %02X | %s | %s ", msg->Time.Sec, msg->Time.USec, msg->MsgSource, id_str, dlc_str);   
// ** Timestamp disabled  
else if (flags == (LOG_FLAG_SOURCE | LOG_FLAG_FD | LOG_FLAG_TYPE))   
  fprintf(LogFile, "%02X | %s %s | %s | %s ", msg->MsgSource, fd_str, type_str, id_str, dlc_str);
else if (flags == (LOG_FLAG_SOURCE | LOG_FLAG_TYPE))   
  fprintf(LogFile, "%02X | %s | %s | %s ", msg->MsgSource, type_str, id_str, dlc_str);   
else if (flags == LOG_FLAG_SOURCE)   
  fprintf(LogFile, "%02X | %s | %s ", msg->MsgSource, id_str, dlc_str);
// ** Source disabled  
else if (flags == (LOG_FLAG_TIMESTAMP | LOG_FLAG_FD | LOG_FLAG_TYPE))   
  fprintf(LogFile, "%8u.%8u | %s %s | %s | %s ", msg->Time.Sec, msg->Time.USec, fd_str, type_str, id_str, dlc_str);
else if (flags == (LOG_FLAG_TIMESTAMP | LOG_FLAG_TYPE))   
  fprintf(LogFile, "%8u.%8u | %s | %s | %s ", msg->Time.Sec, msg->Time.USec, type_str, id_str, dlc_str);   
else if (flags == LOG_FLAG_TIMESTAMP)   
  fprintf(LogFile, "%8u.%8u | %s | %s ", msg->Time.Sec, msg->Time.USec, id_str, dlc_str);     
else    
  fprintf(LogFile, "%s | %s ", id_str, dlc_str);         
// Daten
space_str = LogSpaceLinesTab[space_len - 13];
if (msg->MsgRTR)
  cnt = 0;
ptr = msg_str;
i = 0;
first = 1;  
while (cnt)
  {  
  ptr = msg_str;    
  for (chars = 0; cnt; cnt--)
    {
    if (++chars > 8)
      break;
    ch = msg->MsgData[i++];
    hex = ch >> 4;
    if (hex > 9)
      *ptr++ = 55 + hex;
    else
      *ptr++ = '0' + hex;
    hex = ch & 0x0F;
    if (hex > 9)
      *ptr++ = 55 + hex;
    else
      *ptr++ = '0' + hex;
    *ptr++ = ' ';            
    }      
  *ptr = '\0';
  if (chars)
    {
    if (first)
      {
      first = 0;    
      fprintf(LogFile, "%s\n", msg_str);
      }
    else    
      fprintf(LogFile, "%s%s\n", space_str, msg_str);
    }  
  }
if (first)
  fprintf(LogFile, "\n");   
}

 
/*
******************** LogCanMessage ********************
*/
void LogCanMessage(uint32_t log_type, struct TCanMsg *msg, uint32_t format, const char *fmt, ...)
{
va_list ap;
struct TCanFdMsg fdmsg;

if ((LogEnable) && (log_type & LogFlags))
  {
  DRV_LOCK_ENTER(LogLock);
  if (fmt)
    {
    va_start(ap, fmt);
    LogVPrintf(fmt, ap);
    va_end(ap);
    }
  CanToCanFd(&fdmsg, msg);  
  LogWriteMessage(&fdmsg, format);  
#ifndef LOG_FILE_MODE_BUFFERED
  if (!(LogFlags & LOG_DISABLE_SYNC))
    fflush(LogFile);
#endif
  DRV_LOCK_LEAVE(LogLock);
  }
}


/*
******************** LogCanMessages ********************
*/
void LogCanMessages(uint32_t log_type, struct TCanMsg *msg, int32_t count, uint32_t format, const char *fmt, ...)
{
va_list ap;
struct TCanFdMsg fdmsg;

if ((LogEnable) && (log_type & LogFlags))
  {
  DRV_LOCK_ENTER(LogLock);
  if (fmt)
    {
    va_start(ap, fmt);
    LogVPrintf(fmt, ap);
    va_end(ap);
    }
  for (; count; count--)
    {  
    CanToCanFd(&fdmsg, msg++);  
    LogWriteMessage(&fdmsg, format);
    } 
#ifndef LOG_FILE_MODE_BUFFERED
  if (!(LogFlags & LOG_DISABLE_SYNC))
    fflush(LogFile);
#endif
  DRV_LOCK_LEAVE(LogLock);
  }
}


/*
******************** LogCanFdMessage ********************
*/
void LogCanFdMessage(uint32_t log_type, struct TCanFdMsg *msg, uint32_t format, const char *fmt, ...)
{
va_list ap;

if ((LogEnable) && (log_type & LogFlags))
  {
  DRV_LOCK_ENTER(LogLock);
  if (fmt)
    {
    va_start(ap, fmt);
    LogVPrintf(fmt, ap);
    va_end(ap);
    }
  if (DriverInit != TCAN_DRV_FD_INIT)
    format &= (~LOG_FLAG_FD);  
  LogWriteMessage(msg, format);  
#ifndef LOG_FILE_MODE_BUFFERED
  if (!(LogFlags & LOG_DISABLE_SYNC))
    fflush(LogFile);
#endif
  DRV_LOCK_LEAVE(LogLock);
  }
}


/*
******************** LogCanFdMessages ********************
*/
void LogCanFdMessages(uint32_t log_type, struct TCanFdMsg *msg, int32_t count, uint32_t format, const char *fmt, ...)
{
va_list ap;

if ((LogEnable) && (log_type & LogFlags))
  {
  DRV_LOCK_ENTER(LogLock);  
  if (fmt)
    {
    va_start(ap, fmt);
    LogVPrintf(fmt, ap);
    va_end(ap);
    }
  if (DriverInit != TCAN_DRV_FD_INIT)
    format &= (~LOG_FLAG_FD);    
  for (; count; count--) 
    LogWriteMessage(msg++, format);
#ifndef LOG_FILE_MODE_BUFFERED
  if (!(LogFlags & LOG_DISABLE_SYNC))
    fflush(LogFile);
#endif
  DRV_LOCK_LEAVE(LogLock);
  }
}


/*
******************** LogStringList ********************
  [Bezeichner]=[Value];[Bezeichner]=[Value];[..]=[..]

*/
void LogStringList(uint32_t log_type, const char *str_list, const char *fmt, ...)
{
va_list ap;
char *tmp_str, *str, *key, *value;
int32_t res;

if ((LogEnable) && (log_type & LogFlags))
  {
  DRV_LOCK_ENTER(LogLock);
  if (fmt)
    {
    va_start(ap, fmt);
    LogVPrintf(fmt, ap);
    va_end(ap);
    }
  if (!check_string(str_list))
    fprintf(LogFile, "keine\n");
  else
    {        
    tmp_str = mhs_strdup(str_list);
    str = tmp_str;
    while (*str)
      {
      // **** Key auslesen
      key = get_item_as_string(&str, "=;", GET_ITEM_DEF_FLAGS, &res);
      if ((res < 0) || (!key))
        break;    
      // Führende Leerzeichen löschen
      while (*key == ' ')
        key++;
      if (*key == '\0')
        break;    
      // **** Value auslesen
      value = get_item_as_string(&str, "=;", GET_ITEM_DEF_FLAGS, &res);
      if (res == 1) // = gefunden (Key) fehler
        break;
      // Führende Leerzeichen löschen
      if (value)
        {
        while (*value == ' ')
          value++;
        if (*value == '\0')
          value = NULL;
        }  
      fprintf(LogFile, "   %s = %s\n", key, value);
      }            
    safe_free(tmp_str);
    }            
#ifndef LOG_FILE_MODE_BUFFERED
  if (!(LogFlags & LOG_DISABLE_SYNC))
    fflush(LogFile);
#endif
  DRV_LOCK_LEAVE(LogLock);
  }
}


/*
******************** LogSysError ********************
*/
/*void LogSysError(char *file_name, char *func_name, int32_t error_no) <*>
{
if ((!file_name) || (!func_name))
  return;
if (LogEnable)
  {
  DRV_LOCK_ENTER(LogLock);
  LogTimeStamp();  
  fprintf(LogFile, "FATAL-ERROR: File:%s, Funktion:%s, Error:%d\n", file_name, func_name, (int32_t)error_no);
#ifndef LOG_FILE_MODE_BUFFERED
  if (!(LogFlags & LOG_DISABLE_SYNC))
    fflush(LogFile);
#endif
  DRV_LOCK_LEAVE(LogLock);
  }
} */


/*
******************** LogHexDump ********************
Funktion  :

Eingaben  :

Ausgaben  : keine
*/
/*void LogHexDump(uint32_t log_type, char *data, int32_t count) <*>
{
char str[50];
char *ptr;
int32_t cnt;
unsigned char ch, hex;

if ((LogEnable) && (log_type & LogFlags))
  {
  DRV_LOCK_ENTER(LogLock);
  while (count)
    {
    if (count > 16)
      cnt = 16;
    else
      cnt = count;
    count -= cnt;
    ptr = str;
    for (;cnt;cnt--)
      {
      ch = (unsigned char)*data++;
      hex = ch >> 4;
      if (hex > 9)
        *ptr++ = 55 + hex;
      else
        *ptr++ = '0' + hex;
      hex = ch & 0x0F;
      if (hex > 9)
        *ptr++ = 55 + hex;
      else
        *ptr++ = '0' + hex;
      *ptr++ = ' ';
      }
    *ptr = '\0';
    fprintf(LogFile, "   %s\n", str);
    }
#ifndef LOG_FILE_MODE_BUFFERED
  if (!(LogFlags & LOG_DISABLE_SYNC))
    fflush(LogFile);
#endif
  DRV_LOCK_LEAVE(LogLock);
  }
} */


/*
******************** LogPrintDrvStatus ********************
*/
void LogPrintDrvStatus(uint32_t log_type, uint32_t channel, struct TDeviceStatus *dev_status)
{
int32_t drv_status;
uint8_t can_status, can_bus_status, fifo_status;

drv_status = dev_status->DrvStatus;
can_status = dev_status->CanStatus;
fifo_status = dev_status->FifoStatus;
if ((LogEnable) && (log_type & LogFlags))
  {
  LogTimeStamp();
  if (drv_status > DRV_STATUS_MAX)
    drv_status = DRV_STATUS_MAX;
  if (can_status > CAN_STATUS_MAX)
    can_bus_status = CAN_STATUS_MAX;
  else
    can_bus_status = can_status;
  if (fifo_status > FIFO_STATUS_MAX)
    fifo_status = FIFO_STATUS_MAX;
  DRV_LOCK_ENTER(LogLock);
  if (can_status & BUS_FAILURE)    
    fprintf(LogFile, "STATUS: Ch %d: Drv=%s, Can=%s [BUS-FAILURE], Fifo=%s\n", channel, DrvStatusStrings[drv_status],
      CanStatusStrings[can_bus_status], FifoStatusStrings[fifo_status]);
  else      
    fprintf(LogFile, "STATUS: Ch %d: Drv=%s, Can=%s, Fifo=%s\n", channel, DrvStatusStrings[drv_status],
      CanStatusStrings[can_bus_status], FifoStatusStrings[fifo_status]);
#ifndef LOG_FILE_MODE_BUFFERED
  if (!(LogFlags & LOG_DISABLE_SYNC))
    fflush(LogFile);
#endif
  DRV_LOCK_LEAVE(LogLock);
  }
}


void LogPrintUsbDevices(uint32_t log_type, struct TCanDevicesList *list, int32_t num_devs)
{
int32_t i;

if (!list)
  num_devs = 0;
if ((LogEnable) && (log_type & LogFlags))
  {
  DRV_LOCK_ENTER(LogLock);
  fprintf(LogFile, "    Open    |Tiny-CAN Id | Snr        | USB-Description \n");
  fprintf(LogFile, "============+============+============+=====================\n");
  if (num_devs <= 0)
    fprintf(LogFile, "keine Devices gefunden.\n");
  else
    {
    for (i = 0; i < num_devs; i++)
      {
      fprintf(LogFile, " 0x%08X | 0x%08X | %10s | %s\n", list[i].TCanIdx, list[i].HwId,
          list[i].SerialNumber, list[i].Description);
      }
    }
  fprintf(LogFile, "============+============+============+=====================\n");
#ifndef LOG_FILE_MODE_BUFFERED
  if (!(LogFlags & LOG_DISABLE_SYNC))
    fflush(LogFile);
#endif
  DRV_LOCK_LEAVE(LogLock);
  }
}


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


void LogPrintHwInfo(uint32_t log_type, struct TCanInfoVar *info, uint32_t size)
{
uint32_t i, value, tag, monat, jahr, major,  minor, revision;

if (!info)
  return;
if ((LogEnable) && (log_type & LogFlags))
  {
  DRV_LOCK_ENTER(LogLock);
    
  fprintf(LogFile, "    Key    |   Type   | Size | Value\n");
  fprintf(LogFile, "-----------+----------+------+-------------------------------\n");
  for (i = 0; i < size; i++)
    {  
    fprintf(LogFile, "0x%08X | %-8s |  %-3u | ", info[i].Key, GetValueTypeString(info[i].Type), info[i].Size);

    switch (info[i].Type)
      {
      case VT_BYTE   : {
                       fprintf(LogFile, "%d\n", *((signed char*)(info[i].Data)));
                       break;
                       }
      case VT_UBYTE  : {
                       fprintf(LogFile, "%u\n", *((unsigned char *)(info[i].Data)));
                       break;
                       }
      case VT_HBYTE  : {
                       fprintf(LogFile, "0x%02X\n", *((unsigned char *)(info[i].Data)));
                       break;
                       }
      case VT_WORD   : {
                       fprintf(LogFile, "%d\n", *((signed short *)(info[i].Data)));
                       break;
                       }
      case VT_UWORD  : {
                       fprintf(LogFile, "%u\n", *((uint16_t *)(info[i].Data)));
                       break;
                       }
      case VT_HWORD  : {
                       fprintf(LogFile, "0x%04X\n", *((uint16_t *)(info[i].Data)));
                       break;
                       }
      case VT_LONG   : {
                       fprintf(LogFile, "%d\n", *((int32_t *)(info[i].Data)));
                       break;
                       }
      case VT_ULONG  : {
                       fprintf(LogFile, "%u\n", *((uint32_t *)(info[i].Data)));
                       break;
                       }
      case VT_HLONG  : {
                       fprintf(LogFile, "0x%08X\n", *((uint32_t *)(info[i].Data)));
                       break;
                       }                
      case VT_REVISION :
                       {
                       value = *((uint32_t *)(info[i].Data));
                       major = value / 10000;
                       value = value % 10000;  
                       minor = value / 100;                   
                       revision = value % 100;                   
                       fprintf(LogFile, "%u.%u.%u\n", major, minor, revision); 
                       break; 
                       }
      case VT_DATE   : {
                       value = *((uint32_t *)(info[i].Data));
                       tag = GetBCD(value & 0xFF, 2);
                       monat = GetBCD((value >> 8) & 0xFF, 2);  
                       jahr = GetBCD((value >> 16) & 0xFFFF, 4);
                       fprintf(LogFile, "%02u.%02u.%4u\n", tag, monat, jahr);
                       break; 
                       }                       
      case VT_STRING : {                    
                       fprintf(LogFile, "%s\n", info[i].Data);
                       break;
                       }
      }
    }
  fprintf(LogFile, "-----------+----------+------+-------------------------------\n");  
#ifndef LOG_FILE_MODE_BUFFERED
  if (!(LogFlags & LOG_DISABLE_SYNC))
    fflush(LogFile);
#endif
  DRV_LOCK_LEAVE(LogLock);  
  }    
}


void LogPrintDeviceInfo(uint32_t log_type, struct TCanDeviceInfo *info)
{
char *str;
char str_puf[100];
uint32_t ver, ver2;

if (!info)
  return;
if ((LogEnable) && (log_type & LogFlags))
  {
  DRV_LOCK_ENTER(LogLock);
  fprintf(LogFile, "Device:%s [Snr:%s]\n", info->Description, info->SerialNumber);
  fprintf(LogFile, "   Hardware Id      : 0x%08X\n", info->HwId);
  ver = info->FirmwareVersion / 1000;
  ver2 = info->FirmwareVersion % 1000;
  fprintf(LogFile, "   Firmware Version : %u.%03u\n", ver, ver2);
  fprintf(LogFile, "   CanClock         : %u\n", info->ModulFeatures.CanClock);
  str = str_puf;
  if (info->ModulFeatures.Flags & CAN_FEATURE_LOM)            // Silent Mode (LOM = Listen only Mode)
    str = mhs_stpcpy(str, "LOM ");
  if (info->ModulFeatures.Flags & CAN_FEATURE_ARD)            // Automatic Retransmission disable
    str = mhs_stpcpy(str, "ARD ");
  if (info->ModulFeatures.Flags & CAN_FEATURE_TX_ACK)         // TX ACK
    str = mhs_stpcpy(str, "TX_ACK ");
  if (info->ModulFeatures.Flags & CAN_FEATURE_HW_TIMESTAMP)
    str = mhs_stpcpy(str, "HW_TIMESTAMP ");
  fprintf(LogFile, "   Features-Flags   : %s\n", str_puf);
  fprintf(LogFile, "   CanChannelsCount : %u\n", info->ModulFeatures.CanChannelsCount);
  fprintf(LogFile, "   HwRxFilterCount  : %u\n", info->ModulFeatures.HwRxFilterCount);
  fprintf(LogFile, "   HwTxPufferCount  : %u\n", info->ModulFeatures.HwTxPufferCount);
  fprintf(LogFile, "\n");
#ifndef LOG_FILE_MODE_BUFFERED
  if (!(LogFlags & LOG_DISABLE_SYNC))
    fflush(LogFile);
#endif
  DRV_LOCK_LEAVE(LogLock);  
  }
}


#ifdef MHSTCAN_DRV
void LogProtokollInfo(uint32_t log_type, TCanDevice *dev, struct TBiosVersionInfo *version_info)
{
uint16_t rx_mtu;

if ((!dev) || (!version_info))
  return;
if ((LogEnable) && (log_type & LogFlags))
  {
  DRV_LOCK_ENTER(LogLock);
  if (version_info->Version >= 0x10)    
    rx_mtu = version_info->RxMTU;
  else
    rx_mtu = (uint16_t)dev->ModulDesc->RxMtu;
  fprintf(LogFile, "[DEBUG] Device SM-Protokoll Info:\n");      
  fprintf(LogFile, "     Version : 0x%02X\n", version_info->Version);   // Protokoll Version
  fprintf(LogFile, "     Flags   : 0x%02X\n", version_info->Flags);     // Flags
  fprintf(LogFile, "     TX-MTU  : %u\n", version_info->TxMTU);         // Tx MTU
  fprintf(LogFile, "     RX-MTU  : %u\n", rx_mtu);              // Rx MTU
#ifndef LOG_FILE_MODE_BUFFERED
  if (!(LogFlags & LOG_DISABLE_SYNC))
    fflush(LogFile);
#endif
  DRV_LOCK_LEAVE(LogLock);  
  }
}


void LogPrintTCanUsbList(uint32_t log_type, struct TTCanUsbList *list, const char *fmt, ...)
{
va_list ap;
 
if ((LogEnable) && (log_type & LogFlags))
  {
  DRV_LOCK_ENTER(LogLock);
  if (fmt)
    {
    va_start(ap, fmt);
    LogVPrintf(fmt, ap);
    va_end(ap);
    }
  if (!list)
    fprintf(LogFile, "Tiny-CAN List: keine Devices gefunden.\n");
  else
    {
    fprintf(LogFile, "Tiny-CAN List:\n");
    fprintf(LogFile, "    Status  |   Modul    | Snr        | USB-Description \n");
    fprintf(LogFile, "============+============+============+=====================\n");
    for (; list; list = list->Next)
      {
      if (list->Modul)
        fprintf(LogFile, " 0x%08X | 0x%08X | %10s | %s\n", list->Status, list->Modul->HwId,
            list->Serial, list->Description);
      else
        fprintf(LogFile, " 0x%08X |  --------  | %10s | %s\n", list->Status,
          list->Serial, list->Description);
      }
    fprintf(LogFile, "============+============+============+=====================\n");
    }
#ifndef LOG_FILE_MODE_BUFFERED
  if (!(LogFlags & LOG_DISABLE_SYNC))
    fflush(LogFile);
#endif
  DRV_LOCK_LEAVE(LogLock);
  }
}

#endif