#ifndef __HW_TYPES_H__
#define __HW_TYPES_H__

#include <glib.h>
#include <gtk/gtk.h>
#include "can_types.h"

#ifdef __cplusplus
  extern "C" {
#endif

#define HARDWARE_TYPE_TCAN_M2  1
#define HARDWARE_TYPE_TCAN_M3  2

#define PIN_CFG_NOT_USED 0  // 0 = Nicht verwendet
#define PIN_CFG_DIG_IN   1  // 1 = Digital Input
#define PIN_CFG_DIG_OUT  2  // 2 = Digital Output
#define PIN_CFG_AN_IN    3  // 3 = Analog Input
#define PIN_CFG_AN_OUT   4  // 4 = Analog Output
#define PIN_CFG_KEY      5  // 5 = Taster
#define PIN_CFG_SWITCH   6  // 6 = Schalter
#define PIN_CFG_ENCODER  7  // 7 = Encoder

#define PIN_IN          0x01
#define PIN_OUT         0x02
#define PIN_IN_OUT      0x03
#define PIN_A_IN        0x04
#define PIN_A_OUT       0x08
#define PIN_KEY         0x10
#define PIN_SW          0x20
#define PIN_ENC_A       0x40
#define PIN_ENC_B       0x80


struct TKeyPannel
  {
  unsigned int Zeile;
  unsigned int Spalte;
  unsigned int ZeileCount;
  unsigned int SpalteCount;
  };
    

struct TIoPinDefs
  {
  unsigned int Pin;
  unsigned int Flags;
  unsigned int Zeile;
  unsigned int Spalte;
  const char *CfgName;
  const char *InName;
  const char *OutName;
  const char *KeyName;
  const char *SwitchName;
  const char *AnalogName;
  };

struct TIoPin
  {
  int Mode;
  gchar *Name;
  int DefaultName;
  int In;
  int AIn;
  GtkWidget *Frame;  // <*> alt EventBox
  GtkWidget *OutWidget;
  GtkWidget *InBoxWidget;
  GtkWidget *InWidget;
  GtkWidget *AInBoxWidget;
  GtkWidget *AInWidget;
  GtkWidget *AOutBoxWidget;
  GtkWidget *AOutWidget;
  GtkWidget *ConfigBoxWidget;
  GtkWidget *ConfigWidget;
  GtkWidget *DisableWidget;

  GtkWidget *InNameWidget;
  GtkWidget *AInNameWidget;
  GtkWidget *AOutNameWidget;
  };

struct TCanHw
  {
  uint32_t DeviceIndex;
  char *HardwareInfoStr;
  guint CanPortCount;
  guint ConnectedHardwareType;  
  guint HardwareType;
  
  GtkWidget *BaseWdg;
  GtkWidget *KeyList;
  guint TableRows;
  guint TableColumns; 
  guint IoPinsCount;
  struct TIoPin IoPin[100];
  const struct TIoPinDefs *IoPinDefs;
  const struct TKeyPannel *KeyPannel; 
  guint RunningLightIndex;
  guint RunningLightDelay;
  gboolean RunningLight;
  };

#ifdef __cplusplus
  }
#endif

#endif
