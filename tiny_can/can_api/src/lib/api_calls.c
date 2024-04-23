/***************************************************************************
                         api_calls.c  -  description
                             -------------------
    begin             : 16.01.2011
    last modified     : 20.04.2022
    copyright         : (C) 2011 - 2023 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include "config.h"
#include "global.h"
#include <stdio.h>
#include <string.h>
#include "errors.h"
#include "can_types.h"
#if (defined(ENABLE_CONFIG_FILE)) || (defined(ENABLE_LOG_SUPPORT))
#include "paths.h"
#endif
#include "log.h"
#include "util.h"
#include "var_obj.h"
#include "index.h"
#include "drv_info.h"
#include "can_core.h"
#include "can_obj.h"
#include "can_fifo.h"
#include "can_puffer.h"
#ifndef DISABLE_PNP_SUPPORT
  #if defined(MHSIPCAN_DRV)
    #include "net_pnp.h"
  #elif defined(MHSPASSTHRU_DRV)
    #include "passthru_pnp.h"
  #else
    #include "usb_pnp.h"
  #endif
#endif
#ifdef HAVE_CAN_COOL_CHECK
  #include "can_cool_check.h"
#endif
//#include "tcan_drv.h" <*>
#include "api_cp.h"
#include "mhs_cmd.h"
#include "user_mem.h"
#include "api_ex.h"
#include "api_calls.h"

/*
Initialisierungsvariablen

Bezeichner            | Beschreibung                                    | Initialisierung
======================+=================================================+==========================
CanRxDFifoSize        | Größe des Empfangsfifos in Messages             | CAN_RXD_FIFO_SIZE
----------------------+-------------------------------------------------+--------------------------
CanTxDFifoSize        | Größe des Sendefifos in Messages                | CAN_TXD_FIFO_SIZE
----------------------+-------------------------------------------------+--------------------------
CanRxDMode            | 0 = Die RxD Callbackfunktion übergibt keine     | CAN_RXD_MODE
                      |     CAN Nachrichten                             |
                      | 1 = Die RxD Callbackfunktion pbergibt die       |
                      |     Empfangenen CAN Nachrichten                 |
----------------------+-------------------------------------------------+--------------------------
CanRxDBufferSize      | Größe des Übergabepuffers fr RxD Event Proc.    | CAN_RXD_BUFFER_SIZE
----------------------+-------------------------------------------------+--------------------------
CanCallThread         | 0 = Callback Thread nicht erzeugen              | CAN_CALL_THREAD
                      | 1 = Callback Thread erzeugen                    |
----------------------+-------------------------------------------------+--------------------------
MainThreadPriority    | 0 = THREAD_PRIORITY_NORMAL                      | MAIN_THREAD_PRIORITY
                      | 1 = THREAD_PRIORITY_ABOVE_NORMAL                |
                      | 2 = THREAD_PRIORITY_HIGHEST                     |
                      | 3 = THREAD_PRIORITY_TIME_CRITICAL               |
                      | 4 = THREAD_PRIORITY_REALTIME                    |
----------------------+-------------------------------------------------+--------------------------
CallThreadPriority    | 0 = THREAD_PRIORITY_NORMAL                      | CALL_THREAD_PRIORITY
                      | 1 = THREAD_PRIORITY_ABOVE_NORMAL                |
                      | 2 = THREAD_PRIORITY_HIGHEST                     |
                      | 3 = THREAD_PRIORITY_TIME_CRITICAL               |
----------------------+-------------------------------------------------+--------------------------
MinEventSleepTime     | Min. Wartezeit für das wiederholte aufrufen     | MIN_EVENT_SLEEP_TIME_INIT
                      | von Event Callbacks                             |
----------------------+-------------------------------------------------+--------------------------
Hardware              | Reserviert, sollte nicht gesetzt werden         | HARDWARE
----------------------+-------------------------------------------------+--------------------------
CfgFile               | Config File Name                                |
----------------------+-------------------------------------------------+--------------------------
Section               | Section Name im Cfg File                        |
----------------------+-------------------------------------------------+--------------------------
LogFile               | Log File Name                                   | LOG_FILE
----------------------+-------------------------------------------------+--------------------------
LogFlags              | Log Flags                                       | LOG_FLAGS
======================+=================================================+==========================

* = gelöscht!


Option Variablen

======================+=================================================+==========================
AutoConnect           | 0 = Auto Connect Modus aus                      | AUTO_CONNECT_INIT
                      | 1 = Auto Connect Modus ein                      |
----------------------+-------------------------------------------------+--------------------------
AutoReopen            | 0 = CanDeviceOpen wird nicht automatisch        | AUTO_REOPEN_INIT
                      |     aufgerufen                                  |
                      | 1 = CanDeviceOpen wird automatisch aufgerufen   |
                      |     nachdem die Verbindung wieder hergestellt   |
                      |     wurde                                       |
----------------------+-------------------------------------------------+--------------------------
*/

