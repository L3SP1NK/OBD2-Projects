#ifndef __CAN_SPEED_DB_H__
#define __CAN_SPEED_DB_H__


struct TCanSpeedList
  {
  struct TCanSpeedList *Next;
  char *Description;
  char *ShortDescription;
  unsigned long Value;
  };


void CanSpeedDBDestroy(struct TCanSpeedList **list);
char *ExtractItemString(char **instr);
struct TCanSpeedList *CanSpeedDBOpen(char *file_name);

char *CanSpeedDBGetShortDesciption(struct TCanSpeedList *list, unsigned long value);

#endif
