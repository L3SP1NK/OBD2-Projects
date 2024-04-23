/***************************************************************************
                         api_ex.c  -  description
                             -------------------
    begin             : 03.05.2015
    last modified     : 05.05.2023
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
#include <string.h>
#include "errors.h"
#include "paths.h"
#include "can_types.h"
#ifndef DISABLE_PNP_SUPPORT
  #if defined(MHSIPCAN_DRV)
    #include "net_pnp.h"
  #elif defined(MHSPASSTHRU_DRV)
    #include "passthru_pnp.h"
  #else
    #include "usb_pnp.h"
    #include "usb_hlp.h"
  #endif
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
#include "api_cp.h"
#include "info.h"
#include "user_mem.h"
#include "mhs_user_event.h"
#ifdef HAVE_CAN_COOL_CHECK
  #include "can_cool_check.h"
#endif
#include "api_calls.h"
#include "api_ex.h"


/*
Initialisierungsvariablen

Bezeichner            | Beschreibung                                    | Initialisierung
======================+=================================================+==========================
FdMode                | 0 = Standart CAN Nachrichten verarbeiten        |
                      | 1 = Standart und CAN-FD Nachrichten verarbeiten |
----------------------+-------------------------------------------------+--------------------------
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
Hardware*             | Reserviert, sollte nicht gesetzt werden         | HARDWARE
----------------------+-------------------------------------------------+--------------------------
CfgFile*              | Config File Name                                |
----------------------+-------------------------------------------------+--------------------------
Section*              | Section Name im Cfg File                        |
----------------------+-------------------------------------------------+--------------------------
LogFile               | Log File Name                                   | LOG_FILE
----------------------+-------------------------------------------------+--------------------------
LogFlags              | Log Flags                                       | LOG_FLAGS
----------------------+-------------------------------------------------+--------------------------
FdMode                | 0 = Standart CAN Nachrichten verarbeiten        | FD_MODE_INIT
                      | 1 = Standart und CAN-FD Nachrichten verarbeiten |
----------------------+-------------------------------------------------+--------------------------
AutoOpenReset**       | 0 =                                                | AUTO_OPEN_RESET_INIT
                      | 1 =
----------------------+-------------------------------------------------+--------------------------
FifoOvMode**          | Bit                                             | FIFO_OV_MODE_INIT
                      |  0 -> 0 = CAN Hardware Auto FIFO Reset inaktiv  |
                      |       1 = CAN Hardware Auto FIFO Reset aktiv    |
                      |  8 -> 0 = API FIFO Auto Reset inaktiv           |
                      |       1 = API FIFO Auto Reset aktiv             |
                      | 15 -> 0 = OV CAN Frames erzeugen                |
                      |       1 = Disable OV CAN Frames                 |
======================+=================================================+==========================

* = gelöscht!

** = neu
*/

static const uint32_t MAIN_THREAD_PRIORITY_INIT = {4};
static const unsigned char TIME_STAMP_MODE_INIT = {0};
static const uint32_t CAN_TXD_FIFO_SIZE_INIT    = {2048};
static const uint32_t CAN_RXD_FIFO_SIZE_INIT    = {0};
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
static const char LOG_FILE_INIT[]               = "";
static const uint32_t LOG_FLAGS_INIT            = {0xC0000063};  // LOG_MESSAGE, LOG_STATUS, LOG_ERROR, LOG_WARN,
                                                                 // LOG_WITH_TIME, LOG_DISABLE_SYNC
static const uint8_t COMM_TRY_COUNT_INIT        = {3};
static const uint32_t AUTO_OPEN_RESET_INIT      = {0};
static const uint8_t FD_MODE_INIT               = {0};
static const uint16_t FIFO_OV_MODE_INIT          = {0};

#ifdef HAVE_TX_ACK_BYPASS
static const uint8_t TX_ACK_BYPASS_INIT = {0};
#endif

static const uint8_t CAN_TX_ACK_ENABLE_INIT = {0};
static const uint8_t CAN_ERR_MSGS_ENABLE_INIT = {0};
static const uint8_t AUTO_STOP_CAN_INIT = {0};


static const struct TValueDescription ExMainValues[] = {
// Name                    | Alias | Type     | Flags |MaxSize| Access               | Default
  // *** Initialisierungsvariablen
  {"FdMode",                NULL,   VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_INIT,   &FD_MODE_INIT},
  {"CanTxAckEnable",        NULL,   VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,    &CAN_TX_ACK_ENABLE_INIT},  // <*> neu
  {"CanErrorMsgsEnable",    NULL,   VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,    &CAN_ERR_MSGS_ENABLE_INIT}, // <*> neu
  {"AutoStopCan",           NULL,   VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,    &AUTO_STOP_CAN_INIT}, // <*> neu
  {"MainThreadPriority",    NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_INIT,   &MAIN_THREAD_PRIORITY_INIT},
  {"TimeStampMode",         NULL,   VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_INIT,   &TIME_STAMP_MODE_INIT},
  {"CanRxDFifoSize",        NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_INIT,   &CAN_RXD_FIFO_SIZE_INIT},
  {"CanTxDFifoSize",        NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_INIT,   &CAN_TXD_FIFO_SIZE_INIT},
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
  {"LogFile",               NULL,   VT_STRING,    0,      0,   M_RD_ALL | M_WR_INIT,   &LOG_FILE_INIT},
  {"LogFlags",              NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_INIT,   &LOG_FLAGS_INIT},
  {"AutoOpenReset",         NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_PUBLIC, &AUTO_OPEN_RESET_INIT},
  {"FifoOvMode",            NULL,   VT_UWORD,     0,      0,   M_RD_ALL | M_WR_INIT,   &FIFO_OV_MODE_INIT},
  {"CommTryCount",          NULL,   VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,    &COMM_TRY_COUNT_INIT},
#ifdef HAVE_TX_ACK_BYPASS
  {"TxAckBypass",           NULL,   VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_PUBLIC, &TX_ACK_BYPASS_INIT},
#endif
  {NULL,                    NULL,   0,            0,      0,   0,                      NULL}};


