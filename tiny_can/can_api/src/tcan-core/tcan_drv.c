/***************************************************************************
                        tcan_drv.c  -  description
                             -------------------
    begin             : 01.11.2010
    last modify       : 05.05.2023
    copyright         : (C) 2010 - 2023 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include <stdlib.h>
#include "util.h"
#include "info.h"
#include "read_info.h"
#include "errors.h"
#include "log.h"
#include "can_core.h"
#include "can_fifo.h"
#include "can_puffer.h"
#include "mhs_thread.h"
#include "mhs_cmd.h"
#include "mhs_obj.h"
#include "var_obj.h"
#include "index.h"
#ifndef DISABLE_PNP_SUPPORT
  #include "usb_hlp.h"
  #include "usb_pnp.h"
#endif  
#include "global.h"
#include "api_cp.h"
#include "tcan_setup.h"
#include "mhs_event_list.h"

#include "tar_drv.h"
#include "tcan_com.h"

#include "tar_drv2.h"
#include "tcan_com2.h"
#include "ecu_flash.h"  // <*> neu
#include "can_test.h"
#ifdef ENABLE_RS485_SUPPORT // <*> neu
  #include "rs485.h"
#endif
//#include "tcan_drv.h" <*>


#ifdef HAVE_TX_ACK_BYPASS
uint8_t TxAckBypass; // <*> neu
#endif

const struct TCanModulDesc ExCanModulTable[] = 
  //   ID    | USB-Description |BaudRate |BaudRate |Flags|St. |HW-T |Tx |Err |Features|CAN |Dig|Fifo|Fifo |Filter|Puffer|CAN-Clock|Tx
  //         |                 |   A     |   B     |     |Size|Stamp|ACK|Msgs| Flags  |Ch. |I/O|Size|Limit|Count |Count   (MHz)   |MTU
{{0x43414E00, "Tiny-CAN I-XL",  {3000000, 0},       0,      1,   0,   1,  0,   0x0005,  1,   0,  72,  36,    4,     4,      16    ,124, TINY_CAN_TYPE_I_XL  },  // 1 Tiny-CAN I-XL
 {0x43414E42, "Tiny-CAN II-XL", {3000000, 0},       0,      1,   0,   1,  0,   0x0005,  1,   0,  72,  36,    4,     4,      16    ,124, TINY_CAN_TYPE_II_XL },  // 2 Tiny-CAN II-XL
 {0x43414E43, "Tiny-CAN III-XL",{3000000, 0},       0,      1,   1,   1,  0,   0x8007,  1,   0,  72,  36,    8,    16,      16    ,124, TINY_CAN_TYPE_III_XL},  // 3 Tiny-CAN III-XL
 {0x43414E04, "Tiny-CAN M1",    {3000000, 0},       0,      1,   0,   1,  0,   0x0005,  1,   0,  72,  36,    4,     4,      16    ,124, TINY_CAN_TYPE_M1    },  // 4 Tiny-CAN M1
 {0x43414E05, NULL,             {921600,  0},       0,      1,   0,   1,  0,   0x0005,  1,   0,  36,  18,    8,     4,      16    ,124, TINY_CAN_TYPE_M232  },  // 5 Tiny-CAN M232
 {0x43414E06, "Tiny-CAN IV-XL", {8000000, 6000000}, 1,      1,   1,   1,  1,   0x800F,  1,   0,  72,  36,    4,     8,      16    ,251, TINY_CAN_TYPE_IV_XL },  // 6 Tiny-CAN IV-XL
 {0x43414E07, "Tiny-CAN LS",    {3000000, 0},       0,      2,   1,   1,  1,   0x800F,  1,   0,  72,  36,    4,     4,      16    ,251, TINY_CAN_TYPE_LS    },  // 7 Tiny-CAN LS
 {0x43414E08, "Tiny-CAN M2",    {3000000, 0},       0,      1,   0,   1,  0,   0x8005,  1,   32, 72,  36,    4,     4,      16    ,251, TINY_CAN_TYPE_M2    },  // 8 Tiny-CAN M2
 {0x43414E09, "Tiny-CAN M3",    {8000000, 0},       0,      1,   1,   1,  1,   0x800F,  2,   46, 72,  36,    4,     8,      16    ,508, TINY_CAN_TYPE_M3    },  // 9 Tiny-CAN M3
 {0x43414E10, "Tiny-CAN V-XL", {12000000, 0},       0,      1,   1,   1,  1,   0x800F,  1,   0,  72,  36,    4,     4,      16    ,508, TINY_CAN_TYPE_V_XL  },  // 6 Tiny-CAN V-XL
 {0x00000000, NULL,             {0,       0},       0,      0,   0,   0,  0,   0x0000,  0,   0,   0,   0,    0,     0,      0     ,0,   0}}; // Ende Tabelle

/*
Initialisierungsvariablen

Bezeichner            | Beschreibung                                    | Initialisierung
======================+=================================================+==========================
MainThreadPriority    | 0 = THREAD_PRIORITY_NORMAL                      | MAIN_THREAD_PRIORITY_INIT
                      | 1 = THREAD_PRIORITY_ABOVE_NORMAL                |
                      | 2 = THREAD_PRIORITY_HIGHEST                     |
                      | 3 = THREAD_PRIORITY_TIME_CRITICAL               |
                      | 4 = THREAD_PRIORITY_REALTIME                    |
----------------------+-------------------------------------------------+--------------------------
TimeStampMode         | 0 = Disabled                                    | TIME_STAMP_MODE_INIT
                      | 1 = Software Time Stamps                        |
                      | 2 = Hardware Time Stamps, UNIX-Format           |
                      | 3 = Hardware Time Stamps                        |
                      | 4 = Hardware Time-Stamps verwenden wenn         |
                      |     verfügbar, ansonsten Software Time-Stamps   |
                      | Achtung: Ist der Bus gestartet (RUN) wird die   |
                      | Einstellung erst nach erneuten start wirksam    |
----------------------+-------------------------------------------------+--------------------------
CanRxDFifoSize        | Größe des Empfangsfifos in Messages             | CAN_RXD_FIFO_SIZE
----------------------+-------------------------------------------------+--------------------------
CanTxDFifoSize        | Größe des Sendefifos in Messages                | CAN_TXD_FIFO_SIZE
======================+=================================================+==========================
*/

static const uint32_t MAIN_THREAD_PRIORITY_INIT = {4};
static const uint8_t TIME_STAMP_MODE_INIT = {0};
static const uint32_t CAN_TXD_FIFO_SIZE_INIT    = {0};
static const uint32_t CAN_RXD_FIFO_SIZE_INIT    = {0};

/*
Port Open Variablen

Bezeichner            | Beschreibung                                    | Initialisierung
======================+=================================================+==========================
ComDrvType            | 0 = RS232 Schnittstelle                         | COM_DRV_TYPE_INIT
                      | 1 = USB (FTDI)                                  |
----------------------+-------------------------------------------------+--------------------------
Port                  | Serielle Schnittstelle 1 = COM1 - 4 = COM 4     | PORT_INIT
----------------------+-------------------------------------------------+--------------------------
ComDeviceName         | Device Name (Linux z.b. /dev/ttyS0              | COM_DEVICE_NAME_INIT
----------------------+-------------------------------------------------+--------------------------
BaudRate              | Baudrate, z.B. 34800 = 34800 Baud               | PORT_BAUD_INIT
----------------------+-------------------------------------------------+--------------------------
VendorId              | USB-Produkt Id                                  | VENDOR_ID_INIT
----------------------+-------------------------------------------------+--------------------------
ProductId             | USB-Vendor Id                                   | PRODUCT_ID_INIT
----------------------+-------------------------------------------------+--------------------------
Snr                   | Seriennummer CAN-Device                         | SNR_INIT
======================+=================================================+==========================
*/

static const int32_t COM_DRV_TYPE_INIT = {1};
static const uint16_t PORT_INIT = {1};             // COM 1
static const char COM_DEVICE_NAME_INIT[] = {""};
static const uint32_t PORT_BAUD_INIT = {0};
static const int32_t VENDOR_ID_INIT = {0};         // {0x0403};
static const int32_t PRODUCT_ID_INIT = {0};        // {0x6001};
static const char SNR_INIT[] = {""};
static const uint32_t AUTO_OPEN_RESET_INIT = {0};

/*
Option Variablen

======================+=================================================+==========================
CanTxAckEnable        | CAN TxD ACK Enablen                             | CAN_TX_ACK_ENABLE_INIT
----------------------+-------------------------------------------------+--------------------------
CanErrorMsgsEnable    | CAN Fehler Nachrichten in FIFO schreiben        | CAN_ERR_MSGS_ENABLE_INIT
----------------------+-------------------------------------------------+--------------------------
CanSpeed1             | CAN Übertragungsgeschwindigkeit in kBit/s       | CAN_SPEED_INIT
                      | z.B. 100 = 100kBit/s, 1000 = 1MBit/s            |
----------------------+-------------------------------------------------+--------------------------
CanSpeed1User         |
----------------------+-------------------------------------------------+--------------------------
ExecuteCommandTimeout | Wartezeit für Kommando ausführung in ms         | EXECUTE_COMMAND_TIMEOUT_INIT
----------------------+-------------------------------------------------+--------------------------
LowPollIntervall      | Hardware Polling Intervall in ms                | LOW_POLL_INTERVALL_INIT
----------------------+-------------------------------------------------+--------------------------
IdlePollIntervall     | Hardware Polling Intervall in ms                | IDLE_POLL_INTERVALL_INIT
----------------------+-------------------------------------------------+--------------------------
HighPollIntervall     | Hardware Polling Intervall in ms CAN Bus run    | HIGH_POLL_INTERVALL_INIT
----------------------+-------------------------------------------------+--------------------------
FilterReadIntervall   | Filter Messages alle x ms einlesen              | FILTER_READ_INTERVALL_INIT
======================+=================================================+==========================
*/