static const uint32_t MAIN_THREAD_PRIORITY_INIT = {4};
static const unsigned char TIME_STAMP_MODE_INIT = {0};
static const uint32_t CAN_TXD_FIFO_SIZE_INIT    = {2048};
static const uint32_t CAN_RXD_FIFO_SIZE_INIT    = {32768};
static const uint32_t EXECUTE_COMMAND_TIMEOUT_INIT = {20000};    // 20 Sekunden max. warten
static const uint32_t LOW_POLL_INTERVALL_INIT      = {250};
static const uint32_t IDLE_POLL_INTERVALL_INIT     = {0};
static const uint32_t HIGH_POLL_INTERVALL_INIT     = {0};
static const uint32_t FILTER_READ_INTERVALL_INIT   = {1000};    // Filter alle 1 Sekunden lesen
static const uint32_t TX_CAN_FIFO_EVENT_LIMIT_INIT = {0};
static const unsigned char CAN_RXD_MODE_INIT    = {0};
static const uint32_t CAN_RXD_BUFFER_SIZE_INIT  = {50};
static const unsigned char CAN_CALL_THREAD_INIT = {1};
static const uint32_t MIN_EVENT_SLEEP_TIME_INIT = {5};
static const uint32_t CALL_THREAD_PRIORITY_INIT = {2};
static const unsigned char HARDWARE             = {0};
static const char CFG_FILE[]                    = "";
static const char SECTION[]                     = "Default";
static const char LOG_FILE_INIT[]               = "";
static const uint32_t LOG_FLAGS_INIT            = {0xC0000063};  // LOG_MESSAGE, LOG_STATUS, LOG_ERROR, LOG_WARN,
                                                                 // LOG_WITH_TIME, LOG_DISABLE_SYNC
static const uint32_t AUTO_OPEN_RESET_INIT      = {0};
static const uint8_t COMM_TRY_COUNT_INIT        = {3};

static const unsigned char AUTO_CONNECT_INIT   = {0};
static const unsigned char AUTO_REOPEN_INIT    = {0};
static const uint16_t FIFO_OV_MODE_INIT         = {0};

#ifdef HAVE_TX_ACK_BYPASS
static const uint8_t TX_ACK_BYPASS_INIT = {0};
#endif