struct TCanDevicesList *LocalDeviceList = NULL;
static int32_t LocalDeviceListSize = 0;
static int32_t LocalDeviceListRead = 0;

struct TCanInfoVar *LocalHwInfo = NULL;
static int32_t LocalHwInfoSize = 0;
static int32_t LocalHwInfoRead = 0;

struct TCanInfoVarList *LocalDevicesInfo = NULL;
static uint32_t LocalDevicesInfoSize = 0;
static uint32_t LocalDevicesInfoIdx = 0;
static uint32_t LocalDevicesInfoDevIdx = 0;

// <*> neu
#ifdef HAVE_TX_ACK_BYPASS
static void TxAckBypassSet(TObjValue *obj, void *user_data)
{
(void)user_data;

if (!obj)
  return;
TxAckBypass = obj->Value.U8;
}


void TxAckBypassInit(void)
{
TMhsObjContext *context;
TMhsObj *obj;

context = can_main_get_context();
TxAckBypass = mhs_value_get_as_ubyte("TxAckBypass", ACCESS_INIT, context);
obj = mhs_object_get_by_name("TxAckBypass", context);
mhs_object_cmd_event_connect(obj, 1, (TMhsObjectCB)&TxAckBypassSet, NULL);
}


/*void TxAckBypassDown(void) <*>
{
TMhsObjContext *context;
TMhsObj *obj;

context = can_main_get_context();
obj = mhs_object_get_by_name("TxAckBypass", context);
mhs_object_cmd_event_disconnect(obj, (TMhsObjectCB)&TxAckBypassSet);
} */

#endif
// <*> neu ende


int32_t CALLBACK_TYPE CanExCreateDevice(uint32_t *index, char *options)
{
TCanDevice *dev;
TMhsObj *obj;
TMhsEvent *event_obj;
TMhsObjContext *context;
uint32_t idx, rx_fifo_size, tx_fifo_size;
int32_t err;
uint8_t channel, fd;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
if (DriverInit == TCAN_DRV_FD_INIT)
  fd = 1;
else
  fd = 0;
err = 0;
obj = NULL;
dev = NULL;  // BugFix für Visual Studio 2022
LOG_API_ENTER_STR_LIST("CanExCreateDevice", "Option-Variablen", options);
if (!index)
  err = ERR_PARAM;
if (!err)
  {
  if (!(dev = CreateCanDevice()))
    err = ERR_ALLOCATE_RESOURCES;
  }
context = can_main_get_context();
if (!err)
  {
  (void)mhs_value_set_string_list(options, ACCESS_INIT, dev->Context);
  dev->OvMode = mhs_value_get_as_uword("FifoOvMode", ACCESS_INIT, context);
  if (!(rx_fifo_size = mhs_value_get_as_ulong("CanRxDFifoSize", ACCESS_INIT, dev->Context)))
    rx_fifo_size = mhs_value_get_as_ulong("CanRxDFifoSize", ACCESS_INIT, context);
  if (!(tx_fifo_size = mhs_value_get_as_ulong("CanTxDFifoSize", ACCESS_INIT, dev->Context)))
    tx_fifo_size = mhs_value_get_as_ulong("CanTxDFifoSize", ACCESS_INIT, context);
  if (tx_fifo_size)
    {
    for (channel = 0; channel < MAX_CAN_CHANNELS; channel++)
      {
      idx = dev->Index | (channel << 16) | INDEX_TXT_FLAG;
      (void)mhs_can_fifo_create(idx, NULL, tx_fifo_size, fd);
      }
    (void)mhs_value_set_as_ulong("CanTxDFifoSize", tx_fifo_size, ACCESS_INIT, 0, dev->Context);
    }
  if (rx_fifo_size)
    {
    obj = (TMhsObj *)mhs_can_fifo_create(dev->Index, NULL, rx_fifo_size, fd);
    if (obj)
      {
      ((TObjCan *)obj)->Channels = index_to_channels(dev->Index);
      (void)mhs_value_set_as_ulong("CanRxDFifoSize", rx_fifo_size, ACCESS_INIT, 0, dev->Context);
      }
    }
  if ((event_obj = (TMhsEvent *)GetApiCpTheread()))
    {
    if (obj)
      (void)mhs_object_events_set(obj, event_obj, EVENT_RX_MESSAGES);
    DevStatusEventSetup(dev, event_obj, EVENT_STATUS_CHANGE);
    }
  idx = dev->Index;
  *index = idx;
  }
#ifdef ENABLE_LOG_SUPPORT
if (!err)
  LogPrintf(LOG_API_CALL, "API-Call Exit: CanExCreateDevice, index: 0x%08X", idx);
else
  LOG_API_EXIT("CanExCreateDevice", err);
#endif
if ((err) && (index))
  *index = INDEX_INVALID;
#ifdef HAVE_TX_ACK_BYPASS
TxAckBypassInit();
#endif
return(err);
}


int32_t CALLBACK_TYPE CanExDestroyDevice(uint32_t *index)
{
int32_t err;
uint32_t idx;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
if (index)
  {
  idx = *index;
  *index = INDEX_INVALID;
  }
else
  {
  idx = INDEX_INVALID;
  err = ERR_PARAM;
  }
LOG_API_ENTER_IDX("CanExDestroyDevice", idx);
if (!err)
  err = device_destroy(idx, 1);
LOG_API_EXIT("CanExDestroyDevice", err);
return(err);
}


