#ifndef __TCAN_SETUP__
#define __TCAN_SETUP__

#ifdef __cplusplus
  extern "C" {
#endif

#define CAN_VAR_OBJ                   0x80000000
#define IVAR_OBJ                      0x40000000

#define FILTER_READ_INTERVALL         0x00010000  
#define HIGH_POLL_INTERVALL           0x00020000
#define IDLE_POLL_INTERVALL           0x00030000
#define TX_CAN_FIFO_EVENT_LIMIT       0x00040000
#define COMM_TRY_COUNT                0x00080000

#define IO_VAR_CFG                    0x01000000
#define IO_VAR_PORT_OUT               0x02000000
#define IO_VAR_PORT_ANALOG_OUT        0x04000000

#define SETUP_LOCAL       0x80

#define SETUP_SET_ALL     0x01   
#define SETUP_CAN_STOP    0x02
#define SETUP_CAN_CLEANUP 0x04

// IVar
#define IVAR_D_SPEED      0x01

int32_t ProcessSetup(TCanDevice *dev, uint32_t mode);

#ifdef __cplusplus
  }
#endif

#endif