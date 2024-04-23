#ifndef __SM_DRV_H__
#define __SM_DRV_H__

#include "global.h"
//#include "tcan_drv.h" <*>

#ifdef __cplusplus
  extern "C" {
#endif

                                         // alt Version 4.6.2 
#define ERR_SM_PORT_NOT_OPEN         -501 // -100
#define ERR_SM_TX_CMD                -502 // -101
#define ERR_SM_RX_ACK                -503 // -102
#define ERR_SM_TX_DATA               -504 // -103
#define ERR_SM_RX_DATA               -505 // -104


#define ERR_SM_RX_ACK_TIMEOUT        -506 // -1
#define ERR_SM_RX_PARAM_CNT          -507 // -2
#define ERR_SM_RX_PARAM_CNT_TIMEOUT  -508 // -3
#define ERR_SM_RX_PARAM              -509 // -4
#define ERR_SM_RX_PARAM_TIMEOUT      -510 // -5
#define ERR_SM_RX_PARAM_COUNT        -511 // -6
#define ERR_SM_RX_CRC                -512 // -7
#define ERR_SM_RX_CRC_TIMEOUT        -513 // -8
#define ERR_SM_WRONG_CRC             -514 // -9
#define ERR_SM_WRONG_ACK             -515 // -9

#define ERR_SM_NACK                  -520              

int32_t SMCheckRtsCtsClear(TCanDevice *dev) ATTRIBUTE_INTERNAL;
int32_t SMDrvClear(TCanDevice *dev) ATTRIBUTE_INTERNAL;
int32_t SMDrvCommando(TCanDevice *dev, uint32_t sleep_time) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