static const struct TValueDescription MainValues[] = {
  // Name                  | Alias | Type     | Flags |MaxSize| Access               | Default
  // *** Initialisierungsvariablen
  {"MainThreadPriority",    NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_INIT,   &MAIN_THREAD_PRIORITY_INIT},
  {"TimeStampMode",         NULL,   VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_INIT,   &TIME_STAMP_MODE_INIT},
  {"CanRxDFifoSize",        NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_INIT,   &CAN_TXD_FIFO_SIZE_INIT},
  {"CanTxDFifoSize",        NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_INIT,   &CAN_RXD_FIFO_SIZE_INIT},
  {"ExecuteCommandTimeout", NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,    &EXECUTE_COMMAND_TIMEOUT_INIT},
  {"LowPollIntervall",      NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,    &LOW_POLL_INTERVALL_INIT},
  {"IdlePollIntervall",     NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,    &IDLE_POLL_INTERVALL_INIT},
  {"HighPollIntervall",     NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,    &HIGH_POLL_INTERVALL_INIT},
  {"FilterReadIntervall",   NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,    &FILTER_READ_INTERVALL_INIT},
  {"TxCanFifoEventLimit",   NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,    &TX_CAN_FIFO_EVENT_LIMIT_INIT},
  {"CanRxDMode",            NULL,   VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_INIT,   &CAN_RXD_MODE_INIT},
  {"CanRxDBufferSize",      NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_INIT,   &CAN_RXD_BUFFER_SIZE_INIT},
  {"CanCallThread",         NULL,   VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_INIT,   &CAN_CALL_THREAD_INIT},
  {"CallThreadPriority",    NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_INIT,   &CALL_THREAD_PRIORITY_INIT},
  {"MinEventSleepTime",     NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_PUBLIC, &MIN_EVENT_SLEEP_TIME_INIT},
  {"Hardware",              NULL,   VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_INIT,   &HARDWARE},
  {"CfgFile",               NULL,   VT_STRING,    0,      0,   M_RD_ALL | M_WR_INIT,   &CFG_FILE},
  {"Section",               NULL,   VT_STRING,    0,      0,   M_RD_ALL | M_WR_INIT,   &SECTION},
  {"LogFile",               NULL,   VT_STRING,    0,      0,   M_RD_ALL | M_WR_INIT,   &LOG_FILE_INIT},
  {"LogFlags",              NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_INIT,   &LOG_FLAGS_INIT},
  {"AutoOpenReset",         NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_PUBLIC, &AUTO_OPEN_RESET_INIT},
#ifdef HAVE_TX_ACK_BYPASS
  {"TxAckBypass",           NULL,   VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_PUBLIC, &TX_ACK_BYPASS_INIT},
#endif
  // *** Option Variablen
  {"AutoConnect",           NULL,   VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,  &AUTO_CONNECT_INIT},
  {"AutoReopen",            NULL,   VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,  &AUTO_REOPEN_INIT},
  {"FifoOvMode",            NULL,   VT_UWORD,     0,      0,   M_RD_ALL | M_WR_INIT, &FIFO_OV_MODE_INIT},
  {"CommTryCount",          NULL,   VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,  &COMM_TRY_COUNT_INIT},
  {NULL,                    NULL,   0,            0,      0,   0,                    NULL}};


static const uint32_t CanSetModeTab[] = {
   MCMD_CAN_ONLY_CMD,           // 0 = OP_CAN_NO_CHANGE
   MCMD_CAN_START,              // 1 = OP_CAN_START
   MCMD_CAN_STOP,               // 2 = OP_CAN_STOP
   MCMD_CAN_RESET,              // 3 = OP_CAN_RESET
   MCMD_CAN_START_LOM,          // 4 = OP_CAN_START_LOM
   MCMD_CAN_START_NO_RETRANS,   // 5 = OP_CAN_START_NO_RETRANS
   MCMD_CAN_ECU_FLASH_MODE};    // 6 = OP_CAN_ECU_FLASH_MODE


/***************************************************************/
/*  Treiber Deinitialisieren                                   */
/***************************************************************/
void CALLBACK_TYPE CanDownDriver(void)
{
CanDownDriverInternel();
}


