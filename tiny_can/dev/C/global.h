#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#ifdef WIN32
  #ifndef __WIN32__
    #define __WIN32__
  #endif
#endif


#ifdef __WIN32__
  #define COM1 "Port=1"
  #define COM2 "Port=2"
  #define COM3 "Port=3"
  #define COM4 "Port=4"
#else
  #define COM1 "ComDeviceName=/dev/ttyS0"
  #define COM2 "ComDeviceName=/dev/ttyS1"
  #define COM3 "ComDeviceName=/dev/ttyS2"
  #define COM4 "ComDeviceName=/dev/ttyS3"
#endif

#define USB     0
#define SERIELL 1

#ifndef PUBLIC_INIT_STR
  #define PUBLIC_INIT_STR ""
#endif

#ifdef __WIN32__
  // **** Windows
  #if COM_MODE == USB
    #ifdef HW_SNR
      #define DEVICE_OPEN "Snr="HW_SNR
    #endif
  #else
    #define DEVICE_OPEN COM_PORT";BaudRate="BAUD_RATE
  #endif
  #define TREIBER_NAME NULL
  #include "windows.h"
  #define CALLBACK_TYPE CALLBACK
  #define DRV_LOCK_TYPE CRITICAL_SECTION
  #define DRV_LOCK_INIT(x) InitializeCriticalSection((x))
  #define DRV_LOCK_DESTROY(x) DeleteCriticalSection((x))
  #define DRV_LOCK_ENTER(x) EnterCriticalSection((x))
  #define DRV_LOCK_LEAVE(x) LeaveCriticalSection((x))

  #define UtilInit()
  #define KeyHit kbhit
#else
  // **** Linux
  #if COM_MODE == USB
    #ifdef HW_SNR
      #define DEVICE_OPEN "Snr="HW_SNR
    #endif
  #else
    #define DEVICE_OPEN COM_PORT";BaudRate="BAUD_RATE
  #endif
  #define TREIBER_NAME NULL
/*  #ifdef USE_CODEBLOCKS
    #ifdef __APPLE__
      #define TREIBER_NAME "./../../../../../../can_api/libmhstcan.dylib"
    #else
      #define TREIBER_NAME "./../../../../../../can_api/libmhstcan.so"
    #endif
  #else
    #ifdef __APPLE__
      #define TREIBER_NAME "./../../../can_api/libmhstcan.dylib"
    #else
      #define TREIBER_NAME "./../../../can_api/libmhstcan.so"
    #endif 
  #endif */
  #include <unistd.h>
  #include "linux_util.h"

  #define getch getchar

  #define Sleep(x) usleep((x) * 1000)
  #define CALLBACK_TYPE
  #define DRV_LOCK_TYPE pthread_mutex_t
  #define DRV_LOCK_INIT(x) pthread_mutex_init((x), NULL);
  #define DRV_LOCK_DESTROY(x) pthread_mutex_destroy((x))
  #define DRV_LOCK_ENTER(x) pthread_mutex_lock((x))
  #define DRV_LOCK_LEAVE(x) pthread_mutex_unlock((x))

#endif

#ifndef TREIBER_INIT
#define TREIBER_INIT NULL
#endif

#ifndef DEVICE_OPEN
#define DEVICE_OPEN NULL
#endif


#endif
