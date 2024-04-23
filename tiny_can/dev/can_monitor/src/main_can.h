#ifndef __MAIN_CAN_H__
#define __MAIN_CAN_H__

#include "can_drv.h"
#include "mhs_obj.h"

//#define CAN_PORT_MASK   0x3000
//#define CAN_PORT_A      0x0000
//#define CAN_PORT_B      0x1000

#define CAN_FEATURE_LOM     0x0001 // Silent Mode (LOM = Listen only Mode)
#define CAN_FEATURE_ARD     0x0002 // Automatic Retransmission disable
#define CAN_FEATURE_TX_ACK  0x0004

/**************************************************************************/
/*                     D E F I N E - M A C R O S                          */
/**************************************************************************/
#define ERR_CAN_CONNECT_FATAL -1
#define ERR_CAN_LOAD_DRIVER   -1
#define ERR_CAN_PC_PORT_OPEN_ERROR -1

#define CAN_MODUL_NOT_CONNECT -1
#define ERR_CAN_NOT_START -1

#define TINY_CAN_DRIVER_REOPEN 1
#define TINY_CAN_DRIVER_REINIT 2
#define TINY_CAN_DRIVER_RELOAD 3


/**************************************************************************/
/*                       D A T E N - T Y P E N                            */
/**************************************************************************/
struct TCanModul
  {
  unsigned int Online;
  struct TDeviceStatus DeviceStatus;
  unsigned int Id;
  unsigned int IntPufferCount;
  unsigned int FilterCount;
  unsigned int CanPortCount;
  unsigned int CanFeaturesFlags;
  struct TDriverInfo *DriverInfo;
  char *HardwareInfoStr;
  char *FileName;
  char *CanDeviceOpenStr;
  char *CanInitDriverStr;
  };


/**************************************************************************/
/*                         V A R I A B L E N                              */
/**************************************************************************/
extern struct TCanModul CanModul;


/**************************************************************************/
/*                        F U N K T I O N E N                             */
/**************************************************************************/
int SetupCanSetMode(unsigned long index, unsigned char can_op_mode, unsigned short can_command);

void TinyCanInit(void);
void TinyCanDestroy(void);
void TinyCanDisconnect(void);
int TinyCanConnect(unsigned int mode);

void SetIntervall(void);
void SetFilter(void);
void ClearData(void);

void SyncCanBufferFull(struct TMhsObj *obj, gpointer user_data);
void SyncCanPnPEvent(struct TMhsObj *obj, int status, gpointer user_data);
void SyncCanStatusEvent(struct TMhsObj *obj, struct TDeviceStatus *device_status, gpointer user_data);
void SyncRxDFilterEvent(struct TMhsObj *obj, gpointer user_data);

struct TCanMsg *FilGetMsgCB(unsigned long index);
char *FilGetNameCB(unsigned long index);

#endif