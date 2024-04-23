#ifndef __TAR_DRV_H__
#define __TAR_DRV_H__

#include "global.h"
#include "com_io.h"
#include "sm_drv.h"
//#include "tcan_drv.h" <*>

#ifdef __cplusplus
  extern "C" {
#endif

#define CMD_TYPE_EXT1 0x80
#define CMD_TYPE_EXT2 0xC0

/**************************************************************************/
/*                             Kommandos                                  */
/**************************************************************************/
#define CMD2_GET_STATUS             0x40

#define CMD2_CAN0_SET               0x01 | CMD_TYPE_EXT1
#define CMD2_CAN0_GET               0x02 | CMD_TYPE_EXT1
#define CMD2_CAN0_SET_MODE          0x03 | CMD_TYPE_EXT1
#define CMD2_CAN0_RXD_IDX_CLEAR     0x06 | CMD_TYPE_EXT1
#define CMD2_CAN0_TXD_IDX_CLEAR     0x0A | CMD_TYPE_EXT1

#define CMD2_CAN1_SET               0x0D | CMD_TYPE_EXT1
#define CMD2_CAN1_GET               0x0E | CMD_TYPE_EXT1
#define CMD2_CAN1_SET_MODE          0x0F | CMD_TYPE_EXT1
#define CMD2_CAN1_RXD_IDX_CLEAR     0x12 | CMD_TYPE_EXT1
#define CMD2_CAN1_TXD_IDX_CLEAR     0x16 | CMD_TYPE_EXT1

#define CMD_IO_SET                 0x19 | CMD_TYPE_EXT1
#define CMD_Io_GET                 0x1A | CMD_TYPE_EXT1
#define CMD_IO_SETUP               0x1B | CMD_TYPE_EXT1

#define CMD_CAN_TEST_SET           0x1C | CMD_TYPE_EXT1

//#define CMD2_PING                   0x7E

//#define CMD2_START_READ_INFO        0x52
//#define CMD2_READ_INFO              0x53


#define TAR_CAN_STATUS_OK      0   // Controller Ok
#define TAR_CAN_STATUS_WARNING 1   // Error warning
#define TAR_CAN_STATUS_PASSIV  2   // Error passiv
#define TAR_CAN_STATUS_BUS_OFF 3   // Bus Off

extern const uint8_t CMD2_CANx_SET_MODE[];
extern const uint8_t CMD2_CANx_SET[];
extern const uint8_t CMD2_CANx_RXD_IDX_CLEAR[];
extern const uint8_t CMD2_CANx_TXD_IDX_CLEAR[];


/**********************************/
/* Index für CANx_SET & CANx_GET  */
/**********************************/
#define CAN_VAR_SPEED                   0
#define CAN_VAR_SPEED_BTR               1
#define CAN_VAR_TX_ACK_ENABLE           2
#define CAN_VAR_MODE                    3
#define CAN_VAR_TXD_FIFO_LIMIT          4
#define CAN_VAR_FILTER0                 5
#define CAN_VAR_FILTER1                 6
#define CAN_VAR_FILTER2                 7
#define CAN_VAR_FILTER3                 8
#define CAN_VAR_FILTER4                 9
#define CAN_VAR_FILTER5                 10
#define CAN_VAR_FILTER6                 11
#define CAN_VAR_FILTER7                 12
#define CAN_VAR_TXD_BOX0                13
#define CAN_VAR_TXD_BOX1                14
#define CAN_VAR_TXD_BOX2                15
#define CAN_VAR_TXD_BOX3                16
#define CAN_VAR_TXD_BOX4                17
#define CAN_VAR_TXD_BOX5                18
#define CAN_VAR_TXD_BOX6                19
#define CAN_VAR_TXD_BOX7                20
#define CAN_VAR_OPT                     21
#define CAN_VAR_ERROR_LOG_ENABLE        22 
#define CAN_VAR_TIMESTAMP_ENABLE        23
#define CAN_VAR_FD_FLAGS                24
#define CAN_VAR_SPEED_DBTR              25
#define CAN_VAR_FIFO_OV_MODE            26
#define CAN_VAR_CAN_CLOCK               27

#define CAN_VAR_ISO_TP_FLAGS            28
#define CAN_VAR_ISO_TP_TX_PAD_CONTENT   29
#define CAN_VAR_ISO_TP_TX_ID            30
#define CAN_VAR_ISO_TP_RX_ID            31

#define CAN_VAR_ECU_FL_DATA_OK_DLC      32
#define CAN_VAR_ECU_FL_DATA_OK_MASK     33
#define CAN_VAR_ECU_FL_DATA_OK_CODE     34
#define CAN_VAR_ECU_FL_DATA_ERROR_DLC   35
#define CAN_VAR_ECU_FL_DATA_ERROR_MASK  36
#define CAN_VAR_ECU_FL_DATA_ERROR_CODE  37

#define CAN_VAR_CAN_TEST_MODE           38
#define CAN_VAR_CAN_TEST_FILL_CHAR      39
#define CAN_VAR_CAN_TEST_MSG_FLAGS      40
#define CAN_VAR_CAN_TEST_MSG_LENGTH     41
#define CAN_VAR_CAN_TEST_MSG_ID         42
#define CAN_VAR_CAN_TEST_COUNTER        43
#define CAN_VAR_CAN_TEST_DELAY          44

/***************************************/
/*               Status                */
/***************************************/
#define TAR_CAN_RXD_PEND   0x08
#define TAR_CAN_RXD_OV     0x10

#define TAR_CAN_TXD_MASK   0x60
#define TAR_CAN_TXD_EMPTY  0x00
#define TAR_CAN_TXD_PEND   0x20
#define TAR_CAN_TXD_LIMIT  0x40
#define TAR_CAN_TXD_OV     0x60

//#define TAR_CAN_TXD_LIMIT  0x40
#define TAR_CAN_TXD_ERROR  0x80

#define TXD_STATUS_EMPTY 0
#define TXD_STATUS_PEND  1
#define TXD_STATUS_LIMIT 2
#define TXD_STATUS_OV    3

struct TTarStatus0Flags
  {
  unsigned CanBusStatusBits:3;
  unsigned CanRxDPend:1;
  unsigned CanRxDOv:1;
  unsigned CanTxDStatus:2;
  unsigned CanTxDError:1;
  };

union TTarStatus0
  {
  uint8_t Byte;
  struct TTarStatus0Flags Flag;
  };

struct TTarStatus1Flags
  {
  unsigned BusFailure:1;  
  };

union TTarStatus1
  {
  uint8_t Byte;
  struct TTarStatus1Flags Flag;
  };

struct TTarStatus
  {
  union TTarStatus0 Status;
  union TTarStatus1 ExStatus;
  };
  
    
struct TBiosVersionInfo
  {
  uint8_t Version;
  uint8_t Flags;
  uint16_t TxMTU;
  uint16_t RxMTU;
  };  


/***************************************/
/*            CAN Status               */
/***************************************/
struct TTarInfoVar
  {
  uint8_t Size;
  uint8_t Type;
  uint16_t Index;
  char Data[128];
  };

extern const uint32_t CAN_SPEED_TAB[] ATTRIBUTE_INTERNAL;

#define SetUInt32ToParam(p, i, d) do { \
  (p)[(i)] = (uint8_t)(d); \
  (p)[(i+1)] = (uint8_t)((d) >> 8); \
  (p)[(i+2)] = (uint8_t)((d) >> 16); \
  (p)[(i+3)] = (uint8_t)((d) >> 24); \
  } while(0) 