static const uint16_t CAN_SPEED_INIT     = {125};               // 125 kBit/s
static const uint32_t CAN_SPEED_USER_INIT = {0};
static const uint32_t EXECUTE_COMMAND_TIMEOUT_INIT = {8000};    // 8 Sekunden max. warten
static const uint32_t LOW_POLL_INTERVALL_INIT      = {250};
static const uint32_t IDLE_POLL_INTERVALL_INIT     = {0};
static const uint32_t HIGH_POLL_INTERVALL_INIT     = {0};
static const uint32_t FILTER_READ_INTERVALL_INIT   = {1000};
static const uint32_t TX_CAN_FIFO_EVENT_LIMIT_INIT = {0};
static const uint8_t CAN_TX_ACK_ENABLE_INIT = {0};
static const uint8_t CAN_ERR_MSGS_ENABLE_INIT = {0};
static const uint8_t AUTO_STOP_CAN_INIT = {0};
static const uint8_t COMM_TRY_COUNT_INIT = {3};

static const uint32_t IO_CFG_INIT = {0};
static const uint16_t IO_PORT_IN_INIT = {0};
static const uint16_t IO_PORT_OUT_INIT = {0};
static const uint16_t WORD_NULL_INIT = {0};
static const int32_t LONG_NULL_INIT = {0};
static const uint8_t BYTE_NULL_INIT = {0};

// CAN-FD
static const uint16_t CAN_D_SPEED_INIT = {0};          
static const uint32_t CAN_D_SPEED_USER_INIT = {0};
static const uint8_t CAN_FD_FLAGS_INIT = {0x80};

#define RS232_DEF_BAUDRATE 921600

static const struct TValueDescription DeviceValues[] = {
// Name                    | Alias          | Type     | Flags |MaxSize| Access             | Default
  // *** Initialisierungsvariablen
  {"MainThreadPriority",    NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_INIT, &MAIN_THREAD_PRIORITY_INIT},
  {"TimeStampMode",         NULL,            VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_INIT, &TIME_STAMP_MODE_INIT},
  {"CanRxDFifoSize",        NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_INIT, &CAN_TXD_FIFO_SIZE_INIT},
  {"CanTxDFifoSize",        NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_INIT, &CAN_RXD_FIFO_SIZE_INIT},
  // *** Port Open Variablen                                    
  {"ComDrvType",            NULL,            VT_LONG,      0,      0,   M_RD_ALL | M_WR_INIT, &COM_DRV_TYPE_INIT},
  {"Port",                  NULL,            VT_LONG,      0,      0,   M_RD_ALL | M_WR_INIT, &PORT_INIT},
  {"DeviceName",            NULL,            VT_STRING,    0,      0,   M_RD_ALL | M_WR_INIT, &COM_DEVICE_NAME_INIT},
  {"BaudRate",              NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_INIT, &PORT_BAUD_INIT},
  {"VendorId",              NULL,            VT_LONG,      0,      0,   M_RD_ALL | M_WR_INIT, &VENDOR_ID_INIT},
  {"ProductId",             NULL,            VT_LONG,      0,      0,   M_RD_ALL | M_WR_INIT, &PRODUCT_ID_INIT},
  {"Snr",                   NULL,            VT_STRING,    0,      0,   M_RD_ALL | M_WR_INIT, &SNR_INIT},
  {"AutoOpenReset",         NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_INIT, &AUTO_OPEN_RESET_INIT},
  // *** I / O                                                  
  {"IoCfg0",                NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &IO_CFG_INIT},
  {"IoCfg1",                NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &IO_CFG_INIT},
  {"IoCfg2",                NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &IO_CFG_INIT},
  {"IoCfg3",                NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &IO_CFG_INIT},
  {"IoCfg4",                NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &IO_CFG_INIT},
  {"IoCfg5",                NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &IO_CFG_INIT},
  {"IoCfg6",                NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &IO_CFG_INIT},
  {"IoCfg7",                NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &IO_CFG_INIT},
  {"IoPort0In",             NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &IO_PORT_IN_INIT},
  {"IoPort1In",             NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &IO_PORT_IN_INIT},
  {"IoPort2In",             NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &IO_PORT_IN_INIT},
  {"IoPort3In",             NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &IO_PORT_IN_INIT},
  {"IoPort0Out",            NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &IO_PORT_OUT_INIT},
  {"IoPort1Out",            NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &IO_PORT_OUT_INIT},
  {"IoPort2Out",            NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &IO_PORT_OUT_INIT},
  {"IoPort3Out",            NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &IO_PORT_OUT_INIT},
  {"IoPort0Analog",         NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &WORD_NULL_INIT},
  {"IoPort1Analog",         NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &WORD_NULL_INIT},
  {"IoPort2Analog",         NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &WORD_NULL_INIT},
  {"IoPort3Analog",         NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &WORD_NULL_INIT},
  {"IoPort4Analog",         NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &WORD_NULL_INIT},
  {"IoPort5Analog",         NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &WORD_NULL_INIT},
  {"IoPort6Analog",         NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &WORD_NULL_INIT},
  {"IoPort7Analog",         NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &WORD_NULL_INIT},
  {"IoPort0AnalogOut",      NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &WORD_NULL_INIT},
  {"IoPort1AnalogOut",      NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &WORD_NULL_INIT},
  {"IoPort0Enc",            NULL,            VT_LONG,      0,      0,   M_RD_ALL | M_WR_ALL,  &LONG_NULL_INIT},
  {"IoPort1Enc",            NULL,            VT_LONG,      0,      0,   M_RD_ALL | M_WR_ALL,  &LONG_NULL_INIT},
  {"IoPort2Enc",            NULL,            VT_LONG,      0,      0,   M_RD_ALL | M_WR_ALL,  &LONG_NULL_INIT},
  {"IoPort3Enc",            NULL,            VT_LONG,      0,      0,   M_RD_ALL | M_WR_ALL,  &LONG_NULL_INIT},
  {"Keycode",               NULL,            VT_ULONG_FIFO,0,     255,  M_RD_ALL | M_WR_ALL,  &LONG_NULL_INIT},
  // *** Option Variablen
  {"CanSpeed1",             "CanSpeed",      VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_SPEED_INIT},
  {"CanSpeed1User",         "CanSpeedUser",  VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_SPEED_USER_INIT},
  {"CanSpeed2",             NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_SPEED_INIT},
  {"CanSpeed2User",         NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_SPEED_USER_INIT},
  {"CanSpeed3",             NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_SPEED_INIT},
  {"CanSpeed3User",         NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_SPEED_USER_INIT},
  {"CanSpeed4",             NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_SPEED_INIT},
  {"CanSpeed4User",         NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_SPEED_USER_INIT},
  // CAN-FD                                                         
  {"CanDSpeed1",            "CanDSpeed",     VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_D_SPEED_INIT},
  {"CanDSpeed1User",        "CanDSpeedUser", VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_D_SPEED_USER_INIT},
  {"CanDSpeed2",            NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_D_SPEED_INIT},
  {"CanDSpeed2User",        NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_D_SPEED_USER_INIT},
  {"CanDSpeed3",            NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_D_SPEED_INIT},
  {"CanDSpeed3User",        NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_D_SPEED_USER_INIT},
  {"CanDSpeed4",            NULL,            VT_UWORD,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_D_SPEED_INIT},
  {"CanDSpeed4User",        NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_D_SPEED_USER_INIT},
  {"CanClockIndex",         NULL,            VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,  &BYTE_NULL_INIT},  // <*> neu  
  {"CanFdFlags1",           "CanFdFlags",    VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_FD_FLAGS_INIT},
  {"CanFdFlags2",           NULL,            VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_FD_FLAGS_INIT},
  {"CanFdFlags3",           NULL,            VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_FD_FLAGS_INIT},
  {"CanFdFlags4",           NULL,            VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_FD_FLAGS_INIT},        
                                                                        
  {"Can1TxAckEnable",       "CanTxAckEnable",  VT_UBYTE,   0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_TX_ACK_ENABLE_INIT},
  {"Can1ErrorMsgsEnable",   "CanErrorMsgsEnable", VT_UBYTE,0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_ERR_MSGS_ENABLE_INIT},
  {"Can2TxAckEnable",       NULL,            VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_TX_ACK_ENABLE_INIT},
  {"Can2ErrorMsgsEnable",   NULL,            VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_ERR_MSGS_ENABLE_INIT},
  {"Can3TxAckEnable",       NULL,            VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_TX_ACK_ENABLE_INIT},
  {"Can3ErrorMsgsEnable",   NULL,            VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_ERR_MSGS_ENABLE_INIT},
  {"Can4TxAckEnable",       NULL,            VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_TX_ACK_ENABLE_INIT},
  {"Can4ErrorMsgsEnable",   NULL,            VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,  &CAN_ERR_MSGS_ENABLE_INIT},
  {"ExecuteCommandTimeout", NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &EXECUTE_COMMAND_TIMEOUT_INIT},
  {"LowPollIntervall",      NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &LOW_POLL_INTERVALL_INIT},
  {"IdlePollIntervall",     NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &IDLE_POLL_INTERVALL_INIT},
  {"HighPollIntervall",     NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &HIGH_POLL_INTERVALL_INIT},
  {"FilterReadIntervall",   NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &FILTER_READ_INTERVALL_INIT},
  {"TxCanFifoEventLimit",   NULL,            VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,  &TX_CAN_FIFO_EVENT_LIMIT_INIT},
  {"AutoStopCan",           NULL,            VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,  &AUTO_STOP_CAN_INIT},
  {"CommTryCount",          NULL,            VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,  &COMM_TRY_COUNT_INIT},
  // <*> neu ECU Flash
  {"CanDataBuf",            NULL,            VT_POINTER,    0,      0,   M_RD_ALL | M_WR_ALL,  NULL},
  {"CanDataBufSize",        NULL,            VT_ULONG,      0,      0,   M_RD_ALL | M_WR_ALL,  NULL},
  {"IsoTpFlags",            NULL,            VT_UBYTE,      0,      0,   M_RD_ALL | M_WR_ALL,  NULL},          
  {"IsoTpTxPadContent",     NULL,            VT_UBYTE,      0,      0,   M_RD_ALL | M_WR_ALL,  NULL},     
  {"IsoTpTxId",             NULL,            VT_ULONG,      0,      0,   M_RD_ALL | M_WR_ALL,  NULL},             
  {"IsoTpRxId",             NULL,            VT_ULONG,      0,      0,   M_RD_ALL | M_WR_ALL,  NULL},
  {"IsoTpBlockSize",        NULL,            VT_ULONG,      0,      0,   M_RD_ALL | M_WR_ALL,  NULL},
  {"CanDataOkDlc",          NULL,            VT_UBYTE,      0,      0,   M_RD_ALL | M_WR_ALL,  NULL},
  {"CanDataErrorDlc",       NULL,            VT_UBYTE,      0,      0,   M_RD_ALL | M_WR_ALL,  NULL},               
  {"CanDataOkMask",         NULL,            VT_UBYTE_ARRAY,0,      8,   M_RD_ALL | M_WR_ALL,  NULL},       
  {"CanDataOkCode",         NULL,            VT_UBYTE_ARRAY,0,      8,   M_RD_ALL | M_WR_ALL,  NULL},       
  {"CanDataErrorMask",      NULL,            VT_UBYTE_ARRAY,0,      8,   M_RD_ALL | M_WR_ALL,  NULL},    
  {"CanDataErrorCode",      NULL,            VT_UBYTE_ARRAY,0,      8,   M_RD_ALL | M_WR_ALL,  NULL},
  {"CanDataBufWritten",     NULL,            VT_ULONG,      0,      0,   M_RD_ALL,             NULL},
  {"CanDataStatus",         NULL,            VT_LONG,       0,      0,   M_RD_ALL,             NULL},
  // <*> neu CanTest
  {"CanTestMode",           NULL,            VT_UBYTE,      0,      0,   M_RD_ALL | M_WR_ALL,  NULL},
  {"CanTestFillChar",       NULL,            VT_UBYTE,      0,      0,   M_RD_ALL | M_WR_ALL,  NULL},
  {"CanTestMsgFlags",       NULL,            VT_UBYTE,      0,      0,   M_RD_ALL | M_WR_ALL,  NULL},
  {"CanTestMsgLength",      NULL,            VT_UBYTE,      0,      0,   M_RD_ALL | M_WR_ALL,  NULL},
  {"CanTestMsgId",          NULL,            VT_ULONG,      0,      0,   M_RD_ALL | M_WR_ALL,  NULL},
  {"CanTestCounter",        NULL,            VT_ULONG,      0,      0,   M_RD_ALL | M_WR_ALL,  NULL},
  {"CanTestDelay",          NULL,            VT_ULONG,      0,      0,   M_RD_ALL | M_WR_ALL,  NULL},  
  {NULL,                    NULL,            0,             0,      0,   0,                    NULL}};