int32_t CALLBACK_TYPE CanExCreateFifo(uint32_t index, uint32_t size, TMhsEvent *event_obj, uint32_t event, uint32_t channels)
{
TMhsObj *obj;
int32_t err;
uint8_t fd;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
if (DriverInit == TCAN_DRV_FD_INIT)
  fd = 1;
else
  fd = 0;
err = 0;
#ifdef ENABLE_LOG_SUPPORT

LogPrintf(LOG_API_CALL, "API-Call Enter: CanExCreateFifo, index: 0x%08X, size: %u, event: %u:", index, size, event);
#endif
if (!size)
  mhs_can_fifo_destry(index);
else
  {
  if (mhs_object_get_by_index(index, can_core_get_context()))
    err = -1;  // <*> ändern
  else
    {
    if (mhs_user_event_valid(event_obj))
      err = ERR_PARAM;
    else
      {
      if (!(obj = (TMhsObj *)mhs_can_fifo_create(index, NULL, size, fd)))
        err = ERR_ALLOCATE_RESOURCES;
      else
        {
        ((TObjCan *)obj)->Channels = channels;
        if ((event_obj) && (event))
          (void)mhs_object_events_set(obj, event_obj, event);
        else
          {
          if ((event_obj = (TMhsEvent *)GetApiCpTheread()))
            (void)mhs_object_events_set(obj, event_obj, EVENT_RX_MESSAGES);
          }
        }
      }
    }
  }
LOG_API_EXIT("CanExCreateFifo", err);
return(err);
}


int32_t CALLBACK_TYPE CanExBindFifo(uint32_t fifo_index, uint32_t device_index, uint32_t bind)
{
TMhsObj *obj;
TMhsObjContext *context;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExBindFifo, fifo_index: 0x%08X, device_index: 0x%08X, bind: %u:", fifo_index, device_index, bind);
#endif
context = can_core_get_context();
obj = mhs_object_get_by_index(fifo_index, context);
if (!obj)
  err = ERR_INDEX_RANGE;
else
  {
  if (obj->Class == OBJ_CAN_FIFO)
    {
    if (bind)
      ((TObjCan *)obj)->Channels |= index_to_channels(device_index);
    else
      ((TObjCan *)obj)->Channels &= ~index_to_channels(device_index);
    }
  }
LOG_API_EXIT("CanExBindFifo", err);
return(err);
}


TMhsEvent * CALLBACK_TYPE CanExCreateEvent(void)
{
TMhsEvent *event_obj;
int32_t err;

if (!DriverInit)
  return(NULL);
err = 0;
if (!(event_obj = mhs_user_event_create()))
  err = ERR_ALLOCATE_RESOURCES;
else
  mhs_event_set_event_mask(event_obj, 0xFFFFFFFF);
LOG_API("CanExCreateEvent", err);
return(event_obj);
}


int32_t CALLBACK_TYPE CanExSetObjEvent(uint32_t index, uint32_t source, TMhsEvent *event_obj, uint32_t event)
{
TCanDevice *dev;
TMhsObj *obj;
TMhsObjContext *context;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExSetObjEvent, index: 0x%08X, source: 0x%08X, event: %u:",
            index, source, event);
#endif
if (mhs_user_event_valid(event_obj))
  err = ERR_PARAM;
else
  {
  if ((source == MHS_EVS_STATUS) ||
     (source == MHS_EVS_DIN) ||
     (source == MHS_EVS_ENC) ||
     (source == MHS_EVS_KEY))
    {
    if (!(dev = get_device_and_ref(index)))
      err = ERR_INDEX_NO_DEVICE;
    else
      {
      context = dev->Context;
      switch (source)
        {
        case MHS_EVS_STATUS : {
                              DevStatusEventSetup(dev, event_obj, event);
                              break;
                              }
        case MHS_EVS_DIN    : {
                              obj = mhs_object_get_by_name("IoPort0In", context);
                              mhs_object_events_set(obj, event_obj, event);
                              obj = mhs_object_get_by_name("IoPort1In", context);
                              mhs_object_events_set(obj, event_obj, event);
                              obj = mhs_object_get_by_name("IoPort2In", context);
                              mhs_object_events_set(obj, event_obj, event);
                              obj = mhs_object_get_by_name("IoPort3In", context);
                              mhs_object_events_set(obj, event_obj, event);
                              break;
                              }
        case MHS_EVS_ENC    : {
                              obj = mhs_object_get_by_name("IoPort0Enc", context);
                              mhs_object_events_set(obj, event_obj, event);
                              obj = mhs_object_get_by_name("IoPort1Enc", context);
                              mhs_object_events_set(obj, event_obj, event);
                              obj = mhs_object_get_by_name("IoPort2Enc", context);
                              mhs_object_events_set(obj, event_obj, event);
                              obj = mhs_object_get_by_name("IoPort3Enc", context);
                              mhs_object_events_set(obj, event_obj, event);
                              break;
                              }
        case MHS_EVS_KEY    : {
                              obj = mhs_object_get_by_name("Keycode", context);
                              mhs_object_events_set(obj, event_obj, event);
                              break;
                              }
        }
      device_unref(dev);
      }
    }
  else if (source == MHS_EVS_PNP)
    {
#ifndef DISABLE_PNP_SUPPORT
    PnPEventSetup(event_obj, event);
#endif
    }
  else
    {
    context = can_get_context(index);
    obj = mhs_object_get_by_index(index, context);
    if (!obj)
      err = ERR_INDEX_RANGE;
    else
      {
      if ((event_obj) && (event))
        (void)mhs_object_events_set(obj, event_obj, event);
      else
        {
        if ((event_obj = (TMhsEvent *)GetApiCpTheread()))
          (void)mhs_object_events_set(obj, event_obj, EVENT_RX_MESSAGES);
        else
          (void)mhs_object_events_set(obj, NULL, 0);
        }
      }
    }
  }
LOG_API_EXIT("CanExSetObjEvent", err);
return(err);
}