void CanDownDriverInternel(void)
{
uint32_t i;
TCanDevice *dev;
//void *mem;
TMemList *mem_list;
#if defined(ENABLE_LOG_SUPPORT) && defined(MEM_DEBUG)
char *adr;
const char *file_name;
int32_t line;
#endif

for (i = 0; i < 16; i++)
  {
  if ((dev = DeviceList[i]))
    MhsEventListDestroy(&dev->StatusEventList);
  }
mhs_user_event_set_all(MHS_TERMINATE);
#ifndef DISABLE_PNP_SUPPORT
PnPDestroy();
#endif
DestoryApiCp();
for (i = 0; i < 16; i++)
  {
  if ((dev = DeviceList[i]))
    (void)device_destroy(dev->Index, 1);
  }

can_core_destroy();
#ifdef ENABLE_LOG_SUPPORT
for (mem_list = UserMemList; mem_list; mem_list = mem_list->Next)
  {
  LogPrintf(LOG_API_CALL | LOG_ERROR, 
    "FEHLER [CanDownDriver]: User-Speicher nicht freigegeben (Call:%s Adr:0x%08X)",
    mem_list->Description, mem_list->Ref);
  }
#endif
// Alle User allocierten Daten freigeben
MemDestroy(&UserMemList, 1);
#if defined(ENABLE_LOG_SUPPORT) && defined(MEM_DEBUG)
while ((file_name = mhs_check_for_leaks(&line, &adr)))
  {
  LogPrintf(LOG_API_CALL | LOG_ERROR, 
    "FEHLER [CanDownDriver]: Speicher nicht freigegeben (File:%s Zeile:%u Adr:0x%08X)", file_name, line, adr);
  }
#endif
/*#ifdef ENABLE_CONFIG_FILE
CfgDestroy();
#endif*/
#ifdef ENABLE_LOG_SUPPORT
LogDestroy();
#endif
#if (defined(ENABLE_CONFIG_FILE)) || (defined(ENABLE_LOG_SUPPORT))
PathsDestroy();
#endif
if (DriverInit)
  index_destroy();
mhs_user_event_destroy();
DrvCoreDown();
safe_free(LocalDeviceList);
safe_free(LocalHwInfo);
safe_free(LocalDevicesInfo);

DriverInit = TCAN_DRV_NOT_INIT;
}


/***************************************************************/
/*  Treiber Initialisieren                                     */
/***************************************************************/
int32_t CALLBACK_TYPE CanInitDriver(char *options)
{
TCanDevice *dev;
TMhsObjContext *context;
TMhsObj *obj;
TMhsThread *thread;
uint32_t rx_fifo_size, tx_fifo_size, log_flags, call_prio, can_rxd_buffer_size, min_sleep_time;
uint8_t use_call_thread, can_rxd_mode;
char *log_file;
#ifdef ENABLE_LOG_SUPPORT
char *file_name;
#endif
char *str_list;
int32_t err;

if (DriverInit)
  return(0);
err = 0;
thread = NULL;
if (DrvCoreInit())
  return(-1); // <*>
DriverInit = TCAN_DRV_STD_INIT;
index_create();
#if (defined(ENABLE_CONFIG_FILE)) || (defined(ENABLE_LOG_SUPPORT))
if (PathsCreate())
  return(ERR_ALLOCATE_MEMORY);
#endif
#ifndef DISABLE_PNP_SUPPORT
if (PnPCreate() < 0)
  return(ERR_ALLOCATE_RESOURCES);
#endif
if (can_core_create() < 0)
  return(ERR_ALLOCATE_RESOURCES);
context = can_main_get_context();
if (mhs_values_create_from_list(context, MainValues) < 0)
  return(ERR_ALLOCATE_RESOURCES);
str_list = mhs_strdup(options);
/*#ifdef ENABLE_CONFIG_FILE
CfgInit();
#endif*/

#ifdef ENABLE_LOG_SUPPORT
if ((str_list) && (find_upc(str_list ,"DEBUG")))
  {
  file_name = create_file_name(LogPath, DEF_LOG_FILE_NAME);
  LogCreate(file_name, DEF_LOG_FLAGS);
  safe_free(file_name);
  }
#endif
(void)mhs_value_set_string_list(options, ACCESS_INIT, context);

#ifdef ENABLE_CONFIG_FILE
LoadConfig();
#endif

rx_fifo_size = mhs_value_get_as_ulong("CanRxDFifoSize", ACCESS_INIT, context);
tx_fifo_size = mhs_value_get_as_ulong("CanTxDFifoSize", ACCESS_INIT, context);
can_rxd_mode = mhs_value_get_as_ubyte("CanRxDMode", ACCESS_INIT, context);
can_rxd_buffer_size = mhs_value_get_as_ulong("CanRxDBufferSize", ACCESS_INIT, context);
use_call_thread = mhs_value_get_as_ubyte("CanCallThread", ACCESS_INIT, context);
call_prio = mhs_value_get_as_ulong("CallThreadPriority", ACCESS_INIT, context);
min_sleep_time = mhs_value_get_as_ulong("MinEventSleepTime", ACCESS_INIT, context);
log_file = mhs_value_get_as_string("LogFile", ACCESS_INIT, context);
log_flags = mhs_value_get_as_ulong("LogFlags", ACCESS_INIT, context);

#ifdef ENABLE_LOG_SUPPORT
if ((log_file) && (strlen(log_file)) && (!LogEnable))
  {
  file_name = create_file_name(LogPath, log_file);
  LogCreate(file_name, log_flags);
  safe_free(file_name);
  }
#endif
LOG_API_ENTER_STR_LIST("CanInitDriver", "Option-Variablen", options);
safe_free(str_list);

if (use_call_thread)
  thread = CreateApiCp(call_prio, min_sleep_time);
if (!(dev = CreateCanDevice()))
  return(ERR_ALLOCATE_RESOURCES);
(void)mhs_value_set_string_list(options, ACCESS_INIT, dev->Context);
dev->OvMode = mhs_value_get_as_uword("FifoOvMode", ACCESS_INIT, context);
// RxD Buffer anlegen
if (!err)
  {
  if ((can_rxd_mode) && (can_rxd_buffer_size))
    {
    dev->CanRxDBufferSize = can_rxd_buffer_size;
    dev->CanRxDBuffer = mhs_malloc(can_rxd_buffer_size * sizeof(struct TCanMsg));
    if (!dev->CanRxDBuffer)
      err = ERR_ALLOCATE_RESOURCES;
    }
  }
obj = (TMhsObj *)mhs_can_fifo_create(dev->Index, NULL, rx_fifo_size, 0);
mhs_can_fifo_create(dev->Index | INDEX_TXT_FLAG, NULL, tx_fifo_size, 0);
if (thread)
  (void)RunApiCp(thread, dev);
((TObjCan *)obj)->Channels = index_to_channels(dev->Index);
if (thread)
  {
#ifndef DISABLE_PNP_SUPPORT
  PnPEventSetup((TMhsEvent *)thread, EVENT_PNP_CHANGE);
#endif
  DevStatusEventSetup(dev, (TMhsEvent *)thread, EVENT_STATUS_CHANGE);
  (void)mhs_object_events_set(obj, (TMhsEvent *)thread, EVENT_RX_MESSAGES);
  }
LOG_API_EXIT("CanInitDriver", err);
#ifdef HAVE_TX_ACK_BYPASS
TxAckBypassInit();
#endif
#ifdef HAVE_CAN_COOL_CHECK
if (CheckExeFileSize() < 0)
  CanDownDriverInternel();
#endif
return(err);
}