static const int32_t DefTinyCanType        = 0;

static const struct TValueDescription IoValues[] = {
// Name            | Alias | Type      | Flags | MaxSize | Access | Default
  {"TinyCanType",    NULL,   VT_ULONG,      0,      0,    0xFFFF,   &DefTinyCanType},
  {NULL,             NULL,    0,            0,      0,    0,        NULL}};

static void MainUpdateStatusProc(TCanDevice *dev);

static void MainThreadExecute(TMhsThread *thread);


static void FreeModulDesc(struct TCanModulDesc **desc)
{
struct TCanModulDesc *d;

if (!desc)
  return;
if (!(d = *desc))
  return;
safe_free(d->Description);
mhs_free(d);
*desc = NULL;
}


static struct TCanModulDesc *CopyModulDesc(const struct TCanModulDesc *src_desc)
{
struct TCanModulDesc *desc;

if (!(desc = (struct TCanModulDesc *)mhs_calloc(1, sizeof(struct TCanModulDesc))))
  return(NULL);
memcpy(desc, src_desc, sizeof(struct TCanModulDesc));
desc->Description = mhs_strdup(src_desc->Description);
return(desc);
}


/*******************************************************************************/
/*                      Treiber Initialisierung                                */
/*******************************************************************************/
int32_t DrvCoreInit(void)
{
return(0);
}


/*******************************************************************************/
/*                            Treiber Down                                     */
/*******************************************************************************/
void DrvCoreDown(void)
{
ComIoDrvDestroy();
}


/*******************************************************************************/
/*                           CAN Device erzeugen                               */
/*******************************************************************************/
TCanDevice *CreateCanDevice(void)
{
int32_t err;
TCanDevice *dev;
TMhsObjContext *context;
TMhsObj *obj;
uint8_t channel;
char str[26];
#if (TX_PUFFER_MAX > 0) || (RX_FILTER_MAX > 0)
uint32_t i, channel_idx;
TObjCanPuffer *puffer;
#endif
#if RX_FILTER_MAX > 0
TMhsThread *thread;
#endif

err = 0;
if (!(dev = device_create()))
  return(NULL);
dev->MainThread = mhs_create_thread(MainThreadExecute, dev, 0, 0);
mhs_event_set_event_mask((TMhsEvent *)dev->MainThread, MHS_ALL_EVENTS);
dev->StatusEventList = MhsEventListCreate();
#if TX_PUFFER_MAX > 0
// **** Intervall Boxen anlegen
context = mhs_obj_context_new("", (TMhsEvent *)dev->MainThread);
dev->TxPufferContext = context;
for (channel = 0; channel < MAX_CAN_CHANNELS; channel++)
  {  
  channel_idx = (channel << 16) | dev->Index | INDEX_TXT_FLAG;
  for (i = 1; i <= TX_PUFFER_MAX; i++)
    {
    if (!(puffer = mhs_can_puffer_create(OBJ_CAN_TX_PUFFER, i | channel_idx, NULL, context)))
      {
      err = -1;
      break;
      }
    mhs_can_puffer_set_intervall(puffer, 0);
    }
  }
#endif
#if RX_FILTER_MAX > 0
context = mhs_obj_context_new("", (TMhsEvent *)dev->MainThread);
dev->RxFilterContext = context;
for (channel = 0; channel < MAX_CAN_CHANNELS; channel++)
  {
  channel_idx = (channel << 16) | dev->Index;
  for (i = 1; i <= RX_FILTER_MAX; i++)
    {
    if (!(puffer = mhs_can_puffer_create(OBJ_CAN_RX_PUFFER, i | channel_idx, NULL, context)))
      {
      err = -1;
      break;
      }
    if ((thread = GetApiCpTheread()))
      (void)mhs_object_events_set((TMhsObj *)puffer, (TMhsEvent *)thread, EVENT_RX_FILTER_MESSAGES);
    }
  }
#endif
if (err >= 0)
  {
  context = mhs_obj_context_new("", NULL);
  dev->IoContext = context;
  err = mhs_values_create_from_list(context, IoValues);
  }
if (err >= 0)
  {
  context = mhs_obj_context_new("", (TMhsEvent *)dev->MainThread);
  dev->Context = context;
  err = mhs_values_create_from_list(context, DeviceValues);  
  dev->Cmd = mhs_cmd_create();
  }  
#ifdef ENABLE_RS485_SUPPORT // <*> neu  
if (err >= 0)
  err = RS485Create(dev);
#endif
if (err >= 0)
  {
  /*for (channel = 0; channel < MAX_CAN_CHANNELS; channel++) <*>
    {
    safe_sprintf(str, 25, "CanSpeed%u", channel + 1);
    obj = mhs_object_get_by_name(str, context);
    mhs_object_set_user_data(obj, NULL, channel | (CAN_VAR_SPEED << 8) | CAN_VAR_OBJ);

    safe_sprintf(str, 25, "CanSpeed%uUser", channel + 1);
    obj = mhs_object_get_by_name(str, context);
    mhs_object_set_user_data(obj, NULL, channel | (CAN_VAR_SPEED_BTR << 8) | CAN_VAR_OBJ);
    
    // CAN-FD
    safe_sprintf(str, 25, "CanDSpeed%u", channel + 1);
    obj = mhs_object_get_by_name(str, context);
    mhs_object_set_user_data(obj, NULL, channel | (IVAR_D_SPEED << 8) | IVAR_OBJ); 

    safe_sprintf(str, 25, "CanDSpeed%uUser", channel + 1);
    obj = mhs_object_get_by_name(str, context);
    mhs_object_set_user_data(obj, NULL, channel | (CAN_VAR_SPEED_DBTR << 8) | CAN_VAR_OBJ);
    
    safe_sprintf(str, 25, "CanFdFlags%u", channel + 1);
    obj = mhs_object_get_by_name(str, context);
    mhs_object_set_user_data(obj, NULL, channel | (CAN_VAR_FD_FLAGS << 8) | CAN_VAR_OBJ);

    safe_sprintf(str, 25, "Can%uTxAckEnable", channel + 1);
    obj = mhs_object_get_by_name(str, context);
    mhs_object_set_user_data(obj, NULL, channel | (CAN_VAR_TX_ACK_ENABLE << 8) | CAN_VAR_OBJ);

    safe_sprintf(str, 25, "Can%uErrorMsgsEnable", channel + 1);
    obj = mhs_object_get_by_name(str, context);
    mhs_object_set_user_data(obj, NULL, channel | (CAN_VAR_ERROR_LOG_ENABLE << 8) | CAN_VAR_OBJ);
    } */

  obj = mhs_object_get_by_name("TimeStampMode", context);
  mhs_object_set_user_data(obj, NULL, (CAN_VAR_TIMESTAMP_ENABLE << 8) | CAN_VAR_OBJ);
  for (channel = 0; channel < MAX_IO_CFG; channel++)
    {
    safe_sprintf(str, 25, "IoCfg%u", channel);
    obj = mhs_object_get_by_name(str, context);
    mhs_object_set_user_data(obj, NULL, channel | IO_VAR_CFG);
    }
  for (channel = 0; channel < (MAX_IO_CFG/2); channel++)
    {
    safe_sprintf(str, 25, "IoPort%uOut", channel);
    obj = mhs_object_get_by_name(str, context);
    mhs_object_set_user_data(obj, NULL, channel | IO_VAR_PORT_OUT);
    }
  for (channel = 0; channel < 4; channel++)
    {
    safe_sprintf(str, 25, "IoPort%uAnalogOut", channel);
    obj = mhs_object_get_by_name(str, context);
    mhs_object_set_user_data(obj, NULL, channel | IO_VAR_PORT_ANALOG_OUT);
    }
  obj = mhs_object_get_by_name("FilterReadIntervall", context);
  mhs_object_set_user_data(obj, NULL, FILTER_READ_INTERVALL);
  obj = mhs_object_get_by_name("HighPollIntervall", context);
  mhs_object_set_user_data(obj, NULL, HIGH_POLL_INTERVALL);
  obj = mhs_object_get_by_name("IdlePollIntervall", context);
  mhs_object_set_user_data(obj, NULL, IDLE_POLL_INTERVALL);
  obj = mhs_object_get_by_name("TxCanFifoEventLimit", context);
  mhs_object_set_user_data(obj, NULL, TX_CAN_FIFO_EVENT_LIMIT);
  obj = mhs_object_get_by_name("CommTryCount", context);
  mhs_object_set_user_data(obj, NULL, COMM_TRY_COUNT);  
  EcuFlashCreateDevice(dev); // <*> neu 
  }
if (err < 0)
  (void)device_destroy(dev->Index, 1);
else
  dev->DeviceStatus.DrvStatus = DRV_STATUS_INIT;
return(dev);
}


