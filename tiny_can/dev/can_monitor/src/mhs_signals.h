#ifndef __MHS_SIGNALS__
#define __MHS_SIGNALS__

// Public Signals
#define SIG_CAN_STATUS_CHANGE      0x00000001L  // CAN-Bus Status hat sich geändert
#define SIG_FILTER_EDIT_FINISH     0x00000002L
#define SIG_DATA_RECORD_CHANGE     0x00000003L
#define SIG_SETUP_CHANGE           0x00000004L
#define SIG_CAN_MODUL_INIT         0x00000005L


// Protected Signals
#define SIG_MAIN_MAKRO_REPAINT     0x00000001L
#define SIG_MAIN_CHANGE_MAKRO_NAME 0x00000002L
#define SIG_MAIN_SHOWING_CHANGE    0x00000003L
#define SIG_MAKRO_DLG_REPAINT      0x00000004L

// Signal Klassen
#define SIGC_PROTECTED 1
#define SIGC_PUBLIC    2
#define SIGC_CAN       3

#endif