/***************************************************************/
/*  Treiber Optionen einstellen                                */
/***************************************************************/
int32_t CALLBACK_TYPE CanSetOptions(char *options)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
LOG_API_ENTER_STR_LIST("CanSetOptions", "Option-Variablen", options);
if (!(dev = get_device_and_ref(0)))
  err = ERR_INDEX_NO_DEVICE;
else
  (void)mhs_value_set_string_list(options, ACCESS_PUBLIC, dev->Context);
device_unref(dev);
LOG_API_EXIT("CanSetOptions", err);
return(err);
}


/***************************************************************/
/*  Treiber und Device Informationen abfragen                  */
/***************************************************************/
char * CALLBACK_TYPE CanDrvInfo(void)
{
#ifdef HAVE_CAN_COOL_CHECK
if (CheckExeFile() < 0)
  return(NULL);
#endif
LOG_API_ENTER_STR_LIST("CanDrvInfo", "Driver Info:", (char *)DrvInfoStr);
return((char *)DrvInfoStr);
}


char * CALLBACK_TYPE CanDrvHwInfo(uint32_t index)
{
TCanDevice *dev;
int32_t err;
char *str;

str = NULL;
err = 0;
LOG_API_ENTER_IDX("CanDrvHwInfo", index);
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  str = dev->DrvInfoHwStr;
  LOG_API_STR_LIST("Hardware Info Variablen:", str);
  }
