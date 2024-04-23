#ifndef __HEX_ENTRY_H__
#define __HEX_ENTRY_H__


#define EDIT_MODE_HEX     0
#define EDIT_MODE_DEZIMAL 1
#define EDIT_MODE_BIN     2
#define EDIT_MODE_CHAR    3

#define EDIT_MASK_HEX     0x01
#define EDIT_MASK_DEZIMAL 0x02
#define EDIT_MASK_BIN     0x04
#define EDIT_MASK_CHAR    0x08

#define EDIT_MASK_ALL     0x0F

#define EDIT_SIZE_8_BIT  0
#define EDIT_SIZE_12_BIT 1
#define EDIT_SIZE_16_BIT 2
#define EDIT_SIZE_24_BIT 3
#define EDIT_SIZE_32_BIT 4

#define EDIT_EVENT_RETURN        0x0001
#define EDIT_EVENT_CHANGE_VALUE  0x0002
#define EDIT_EVENT_FOUCUS_OUT    0x0004



struct THexEntry
  {
  GtkWidget *BaseWdg;
  GtkWidget *Frame;  
  GtkWidget *Hbox;
  GtkWidget *EventBox;
  GtkWidget *Label;
  GtkWidget *Entry;
  int EditMode;
  int EditModeOld;
  char StrValue[50];
  int ValueWidth;
  int EditMask;
  unsigned long Value;
  unsigned long EventFlags;
  unsigned long ChangeValue;
  gpointer UserData;
  void (*EventProc)(struct THexEntry *he, int event, unsigned long value, gpointer user_data);
  };

typedef void (*THexEntryEventCB)(struct THexEntry *he, int event, unsigned long value, gpointer user_data);

struct THexEntry *HexEntryNew(GtkWidget *parent, unsigned long value, int value_width, int edit_mode, 
  int edit_mask, unsigned int event_flags, THexEntryEventCB event_proc, gpointer user_data);
void HexEntryDestroy(struct THexEntry *he);
void HexEntrySetEditMode(struct THexEntry *he, unsigned int mode);
void HexEntrySetColor(struct THexEntry *he, GdkColor *text_color, GdkColor *bg_color);
void HexEntrySetValueWidth(struct THexEntry *he, int value_width);
unsigned long HexEntryGetValue(struct THexEntry *he);
void HexEntrySetValue(struct THexEntry *he, unsigned long value);

void HexEntryCreateCursor(struct THexEntry *he);
void HexEntrySetEntryWidth(struct THexEntry *he);
void HexEntryWriteValue(struct THexEntry *he, unsigned long value);
void HexEntryPrintValue(struct THexEntry *he);
void HexEntryReadValueEx(struct THexEntry *he, unsigned int auto_edit_mode, unsigned int reprint);
void HexEntryRepaintMode(struct THexEntry *he);
int HexEntryDetectMode(char **str, int edit_mask);

#endif
