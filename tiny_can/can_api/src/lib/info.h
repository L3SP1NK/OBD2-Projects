#ifndef __INFO_H__
#define __INFO_H__

#include "global.h"
//#include "tcan_drv.h" <*>

#ifdef __cplusplus
  extern "C" {
#endif

#define IT_BYTE     0x01
#define IT_UBYTE    0x02
#define IT_HBYTE    0x0A
#define IT_WORD     0x03
#define IT_UWORD    0x04
#define IT_HWORD    0x0C
#define IT_LONG     0x05
#define IT_ULONG    0x06
#define IT_HLONG    0x0E
#define IT_REVISION 0x40     // Software Version
#define IT_DATE     0x41     // Datum
#define IT_STRING   0x80

// In tcan_drv.h
/*struct TInfoVarList
  {
  struct TInfoVarList *Next;
  unsigned char Size;
  unsigned char Type;
  uint32_t Index;
  const char *IndexString;
  char *Data;
  }; */


void InfoVarDestroy(struct TInfoVarList **liste) ATTRIBUTE_INTERNAL;
struct TInfoVarList *InfoVarAdd(struct TInfoVarList **liste) ATTRIBUTE_INTERNAL;
int32_t InfoVarAddEx(TCanDevice *dev, uint32_t key, uint8_t type, char *data, uint8_t size) ATTRIBUTE_INTERNAL;
const char *GetIndexString(uint32_t index) ATTRIBUTE_INTERNAL;

struct TInfoVarList *GetInfoByIndex(TCanDevice *dev, uint32_t index) ATTRIBUTE_INTERNAL;
uint32_t GetInfoValueAsULong(struct TInfoVarList *info) ATTRIBUTE_INTERNAL;
char *GetInfoValueAsString(struct TInfoVarList *info) ATTRIBUTE_INTERNAL;
int32_t GetHardwareInfo(TCanDevice *dev, struct TCanInfoVar **info_var) ATTRIBUTE_INTERNAL;

char *CreateInfoString(struct TInfoVarList *info) ATTRIBUTE_INTERNAL;


#ifdef __cplusplus
  }
#endif

#endif