device_unref(dev);
LOG_API_EXIT("CanDrvHwInfo", err);
return(str);
}


/***************************************************************/
/*  Status auslesen                                            */
/***************************************************************/
int32_t CALLBACK_TYPE CanGetDeviceStatus(uint32_t index, struct TDeviceStatus *status)
{
TCanDevice *dev;
int32_t err;

err = 0;
#ifdef ENABLE_LOG_SUPPORT
if (LogEnable)
  LogPrintf(LOG_API_CALL_STATUS, "API-Call Enter: CanGetDeviceStatus, index: %#08X", index);
#endif
if (!(dev = get_device_and_ref(index)))
  {
  status->CanStatus = CAN_STATUS_UNBEKANNT;
  status->FifoStatus = FIFO_STATUS_UNBEKANNT;
  status->DrvStatus = DRV_STATUS_NOT_INIT;
  }
else
  {
  status->CanStatus = dev->DeviceStatus.CanStatus;
  status->FifoStatus = dev->DeviceStatus.FifoStatus;
  status->DrvStatus = dev->DeviceStatus.DrvStatus;
  }
device_unref(dev);
#ifdef ENABLE_LOG_SUPPORT
if (LogEnable)
  {
  if (err < 0)
    LogPrintf(LOG_API_CALL_STATUS | LOG_ERROR, "API-Call Exit: CanGetDeviceStatus, result: %s", GetErrorString(err));
  else
    LogPrintf(LOG_API_CALL_STATUS, "API-Call Exit: CanGetDeviceStatus, result: Ok");  
  }
#endif
return(err);
}


/***************************************************************/
/*  CAN Open                                                   */
/***************************************************************/
int32_t CALLBACK_TYPE CanDeviceOpen(uint32_t index, char *parameter)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT 
  LogStringList(LOG_API_CALL, parameter, "API-Call Enter: CanDeviceOpen, index: %#08X, Parameter-Liste:", index);  
#endif
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  (void)mhs_value_set_string_list(parameter, ACCESS_PORT_OPEN, dev->Context);
#ifdef HAVE_CAN_COOL_CHECK
  if (!(err = CheckPkgInstall()))
#endif
    err = OpenCanDevice(dev);
  device_unref(dev);
  }
LOG_API_EXIT("CanDeviceOpen", err);
return(err);
}


/***************************************************************/
/*  CAN Close                                                  */
/***************************************************************/
int32_t CALLBACK_TYPE CanDeviceClose(uint32_t index)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
LOG_API_ENTER_IDX("CanDeviceClose", index);
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  CloseCanDevice(dev);
  device_unref(dev);
  }
LOG_API_EXIT("CanDeviceClose", err);
return(err);
}


/***************************************************************/
/*  CAN Speed, CAN ACC Mask u. CAN ACC Code einstellen         */
/*  Funktion nicht mehr verwenden, nur noch aus gründen der    */
/*  Kompatibilität enthalten                                   */
/***************************************************************/
int32_t CALLBACK_TYPE CanApplaySettings(uint32_t index)
{
(void)index;

return(0);
}


/***************************************************************/
/*  Betriebsart einstellen                                     */
/***************************************************************/
int32_t CALLBACK_TYPE CanSetMode(uint32_t index, unsigned char can_op_mode, uint16_t can_command)
{
TCanDevice *dev;
struct TCmdSetModeParam param;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanSetMode, index: %#08X, can_op_mode: %#02X, can_command: %#04X",
      index, can_op_mode, can_command);
#endif
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  if (can_op_mode >= sizeof(CanSetModeTab))
    err = ERR_PARAM;
  else
    {
    param.Index = index;
    param.CanCommand = can_command;
    err = mhs_cmd_execute(dev, CanSetModeTab[can_op_mode], &param, sizeof(struct TCmdSetModeParam), -1);
    }
  device_unref(dev);
  }
LOG_API_EXIT("CanSetMode", err);
return(err);
}


/***************************************************************/
/*  Can Parameter setzen                                       */
/***************************************************************/
int32_t CALLBACK_TYPE CanSet(uint32_t index, uint16_t obj_index, uint16_t obj_sub_index,
  void *data, int32_t size)
{
(void)index;
(void)obj_index;
(void)obj_sub_index;
(void)data;
(void)size;

return(0);
}


