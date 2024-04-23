#ifndef __TCAN_COM2_H__
#define __TCAN_COM2_H__

#include "global.h"
//#include "tcan_drv.h" <*>
#include "tar_drv2.h"

#ifdef __cplusplus
  extern "C" {
#endif

#define DATA_STREAM_COMMAND_FLAG       0x80

#define DATA_STREAM_CMD_TIME_STAMP     (DATA_STREAM_COMMAND_FLAG | 0x01)  // Time-Stamp
#define DATA_STREAM_CMD_CAN_STATUS     (DATA_STREAM_COMMAND_FLAG | 0x02)  // CAN Status
#define DATA_STREAM_CMD_TX_CAN_STATUS  (DATA_STREAM_COMMAND_FLAG | 0x03)  // TxCAN Status
#define DATA_STREAM_CMD_GET_KEYS       (DATA_STREAM_COMMAND_FLAG | 0x06)
#define DATA_STREAM_CMD_GET_AD_IN      (DATA_STREAM_COMMAND_FLAG | 0x07)  // Read Analog Inputs
#define DATA_STREAM_CMD_RS485          (DATA_STREAM_COMMAND_FLAG | 0x08)  // RS485 I/O
#define DATA_STREAM_CMD_SET_DIG_OUT    (DATA_STREAM_COMMAND_FLAG | 0x10)  // Set Digital Outputs
#define DATA_STREAM_CMD_GET_DIG_IN     (DATA_STREAM_COMMAND_FLAG | 0x20)  // Read Digital Inputs
#define DATA_STREAM_CMD_GET_ENC        (DATA_STREAM_COMMAND_FLAG | 0x30)
#define DATA_STREAM_CMD_SET_AD_OUT     (DATA_STREAM_COMMAND_FLAG | 0x40)
#define DATA_STREAM_CMD_DATA           (DATA_STREAM_COMMAND_FLAG | 0x50)
#define DATA_STREAM_CMD_TX_PUFFER_MSG  (DATA_STREAM_COMMAND_FLAG | 0x60)
#define DATA_STREAM_CMD_FILTER_MSG     (DATA_STREAM_COMMAND_FLAG | 0x70)  // Filter Message

void SetTar2StatusToDev(TCanDevice *dev, struct TTar2Status *tar_status) ATTRIBUTE_INTERNAL;

int32_t MainComm2Proc(TCanDevice *dev, uint32_t mode) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
