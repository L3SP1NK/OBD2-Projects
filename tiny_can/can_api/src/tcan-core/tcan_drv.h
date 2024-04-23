#ifndef __TCAN_DRV_H__
#define __TCAN_DRV_H__


//#include "global.h" // <*> 
#include "os_core.h" // Definition von ATTRIBUTE_INTERNAL
#include "drv_types.h"
#include "com_io.h"
#include "var_obj.h"
#include "mhs_thread.h"
#include "mhs_event_list.h"
#include "uc_fifo.h"
//#include "api_ex.h" <*>

#ifdef __cplusplus
  extern "C" {
#endif

#define MCMD_DEVICE_CLOSE     1
#define MCMD_DEVICE_OPEN      2
#define MCMD_CAN_STOP         4
#define MCMD_CAN_START        5
#define MCMD_CAN_RESET        6
#define MCMD_CAN_ONLY_CMD     12

#define MCMD_CAN_START_LOM 14
#define MCMD_CAN_START_NO_RETRANS 15
//#define MCMD_CAN_SET_TX_ACK_ENABLE 16 <*>
#define MCMD_CAN_ECU_FLASH_MODE     16

#define DEV_LIST_SHOW_TCAN_ONLY 0x01
#define DEV_LIST_SHOW_UNCONNECT 0x02


#define TINY_CAN_TYPE_I_XL    1  
#define TINY_CAN_TYPE_II_XL   2
#define TINY_CAN_TYPE_III_XL  3
#define TINY_CAN_TYPE_M1      4
#define TINY_CAN_TYPE_M232    5
#define TINY_CAN_TYPE_IV_XL   6
#define TINY_CAN_TYPE_LS      7
#define TINY_CAN_TYPE_M2      8
#define TINY_CAN_TYPE_M3      9
#define TINY_CAN_TYPE_V_XL   10

struct TCmdSetModeParam
  {
  uint32_t Index;
  uint16_t CanCommand;
  };

struct TInfoVarList
  {
  struct TInfoVarList *Next;
  uint8_t Size;
  uint8_t Type;
  uint32_t Index;
  const char *IndexString;
  char *Data;
  };

struct TCanModulDesc
  {
  uint32_t HwId;
  char *Description;
  uint32_t BaudRate[2];
  uint32_t Flags;
  uint32_t StatusSize;
  uint32_t SupportHwTimestamp;
  uint32_t SupportTxAck;
  uint32_t SupportCanErrorMsgs;
  uint32_t CanFeaturesFlags;
  uint32_t CanChannels;
  uint32_t DigIoCount;
  uint32_t HwTxFifoSize;
  uint32_t HwTxFifoLimit;
  uint32_t HwRxFilterCount;
  uint32_t HwTxPufferCount;
  uint32_t CanClock;
  uint32_t RxMtu;
  uint32_t TinyCanType;
  };

#define SM_DRV_PARAMETER_COUNT 4096

struct TSMDrvParameter
  {
  uint8_t Data[SM_DRV_PARAMETER_COUNT];
  uint32_t Count;
  };

#ifdef ENABLE_RS485_SUPPORT
typedef struct _TRS485Port TRS485Port; 

struct _TRS485Port
  {
  volatile uint8_t Status;
  volatile uint8_t Command;
  volatile uint8_t IsOpen;
  volatile uint8_t TxStatus;
  volatile uint32_t TxSize;
  uint32_t TxOfs;
  uint8_t TxBuffer[4096];
  volatile uint8_t RxStatus;
  TCharFifo *RxFifo;
  TMhsEvent *Event;
  };
#endif


typedef struct _TMhsCmd TMhsCmd;

struct _TMhsCmd
  {
  TMhsEvent Event;
#ifdef __WIN32__  
  CRITICAL_SECTION CmdLock;
#else  
  pthread_mutex_t CmdLock;
#endif  
  uint32_t Command;
  uint32_t ParamCount;
  char *Param;
  uint32_t Result;
  };

typedef struct _TCanDevice TCanDevice;

struct _TCanDevice
  {
  // **** Device Index
  uint32_t Index;
  // **** Reference Counter
  volatile uint32_t RefCount;
  // **** Context
  TMhsObjContext *IoContext;
  TMhsObjContext *Context;
  TMhsObjContext *TxPufferContext;
  TMhsObjContext *RxFilterContext;
  // **** Kommando
  TMhsCmd *Cmd;
  // **** Main Thread
  TMhsThread *MainThread;
  struct TIo *Io;
  // **** Modul Beschreibung  
  struct TCanModulDesc *ModulDesc;
  struct TInfoVarList *InfoVars;
  char *DrvInfoHwStr;
  uint32_t HaveRtsCtsClear;
  uint32_t Protokoll;
  // **** Uebergabepuffer für Callback-Funktion
  struct TCanMsg *CanRxDBuffer;
  uint32_t CanRxDBufferSize;

  // **** Variablen
  uint32_t TimeStampMarker;
  uint8_t UseDigIo;
  uint32_t TimeNow;
  uint32_t StatusTimeStamp;
  uint32_t FilterTimeStamp;
  uint32_t HwTxDFifoCount[MAX_CAN_CHANNELS];
  uint32_t ReadStatusFlag;
  struct TCanFdMsg TxCanBuffer[HW_TX_FIFO_MAX];  
  uint32_t CommIdleCounter;
  uint8_t GlobalTxAckEnable; // <*> neu
  
  uint8_t DigOutSet;
  uint16_t DigOut[MAX_IO_CFG/2];
  uint8_t AnalogOutSet;  
  uint16_t AnalogOut[4];
  // **** Konfigurationsvariablen
  uint32_t TxCanFifoEventLimit;  
  uint8_t TimeStampMode;
  uint32_t FilterReadIntervall;
  uint32_t HighPollIntervall;
  uint32_t IdlePollIntervall;
  uint8_t CommTryCount;  
  // **** Device Status
  struct TDeviceStatus DeviceStatus;
  struct TDeviceStatus LastDeviceStatus;
  int32_t StatusChange;
  TMhsEventList *StatusEventList;
  // **** SM
  uint8_t TxDDataPuffer[SM_DRV_PARAMETER_COUNT+4];
  uint8_t Kommando;
  struct TSMDrvParameter TxDParameter;  // Sende Parameter
  struct TSMDrvParameter RxDParameter;  // Empfangene Parameter
  uint8_t ACKByte;                // Empfangenes ACK Byte
  uint8_t CanFd;
  uint16_t OvMode;
  // **** ECU Flash <*> neu
  TCanGetDataBlockCB GetDataBlockCb;
  void *GetDataBlockUserData;
  uint8_t EcuFlashMode;
  uint8_t CanDataExec;
  uint32_t IsoTpBlockSize;
  uint32_t IsoTpBlockWritten;
  uint8_t IsoTpTxBuf[MAX_ISO_TP_MSG_LENGTH];
  uint8_t *CanDataBuf;
  uint32_t CanDataBufSize;
  uint32_t CanDataBufWritten;
  int32_t CanDataStatus;
  int32_t CanDataFreeSize;
#ifdef ENABLE_RS485_SUPPORT  
  TRS485Port *RS485Port;
#endif    
  };
  
 
#ifdef HAVE_TX_ACK_BYPASS
extern uint8_t TxAckBypass; // <*> neu
#endif

extern const struct TCanModulDesc ExCanModulTable[] ATTRIBUTE_INTERNAL; 

int32_t DrvCoreInit(void) ATTRIBUTE_INTERNAL;
void DrvCoreDown(void) ATTRIBUTE_INTERNAL;
TCanDevice *CreateCanDevice(void) ATTRIBUTE_INTERNAL;
void DestroyCanDevice(TCanDevice **device) ATTRIBUTE_INTERNAL;
int32_t GetDeviceInfo(TCanDevice *dev, struct TCanDeviceInfo *device_info) ATTRIBUTE_INTERNAL;
int32_t OpenCanDevice(TCanDevice *dev) ATTRIBUTE_INTERNAL;
void CloseCanDevice(TCanDevice *dev) ATTRIBUTE_INTERNAL;
int32_t SetIntervallBox(TCanDevice *dev, uint32_t index, uint16_t cmd, uint32_t time) ATTRIBUTE_INTERNAL;

void DevStatusEventSetup(TCanDevice *dev, TMhsEvent *event, uint32_t events) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