int32_t TARCommSync(TCanDevice *dev) ATTRIBUTE_INTERNAL;
int32_t TARPing(TCanDevice *dev) ATTRIBUTE_INTERNAL;
int32_t TARGetBiosVersion(TCanDevice *dev, struct TBiosVersionInfo *info) ATTRIBUTE_INTERNAL;
int32_t TARInfoStartRead(TCanDevice *dev) ATTRIBUTE_INTERNAL;
int32_t TARInfoGetNext(TCanDevice *dev, struct TTarInfoVar *info) ATTRIBUTE_INTERNAL;

int32_t TARSetFilter(TCanDevice *dev, uint32_t index, struct TMsgFilter *msg_filter) ATTRIBUTE_INTERNAL;
int32_t TARGetStatus(TCanDevice *dev) ATTRIBUTE_INTERNAL;
int32_t TARSetCANMode(TCanDevice *dev, uint8_t can_mode) ATTRIBUTE_INTERNAL;
int32_t TARCanSetTxAckEnable(TCanDevice *dev, uint8_t enable) ATTRIBUTE_INTERNAL;
int32_t TARCanSetErrorLogEnable(TCanDevice *dev, uint8_t enable) ATTRIBUTE_INTERNAL;
int32_t TARCanSetSpeed(TCanDevice *dev, uint32_t speed) ATTRIBUTE_INTERNAL;
int32_t TARCanSetBtr(TCanDevice *dev, uint32_t btr) ATTRIBUTE_INTERNAL;
int32_t TARCanSetIntervall(TCanDevice *dev, uint32_t index, uint32_t intervall) ATTRIBUTE_INTERNAL;
int32_t TARCANReset(TCanDevice *dev) ATTRIBUTE_INTERNAL;
int32_t TARRxFifoClear(TCanDevice *dev, uint32_t index) ATTRIBUTE_INTERNAL;
int32_t TARTxFifoPut(TCanDevice *dev, uint32_t index, struct TCanMsg *msgs, uint16_t count) ATTRIBUTE_INTERNAL;
int32_t TARTxFifoClear(TCanDevice *dev, uint32_t index) ATTRIBUTE_INTERNAL;
int32_t TARSetTimeStamp(TCanDevice *dev, uint8_t enabled) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
