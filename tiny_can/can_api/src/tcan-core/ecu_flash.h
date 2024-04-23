#ifndef __ECU_FLASH_H__
#define __ECU_FLASH_H__

#include "global.h"
#include "com_io.h"
#include "sm_drv.h"
//#include "tcan_drv.h" <*>

#ifdef __cplusplus
  extern "C" {
#endif

/****************************/
/*  ECU Flash               */
/****************************/
#define ECU_DATA_CMD_CLEAR   0x01  
#define ECU_DATA_CMD_FINISH  0x02
#define ECU_DATA_CMD_CANCEL  0x03
#define ECU_DATA_FLAG_DATA   0x08

#define ECU_DATA_ST_ERROR       0x80

#define ECU_DATA_ST_RX_TIMEOUT  0x10
#define ECU_DATA_ST_RX_NACK     0x20
#define ECU_DATA_ST_OVERFLOW    0x30
#define ECU_DATA_ST_ISOTP_ERROR 0x40

#define ECU_DATA_ST_RX_ACK      0x02
#define ECU_DATA_ST_BUF_FREE    0x01

#define CAN_ECU_LAST_BLOCK  0x02
#define CAN_ECU_BLOCK_END   0x01

#define CAN_DATA_EXEC_IDLE     0
#define CAN_DATA_EXEC_START    1
#define CAN_DATA_EXEC_RUN      2
#define CAN_DATA_EXEC_CANCEL   3
#define CAN_DATA_EXEC_END_WAIT 4

#define CAN_DATA_ST_IDLE             0
#define CAN_DATA_ST_RUN              1
#define CAN_DATA_ST_FINISH           2
#define CAN_DATA_ST_ERR_ACK_TIMEOUT  -1
#define CAN_DATA_ST_ERR_NACK         -2
#define CAN_DATA_ST_ERR_OVERFLOW     -3     
#define CAN_DATA_ST_ERR_ISOTP        -4

void EcuFlashCreateDevice(TCanDevice *dev);
int32_t TAR2SetEcuFlashVars(TCanDevice *dev);

#ifdef __cplusplus
  }
#endif

#endif