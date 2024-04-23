#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "os_core.h"
#include "config.h"

#ifdef __cplusplus
  extern "C" {
#endif

#define CANFD_MAX_DLEN 64

#define MAIN_CMD_EVENT       0x00000001L

// werden von SL-CAN Treiber benutzt
#define TX_MESSAGES_EVENT    0x00000002L
#define FTDI_EVENT           0x20000000L


#define MAX_LINE_SIZE 200
#define MAX_FILENAME_SIZE 200


#ifndef MAX_CAN_CHANNELS
  #define MAX_CAN_CHANNELS 1
#endif


#define ACCESS_PUBLIC     0xFF01
#define ACCESS_PORT_OPEN  0xFF02
#define ACCESS_INIT       0xFF04


#define M_RD_ALL    0xFF00

#define M_WR_PUBLIC 0x0001
#define M_WR_OPEN   0x0003
#define M_WR_INIT   0x0007

#define M_WR_ALL    0x00FF

#define EVENT_RX_MESSAGES         0x0001
#define EVENT_RX_FILTER_MESSAGES  0x0002
#define EVENT_STATUS_CHANGE       0x0004
#define EVENT_PNP_CHANGE          0x0008  

// <*> neu, verschoben von api_ex.h
typedef int32_t(CALLBACK_TYPE *TCanGetDataBlockCB)(uint8_t *tx_data, uint16_t tx_limit, void *user_data);

#include "os_mhstcan.h"

#include "can_types.h"
#include "drv_types.h"
#if defined(MHSPASSTHRU_DRV)
  #include "passthru_drv.h"      
#elif defined(MHSIPCAN_DRV)
  #include "ipcan_drv.h"
#else
  #include "tcan_drv.h"
#endif

#ifdef __cplusplus
  }
#endif

#endif