/*******************************************************************************/
/*                           CAN Device löschen                                */
/*******************************************************************************/
void DestroyCanDevice(TCanDevice **device)
{
TCanDevice *dev;

if (!device)
  return;
dev = *device;
if (dev)
  {
  (void)mhs_cmd_execute(dev, MCMD_DEVICE_CLOSE, NULL, 0, 1000);
  (void)mhs_join_thread(dev->MainThread, 5000);  // Timeout 5 Sek.
  clear_device_from_list(dev->Index);
  MhsEventListDestroy(&dev->StatusEventList);
  CloseCanDevice(dev);
  safe_free(dev->DrvInfoHwStr);
  InfoVarDestroy(&dev->InfoVars);
#ifdef ENABLE_RS485_SUPPORT // <*> neu
  RS485Destroy(dev);
#endif  
  mhs_obj_context_destroy(&dev->Context);
  mhs_obj_context_destroy(&dev->IoContext);
#if TX_PUFFER_MAX > 0
  mhs_obj_context_destroy(&dev->TxPufferContext);
#endif
#if RX_FILTER_MAX
  mhs_obj_context_destroy(&dev->RxFilterContext);
#endif
  mhs_cmd_destroy(&dev->Cmd);
  //clear_device_from_list(dev->Index);
  (void)mhs_destroy_thread(&dev->MainThread, 1);
  mhs_free(dev);
  }
*device = NULL;
}


int32_t GetDeviceInfo(TCanDevice *dev, struct TCanDeviceInfo *device_info)
{
uint32_t snr;
char *desc;

if (!device_info)
  return(ERR_PARAM);
desc = GetInfoValueAsString(GetInfoByIndex(dev, 0x0001)); // Hardware ID String
if (desc)
  safe_strcpy(device_info->Description, DESCRIPTION_MAX_SIZE, desc);
device_info->FirmwareVersion = GetInfoValueAsULong(GetInfoByIndex(dev, 0x1002)); // Version
snr = GetInfoValueAsULong(GetInfoByIndex(dev, 0x0000)); // Hardware Snr
safe_sprintf(device_info->SerialNumber, SERIAL_NUMBER_MAX_SIZE, "%08X", snr);

if (dev->ModulDesc)
  {
  device_info->ModulFeatures.CanClock = dev->ModulDesc->CanClock;
  device_info->ModulFeatures.Flags = dev->ModulDesc->CanFeaturesFlags;
  device_info->ModulFeatures.CanChannelsCount = dev->ModulDesc->CanChannels;
  device_info->ModulFeatures.HwRxFilterCount = dev->ModulDesc->HwRxFilterCount;
  device_info->ModulFeatures.HwTxPufferCount = dev->ModulDesc->HwTxPufferCount;
  device_info->HwId = dev->ModulDesc->HwId;
  }
else
  {
  device_info->ModulFeatures.CanClock = 24;
  device_info->ModulFeatures.Flags = 0;
  device_info->ModulFeatures.CanChannelsCount = 1;
  device_info->ModulFeatures.HwRxFilterCount = 4;
  device_info->ModulFeatures.HwTxPufferCount = 4;
  device_info->HwId = 0;
  }
return(0);
}


static int32_t SetupCanDevice(TCanDevice *dev)
{
int32_t res;
struct TTCanUsbList *usb_device;
TMhsObjContext *context, *io_context, *main_context;
int32_t usr_drv_type, usr_port, usr_vendor_id, usr_product_id;
uint32_t baud_rate, usr_baud_rate, auto_open_reset, tiny_can_type;
char *usr_snr;
#ifndef __WIN32__
char *usr_device_name, *s;
#endif

io_context = dev->IoContext;
context = dev->Context;
main_context = can_main_get_context();
if (mhs_value_get_status("AutoOpenReset", context) & MHS_VAL_FLAG_WRITE)
  auto_open_reset = mhs_value_get_as_ulong("AutoOpenReset", ACCESS_INIT, context);
else
  auto_open_reset = mhs_value_get_as_ulong("AutoOpenReset", ACCESS_INIT, main_context);
usr_drv_type = mhs_value_get_as_long("ComDrvType", ACCESS_INIT, context);
usr_port = mhs_value_get_as_long("Port", ACCESS_INIT, context);
usr_baud_rate = mhs_value_get_as_ulong("BaudRate", ACCESS_INIT, context);
usr_snr = mhs_value_get_as_string("Snr", ACCESS_INIT, context);
usr_vendor_id = mhs_value_get_as_long("VendorId", ACCESS_INIT, context);
usr_product_id = mhs_value_get_as_long("ProductId", ACCESS_INIT, context);
#ifndef __WIN32__
usr_device_name = mhs_value_get_as_string("DeviceName", ACCESS_INIT, context);
#endif

FreeModulDesc(&dev->ModulDesc);
baud_rate = 0;
usb_device = NULL;
res = 0;
tiny_can_type = 0;
if (!usr_drv_type)
  {  // RS 232
  res = 1;
  if (usr_baud_rate)
    baud_rate = usr_baud_rate;
  else
    baud_rate = RS232_DEF_BAUDRATE;
  (void)mhs_value_set_as_long("PortNr", usr_port, ACCESS_INIT, 0, io_context);
  }
else
  {  // USB FTDI
  // Plug & Play aus ?
  if ((usr_vendor_id) || (usr_product_id))
    {  // ja
    if (usr_vendor_id)
      (void)mhs_value_set_as_long("VendorId", usr_vendor_id, ACCESS_INIT, 0, io_context);
    if (usr_product_id)
      (void)mhs_value_set_as_long("ProductId", usr_product_id, ACCESS_INIT, 0, io_context);
    res = 1;
    }
#ifdef __WIN32__
  else
#else
  else if (!((usr_device_name) && (strlen(usr_device_name))))
#endif
    {
    if ((usb_device = GetUsbDevice(usr_snr)))
      {
      res = 1;
      dev->ModulDesc = CopyModulDesc(usb_device->Modul);
      tiny_can_type = dev->ModulDesc->TinyCanType;
      if (dev->ModulDesc->Description)
        (void)mhs_value_set_as_string("UsbDescription", dev->ModulDesc->Description, ACCESS_INIT, 0, io_context);
#ifndef __WIN32__
      usr_device_name = usb_device->DeviceName[0];
#endif
      usr_snr = usb_device->Serial;
      baud_rate = dev->ModulDesc->BaudRate[0];
      }
    }
#ifndef __WIN32__
  else
    res = 1;
#endif
  // **** Device Name u. Baudrate durch "User Werte" überschreiben
#ifndef __WIN32__
// ****** Linux, Mac
  if ((usr_device_name) && (strlen(usr_device_name)))
    (void)mhs_value_set_as_string("DeviceName", usr_device_name, ACCESS_INIT, 0, io_context);
  if (usb_device)  
    s = usb_device->DeviceName[1];
  else
    s = NULL;  
  (void)mhs_value_set_as_string("DeviceNameB", s, ACCESS_INIT, 0, io_context);  
#endif
  if (usr_baud_rate)
    baud_rate = usr_baud_rate;
  (void)mhs_value_set_as_string("SerialNr", usr_snr, ACCESS_INIT, 0, io_context);  
  if (usb_device)
    (void)mhs_value_set_as_ulong("UsbDeviceType", usb_device->DeviceType, ACCESS_INIT, 0, io_context);
  else
    (void)mhs_value_set_as_ulong("UsbDeviceType", FT_DEVICE_232R, ACCESS_INIT, 0, io_context);
  }
(void)mhs_value_set_as_ulong("TinyCanType", tiny_can_type, ACCESS_INIT, 0, io_context);  
(void)mhs_value_set_as_ulong("BaudRate", baud_rate, ACCESS_INIT, 0, io_context);
(void)mhs_value_set_as_ulong("AutoOpenReset", auto_open_reset, ACCESS_INIT, 0, io_context);
return(res);
}