void CALLBACK_TYPE CanExSetEvent(TMhsEvent *event_obj, uint32_t event)
{
if (!DriverInit)
  return;
if (mhs_user_event_valid(event_obj))
  {
#ifdef ENABLE_LOG_SUPPORT
  if (LogEnable)
    LogPrintf(LOG_API_CALL, "API-Call: CanExSetEvent event_obj invalid");
#endif
  return;
  }
mhs_event_set(event_obj, event);
#ifdef ENABLE_LOG_SUPPORT
if (LogEnable)
  LogPrintf(LOG_API_CALL, "API-Call: CanExSetEvent, event_obj: 0x%08X, event: 0x%08X", event_obj, event);
#endif
}


void CALLBACK_TYPE CanExSetEventAll(uint32_t event)
{
if (!DriverInit)
  return;
#ifdef ENABLE_LOG_SUPPORT
if (LogEnable)
  LogPrintf(LOG_API_CALL, "API-Call: CanExSetEventAll, event: 0x%08X", event);
#endif
mhs_user_event_set_all(event);
}


void CALLBACK_TYPE CanExResetEvent(TMhsEvent *event_obj, uint32_t event)
{
if (!DriverInit)
  return;
if (mhs_user_event_valid(event_obj))
  {
#ifdef ENABLE_LOG_SUPPORT
  if (LogEnable)
    LogPrintf(LOG_API_CALL, 0, "API-Call: CanExResetEvent event_obj invalid");
#endif
  return;
  }
mhs_event_clear(event_obj, event);
#ifdef ENABLE_LOG_SUPPORT
if (LogEnable)
  LogPrintf(LOG_API_CALL, "API-Call: CanExResetEvent, event_obj: 0x%08X, event: 0x%08X", event_obj, event);
#endif
}


uint32_t CALLBACK_TYPE CanExWaitForEvent(TMhsEvent *event_obj, uint32_t timeout)
{
uint32_t events;

if (!DriverInit)
  return(MHS_TERMINATE);
if (mhs_user_event_valid(event_obj))
  {
  events = MHS_TERMINATE;
#ifdef ENABLE_LOG_SUPPORT
if (LogEnable)
  LogPrintf(LOG_API_CALL, "API-Call: CanExWaitForEvent event_obj invalid");
#endif
  }
else
  {
#ifdef ENABLE_LOG_SUPPORT
  if (LogEnable)
    LogPrintf(LOG_API_CALL, "API-Call Enter: CanExWaitForEvent, event_obj: 0x%08X, timeout: %u", event_obj, timeout);
#endif
  events = mhs_wait_for_event(event_obj, timeout);
#ifdef ENABLE_LOG_SUPPORT
  if (LogEnable)
    LogPrintf(LOG_API_CALL, "API-Call Exit: CanExWaitForEvent, events: 0x%08X", events);
#endif
  }
return(events);
}


/***************************************************************/
/*  Treiber Initialisieren                                     */
/***************************************************************/
int32_t CALLBACK_TYPE CanExInitDriver(char *options)
{
TMhsObjContext *context;
#ifndef DISABLE_PNP_SUPPORT
TMhsThread *thread;
#endif
uint32_t log_flags, call_prio, can_rxd_buffer_size, min_sleep_time;
unsigned char use_call_thread, can_rxd_mode;
char *log_file;
#ifdef ENABLE_LOG_SUPPORT
char *file_name;
#endif
char *str_list;
int32_t err;

if (DriverInit)
  return(0);
err = 0;
if (DrvCoreInit())
  return(-1); // <*>
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
if (mhs_values_create_from_list(context, ExMainValues) < 0)
  return(ERR_ALLOCATE_RESOURCES);
str_list = mhs_strdup(options);
/*#ifdef ENABLE_CONFIG_FILE
CfgInit();
#endif*/

#ifdef ENABLE_LOG_SUPPORT
#ifndef DEBUG_LOG_ENABLE
if ((str_list) && (find_upc(str_list ,"DEBUG")))
  {
#endif
  file_name = create_file_name(LogPath, DEF_LOG_FILE_NAME);
  LogCreate(file_name, DEF_LOG_FLAGS);
  safe_free(file_name);
#ifndef DEBUG_LOG_ENABLE
  }
#endif
#endif
(void)mhs_value_set_string_list(options, ACCESS_INIT, context);
if (mhs_value_get_as_ubyte("FdMode", ACCESS_INIT, context))
  DriverInit = TCAN_DRV_FD_INIT;
else
  DriverInit = TCAN_DRV_EX_INIT;

#ifdef ENABLE_CONFIG_FILE
LoadConfig();
#endif

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
LOG_API_ENTER_STR_LIST("CanExInitDriver", "Option-Variablen", options);
safe_free(str_list);

// RxD Buffer anlegen
if (!err)
  {
  if ((can_rxd_mode) && (can_rxd_buffer_size))
    err = ApiCpCreateRxDBuffer(can_rxd_buffer_size);
  }
if (!err)
  {
#ifndef DISABLE_PNP_SUPPORT
  thread = NULL;
  if (use_call_thread)
    thread = ExCreateApiCp(call_prio, min_sleep_time);
  if (thread)
    PnPEventSetup((TMhsEvent *)thread, EVENT_PNP_CHANGE);
#else
  if (use_call_thread)
    (void)ExCreateApiCp(call_prio, min_sleep_time);
#endif
  }
LOG_API_EXIT("CanExInitDriver", err);
#ifdef HAVE_CAN_COOL_CHECK
if (CheckExeFileSize() < 0)
  CanDownDriverInternel();
#endif
return(err);
}


int32_t CALLBACK_TYPE CanExGetDeviceCount(int32_t flags)
{
int32_t res;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
res = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExGetDeviceCount, flags: 0x%08X", flags);
#endif
#ifndef DISABLE_PNP_SUPPORT
res = GetDeviceCount(flags);
#endif
LOG_API_EXIT_COUNT("CanExGetDeviceCount", res);
return(res);
}


