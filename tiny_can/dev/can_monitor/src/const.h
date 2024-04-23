#ifndef __CONST_H__
#define __CONST_H__

#include <gtk/gtk.h>
#include "about_dlg.h"

#define SPLASH_FIRST _("Programm wird gestartet")
#define SPLASH_LOAD_SETUP _("Setup, Makros und Filter laden")
#define SPLASH_SCAN_PLUGINS _("Plugins laden")

/* extern const char OPEN_DLG_MAKRO_TITLE[];
extern const char SAVE_DLG_MAKRO_TITLE[];
extern const char OPEN_DLG_FILTER_TITLE[];
extern const char SAVE_DLG_FILTER_TITLE[];
extern const char OPEN_DLG_DATA_TITLE[];
extern const char SAVE_DLG_DATA_TITLE[];

extern const char STATUS_DRV_NOT_LOAD[];
extern const char STATUS_NOT_INIT_MSG[];
extern const char STATUS_INT_AUTO_MSG[];
extern const char STATUS_INT_MSG[];
extern const char STATUS_PORT_NOT_OPEN[];
extern const char STATUS_HW_INIT_MSG[];
extern const char STATUS_ONLINE_MSG[];
extern const char STATUS_BUSY_MSG[];

extern const char STATUS_NONE_MSG[];

extern const char STATUS_CAN_OK_MSG[];
extern const char STATUS_CAN_ERROR_MSG[];
extern const char STATUS_CAN_WARNING_MSG[];
extern const char STATUS_CAN_PASSIV_MSG[];
extern const char STATUS_CAN_BUS_OFF_MSG[];
extern const char STATUS_CAN_UNBEKANNT_MSG[];

extern const char STATUS_RECORD_START_MSG[];
extern const char STATUS_RECORD_STOP_MSG[];
extern const char STATUS_RECORD_OV_MSG[];
extern const char STATUS_RECORD_LIMIT_MSG[];

extern const char STATUS_10_KBIT_MSG[];
extern const char STATUS_20_KBIT_MSG[];
extern const char STATUS_50_KBIT_MSG[];
extern const char STATUS_100_KBIT_MSG[];
extern const char STATUS_125_KBIT_MSG[];
extern const char STATUS_250_KBIT_MSG[];
extern const char STATUS_500_KBIT_MSG[];
extern const char STATUS_800_KBIT_MSG[];
extern const char STATUS_1_MBIT_MSG[];

extern const char INFO_WIN_TITLE[];
extern const char INFO_TITLE_HARDWARE[];
extern const char INFO_TITLE_DRIVER[];

extern const char SETUP_DRV_NOT_LOAD[];
extern const char SETUP_DRV_LOAD[];

extern const char SETUP_ERR_DRV_LOAD[];
extern const char SETUP_ERR_NO_DRV_LOAD[];
extern const char SETUP_ERR_NO_PORT_SETUP[]; */

extern const struct TSplashData SplashData;

extern const struct TAboutTxt AboutTxt;


#define SENSITIVE_LISTE_SIZE 18

extern GtkWidget **SensitiveListe[SENSITIVE_LISTE_SIZE];
extern unsigned char SelectAllOffListe[];
extern unsigned char SelectAllOnListe[];
extern unsigned char SelectAllOnMakroOffListe[];
extern unsigned char SelectCanOffListe[];
extern unsigned char SelectAllCanOffListe[];


//extern const char *ModulNames[];

#endif
