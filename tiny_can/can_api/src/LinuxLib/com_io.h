#ifndef __COM_IO_H__
#define __COM_IO_H__

#include "global.h"
#include "mhs_obj.h"

#ifdef __cplusplus
  extern "C" {
#endif


#define ComReadChar(io, c) ComReadData(io, (c), 1)
#define ComReadByte(io, c) ComReadData(io, (char *)(c), 1)


  // **** Übertragungsrate
#define BR_110     110
#define BR_300     300
#define BR_600     600
#define BR_1200    1200
#define BR_2400    2400
#define BR_4800    4800
#define BR_9600    9600
#define BR_10400   10400
#define BR_14400   14400
#define BR_19200   19200
#define BR_28800   28800
#define BR_38400   38400
#define BR_56000   56000
#define BR_57600   57600
#define BR_115200  115200

#define BR_230400  230400
#define BR_460800  460800
#define BR_614400  614400
#define BR_921600  921600
#define BR_1228800 1228800
#define BR_3M      3000000

#define ST_CTS  0x01
#define ST_DSR  0x02
#define ST_RNG  0x04
#define ST_CAR  0x08


struct TComIoDriver
  {
  struct TIo *(*Create)(TMhsObjContext *context);
  void (*Destroy)(struct TIo **io);
  int32_t (*Open)(struct TIo *io);
  void (*Close)(struct TIo *io);
  int32_t (*IsOpen)(struct TIo *io);
  void (*FlushBuffer)(struct TIo *io);
  int32_t (*CountRx)(struct TIo *io);
  int32_t (*ReadData)(struct TIo *io, char *data, uint32_t max);
  int32_t (*WriteData)(struct TIo *io, char *data, int32_t size);
  uint32_t (*GetLineStatus)(struct TIo *io, uint32_t index);
  int32_t (*WriteDTR)(struct TIo *io, uint32_t index, uint32_t status);
  int32_t (*WriteRTS)(struct TIo *io, uint32_t index, uint32_t status);
  };


struct TIo
  {
  const struct TComIoDriver *ComIoDriver;
  // COM spezifisch
  int32_t ComPortFd;
  int32_t ComPortFdB;
  uint32_t RxTimeout;
  // Allgemein
  TMhsObjContext *Context;
  int32_t ErrorCode;
  char ErrorString[512];
  };

void ComIoDrvCreate(void) ATTRIBUTE_INTERNAL;
void ComIoDrvDestroy(void) ATTRIBUTE_INTERNAL;

struct TIo *ComIoCreate(int32_t driver_index, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
void ComIoDestroy(struct TIo **io) ATTRIBUTE_INTERNAL;
int32_t ComOpen(struct TIo *io) ATTRIBUTE_INTERNAL;
void ComClose(struct TIo *io) ATTRIBUTE_INTERNAL;
int32_t ComIsOpen(struct TIo *io) ATTRIBUTE_INTERNAL;
void ComFlushBuffer(struct TIo *io) ATTRIBUTE_INTERNAL;
int32_t ComCountRx(struct TIo *io) ATTRIBUTE_INTERNAL;
int32_t ComReadData(struct TIo *io, char *data, uint32_t max) ATTRIBUTE_INTERNAL;
int32_t ComWriteData(struct TIo *io, char *data, int32_t size) ATTRIBUTE_INTERNAL;
uint32_t ComGetLineStatus(struct TIo *io, uint32_t index) ATTRIBUTE_INTERNAL;
int32_t ComWriteDTR(struct TIo *io, uint32_t index, uint32_t status) ATTRIBUTE_INTERNAL;
int32_t ComWriteRTS(struct TIo *io,  uint32_t index, uint32_t status) ATTRIBUTE_INTERNAL;
int32_t ComWriteByte(struct TIo *io, unsigned char value) ATTRIBUTE_INTERNAL;
int32_t ComWriteChar(struct TIo *io, char value) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