/***************************************************************/
/* OpenCanDevice                                               */
/***************************************************************/
int32_t OpenCanDevice(TCanDevice *dev)
{
TMhsObjContext *context, *io_context, *main_context;
int32_t drv_type;
uint32_t priority;

CloseCanDevice(dev);
context = dev->Context;
io_context = dev->IoContext;
main_context = can_main_get_context();
if (mhs_value_get_status("MainThreadPriority", context) & MHS_VAL_FLAG_WRITE)
  priority = mhs_value_get_as_ulong("MainThreadPriority", ACCESS_INIT, context);
else
  priority = mhs_value_get_as_ulong("MainThreadPriority", ACCESS_INIT, main_context);
drv_type = mhs_value_get_as_long("ComDrvType", ACCESS_INIT, context);
// RS232 / USB Treiber erzeugen
if (!(dev->Io = ComIoCreate(drv_type, io_context)))
  {
#ifdef ENABLE_LOG_SUPPORT
  LogPrintf(LOG_ERROR | LOG_MESSAGE, "FEHLER: Open CAN-Device");
#endif
  return(ERR_INIT_USB_COM);
  }
if (SetupCanDevice(dev) < 1)
  return(ERR_DEVICE_UNPLUGGED);
mhs_cmd_clear(dev);
mhs_thread_set_priority(dev->MainThread, priority);
if (mhs_run_thread(dev->MainThread) < 0)
  return(ERR_START_MAIN_THREAD);
return(mhs_cmd_execute(dev, MCMD_DEVICE_OPEN, NULL, 0, -1));
}


void CloseCanDevice(TCanDevice *dev)
{
int32_t flag;

flag = 0;
(void)mhs_cmd_execute(dev, MCMD_DEVICE_CLOSE, NULL, 0, 1000);
if (mhs_thread_join_status(dev->MainThread))
  {
  if (mhs_join_thread(dev->MainThread, 5000) < -1) // Timeout 5 Sek.
    flag = 1;
  }
else
  flag = 1;
if (flag)
  {
  dev->DeviceStatus.DrvStatus = DRV_STATUS_INIT;    // Com Port geschlossen
  MainUpdateStatusProc(dev);
  PnPSetDeviceStatus(dev->Index, NULL, PNP_DEVICE_PLUGED, 1);
  }
ComIoDestroy(&dev->Io);
}


int32_t SetIntervallBox(TCanDevice *dev, uint32_t index, uint16_t cmd, uint32_t time)
{
TMhsObjContext *context;
TObjCanPuffer *puffer;

if (!dev)
  return(-1);
context = dev->TxPufferContext;
if (!dev->ModulDesc)
  return(-1);
if ((index & 0xFFFF) > dev->ModulDesc->HwTxPufferCount)
  return(-1);
if ((puffer = (TObjCanPuffer *)mhs_object_get_by_index(index, context)))
  {
  if (cmd & 0x8000)
    mhs_can_puffer_set_intervall(puffer, time);
  }
return(0);
}


/***************************************************************/
/*  PC Port USB/RS232 öffnen                                   */
/***************************************************************/
static int32_t PCPortOpen(TCanDevice *dev, uint32_t cnt, uint32_t max)
{
TMhsObjContext *context, *io_context;
char *snr;
int32_t res, drv_type;
uint32_t baud_rate;
char *usb_description;
#ifdef __WIN32__
int32_t port;
#else
char *device_name;  // ****** Linux, Mac
#endif

context = dev->Context;
io_context = dev->IoContext;
drv_type = mhs_value_get_as_long("ComDrvType", ACCESS_INIT, context);
snr = mhs_value_get_as_string("SerialNr", ACCESS_INIT, io_context);
#ifdef ENABLE_LOG_SUPPORT
baud_rate = mhs_value_get_as_ulong("BaudRate", ACCESS_INIT, io_context);
#ifdef __WIN32__
port = mhs_value_get_as_long("PortNr", ACCESS_INIT, io_context);
#endif
usb_description = mhs_value_get_as_string("UsbDescription", ACCESS_INIT, io_context);
#ifndef __WIN32__
// ****** Linux, Mac
device_name = mhs_value_get_as_string("DeviceName", ACCESS_INIT, io_context);
#endif
#endif
#ifdef __WIN32__
res = ComOpen(dev->Io, NULL, 0, 0);
#else
res = ComOpen(dev->Io);
#endif
if (res < 0)
  {  // Fehler
#ifdef ENABLE_LOG_SUPPORT
  if (!drv_type)   // RS232
    {
#ifdef __WIN32__
// ****** Windows
    LogPrintf(LOG_ERROR | LOG_MESSAGE, "FEHLER[%u/%u]: Open CAN-Device (Port:%d Baudrate:%d): [%d] %s", cnt, max,
    port, baud_rate, dev->Io->ErrorCode, dev->Io->ErrorString);
#else
// ****** Linux, Mac
    LogPrintf(LOG_ERROR | LOG_MESSAGE, "FEHLER[%u/%u]: Open CAN-Device (Device:%s Baudrate:%d): [%d] %s", cnt, max,
      device_name, baud_rate, dev->Io->ErrorCode, dev->Io->ErrorString);
#endif
    }
  else
    {
    LogPrintf(LOG_ERROR | LOG_MESSAGE, "FEHLER[%u/%u]: Open CAN-Device (%s Snr.:%s): [%d] %s",
      cnt, max, usb_description, snr, dev->Io->ErrorCode, dev->Io->ErrorString);
    }
#endif
  }
return(res);
}


#ifdef ENABLE_LOG_SUPPORT

static void LogDeviceOpen(TCanDevice *dev, int32_t res)
{
TMhsObjContext *context, *io_context;
char *snr;
int32_t drv_type;
uint32_t baud_rate;
char *usb_description;
#ifdef __WIN32__
int32_t port;
#else
char *device_name;  // ****** Linux, Mac
#endif

context = dev->Context;
io_context = dev->IoContext;
drv_type = mhs_value_get_as_long("ComDrvType", ACCESS_INIT, context);
snr = mhs_value_get_as_string("SerialNr", ACCESS_INIT, io_context);
baud_rate = mhs_value_get_as_ulong("BaudRate", ACCESS_INIT, io_context);
#ifdef __WIN32__
port = mhs_value_get_as_long("PortNr", ACCESS_INIT, io_context);
#endif
usb_description = mhs_value_get_as_string("UsbDescription", ACCESS_INIT, io_context);
#ifndef __WIN32__
// ****** Linux, Mac
device_name = mhs_value_get_as_string("DeviceName", ACCESS_INIT, io_context);
#endif
if (res < 0)
  {
  if (!drv_type)   // RS232
    {
#ifdef __WIN32__
// ****** Windows
    LogPrintf(LOG_ERROR | LOG_MESSAGE, "FEHLER: Open CAN-Device (Port:%d Baudrate:%d): %s", port,
      baud_rate, GetErrorString(res));
#else
// ****** Linux, Mac
    LogPrintf(LOG_ERROR | LOG_MESSAGE, "FEHLER: Open CAN-Device (Device:%s Baudrate:%d): %s", device_name,
      baud_rate, GetErrorString(res)); 
#endif
    }
  else
    {
    LogPrintf(LOG_ERROR | LOG_MESSAGE, "FEHLER: Open CAN-Device (%s Snr.:%s): %s",
      usb_description, snr, GetErrorString(res));
    }
  }
else
  { // Device erfolgreich geöffnet
  if (!drv_type)   // RS232
    {
#ifdef __WIN32__
// ****** Windows
    LogPrintf(LOG_MESSAGE, "MESSAGE: CAN-Device erfolgreich geöffnet (Port:%d Baudrate:%d)", port, baud_rate);
#else
// ****** Linux, Mac
    LogPrintf(LOG_MESSAGE, "MESSAGE: CAN-Device erfolgreich geöffnet (Device:%s Baudrate:%d)", device_name,
      baud_rate);
#endif
    }
  else
    {
    LogPrintf(LOG_MESSAGE, "MESSAGE: CAN-Device erfolgreich geöffnet (%s Snr.:%s)",
      usb_description, snr);
    }
  LogPrintf(LOG_DEBUG, "CAN Hardware Info:");
  LogPrintf(LOG_DEBUG, "    Protokoll Version: %u", dev->Protokoll);
  LogPrintf(LOG_DEBUG, "    CAN Features Flags: 0x%08X", dev->ModulDesc->CanFeaturesFlags);
  LogPrintf(LOG_DEBUG, "    Anzahl Interval Puffer: %u", dev->ModulDesc->HwTxPufferCount);
  LogPrintf(LOG_DEBUG, "    Anzahl Filter: %u", dev->ModulDesc->HwRxFilterCount);
  LogPrintf(LOG_DEBUG, "    Anzahl CAN Kanaele: %u", dev->ModulDesc->CanChannels);
  LogPrintf(LOG_DEBUG, "    Anzahl Digital I/Os: %u", dev->ModulDesc->DigIoCount);            
  }
}