int32_t CALLBACK_TYPE CanGet(uint32_t index, uint16_t obj_index, uint16_t obj_sub_index,
  void *data, int32_t size)
{
(void)index;
(void)obj_index;
(void)obj_sub_index;
(void)data;
(void)size;

return(0);
}


/***************************************************************/
/*  CAN Telegramm(e) senden                                    */
/***************************************************************/
int32_t CALLBACK_TYPE CanTransmit(uint32_t index, struct TCanMsg *msg, int32_t count)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
index |= INDEX_TXT_FLAG;
#ifdef ENABLE_LOG_SUPPORT
if (LogEnable)
  LogCanMessages(LOG_API_CALL_TX, msg, count, LOG_FLAG_TYPE,
     "API-Call Enter: CanTransmit, Index: %#08X, Messages: %d", index, count);
#endif
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  err = can_put_msgs_api(index, msg, count);
  device_unref(dev);
  }
#ifdef ENABLE_LOG_SUPPORT  
if (LogEnable)  
  {
  if (err < 0)
    LogPrintf(LOG_API_CALL_TX | LOG_ERROR, "API-Call Exit: CanTransmit, result: %s", GetErrorString(err));
  else
    LogPrintf(LOG_API_CALL_TX, "API-Call Exit: CanTransmit, result: Ok");  
  }
#endif  
LOG_API_EXIT("CanTransmit", err);
return(err);
}


/***************************************************************/
/*  CAN Sende Fifo lï¿½schen                                     */
/***************************************************************/
void CALLBACK_TYPE CanTransmitClear(uint32_t index)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return;
index |= INDEX_TXT_FLAG;
LOG_API_ENTER_IDX("CanTransmitClear", index);
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  err = can_clear_msgs(index);
  device_unref(dev);
  }
LOG_API_EXIT("CanTransmitClear", err);
}


/***************************************************************/
/*  Anzahl der Nachrichten im Sende-Fifo                       */
/***************************************************************/
uint32_t CALLBACK_TYPE CanTransmitGetCount(uint32_t index)
{
TCanDevice *dev;
int32_t res;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
res = 0;
index |= INDEX_TXT_FLAG;
#ifdef ENABLE_LOG_SUPPORT
if (LogEnable)
  LogPrintf(LOG_API_CALL_TX, "API-Call Enter: CanTransmitGetCount, index: %#08X", index);
#endif
if ((dev = get_device_and_ref(index)))
  {
  res = can_get_msg_count(index);
  device_unref(dev);
  }
#ifdef ENABLE_LOG_SUPPORT
if (LogEnable)
  {
  if (res < 0)
    LogPrintf(LOG_API_CALL_TX | LOG_ERROR, "API-Call Exit: CanTransmitGetCount, result: %s", GetErrorString(res));
  else
    LogPrintf(LOG_API_CALL_TX, "API-Call Exit: CanTransmitGetCount, result: Ok");  
  }
#endif
if (res < 0)
  return(0);
return(res);
}


/***************************************************************/
/*  Intervalltimer setzten, Puffer senden                      */
/***************************************************************/
int32_t CALLBACK_TYPE CanTransmitSet(uint32_t index, uint16_t cmd, uint32_t time)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
index |= INDEX_TXT_FLAG;
#ifdef ENABLE_LOG_SUPPORT
if (LogEnable)
  LogPrintf(LOG_API_CALL, "API-Call Enter: CanTransmitSet, index: %#08X, cmd: %#04X, %lu", index, cmd, time);
#endif
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  err = SetIntervallBox(dev, index, cmd, time);
  device_unref(dev);
  }
LOG_API_EXIT("CanTransmitSet", err);
return(err);
}


/***************************************************************/
/*  CAN Telegramm(e) empfangen                                 */
/***************************************************************/
int32_t CALLBACK_TYPE CanReceive(uint32_t index, struct TCanMsg *msg, int32_t count)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
if (LogEnable)
  LogPrintf(LOG_API_CALL_RX, "API-Call Enter: CanReceive, index: %#08X, count: %d", index, count);
 #endif
