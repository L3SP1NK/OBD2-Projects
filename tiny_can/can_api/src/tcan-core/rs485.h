#ifndef __RS485_H__
#define __RS485_H__

#include "global.h"

#ifdef __cplusplus
  extern "C" {
#endif

#define RS485_MIN_CMD_SIZE 8

#define RS485_COMMAND_OPEN   1
#define RS485_COMMAND_CLOSE  2


#define RS485_REQ_RX_FINISH  0x01
#define RS485_REQ_RX_TIMEOUT 0x02
#define RS485_REQ_RX_ERROR   0x04    
#define RS485_REQ_RX_OV      0x08
 
#define RS485_REQ_TX_FINISH  0x40
#define RS485_REQ_TX_ERROR   0x80

#define RS485_STATUS_IDLE      0
#define RS485_STATUS_TX        1
#define RS485_STATUS_WAIT_RESP 2


int32_t RS485Create(TCanDevice *dev);
void RS485Destroy(TCanDevice *dev);
int32_t ProcessRS485Cmd(uint8_t **dst, uint32_t available_size, TCanDevice *dev);
int32_t ExtractRS485Data(TCanDevice *dev, uint8_t **src, int32_t *size);

int32_t RS485MsIoRw(TCanDevice *dev, TRS485MsData *tx_data, uint32_t size, uint32_t timeout);

int32_t RS485IoWrite(TCanDevice *dev, TRS485MsData *tx_data, uint32_t *size, uint32_t timeout);
int32_t RS485IoRead(TCanDevice *dev, uint8_t *rx_data, uint32_t *size, uint32_t timeout);

#ifdef __cplusplus
  }
#endif

#endif