#endif


/***************************************************************/
/*  Device öffnen                                              */
/***************************************************************/
static int32_t ExecuteDeviceOpen(TCanDevice *dev)
{
TMhsObjContext *context, *io_context;
struct TBiosVersionInfo version_info;
int32_t res, drv_type, port_open;
uint32_t max, cnt, hw_id, modul_id;
const struct TCanModulDesc *modul_desc;
struct TInfoVarList *info;
char *snr;
uint32_t i, value, use_baud_rate_b, check_rts_cts_clear, baud_rate, baud_rate_b;
uint8_t channel;

context = dev->Context;
io_context = dev->IoContext;
drv_type = mhs_value_get_as_long("ComDrvType", ACCESS_INIT, context);
check_rts_cts_clear = 0;
use_baud_rate_b = 0;
baud_rate_b = 0;
if (dev->ModulDesc)
  {
  check_rts_cts_clear = dev->ModulDesc->Flags;
  if (!mhs_value_get_as_ulong("BaudRate", ACCESS_INIT, context))
    {
    if ((baud_rate_b = dev->ModulDesc->BaudRate[1]))
      use_baud_rate_b = 1;
    baud_rate = dev->ModulDesc->BaudRate[0];
    } 
  } 

if (!drv_type)
  max = 2;    // RS 232 -> Nur 2 mal versuchen
else
  max = 5;    // USB -> Öffnen des Devices 5 mal versuchen
port_open = 1;
for (cnt = 0; cnt < max; cnt++)
  {
  if (cnt)
    {
    if (mhs_sleep_ex((TMhsEvent *)dev->MainThread, 1000))
      break;
    }  
  if (port_open)
    {  
    if ((res = PCPortOpen(dev, cnt, max)) < 0)
      continue;
    port_open = 0;
    }
  else
    {
    if ((use_baud_rate_b) && (dev->HaveRtsCtsClear))
      (void)mhs_value_set_as_ulong("BaudRate", baud_rate, ACCESS_INIT, 0, io_context);
    }
  dev->HaveRtsCtsClear = 0;      
  if (check_rts_cts_clear)
    {
    if ((res = SMCheckRtsCtsClear(dev)) < 0)
      continue;
    if (res)
      dev->HaveRtsCtsClear = 1;   
    }
  if ((use_baud_rate_b) && (dev->HaveRtsCtsClear))
    (void)mhs_value_set_as_ulong("BaudRate", baud_rate_b, ACCESS_INIT, 0, io_context);  
  if ((res = SMDrvClear(dev)) < 0)
    continue;
  // **** Hardware Online ?
  if ((res = TARCommSync(dev)) < 0)
    {
    if ((res >= ERR_SM_PORT_NOT_OPEN) && (res < ERR_SM_RX_ACK_TIMEOUT))
      port_open = 1;
    continue;
    }
  if ((res = TARPing(dev)) < 0)
    {
    if ((res >= ERR_SM_PORT_NOT_OPEN) && (res < ERR_SM_RX_ACK_TIMEOUT))
      port_open = 1;
    continue;
    }
  // **** Übertragungsprotokoll Version abfragen
  if ((res = TARGetBiosVersion(dev, &version_info)) < 0)
    {
    if ((res >= ERR_SM_PORT_NOT_OPEN) && (res < ERR_SM_RX_ACK_TIMEOUT))
      port_open = 1;
    continue;
    }    
  if (version_info.Version >= 0x10)
    dev->Protokoll = 2;
  else
    dev->Protokoll = 1;
  // **** Hardware Info Datenbank abfragen
  if ((res = HwReadInfoVar(dev)) < 0)
    {
    if ((res >= ERR_SM_PORT_NOT_OPEN) && (res < ERR_SM_RX_ACK_TIMEOUT))
      port_open = 1;
    continue;
    }    

  modul_id = GetInfoValueAsULong(GetInfoByIndex(dev, 0x1000));
  for (modul_desc = &ExCanModulTable[0]; (hw_id = modul_desc->HwId); modul_desc++)
    {
    if (hw_id == modul_id)
      {
      FreeModulDesc(&dev->ModulDesc);
      dev->ModulDesc = CopyModulDesc(modul_desc);
      break;
      }
    }
  if (!hw_id)
    {
    res = -1;      // Modul unbekannt
    break;
    }
  if (version_info.Version >= 0x10)
    {
    dev->ModulDesc->RxMtu = version_info.RxMTU;
    }  
  // **** Modul Description an Hardware Info Datenbank anpassen
  if ((info = GetInfoByIndex(dev, 0x8001)))  // CAN Features Flags
    {
    value = GetInfoValueAsULong(info);
    if (value & 0x80)
      {
      dev->ModulDesc->SupportHwTimestamp = 1;
      value = (value & 0x7F) | 0x8000;
      }
    dev->ModulDesc->CanFeaturesFlags = value;
    if (value & 0x0004)
      dev->ModulDesc->SupportTxAck = 1;
    else
      dev->ModulDesc->SupportTxAck = 0;
    if (value & 0x0008)
      dev->ModulDesc->SupportCanErrorMsgs = 1;
    else
      dev->ModulDesc->SupportCanErrorMsgs = 0;
    }
  if ((info = GetInfoByIndex(dev, 0x8002)))  // CAN Features Flags 2
    {
    value = GetInfoValueAsULong(info);
    dev->ModulDesc->CanFeaturesFlags = (dev->ModulDesc->CanFeaturesFlags & 0x80FF) | (value << 8); // CAN_FEATURE_CAN_TEST
    }  
  if ((info = GetInfoByIndex(dev, 0x8050)))  // Anzahl Interval Puffer
    dev->ModulDesc->HwTxPufferCount = GetInfoValueAsULong(info);
  if ((info = GetInfoByIndex(dev, 0x8060)))  // Anzahl Filter
    dev->ModulDesc->HwRxFilterCount = GetInfoValueAsULong(info);
  if ((info = GetInfoByIndex(dev, 0x8000)))  // Anzahl CAN Kanäle
    dev->ModulDesc->CanChannels = GetInfoValueAsULong(info);
  if ((info =  GetInfoByIndex(dev, 0x8300)))  // Anzahl Digital I/Os
    dev->ModulDesc->DigIoCount = GetInfoValueAsULong(info);
  if ((DriverInit == TCAN_DRV_FD_INIT) && 
      (dev->ModulDesc->CanFeaturesFlags & CAN_FEATURE_FD_HARDWARE))
    dev->CanFd = 1;
  else
    dev->CanFd = 0;
  // <*> Neu CAN Clocks
  for (i = 0; i < 6; i++)
    {
    if (!(info = GetInfoByIndex(dev, TCAN_INFO_KEY_FW_CAN_CLOCK1 + i)))
      break;
    if (!(value = GetInfoValueAsULong(info)))
      break;
    InfoVarAddEx(dev, TCAN_INFO_KEY_CAN_CLOCK1 + i, IT_ULONG, (char *)&value, 4);  
    }    
  if (!i)
    {
    if (dev->ModulDesc->CanFeaturesFlags & CAN_FEATURE_FD_HARDWARE)
      value = 48;
    else
      value = 16;
    InfoVarAddEx(dev, TCAN_INFO_KEY_CAN_CLOCK1, IT_ULONG, (char *)&value, 4);
    i = 1;
    }    
  (void)InfoVarAddEx(dev, TCAN_INFO_KEY_CAN_CLOCKS, IT_ULONG, (char *)&i, 4);
            
  if (dev->Protokoll == 2)
    {
    if ((res = TAR2GetStatus(dev)) < 0)
      {
      if ((res >= ERR_SM_PORT_NOT_OPEN) && (res < ERR_SM_RX_ACK_TIMEOUT))
        port_open = 1;
      continue;
      }
    }
  else
    {
    if ((res = TARGetStatus(dev)) < 0)
      {
      if ((res >= ERR_SM_PORT_NOT_OPEN) && (res < ERR_SM_RX_ACK_TIMEOUT))
        port_open = 1;
      continue;
      }
    }
  // **** FIFOs löschen
  if (dev->Protokoll == 2)
    {
    for (channel = 0; channel < (uint8_t)dev->ModulDesc->CanChannels; channel++)
      {
      // Rx & Tx FIFO löschen, Rx-OV & Tx-OV löschen
      if ((res = TAR2SetCANMode(dev, channel, 0xA0)) < 0)
        break;
      }
    if (res < 0)
      continue;  
    }                      
  else
    {
    if ((res = TARRxFifoClear(dev, 0)) < 0)
      continue;
    if ((res = TARTxFifoClear(dev, 0)) < 0)
      continue;
    }
  break;
  }
#ifdef LOG_DEBUG_ENABLE    
LogProtokollInfo(LOG_DEBUG, dev, &version_info);
#endif
#ifdef ENABLE_LOG_SUPPORT
LogDeviceOpen(dev, res);
#endif

if (res < 0)
  { 
  res = ERR_HARDWARE_OFFLINE;
  dev->DeviceStatus.DrvStatus = DRV_STATUS_PORT_NOT_OPEN; // Port kann nicht geöffnet werden
  }
else
  { // Port erfolgrich geöffnet
  dev->DeviceStatus.DrvStatus = DRV_STATUS_PORT_OPEN;    // Com Port erfolgreich geöffnet
  snr = mhs_value_get_as_string("SerialNr", ACCESS_INIT, io_context);
  if ((!res) && (drv_type))  // kein Fehler, drv_type = USB
    PnPSetDeviceStatus(dev->Index, snr, PNP_DEVICE_ONLINE, 1);
  }
return(res);
}