dev = get_device_and_ref(index);
err = can_get_msgs_api(index, msg, count);
device_unref(dev);
#ifdef ENABLE_LOG_SUPPORT
if (LogEnable)
  {
  if (err >= 0)     
    LogCanMessages(LOG_API_CALL_RX, msg, err, LOG_FMT_TYPE_SOURCE,
      "API-Call Exit: CanReceive, count: %d", err); 
  else
    LogPrintf(LOG_API_CALL_RX | LOG_ERROR, "API-Call Exit: CanReceive, result: %s", GetErrorString(err));
  }
#endif
return(err);
}


/***************************************************************/
/*  CAN Empfangs Fifo lï¿½schen                                  */
/***************************************************************/
void CALLBACK_TYPE CanReceiveClear(uint32_t index)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return;
err = 0;
LOG_API_ENTER_IDX("CanReceiveClear", index);
dev = get_device_and_ref(index);
err = can_clear_msgs(index);
device_unref(dev);
LOG_API_EXIT("CanReceiveClear", err);
}


/***************************************************************/
/*  Anzahl der Nachrichent im Empfangs-Fifo                    */
/***************************************************************/
uint32_t CALLBACK_TYPE CanReceiveGetCount(uint32_t index)
{
TCanDevice *dev;
int32_t res;

if (!DriverInit)
  return(0);
res = 0;
#ifdef ENABLE_LOG_SUPPORT
if (LogEnable)  
  LogPrintf(LOG_API_CALL_RX, "API-Call Enter: CanReceiveGetCount, index: %#08X", index);
#endif
dev = get_device_and_ref(index);
res = can_get_msg_count(index);
device_unref(dev);
#ifdef ENABLE_LOG_SUPPORT
if (LogEnable)
  {
  if (res < 0)
    LogPrintf(LOG_API_CALL_RX | LOG_ERROR, "API-Call Exit: CanReceiveGetCount, result: %s", GetErrorString(res));
  else
    LogPrintf(LOG_API_CALL_RX, "API-Call Exit: CanReceiveGetCount, count: %d", res);
  }
#endif  
if (res < 0)
  return(0);
return(res);
}


/***************************************************************/
/*                                                             */
/***************************************************************/
int32_t CALLBACK_TYPE CanSetSpeed(uint32_t index, uint16_t speed)
{
TCanDevice *dev;
int32_t err;
uint32_t channel;
char str[16];

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanSetSpeed, index: %#08X, speed: %u", index, speed);
#endif
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  channel = get_can_channel(index) + 1;
  safe_sprintf(str, 16, "CanSpeed%u", channel);
  err = mhs_value_set_as_uword(str, speed, ACCESS_PUBLIC, 0, dev->Context);
  device_unref(dev);
  }
LOG_API_EXIT("CanSetSpeed", err);
if (err > 0)
  err = 0;
return(err);
}


/***************************************************************/
/*                                                             */
/***************************************************************/
int32_t CALLBACK_TYPE CanSetSpeedUser(uint32_t index, uint32_t value)
{
TCanDevice *dev;
int32_t err;
uint32_t channel;
char str[16];

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanSetSpeedUser, index: %#08X, value: %#08X", index, value);
#endif
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  channel = get_can_channel(index) + 1;
  safe_sprintf(str, 15, "CanSpeed%uUser", channel);
  err = mhs_value_set_as_ulong(str, value, ACCESS_PUBLIC, 0, dev->Context);
  device_unref(dev);
  }
LOG_API_EXIT("CanSetSpeedUser", err);
if (err > 0)
  err = 0;
return(err);
}


int32_t CALLBACK_TYPE CanSetFilter(uint32_t index, struct TMsgFilter *msg_filter)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
LOG_API_ENTER_IDX("CanSetFilter", index);
dev = get_device_and_ref(index);
err = can_set_filter(index, msg_filter);
device_unref(dev);
LOG_API_EXIT("CanSetFilter", err);
return(err);
}