int32_t CALLBACK_TYPE CanExGetDeviceList(struct TCanDevicesList **devices_list, int32_t flags)
{
int32_t res;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
res = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExGetDeviceList, flags: 0x%08X", flags);
#endif
#ifndef DISABLE_PNP_SUPPORT
res = GetDeviceList(devices_list, flags);
if (res > 0)
  (void)MemAddRef(&UserMemList, "CanExGetDeviceList", *devices_list);
#endif
#ifdef ENABLE_LOG_SUPPORT
if (res >= 0)
  LogPrintUsbDevices(LOG_API_CALL, *devices_list, res);
#endif
LOG_API_EXIT_COUNT("CanExGetDeviceList", res);
return(res);
}


int32_t CALLBACK_TYPE CanExGetDeviceListPerform(int32_t flags)
{
int32_t res;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
res = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExGetDeviceListPerform, flags: 0x%08X", flags);
#endif
#ifndef DISABLE_PNP_SUPPORT
safe_free(LocalDeviceList);
res = GetDeviceList(&LocalDeviceList, flags);
LocalDeviceListSize = res;
LocalDeviceListRead = 0;
#endif
#ifdef ENABLE_LOG_SUPPORT
if (res >= 0)
  LogPrintUsbDevices(LOG_API_CALL, LocalDeviceList, res);
#endif
LOG_API_EXIT_COUNT("CanExGetDeviceList", res);
return(res);
}


int32_t CALLBACK_TYPE CanExGetDeviceListGet(struct TCanDevicesList *item)
{
int32_t i;
struct TCanDevicesList *list_item;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
if (!item)
  return(ERR_PARAM);
list_item = NULL;
if ((LocalDeviceList) && (LocalDeviceListSize > 0))
  {
  if ((i = LocalDeviceListRead) < LocalDeviceListSize)
    {
    list_item = &LocalDeviceList[i];
    LocalDeviceListRead++;
    }
  }
if (list_item)
  {
#ifdef ENABLE_LOG_SUPPORT
  LogPrintf(LOG_API_CALL, "API-Call: CanExGetDeviceListGet, ReadIndex: %d", i);
#endif
  memcpy(item, list_item, sizeof(struct TCanDevicesList));
  return(1);
  }
else
  {
#ifdef ENABLE_LOG_SUPPORT
  LogPrintf(LOG_API_CALL, "API-Call: CanExGetDeviceListGet, End");
#endif
  return(0);
  }
}


// <*> ganz neu
int32_t CALLBACK_TYPE CanExGetInfoList(uint32_t dev_idx, const char *name, struct TCanInfoVarList **devices_info, int32_t flags)
{
int32_t res;
(void)dev_idx;
(void)name;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
res = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExGetInfoList, flags: 0x%08X", flags);
#endif
#ifndef DISABLE_PNP_SUPPORT
res = GetDevicesInfo(devices_info, flags);
if (res > 0)
  (void)MemAddRef(&UserMemList, "CanExGetInfoList", *devices_info);
#endif
#ifdef ENABLE_LOG_SUPPORT
/*if (res >= 0) <*>
  LogPrintDevicesInfo(LOG_API_CALL, 0, *devices_list, res);*/
#endif
LOG_API_EXIT_COUNT("CanExGetInfoList", res);
return(res);
}


int32_t CALLBACK_TYPE CanExGetInfoListPerform(uint32_t dev_idx,  const char *name, int32_t flags)
{
int32_t res;
(void)dev_idx;
(void)name;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
res = 0;
safe_free(LocalDevicesInfo);
LocalDevicesInfoSize = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExGetInfoListPerform, flags: 0x%08X", flags);
#endif
LocalDevicesInfoIdx = 0;
LocalDevicesInfoDevIdx = 0;
#ifndef DISABLE_PNP_SUPPORT
if ((res = GetDevicesInfo(&LocalDevicesInfo, flags)) > 0)
  LocalDevicesInfoSize = res;
#endif
return(res);
}


int32_t CALLBACK_TYPE CanExGetInfoListGet(uint32_t list_idx, struct TCanInfoVar *item)
{
int32_t i, size;
struct TCanInfoVar *devices_info, *list_item;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
if (!item)
  return(ERR_PARAM);
list_item = NULL;
if ((LocalDevicesInfo) && (LocalDevicesInfoSize > list_idx))
  {
  size = (int32_t)LocalDevicesInfo[list_idx].Size;
  devices_info = LocalDevicesInfo[list_idx].List;
  if (LocalDevicesInfoDevIdx != list_idx)
    {
    LocalDevicesInfoDevIdx = list_idx;
    LocalDevicesInfoIdx = 0;
    }
  if ((i = LocalDevicesInfoIdx) < size)
    {
    list_item = &devices_info[i];
    LocalDevicesInfoIdx++;
    }
  }
if (list_item)
  {
#ifdef ENABLE_LOG_SUPPORT
  LogPrintf(LOG_API_CALL, "API-Call: CanExGetInfoListGet, ReadIndex: %d", i);
#endif
  memcpy(item, list_item, sizeof(struct TCanInfoVar));
  return(1);
  }
else
  {
#ifdef ENABLE_LOG_SUPPORT
  LogPrintf(LOG_API_CALL, "API-Call: CanExGetInfoListGet, End");
#endif
  return(0);
  }
}

// <*> ganz neu, ende


int32_t CALLBACK_TYPE CanExGetDeviceInfo(uint32_t index, struct TCanDeviceInfo *device_info,
  struct TCanInfoVar **hw_info, uint32_t *hw_info_size)
{
TCanDevice *dev;
int32_t res;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
res = 0;
LOG_API_ENTER_IDX("CanExGetDeviceInfo", index);
if (!(dev = get_device_and_ref(index)))
  res = ERR_INDEX_NO_DEVICE;
else
  {
  if (device_info)
    res = GetDeviceInfo(dev, device_info);
#ifndef DISABLE_HARDWARE_DEVICE_INFO
  if ((res >= 0) && (hw_info) && (hw_info_size))
    {
    *hw_info_size = 0;
    if ((res = GetHardwareInfo(dev, hw_info)) > 0)
      {
      (void)MemAddRef(&UserMemList, "CanExGetDeviceInfo", *hw_info);
      *hw_info_size = (uint32_t)res;
      res = 0;
      }
    }
#endif
  device_unref(dev);
  }
#ifdef ENABLE_LOG_SUPPORT
if (res >= 0)
  {
  if (device_info)
    LogPrintDeviceInfo(LOG_API_CALL, device_info);
  if ((hw_info) && (hw_info_size))
    LogPrintHwInfo(LOG_API_CALL, *hw_info, *hw_info_size);
  }
#endif
LOG_API_EXIT_COUNT("CanExGetDeviceList", res);
return(res);
}


