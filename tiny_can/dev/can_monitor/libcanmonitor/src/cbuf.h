#ifndef __C_BUFFER__
#define __C_BUFFER__


#include "can_types.h"

#define CBUF_EVENT_CLEAR       1
#define CBUF_EVENT_LOAD        2
#define CBUF_EVENT_CHANGE_DATA 3
#define CBUF_EVENT_ADD_DATA    4

struct TCanBuffer
  {
  struct TCanMsg *Data;
  unsigned int BufferSize;
  unsigned int UsedSize;
  unsigned int TimeFormat;
  char *InfoString;
  int LastEvent;
  gint MhsEventId;
  gpointer UserData;
  void(*EventProc)(struct TCanBuffer *cbuf, gpointer user_data, int last_event);
  };

typedef void(*TMhsBufferCB)(struct TCanBuffer *cbuf, gpointer user_data, int last_event);

/*extern const char FORMAT_EFF_RTR_STR[];
extern const char FORMAT_EFF_STR[];
extern const char FORMAT_STD_RTR_STR[];
extern const char FORMAT_STD_STR[];
extern const char FORMAT_INVALID[];

extern const char DIR_RX_STR[];
extern const char DIR_TX_STR[];*/

struct TCanBuffer *CBufferCreate(unsigned int buffer_size);
void CBufferDestroy(struct TCanBuffer **cbuf);
int CBufferDataAdd(struct TCanBuffer *cbuf, struct TCanMsg *msgs, unsigned int count);
int CBufferDataClear(struct TCanBuffer *cbuf);
int CBufferGetUsedSize(struct TCanBuffer *cbuf);
int CBufferGetSize(struct TCanBuffer *cbuf);
int CBufferSetSize(struct TCanBuffer *cbuf, unsigned int buffer_size);
char *CBufferGetInfoString(struct TCanBuffer *cbuf);
void CBufferSetInfoString(struct TCanBuffer *cbuf, char *info_string);
int CBufferEventConnect(struct TCanBuffer *cbuf, TMhsBufferCB proc, gpointer user_data);
void CBufferEventDisconnect(struct TCanBuffer *cbuf, TMhsBufferCB proc);
int CBufferLoadFile(struct TCanBuffer *cbuf, char *file_name);
int CBufferSaveFile(struct TCanBuffer *cbuf, char *file_name);


#endif
