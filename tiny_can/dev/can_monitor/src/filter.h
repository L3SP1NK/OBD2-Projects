#ifndef __FILTER_H__
#define __FILTER_H__

#include "can_types.h"

#define STANDART_FILTER           0x00000001L
#define PLUGIN_FILTER             0x00000002L

#define PLUGIN_FILTER_EDIT_AKTIV  0x40000000L
#define PLUGIN_FILTER_EDIT_SHOW   0x20000000L
#define PLUGIN_FILTER_EDIT_HW     0x10000000L
#define PLUGIN_FILTER_EDIT_PASS   0x08000000L
#define PLUGIN_FILTER_EDIT_FILTER 0x04000000L

#define FILTER_ALL                0x0000000FL
#define FILTER_VISIBLE            0x00000010L
#define FILTER_ENABLED            0x00000004L
#define FILTER_SHOW               0x00000008L

/**************************************************************************/
/*                       D A T E N - T Y P E N                            */
/**************************************************************************/
struct TFilter
  {
  struct TFilter *Next;
  struct TFilter *Prev;
  char Name[50];
  unsigned long Flags;
  struct TPlugin *Plugin;
  // unsigned char Enabled;
  unsigned char UpdateMsg;
  unsigned char ChangeMsg;
  unsigned char UpdateParam;
  unsigned char HardwareFilter;
  unsigned char PassMessage;
  unsigned char IdMode;
  unsigned char FormatEff;
  unsigned long Id1;
  unsigned long Id2;
  unsigned long Index;
  struct TCanMsg CanMsg;
  };



/**************************************************************************/
/*                         V A R I A B L E N                              */
/**************************************************************************/
//extern struct TFilter *Filter;


/**************************************************************************/
/*                        F U N K T I O N E N                             */
/**************************************************************************/
int FilCreate(void);
void FilDestroy(void);

void FilClearListe(unsigned long maske);
void FilClearPlugins(struct TPlugin *plugin);
void FilRemove(struct TFilter *filter);

struct TFilter *FilCreateNew(void);
int FilAddNew(struct TFilter *filter);
int FilChange(struct TFilter *filter, struct TFilter *new_filter);

int FilMoveBefore(struct TFilter *filter);
int FilMoveAfter(struct TFilter *filter);

struct TFilter *FilGetLast(void);
struct TFilter *FilGetFirst(void);

unsigned int FilGetLength(unsigned long maske);

struct TFilter *FilGetByPos(unsigned long maske, unsigned long pos);
struct TFilter *FilGetByIdx(unsigned long index);

void FilCreateIndexList(void);
unsigned long FilGetNewIndex(unsigned long can_kanal, unsigned char hw_filter);
int FilSetToDriver(unsigned int all);

struct TFilter *FilReadUpdMessage(struct TFilter *filter, struct TCanMsg *msg);
struct TFilter *FilWriteMessageByIdx(unsigned long index, struct TCanMsg *msg);

int FilterReadFromFile(char *file_name);
int FilterWriteToFile(char *file_name);

int FilSetByPlugin(struct TPlugin *plugin, char *name, struct TFilter *filter);
int FilGetByPlugin(struct TPlugin *plugin, char *name, struct TFilter *filter);
int FilClearByPlugin(struct TPlugin *plugin, char *name);

void FilEditingFinish(void);

#endif