int32_t CALLBACK_TYPE CanExGetDeviceInfoPerform(uint32_t index, struct TCanDeviceInfo *device_info)
{
TCanDevice *dev;
int32_t res;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
res = 0;
safe_free(LocalHwInfo);
LocalHwInfoRead = 0;
LocalHwInfoSize = 0;
LOG_API_ENTER_IDX("CanExGetDeviceInfo", index);
if (!(dev = get_device_and_ref(index)))
  res = ERR_INDEX_NO_DEVICE;
else
  {
  if (device_info)
    res = GetDeviceInfo(dev, device_info);
#ifndef DISABLE_HARDWARE_DEVICE_INFO
  if (res >= 0)
    {
    if ((res = GetHardwareInfo(dev, &LocalHwInfo)) > 0)
      LocalHwInfoSize = res;
    }
#endif
  device_unref(dev);
  }
return(res);
}


int32_t CALLBACK_TYPE CanExGetDeviceInfoGet(struct TCanInfoVar *item)
{
int32_t i;
struct TCanInfoVar *list_item;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
if (!item)
  return(ERR_PARAM);
list_item = NULL;
if ((LocalHwInfo) && (LocalHwInfoSize > 0))
  {
  if ((i = LocalHwInfoRead) < LocalHwInfoSize)
    {
    list_item = &LocalHwInfo[i];
    LocalHwInfoRead++;
    }
  }
if (list_item)
  {
#ifdef ENABLE_LOG_SUPPORT
  LogPrintf(LOG_API_CALL, "API-Call: CanExGetDeviceInfoGet, ReadIndex: %d", i);
#endif
  memcpy(item, list_item, sizeof(struct TCanInfoVar));
  return(1);
  }
else
  {
#ifdef ENABLE_LOG_SUPPORT
  LogPrintf(LOG_API_CALL, "API-Call: CanExGetDeviceInfoGet, End");
#endif
  return(0);
  }
}


void CALLBACK_TYPE CanExDataFree(void **data)
{
int32_t err;
void *mem;
TMemList *mem_list;

if (!DriverInit)
  return;
LOG_API_ENTER("CanExDataFree");
err = 0;
if (!data) // Alle User allocierten Daten freigeben
  MemDestroy(&UserMemList, 1);
else
  {
  mem = *data;
  *data = NULL;
  if (!(mem_list = GetEntryByRef(UserMemList, mem)))
    err = ERR_USER_ALLOC_MEM_NOT_FOUND;
  else
    MemDestroyEntry(&UserMemList, mem_list, 1);
  }
LOG_API_EXIT("CanExDataFree", err);
}


/***************************************************************/
/*  Treiber Optionen einstellen                                */
/***************************************************************/
int32_t CALLBACK_TYPE CanExSetOptions(uint32_t index, const char *options)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
LOG_API_ENTER_STR_LIST("CanSetOptions", "Option-Variablen", options);
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  (void)mhs_value_set_string_list(options, ACCESS_PUBLIC, dev->Context);
  device_unref(dev);
  }
LOG_API_EXIT("CanSetOptions", err);
return(err);
}


/***************************************************************/
/*  Eine "Byte Variable (8 Bit)" setzen                        */
/***************************************************************/
int32_t CALLBACK_TYPE CanExSetAsByte(uint32_t index, const char *name, char value)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExSetAsByte, index: 0x%08X, name: %s, value: %u", index, name, value);
#endif
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  err = mhs_value_set_as_byte(name, value, ACCESS_PUBLIC, 0, dev->Context);
  device_unref(dev);
  }
LOG_API_EXIT("CanExSetAsByte", err);
if (err > 0)
  err = 0;
return(err);
}


/***************************************************************/
/*  Eine "Word Variable (16 Bit)" setzen                       */
/***************************************************************/
int32_t CALLBACK_TYPE CanExSetAsWord(uint32_t index, const char *name, int16_t value)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExSetAsWord, index: 0x%08X, name: %s, value: %u", index, name, value);
#endif
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  err = mhs_value_set_as_word(name, value, ACCESS_PUBLIC, 0, dev->Context);
  device_unref(dev);
  }
LOG_API_EXIT("CanExSetAsWord", err);
if (err > 0)
  err = 0;
return(err);
}


/***************************************************************/
/*  Eine "Long Variable (32 Bit)" setzen                       */
/***************************************************************/
int32_t CALLBACK_TYPE CanExSetAsLong(uint32_t index, const char *name, int32_t value)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExSetAsLong, index: 0x%08X, name: %s, value: %u", index, name, value);
#endif
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  err = mhs_value_set_as_long(name, value, ACCESS_PUBLIC, 0, dev->Context);
  device_unref(dev);
  }
LOG_API_EXIT("CanExSetAsLong", err);
if (err > 0)
  err = 0;
return(err);
}


/***************************************************************/
/*  Eine "Unsigned Byte Variable (8 Bit)" setzen               */
/***************************************************************/
int32_t CALLBACK_TYPE CanExSetAsUByte(uint32_t index, const char *name, unsigned char value)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExSetAsUByte, index: 0x%08X, name: %s, value: %u", index, name, value);
#endif
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  err = mhs_value_set_as_ubyte(name, value, ACCESS_PUBLIC, 0, dev->Context);
  device_unref(dev);
  }
LOG_API_EXIT("CanExSetAsUByte", err);
if (err > 0)
  err = 0;