/***************************************************************/
/*  Betriebsart einstellen                                     */
/***************************************************************/
static int32_t ExecuteCanCommand(TCanDevice *dev, uint8_t channel, uint32_t can_op_mode, uint16_t can_command)
{
TMhsObjContext *context;
int32_t err;
uint32_t can_features_flags;
uint8_t cmd, tar_can_op_mode; //, i;
#if (TX_PUFFER_MAX > 0) || (RX_FILTER_MAX > 0)
uint8_t channels; 
union TCanIndex index;
uint32_t i;
#endif
#if RX_FILTER_MAX > 0
TObjCan *filter;
#endif
#if TX_PUFFER_MAX > 0
TObjCanPuffer *puffer;
uint8_t ch;
#endif

#if (TX_PUFFER_MAX > 0) || (RX_FILTER_MAX > 0)
if (dev->ModulDesc)
  channels = (uint8_t)dev->ModulDesc->CanChannels;
else
  channels = 1;
#endif
cmd = 0;
switch (can_op_mode)
  {
  case MCMD_CAN_RESET     : {
                            tar_can_op_mode = OP_CAN_RESET;
                            break;
                            }
  case MCMD_CAN_START     : {
                            tar_can_op_mode = OP_CAN_START;
                            break;
                            }
  case MCMD_CAN_START_LOM : {
                            tar_can_op_mode = OP_CAN_START_LOM;
                            break;
                            }
  case MCMD_CAN_START_NO_RETRANS :
                            {
                            tar_can_op_mode = OP_CAN_START_NO_RETRANS;
                            break;
                            }
  case MCMD_CAN_STOP :      {
                            tar_can_op_mode = OP_CAN_STOP;
                            break;
                            }
  case MCMD_CAN_ECU_FLASH_MODE :
                            {                            
                            tar_can_op_mode = OP_CAN_ECU_FLASH_MODE;
                            break;
                            }
  default :                 tar_can_op_mode = OP_CAN_NO_CHANGE;
  }
// RxD Fifo löschen
if (can_command & CAN_CMD_RXD_FIFOS_CLEAR)
  {
  mhs_can_fifo_clear_by_index(dev->Index);
  cmd |= 0x20;
  }
// RxD Overrun Flag löschen
else if (can_command & CAN_CMD_RXD_OVERRUN_CLEAR)
  {
  mhs_can_fifo_overrun_clear_by_index(dev->Index);
  cmd |= 0x10;
  }
// TxD Fifo löschen
if (can_command & CAN_CMD_TXD_FIFOS_CLEAR)
  {
  mhs_can_fifo_clear_by_index(dev->Index | INDEX_TXT_FLAG);
  cmd |= 0x80;
  }
// TxD Overrun Flag löschen
else if (can_command & CAN_CMD_TXD_OVERRUN_CLEAR)
  {
  mhs_can_fifo_overrun_clear_by_index(dev->Index | INDEX_TXT_FLAG);
  cmd |= 0x40;
  }
// Alle Hardware Filter löschen
if ((dev->ModulDesc) && (can_command & CAN_CMD_HW_FILTER_CLEAR))
  {
#if RX_FILTER_MAX > 0
  context = dev->RxFilterContext;
  index.Long = dev->Index;
  if (dev->Protokoll == 2)
    {
    for (ch = 0; ch < channels; ch++)
      {
      index.Item.Channel = ch;
      for (i = 1; i <= RX_FILTER_MAX; i++)
        {          
        index.Item.SubIndex = i;                      
        if ((filter = (TObjCan *)mhs_object_get_by_index(index.Long, context)))
          {
          filter->Filter.Maske = 0;
          filter->Filter.Code = 0;
          filter->Filter.FilFlags = 0;
          if (i <= dev->ModulDesc->HwRxFilterCount)
            (void)TAR2SetFilter(dev, ch, i, &filter->Filter);
          }  
        }
      }
    }
  else
    {
    for (i = 1; i <= RX_FILTER_MAX; i++)
      {
      if ((filter = (TObjCan *)mhs_object_get_by_index(i, context)))
        {
        filter->Filter.Maske = 0;
        filter->Filter.Code = 0;
        filter->Filter.FilFlags = 0;
        if (i <= dev->ModulDesc->HwRxFilterCount)
          (void)TARSetFilter(dev, i, &filter->Filter);
        }
      }
    }
#endif
  }
// Alle Software Filter löschen
if (can_command & CAN_CMD_SW_FILTER_CLEAR)
  can_sw_filter_remove_all();
if ((dev->ModulDesc) && (can_command & CAN_CMD_TXD_PUFFERS_CLEAR))
  {
#if TX_PUFFER_MAX > 0
// **** Intervall Boxen setzen
  context = dev->TxPufferContext;
  index.Long = dev->Index | INDEX_TXT_FLAG;
  if (dev->Protokoll == 2)
    {            
    for (ch = 0; ch < channels; ch++)
      {
      index.Item.Channel = ch;
      for (i = 1; i <= dev->ModulDesc->HwTxPufferCount; i++)
        {          
        index.Item.SubIndex = i;
        if ((puffer = (TObjCanPuffer *)mhs_object_get_by_index(index.Long, context)))
          {
          mhs_can_puffer_set_intervall(puffer, 0);
          if ((err = TAR2CanSetIntervall(dev, ch, i, 0)) < 0)
            return(err);
          }
        }  
      }    
    }
  else
    {
    for (i = 1; i <= dev->ModulDesc->HwTxPufferCount; i++)
      {
      index.Item.SubIndex = i;
      if ((puffer = (TObjCanPuffer *)mhs_object_get_by_index(index.Long, context)))
        {
        mhs_can_puffer_set_intervall(puffer, 0);
        if ((err = TARCanSetIntervall(dev, i, 0)) < 0)
          return(err);
        }
      }
    }
#endif
  }
if (dev->ModulDesc)
  can_features_flags = dev->ModulDesc->CanFeaturesFlags;
else
  can_features_flags = 0;
switch (can_command & 0xF000)
  {    
  case CAN_CMD_START_DATA_TRANSFER :
           {
           dev->CanDataExec = CAN_DATA_EXEC_START;
           break;
           }
  case CAN_CMD_CANCEL_DATA_TRANSFER :
           {
           dev->CanDataExec = CAN_DATA_EXEC_CANCEL;
           break;
           }
  case CAN_CMD_START_TEST :
           {
           if ((dev->Protokoll == 2) && (can_features_flags & CAN_FEATURE_CAN_TEST))
             TAR2CanTestExecute(dev, 1);
           break;
           }           
  case CAN_CMD_STOP_TEST :
           {
           if ((dev->Protokoll == 2) && (can_features_flags & CAN_FEATURE_CAN_TEST))
             TAR2CanTestExecute(dev, 0);
           break;
           }  
  }
if (dev->Protokoll == 2)
  return(TAR2SetCANMode(dev, channel, (uint8_t)(tar_can_op_mode | cmd)));
else
  return(TARSetCANMode(dev, (uint8_t)(tar_can_op_mode | cmd)));
}


static int32_t CanStop(TCanDevice *dev, uint8_t channel) 
{
int32_t res;

res = 0;
if (dev->Protokoll == 2)
  {
  if (channel == 0xFF)
    {
    if (dev->ModulDesc) 
      {
      for (channel = 0; channel < (uint8_t)dev->ModulDesc->CanChannels; channel++)
        {
        if ((res = TAR2SetCANMode(dev, channel, OP_CAN_STOP)) < 0)
          break;
        } 
      }                                    
    }
  else
    res = TAR2SetCANMode(dev, channel, OP_CAN_STOP);
  }
else
  res = TARSetCANMode(dev, OP_CAN_STOP);
return(res);
}  


