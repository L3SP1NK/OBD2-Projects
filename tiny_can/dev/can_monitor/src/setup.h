#ifndef __SETUP_H__
#define __SETUP_H__

#include <glib.h>
#include "drv_win.h"
#include "can_speed_win.h"

#define SETUP_SHOW_CAN_SPEED 0x0001
#define SETUP_SHOW_DATA_LIST 0x0002
#define SETUP_SHOW_ANSICHT   0x0004
#define SETUP_SHOW_FILTER    0x0008
#define SETUP_SHOW_SENDEN    0x0010
#define SETUP_SHOW_HARDWARE  0x0020
#define SETUP_SHOW_DRIVER    0x0040

struct TSetupData
  {// Abschnitt: Hardware
  unsigned short CanSpeedMode; // 0 = Use CAN Speed
                               // 1 = Use UserCanSpeed1/2
  unsigned short CanSpeed;     // 10   = 10kBit/s
                               // 20   = 20kBit/s
                               // 50   = 50kBit/s
                               // 100  = 100 kBit/s
                               // 125  = 125 kBit/s
                               // 250  = 250 kBit/s
                               // 500  = 500 kBit/s
                               // 800  = 800 kBit/s
                               // 1000 = 1 MBit/s
  unsigned long UserCanSpeed;
  unsigned long CanOpMode;     // 0 = Normal
                               // 1 = Automatic Retransmission disable
                               // 2 = Silent Mode (LOM = Listen only Mode)

  unsigned char DataClearMode; // 0 = Automatisch löschen
                               // 1 = Benutzer fragen
                               // 2 = nicht löschen
  unsigned long RxDLimit;
  char *MakroFile;
  char *FilterFile;
  char *DriverPath;
  char *DriverFile;
  unsigned char PortIndex;     // 0 = COM 1
                               // 1 = COM 2
                               // 2 = COM 3
                               // 3 = COM 4
  unsigned char FilterTimer;   // 0 = 100 ms
                               // 1 = 250 ms
                               // 2 = 500 ms
                               // 3 = 1s
  unsigned char AnsStandart;
  unsigned char AnsMakro;
  unsigned char AnsSenden;
  unsigned char AnsButtonText;
  unsigned char AutoConnect;
  char SnrEnabled;
  char *Snr;
  int ComDrvMode;
  unsigned long BaudRate;
  /* unsigned char TermOn;
  unsigned char HighSpeedOn; */
  unsigned char CanPortNumber;
  unsigned char USBEnabled;
  unsigned char USBAuto;
  // unsigned char ModulIndex;
  char *ComDeviceName;
  char *DriverInitStr;
  // CAN-View Setup
  GdkColor WdgTimeColor;
  GdkColor WdgTypeColor;
  GdkColor WdgIdColor;
  GdkColor WdgDlcColor;
  GdkColor WdgData1Color;
  GdkColor WdgData2Color;
  gchar *WdgFontName;
  unsigned char ShowTime;
  unsigned char ShowDir;
  unsigned char ShowType;
  unsigned char ShowDlc;
  unsigned char ShowData1;
  unsigned char ShowData2;
  int SendenEntrysCount;
  int SendOnReturn;
  int FilterFrame;
  };


struct TDataListSetupWin
  {
  GtkWidget *Base;
  GSList *DatenListe_group;
  GtkWidget *DatenListe[3];
  GtkObject *DatenLimitEdit_adj;
  GtkWidget *DatenLimitEdit;
  };


struct TAnsichtSetupWin
  {
  GtkWidget *Base;
  GtkWidget *FontSel;
  GtkWidget *IdColorBtn;
  GtkWidget *DlcColorBtn;
  GtkWidget *Data1ColorBtn;
  GtkWidget *Data2ColorBtn;
  GtkWidget *TimeColorBtn;
  GtkWidget *TypeColorBtn;
  GtkWidget *TimeStampBox;
  GtkWidget *DirShowCheck;
  GtkWidget *TypeShowCheck;
  GtkWidget *DlcShowCheck;
  GtkWidget *Data1Box;
  GtkWidget *Data2Box;
  };


struct TFilterSetupWin
  {
  GtkWidget *Base;
  GSList *FilterIntervall_group;
  GtkWidget *FilterIntervall[4];
  };


struct TSendenSetupWin
  {
  GtkWidget *Base;
  GtkObject *TxEntryCountEdit_adj;
  GtkWidget *TxEntryCountEdit;
  GtkWidget *SendOnReturnCheck;
  };


struct THardwareSetupWin
  {
  GtkWidget *Base;
  GtkWidget *AutoConnectSwitch;
  GtkWidget *SnrFrame;
  GtkWidget *SnrEnableSwitch;
  GtkWidget *SnrEdit;
#ifdef __WIN32__
  GtkWidget *ComDrvMode[2];
  GSList *ComDrvMode_group;
  GtkWidget *Rs232Frame;
  GtkWidget *PortBox;
  GtkWidget *BaudRateBox;
#else
  GtkWidget *PortSetupButton;
#endif
  GtkWidget *DriverInitStrEdit;
  };


struct TSetupWin
  {
  GtkWidget *SetupDlg;
  struct TCanSpeedWin *CanSpeedWin;
  struct TDataListSetupWin *DataListWin;
  struct TAnsichtSetupWin *AnsichtWin;
  struct TFilterSetupWin *FilterWin;
  struct TSendenSetupWin *SendenWin;
  struct THardwareSetupWin *HardwareWin;
  struct TDriverSetupWin *DriverWin;

  GtkWidget *CancelButton;
  GtkWidget *OkButton;
  };


extern struct TSetupData SetupData;

void SetupInit(void);
void SetupDestroy(void);
void SetupSetDefaults(void);
int LoadSetup(void);
int SaveSetup(void);
void SetSetup(void);
void ExecuteSetup(void);

#endif