return(err);
}


/***************************************************************/
/*  Eine "Unsigned Word Variable (16 Bit)" setzen              */
/***************************************************************/
int32_t CALLBACK_TYPE CanExSetAsUWord(uint32_t index, const char *name, uint16_t value)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExSetAsUWord, index: 0x%08X, name: %s, value: %u", index, name, value);
#endif
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  err = mhs_value_set_as_uword(name, value, ACCESS_PUBLIC, 0, dev->Context);
  device_unref(dev);
  }
LOG_API_EXIT("CanExSetAsUWord", err);
if (err > 0)
  err = 0;
return(err);
}


/***************************************************************/
/*  Eine "Unsigned Long Variable (32 Bit)" setzen              */
/***************************************************************/
int32_t CALLBACK_TYPE CanExSetAsULong(uint32_t index, const char *name, uint32_t value)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExSetAsULong, index: 0x%08X, name: %s, value: %u", index, name, value);
#endif
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  err = mhs_value_set_as_ulong(name, value, ACCESS_PUBLIC, 0, dev->Context);
  device_unref(dev);
  }
LOG_API_EXIT("CanExSetAsULong", err);
if (err > 0)
  err = 0;
return(err);
}


/***************************************************************/
/*  Eine "String Variable" setzen                              */
/***************************************************************/
int32_t CALLBACK_TYPE CanExSetAsString(uint32_t index, const char *name, char *value)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExSetAsString, index: 0x%08X, name: %s, value: %s", index, name, value);
#endif
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  err = mhs_value_set_as_string(name, value, ACCESS_PUBLIC, 0, dev->Context);
  device_unref(dev);
  }
LOG_API_EXIT("CanExSetAsString", err);
if (err > 0)
  err = 0;
return(err);
}


/***************************************************************/
/*  Eine "Byte Variable (8 Bit)" abfragen                      */
/***************************************************************/
int32_t CALLBACK_TYPE CanExGetAsByte(uint32_t index, const char *name, char *value)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExGetAsByte, index: 0x%08X, name: %s", index, name);
#endif
if (!value)
  err = ERR_PARAM;
else
  {
  if (!(dev = get_device_and_ref(index)))
    err = ERR_INDEX_NO_DEVICE;
  else
    {
    *value = mhs_value_get_as_byte(name, ACCESS_PUBLIC, dev->Context);
    device_unref(dev);
    }
  }
LOG_API_EXIT_VALUE("CanExGetAsByte", err, value);
if (err > 0)
  err = 0;
return(err);
}


/***************************************************************/
/*  Eine "Word Variable (16 Bit)" abfragen                     */
/***************************************************************/
int32_t CALLBACK_TYPE CanExGetAsWord(uint32_t index, const char *name, int16_t *value)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExGetAsWord, index: 0x%08X, name: %s", index, name);
#endif
if (!value)
  err = ERR_PARAM;
else
  {
  if (!(dev = get_device_and_ref(index)))
    err = ERR_INDEX_NO_DEVICE;
  else
    {
    *value = mhs_value_get_as_word(name, ACCESS_PUBLIC, dev->Context);
    device_unref(dev);
    }
  }
LOG_API_EXIT_VALUE("CanExGetAsWord", err, value);
if (err > 0)
  err = 0;
return(err);
}


/***************************************************************/
/*  Eine "Long Variable (32 Bit)" abfragen                     */
/***************************************************************/
int32_t CALLBACK_TYPE CanExGetAsLong(uint32_t index, const char *name, int32_t *value)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExGetAsLong, index: 0x%08X, name: %s", index, name);
#endif
if (!value)
  err = ERR_PARAM;
else
  {
  if (!(dev = get_device_and_ref(index)))
    err = ERR_INDEX_NO_DEVICE;
  else
    {
    *value = mhs_value_get_as_long(name, ACCESS_PUBLIC, dev->Context);
    device_unref(dev);
    }
  }
LOG_API_EXIT_VALUE("CanExGetAsLong", err, value);
if (err > 0)
  err = 0;
return(err);
}


/***************************************************************/
/*  Eine "Unsigned Byte Variable (8 Bit)" abfragen             */
/***************************************************************/
int32_t CALLBACK_TYPE CanExGetAsUByte(uint32_t index, const char *name, unsigned char *value)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExGetAsUByte, index: 0x%08X, name: %s", index, name);
#endif
if (!value)
  err = ERR_PARAM;
else
  {
  if (!(dev = get_device_and_ref(index)))
    err = ERR_INDEX_NO_DEVICE;
  else
    {
    *value = mhs_value_get_as_ubyte(name, ACCESS_PUBLIC, dev->Context);
    device_unref(dev);
    }
  }
LOG_API_EXIT_VALUE("CanExGetAsUByte", err, value);
if (err > 0)
  err = 0;
return(err);
}


/***************************************************************/
/*  Eine "Unsigned Word Variable (16 Bit)" abfragen            */
/***************************************************************/
int32_t CALLBACK_TYPE CanExGetAsUWord(uint32_t index, const char *name, uint16_t *value)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExGetAsUWord, index: 0x%08X, name: %s", index, name);
#endif
if (!value)
  err = ERR_PARAM;
else
  {
  if (!(dev = get_device_and_ref(index)))
    err = ERR_INDEX_NO_DEVICE;
  else
    {
    *value = mhs_value_get_as_uword(name, ACCESS_PUBLIC, dev->Context);
    device_unref(dev);
    }
  }
LOG_API_EXIT_VALUE("CanExGetAsUWord", err, value);
if (err > 0)
  err = 0;
return(err);
}


/***************************************************************/
/*  Eine "Unsigned Long Variable (32 Bit)" abfragen            */
/***************************************************************/
int32_t CALLBACK_TYPE CanExGetAsULong(uint32_t index, const char *name, uint32_t *value)
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExGetAsULong, index: 0x%08X, name: %s", index, name);
#endif
if (!value)
  err = ERR_PARAM;
