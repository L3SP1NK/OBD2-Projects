#ifndef __MAIN_H__
#define __MAIN_H__

#include <glib.h>
#include "cview.h"
#include "can_types.h"
#include "can_speed_db.h"

#define SELECT_ALL_ON      1
#define SELECT_CAN_OFF     2
#define SELECT_ALL_CAN_OFF 3
#define SELECT_ALL_OFF     4

#define DATA_RECORD_START 1
#define DATA_RECORD_STOP  2
#define DATA_RECORD_OV    3
#define DATA_RECORD_LIMIT 4

//#define CheckEventMSendenWin()     MainEventsMask & 0x0001
#define CheckEventMMakBearbeiten() MainEventsMask & 0x0002
#define CheckEventMFilMessages()   MainEventsMask & 0x0004
#define CheckEventButtonLeisten()  MainEventsMask & 0x0008
#define CheckEventCanStartStop()   MainEventsMask & 0x0010

//#define DisableEventMSendenWin()     MainEventsMask |= 0x0001
#define DisableEventMMakBearbeiten() MainEventsMask |= 0x0002
#define DisableEventMFilMessages()   MainEventsMask |= 0x0004
#define DisableEventButtonLeisten()  MainEventsMask |= 0x0008
#define DisableEventCanStartStop()   MainEventsMask |= 0x0010

//#define EnableEventMSendenWin()     MainEventsMask &= (~0x0001)
#define EnableEventMMakBearbeiten() MainEventsMask &= (~0x0002)
#define EnableEventMFilMessages()   MainEventsMask &= (~0x0004)
#define EnableEventButtonLeisten()  MainEventsMask &= (~0x0008)
#define EnableEventCanStartStop()   MainEventsMask &= (~0x0010)


struct TMainVar
  {
  //unsigned char SendenWindow;
  int CanTransmitEntrysCount;
  unsigned char MakroWindow;
  unsigned char FilterWindow;
  };

struct TMainWin
  {
  GtkWidget *MainWin;
  // Menue: Datei
  GtkWidget *MDatei_menu;
  GtkWidget *MDatei;
  GtkWidget *MNeu;
  GtkWidget *MOpen;
  GtkWidget *MSave;
  GtkWidget *MDateiInfo;
  GtkWidget *MBeenden;
  // Menue: CAN
  GtkWidget *MCan_menu;
  GtkWidget *MCan;
  GtkWidget *MCanStartStop;
  GtkWidget *MCanReset;
  // Menue: Makro
  GtkWidget *MMakro;
  GtkWidget *MMakro_menu;
  GtkWidget *MMakBearbeiten;
  GtkWidget *MMakOpen;
  GtkWidget *MMakSave;
  // Menue: Filter
  GtkWidget *MFilter_menu;
  GtkWidget *MFilter;
  GtkWidget *MFilMessages;
  GtkWidget *MFilBearbeiten;
  GtkWidget *MFilOpen;
  GtkWidget *MFilSave;
  // Meneu: Plugins
  GtkWidget *MPlugins_menu;
  GtkWidget *MPlugins;
  // Menue: Ansicht
  GtkWidget *MAnsicht_menu;
  GtkWidget *MAnsicht;
  GtkWidget *MStandartLeiste;
  GtkWidget *MSendenLeiste;
  GtkWidget *MMakroLeiste;
  GtkWidget *MButtonsText;
  // Menue: Optionen
  GtkWidget *MOptionen_menu;
  GtkWidget *MOptionen;
  GtkWidget *MEinstellungen;
  GtkWidget *MPluginsSetup;
  GtkWidget *MConnect;
  GtkWidget *MDeviceInfo;
  // Menue: Hilfe
  GtkWidget *MHilfe_menu;
  GtkWidget *MHilfe;
  GtkWidget *MInfo;

  // Standart Leiste
  GtkWidget *NeuButton;
  GtkWidget *FileOpenButton;
  GtkWidget *FileSaveButton;
  GtkWidget *BeendenButton;
  GtkWidget *CanStartStopButton;
  GtkWidget *CanResetButton;
  GtkWidget *EinstellungenButton;

  GtkWidget *MainToolbar;

  GtkWidget *MakrosFrame;
  GtkWidget *MakroListView;
  GtkWidget *MakrosToolbar;
  GtkWidget *MakroSendenButton;
  GtkWidget *MakroPasteButton;

  GtkWidget *CanTxWindow;
  GtkWidget *Statusbar;

  GtkWidget *CanDataView;
  GtkWidget *CanFilterDataView;

  struct TCanView *CanView;
  struct TCanBuffer *CanBuffer;
  struct TCanView *CanFilterView;
  };


extern struct TMainWin MainWin;

extern struct TMainVar MainVar;
extern unsigned short MainEventsMask;
extern unsigned int ProgAktion;
extern unsigned int DataRecord;

extern struct TMhsSignal *MhsCanViewSignals;

extern struct TCanSpeedList *UserCanSpeedList;

void MakroChangeEvent(void);

//void SetSendenWindow(void);
void SetMakroWindow(void);
void DeleteSendenWidget(void);
void CreateSendenWidget(int used_lines);
void SetDataRecord(void);


#endif
