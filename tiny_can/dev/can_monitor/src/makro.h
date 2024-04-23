#ifndef __MAKRO_H__
#define __MAKRO_H__

#include "can_types.h"


#define STANDART_MAKRO              0x00000001L
#define PLUGIN_MAKRO                0x80000002L

#define PLUGIN_MAKRO_EDIT_SHOW      0x40000000L
#define PLUGIN_MAKRO_EDIT_INT_EN    0x20000000L
#define PLUGIN_MAKRO_EDIT_INT_TIME  0x10000000L
#define PLUGIN_MAKRO_EDIT_MSG       0x08000000L

#define MAKRO_ALL                   0x0000000FL
#define MAKRO_SHOW                  0x00000004L


/**************************************************************************/
/*                       D A T E N - T Y P E N                            */
/**************************************************************************/
struct TMakro
  {
  struct TMakro *Next;
  struct TMakro *Prev;
  char Name[50];
  struct TCanMsg CanMsg; 
  unsigned long Flags; 
  struct TPlugin *Plugin;
  unsigned long IntervallTime;
  unsigned char IntervallEnabled;
  };


/**************************************************************************/
/*                         V A R I A B L E N                              */
/**************************************************************************/
extern struct TMakro *Makro;


/**************************************************************************/
/*                        F U N K T I O N E N                             */
/**************************************************************************/
int MakCreate(void);
void MakDestroy(void);

void MakClearListe(unsigned long makse);
void MakClearPlugins(struct TPlugin *plugin);
void MakRemove(struct TMakro *makro);

struct TMakro *MakCreateNew(void);
int MakAddNew(struct TMakro *makro);
void MakCreateNewEnd(struct TMakro *makro);
void MakChangeShowing(struct TMakro *makro);
void MakRemove(struct TMakro *makro);
int MakMoveBefore(struct TMakro *makro);
int MakMoveAfter(struct TMakro *makro);
struct TMakro *MakGetLast(void);
struct TMakro *MakGetFirst(void);
unsigned int MakGetLength(unsigned long makse);
void MakSetIntervall(void);

int MakroReadFromFile(char *file_name);
int MakroWriteToFile(char *file_name);

int MakSetByPlugin(struct TPlugin *plugin, char *name, struct TCanMsg *msg, unsigned long intervall);
int MakGetByPlugin(struct TPlugin *plugin, char *name, struct TCanMsg *msg, unsigned long *intervall);
int MakClearByPlugin(struct TPlugin *plugin, char* name);
int MakSendByPlugin(struct TPlugin *plugin, char *name, unsigned long index);

#endif