else
  {
  if (!(dev = get_device_and_ref(index)))
    err = ERR_INDEX_NO_DEVICE;
  else
    {
    err = mhs_value_get_as_ulong_ex(name, ACCESS_PUBLIC, dev->Context, value);
    //*value = mhs_value_get_as_ulong(name, ACCESS_PUBLIC, dev->Context);
    device_unref(dev);
    }
  }
LOG_API_EXIT_VALUE("CanExGetAsULong", err, value);
/*if (err > 0)
  err = 0; */
return(err);
}


/***************************************************************/
/*  Eine "String Variable" abfragen                            */
/***************************************************************/
int32_t CALLBACK_TYPE CanExGetAsString(uint32_t index, const char *name, char **str)
{
TCanDevice *dev;
int32_t err;
char *s;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
s = NULL;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExGetAsString, index: 0x%08X, name: %s", index, name);
#endif
if (!str)
  err = ERR_PARAM;
else
  {
  *str = NULL;
  if (!(dev = get_device_and_ref(index)))
    err = ERR_INDEX_NO_DEVICE;
  else
    {
    s = mhs_value_get_as_string_dup(name, ACCESS_PUBLIC, dev->Context);
    (void)MemAddRef(&UserMemList, "CanExGetAsString", s);
    *str = s;
    device_unref(dev);
    }
  }
LOG_API_EXIT_STR_VALUE("CanExGetAsString", err, s);
if (err > 0)
  err = 0;
return(err);
}


/***************************************************************/
/*  Eine "String Variable" in Ziel kopieren                    */
/***************************************************************/
int32_t CALLBACK_TYPE CanExGetAsStringCopy(uint32_t index, const char *name, char *dest, uint32_t *dest_size)
{
TCanDevice *dev;
int32_t err;
uint32_t size, max;
char *s;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
s = NULL;
size = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExGetAsStringCopy, index: 0x%08X, name: %s", index, name);
#endif
if (!dest_size)
  err = ERR_PARAM;
else
  {
  if (!(dev = get_device_and_ref(index)))
    err = ERR_INDEX_NO_DEVICE;
  else
    {
    if ((s = mhs_value_get_as_string(name, ACCESS_PUBLIC, dev->Context)))
      size = ((uint32_t)strlen(s)) + 1;
    else
      size = 1;
    if (dest)
      {
      max = *dest_size;
      if (size > max)
        size = max;
      if (s)
        memcpy(dest, s, size);
      dest[size-1] = '\0';
      }
    *dest_size = size;
    device_unref(dev);
    }
  }
if (dest)
  LOG_API_EXIT_STR_VALUE("CanExGetAsStringCopy", err, s);
else
  LOG_API_EXIT_VALUE("CanExGetAsStringCopy", err, size);
if (err > 0)
  err = 0;
return(err);
}


/***************************************************************/
/*  Eine "Unsigned Byte Variable (8 Bit)" setzen               */
/***************************************************************/
int32_t CALLBACK_TYPE CanExSetAsData(uint32_t index, const char *name, void *data, uint32_t size)  // <*> neu
{
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExSetAsData, index: 0x%08X, name: %s, size: %u", index, name, size);
#endif
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  err = mhs_value_set_as_ubyte_array(name, (uint8_t *)data, size, ACCESS_PUBLIC, 0, dev->Context);
  device_unref(dev);
  }
LOG_API_EXIT("CanExSetAsData", err);
if (err > 0)
  err = 0;
return(err);
}


/***************************************************************/
/*  Eine "Unsigned Byte Variable (8 Bit)" setzen               */
/***************************************************************/
int32_t CALLBACK_TYPE CanExSetDataPtr(uint32_t index, const char *name, void *data_ptr, uint32_t size)  // <*> neu
{
char *size_name;
TCanDevice *dev;
int32_t err;

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExSetDataPtr, index: 0x%08X, name: %s, size: %u", index, name, size);
#endif
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
  err = mhs_value_set_as_ptr(name, NULL, ACCESS_PUBLIC, 0, dev->Context);
  if (!err)
    {
    size_name = mhs_strconcat(name, "Size", NULL);
    if (!(err = mhs_value_set_as_ulong(size_name, size, ACCESS_PUBLIC, 0, dev->Context)))
      (void)mhs_value_set_as_ptr(name, data_ptr, ACCESS_PUBLIC, 0, dev->Context);
    safe_free(size_name);
    }
  device_unref(dev);
  }
LOG_API_EXIT("CanExSetDataPtr", err);
if (err > 0)
  err = 0;
return(err);
}

// <*> neu
int32_t CALLBACK_TYPE CanExSetDataBlockCB(uint32_t index, TCanGetDataBlockCB func, void *user_data)
{
TCanDevice *dev;
int32_t err;
#ifndef HAVE_CAN_DATA_BUF
(void)func;
(void)user_data;
#endif

if (!DriverInit)
  return(ERR_DRIVER_NOT_INIT);
err = 0;
#ifdef ENABLE_LOG_SUPPORT
LogPrintf(LOG_API_CALL, "API-Call Enter: CanExSetDataBlockCB, index: 0x%08X", index);
#endif
if (!(dev = get_device_and_ref(index)))
  err = ERR_INDEX_NO_DEVICE;
else
  {
#ifdef HAVE_CAN_DATA_BUF
  mhs_value_set_as_ptr("CanDataBuf", NULL, ACCESS_PUBLIC, 0, dev->Context);
  mhs_value_set_as_ulong("CanDataBufSize", 0, ACCESS_PUBLIC, 0, dev->Context);
  dev->GetDataBlockCb = func;
  dev->GetDataBlockUserData = user_data;

  dev->CanDataBuf = NULL;
  dev->CanDataBufSize = 0;
#endif
  device_unref(dev);
  }
LOG_API_EXIT("CanExSetDataBlockCB", err);
return(err);
}
