/******************/
/*  CONFIG.H      */
/******************/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#define MHSTCAN_DRV
#define HAVE_TX_ACK_BYPASS  // <*> neu
#define HAVE_CAN_DATA_BUF   // <*> neu
//#define ENABLE_RS485_SUPPORT // <*> neu
//#define ENABLE_EX_IO_SUPPORT // <*> neu
//#define CMD_TIMEOUT_DISABLE

#define MHS_OBJ_USE_INDEX
#define MHS_OBJ_USE_EVENTS

// #define DEBUG_LOG_ENABLE  // Log File schreiben
// #define LOG_FILE_MODE_BUFFERED

//#define ENABLE_CONFIG_FILE
#define ENABLE_LOG_SUPPORT
#define LOG_DEBUG_ENABLE
//#define MEM_DEBUG

#define DLL_NAME "mhstcan.dll"

/*
#define DEF_CFG_FILE_NAME "tiny_can.cfg"
#define DEF_CFG_SECTION   "DEFAULT"
*/
#define DEF_LOG_FLAGS 0xC8000063  // LOG_MESSAGE, LOG_STATUS, LOG_ERROR, LOG_WARN, LOG_DEBUG,
                                  // LOG_WITH_TIME, LOG_DISABLE_SYNC
#define DEF_LOG_FILE_NAME "tiny_can.log"

#define RX_FILTER_MAX 8
#define TX_PUFFER_MAX 16

#define HW_TX_FIFO_MAX 72

/*
// Zwischenpuffer für den CAN Nachrichten Empfang und Senden
#define RX_CAN_BUFFER_SIZE 30 */
#define TX_CAN_TX_LIMIT 9

#define MAX_CAN_CHANNELS 4

#define MAX_IO_CFG 8

#define MAX_ISO_TP_MSG_LENGTH 4095

#endif