/***************************************************************/
/*  Main Commandos Event Funktion                              */
/***************************************************************/
static void MainCommandoProc(TCanDevice *dev)
{
TMhsObjContext *context, *main_context;
int32_t res;
uint8_t channel, auto_stop_can;
struct TCmdSetModeParam *set_mode_param;

res = 0;
main_context = can_main_get_context();
context = dev->Context;
switch (dev->Cmd->Command)
  {
  case MCMD_DEVICE_OPEN     : {
                              res = ExecuteDeviceOpen(dev);
                              if (res >= 0)
                                res = ProcessSetup(dev, SETUP_SET_ALL | SETUP_CAN_STOP | SETUP_CAN_CLEANUP);
                              if (res >= 0)
                                dev->DeviceStatus.DrvStatus = DRV_STATUS_CAN_OPEN;
                              else
                                dev->DeviceStatus.DrvStatus = DRV_STATUS_PORT_NOT_OPEN;
                              break;
                              }
  case MCMD_DEVICE_CLOSE    : {
                              auto_stop_can = mhs_value_get_as_ubyte("AutoStopCan", ACCESS_PORT_OPEN, main_context);
                              if (mhs_value_get_status("AutoStopCan", context) & MHS_VAL_FLAG_WRITE)
                                auto_stop_can = mhs_value_get_as_ubyte("AutoStopCan", ACCESS_INIT, context);
                              if (auto_stop_can)
                                (void)CanStop(dev, 0xFF);
                                /*{
                                if (dev->Protokoll == 2)
                                  {
                                  if (dev->ModulDesc)
                                    {
                                    for (channel = 0; channel < (uint8_t)dev->ModulDesc->CanChannels; channel++)
                                      (void)TAR2SetCANMode(dev, channel, OP_CAN_STOP);
                                    }
                                  }
                                else
                                  (void)TARSetCANMode(dev, OP_CAN_STOP);
                                } */ 
                              ComClose(dev->Io);
                              dev->DeviceStatus.DrvStatus = DRV_STATUS_PORT_NOT_OPEN;                                
                              break;
                              }
  case MCMD_CAN_ECU_FLASH_MODE :
                              {
                              if (!(dev->ModulDesc->CanFeaturesFlags & CAN_FEATURE_ECU_FLASH))
                                {
                                res = ERR_UNSUPPORTED_FEATURE;
                                break;
                                }
                              [[fallthrough]];  // <*> neu, Windows ?  __fallthrough; ?  processHacker                               
                              }        
  case MCMD_CAN_START       :
  case MCMD_CAN_START_LOM   :
  case MCMD_CAN_START_NO_RETRANS :
  case MCMD_CAN_RESET       :
                              {                              
                              if (dev->DeviceStatus.DrvStatus < DRV_STATUS_CAN_OPEN)  // <*> neu
                                break;
                              set_mode_param = (struct TCmdSetModeParam *)dev->Cmd->Param;
                              channel = (uint8_t)(set_mode_param->Index >> 16) & 0x03;  
                              res = CanStop(dev, channel);  
                              if (!res)  
                                res = ProcessSetup(dev, SETUP_SET_ALL);
                              if (res < 0)
                                {
                                dev->DeviceStatus.DrvStatus = DRV_STATUS_PORT_NOT_OPEN;
                                break;
                                }
                              [[fallthrough]];  // <*> neu, Windows ?  __fallthrough; ?  processHacker                                                                                                    
                              }
  case MCMD_CAN_STOP        :
  case MCMD_CAN_ONLY_CMD    : {
                              if (dev->DeviceStatus.DrvStatus < DRV_STATUS_CAN_OPEN)  // <*> neu
                                break;
                              if (dev->Cmd->ParamCount != sizeof(struct TCmdSetModeParam))
                                {
                                res = ERR_PARAM;
                                break;
                                }
                              set_mode_param = (struct TCmdSetModeParam *)dev->Cmd->Param;
                              channel = (uint8_t)(set_mode_param->Index >> 16) & 0x03;
                              res = ExecuteCanCommand(dev, channel, dev->Cmd->Command, set_mode_param->CanCommand);
                              dev->StatusTimeStamp = get_tick();     // Status Timeout resetten
                              dev->FilterTimeStamp = get_tick();     // Filter Timeout resetten
                              dev->DeviceStatus.CanStatus = CAN_STATUS_OK;
                              dev->DeviceStatus.FifoStatus = FIFO_OK;
                              if (dev->Cmd->Command != MCMD_CAN_ONLY_CMD)
                                {
                                if ((dev->Cmd->Command == MCMD_CAN_ECU_FLASH_MODE) && (!res))
                                  dev->EcuFlashMode = 1;
                                else
                                  dev->EcuFlashMode = 0; 
                                }
                              if (res < 0)
                                dev->DeviceStatus.DrvStatus = DRV_STATUS_PORT_NOT_OPEN;
                              else if (dev->Cmd->Command != MCMD_CAN_STOP)
                                dev->DeviceStatus.DrvStatus = DRV_STATUS_CAN_RUN;
                              else
                                dev->DeviceStatus.DrvStatus = DRV_STATUS_CAN_OPEN;
                              break;
                              }
  default : break;                              
  }
mhs_cmd_finish(dev, res);
}


void DevStatusEventSetup(TCanDevice *dev, TMhsEvent *event, uint32_t events)
{
MhsEventListAddDelete(dev->StatusEventList, event, events);
}


/***************************************************************/
/*  Treiber Status auf veränderung prüfen                      */
/***************************************************************/
static void MainUpdateStatusProc(TCanDevice *dev)
{
uint32_t change;

change = 0;
// Fifo Overrun Status anpassen
//if (CanFifoOverrun(CanInt[0].CanRxDFifo))
//  CanInt[0].DeviceStatus.FifoStatus |= FIFO_SW_OVERRUN;
if (dev->DeviceStatus.DrvStatus < DRV_STATUS_CAN_RUN)
  {
  dev->DeviceStatus.CanStatus = CAN_STATUS_UNBEKANNT;
  dev->DeviceStatus.FifoStatus = FIFO_STATUS_UNBEKANNT;
  }
if (dev->DeviceStatus.DrvStatus != dev->LastDeviceStatus.DrvStatus)
  {
  dev->LastDeviceStatus.DrvStatus = dev->DeviceStatus.DrvStatus;
  change = 1;
  }
if (dev->DeviceStatus.CanStatus != dev->LastDeviceStatus.CanStatus)
  {
  dev->LastDeviceStatus.CanStatus = dev->DeviceStatus.CanStatus;
  change = 1;
  }
if (dev->DeviceStatus.FifoStatus != dev->LastDeviceStatus.FifoStatus)
  {
  dev->LastDeviceStatus.FifoStatus = dev->DeviceStatus.FifoStatus;
  change = 1;
  }
if (change)
  {
  dev->StatusChange = 1;
#ifdef ENABLE_LOG_SUPPORT
  LogPrintDrvStatus(LOG_STATUS, convert_device_idx_to_idx(dev->Index), &dev->DeviceStatus);
#endif
  MhsEventListExecute(dev->StatusEventList);
  }
}


/***************************************************************/
/*  Main Thread                                                */
/***************************************************************/
static void MainThreadExecute(TMhsThread *thread)
{
int32_t res;
TCanDevice *dev;
TMhsObjContext *main_context;
uint32_t events, sleep_time, low_poll_intervall, idle_poll_intervall, high_poll_intervall;

dev = (TCanDevice *)thread->Data;
main_context = can_main_get_context();
// ****** Thread starten
if (mhs_value_get_status("LowPollIntervall", dev->Context) & MHS_VAL_FLAG_WRITE)
  low_poll_intervall = mhs_value_get_as_ulong("LowPollIntervall", ACCESS_INIT, dev->Context);
else
  low_poll_intervall = mhs_value_get_as_ulong("LowPollIntervall", ACCESS_INIT, main_context);
dev->StatusTimeStamp = get_tick();         // Status Timeout resetten
dev->FilterTimeStamp = get_tick();         // Filter Timeout resetten
dev->DeviceStatus.DrvStatus = DRV_STATUS_PORT_NOT_OPEN;
(void)ProcessSetup(dev, SETUP_LOCAL);
MainUpdateStatusProc(dev);
// ****** Thread Schleife ******
do
  {
  dev->TimeNow = get_tick();
  // **** Events
  events = mhs_event_get((TMhsEvent *)thread, 1);  // Events abfragen u. löschen
  // **** Events verarbeiten
  if (events & MHS_TERMINATE)
    break;
  if (events & MAIN_CMD_EVENT)
    MainCommandoProc(dev);
  if (dev->DeviceStatus.DrvStatus >= DRV_STATUS_CAN_OPEN)
    {
    if (ProcessSetup(dev, 0) < 0)
      break;
    }

  if (((dev->DeviceStatus.DrvStatus >= DRV_STATUS_CAN_OPEN) && (dev->UseDigIo)) ||
       (dev->DeviceStatus.DrvStatus >= DRV_STATUS_CAN_RUN_TX))
    {
    if (dev->Protokoll == 2)
      {
      if ((res = MainComm2Proc(dev, 0)) <= ERR_SM_PORT_NOT_OPEN)  // <*> mode parameter noch anpassen
        break;
      }
    else
      {
      if ((res = MainCommProc(dev)) <= ERR_SM_PORT_NOT_OPEN)
        break;
      }
	  if (res < 0)
      {
      dev->DeviceStatus.DrvStatus = DRV_STATUS_PORT_OPEN;
      PnPSetDeviceStatus(dev->Index, NULL, PNP_DEVICE_OFFLINE, 0);
      }
    else
      {
      idle_poll_intervall = dev->IdlePollIntervall;
      high_poll_intervall = dev->HighPollIntervall;

      if (!dev->CommIdleCounter)
        {
        if ((sleep_time = idle_poll_intervall) < high_poll_intervall )
          sleep_time = high_poll_intervall ;
        }
      else
        sleep_time = high_poll_intervall ;
      if (sleep_time)
        mhs_sleep_ex((TMhsEvent *)thread, sleep_time);
      }
    }
  else
    {
    // Noch keine Verbindug zur Hardware hergestellt
    // Prüfen ob Verbindung hergestellt werden kann
    if (dev->DeviceStatus.DrvStatus >= DRV_STATUS_CAN_OPEN)
      {
      if ((res = TARCommSync(dev)) < 0) //ERR_SM_PORT_NOT_OPEN)
        break;
      }  
    dev->CommIdleCounter = 0;      
    mhs_sleep_ex((TMhsEvent *)thread, low_poll_intervall);
    }
  MainUpdateStatusProc(dev);
  }
while (thread->Run);
//thread->Run = 0;   // Für den Fall das die Schleife mit "break" beendet wurde
// ****** Thread beenden ******
mhs_cmd_exit_thread(dev);
ComClose(dev->Io);  // Nur sicherheitshalber
dev->DeviceStatus.DrvStatus = DRV_STATUS_INIT;    // Com Port geschlossen
MainUpdateStatusProc(dev);
PnPSetDeviceStatus(dev->Index, NULL, PNP_DEVICE_PLUGED, 0);
}
