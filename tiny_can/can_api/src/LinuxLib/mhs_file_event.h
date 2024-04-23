#ifndef __MHS_FILE_EVENT_H__
#define __MHS_FILE_EVENT_H__

#ifdef __cplusplus
  extern "C" {
#endif

struct TFileEvent
  {
  int EventPipe[2];
  };


struct TFileEvent *create_file_event(void) ATTRIBUTE_INTERNAL;
void destroy_file_event(struct TFileEvent **file_event) ATTRIBUTE_INTERNAL;
int32_t set_file_event(struct TFileEvent *file_event, unsigned char event) ATTRIBUTE_INTERNAL;
unsigned char get_file_event(struct TFileEvent *file_event) ATTRIBUTE_INTERNAL;
int32_t file_event_get_fd(struct TFileEvent *file_event) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
