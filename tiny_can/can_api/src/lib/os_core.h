#ifndef __OS_CORE_H__
#define __OS_CORE_H__

#ifdef __cplusplus
  extern "C" {
#endif

#if defined(WIN32) || defined(_WIN32)
  #ifndef __WIN32__
    #define __WIN32__
  #endif
#endif

#ifdef __GNUC__
  #ifndef _GNU_SOURCE
    #define _GNU_SOURCE
  #endif
  
  #ifndef M_FORCE_INLINE
    #define M_FORCE_INLINE static __inline__ __attribute__((__always_inline__,__gnu_inline__)) 
  #endif
#else
  #ifndef M_FORCE_INLINE
    #define M_FORCE_INLINE static __forceinline
  #endif 
#endif

#ifdef __WIN32__
// Windows
  #if !defined(WINVER)
    #define WINVER 0x0500
  #endif
  #if !defined(_WIN32_IE)
    #define _WIN32_IE 0x0501
  #endif
  #include <windows.h>
  #if defined(__GNUC__) || (defined(_MSC_VER) && _MSC_VER >= 1900)
    #include <stdint.h>
  #else
    typedef __int32           int32_t;
    typedef unsigned __int32  uint32_t;
    typedef __int16           int16_t;
    typedef unsigned __int16  uint16_t;
    typedef __int8            int8_t;
    typedef unsigned __int8   uint8_t;
  #endif
#else
// Linux & Mac
  #include <stdint.h>
#endif

#define API_CALL  __stdcall

#ifdef __WIN32__
  // ****** Windows
  #define ATTRIBUTE_INTERNAL
  #define EXPORT_FUNC __declspec(dllexport)  
#elif __APPLE__
  // ****** Apple
  #define ATTRIBUTE_INTERNAL __attribute((visibility("hidden")))
  #define EXPORT_FUNC
#else
  // ****** Linux
  #define ATTRIBUTE_INTERNAL __attribute((visibility("internal")))
  #define EXPORT_FUNC  
#endif

#ifdef WINCE

struct TTime
  {
  uint32_t tv_sec;
  uint32_t tv_usec;
  };

  #define timeval TTime
#endif


#ifdef __WIN32__
  // ****** Windows
  #include <stdlib.h>
  
  #ifndef __FUNCTION__
    #define __FUNCTION__ "FUNCTION"
  #endif

  #define CALLBACK_TYPE __stdcall  //CALLBACK
  #define DRV_LOCK_TYPE CRITICAL_SECTION
  #define DRV_LOCK_INIT(x) do { \
     if (!(x)) \
       { \
       (x) = malloc(sizeof(CRITICAL_SECTION)); \
       InitializeCriticalSection((x)); \
       } \
     } while(0)
  #define DRV_LOCK_DESTROY(x) do { \
     if ((x)) \
       { \
       DeleteCriticalSection((x)); \
       free((x)); \
       (x) = NULL; \
       } \
     } while(0)
  #define DRV_LOCK_ENTER(x) do {if (x) EnterCriticalSection((x));} while(0)
  #define DRV_LOCK_LEAVE(x) do {if (x) LeaveCriticalSection((x));} while(0)
  #define mhs_sleep(x) Sleep(x)
  #define get_tick() GetTickCount()
#else
  // ****** Linux
  #include <pthread.h>
  #include <unistd.h>
  #include <stdlib.h>
  #include <sys/time.h>

  #define CALLBACK_TYPE
  #define DRV_LOCK_TYPE pthread_mutex_t

  #define DRV_LOCK_INIT(x) do { \
     if (!(x)) \
       { \
       (x) = malloc(sizeof(pthread_mutex_t)); \
       pthread_mutex_init((x), NULL); \
       } \
     } while(0)
   
  #define DRV_LOCK_DESTROY(x) do { \
     if ((x)) \
       { \
       pthread_mutex_destroy((x)); \
       free((x)); \
       (x) = NULL; \
       } \
     } while(0) 
  #define DRV_LOCK_ENTER(x) do {if (x)  pthread_mutex_lock((x));} while(0)
  #define DRV_LOCK_LEAVE(x) do {if (x)  pthread_mutex_unlock((x));} while(0)
  #define mhs_sleep(x) usleep((x) * 1000)

  #define FT_DEVICE_BM       0
  #define FT_DEVICE_AM       1
  #define FT_DEVICE_100AX    2
  #define FT_DEVICE_UNKNOWN  3
  #define FT_DEVICE_2232C    4
  #define FT_DEVICE_232R     5
  #define FT_DEVICE_2232H    6
  #define FT_DEVICE_4232H    7
  #define FT_DEVICE_232H     8
  #define FT_DEVICE_X_SERIES 9
#endif

#ifdef __GNUC__

//#pragma GCC diagnostic push
//  #pragma GCC diagnostic ignored "-Wstringop-truncation" <*>
//#pragma GCC diagnostic pop
//  #define safe_strcpy(d, m, s) strncpy((d), (s), (m))<*>
  #define safe_sprintf snprintf
  #define safe_vsprintf vsnprintf
#endif

#ifdef __WIN32__
  #ifndef __GNUC__
    #if defined(_MSC_VER) && _MSC_VER >= 1500
   //   #define safe_strcpy strcpy_s <*>  
      #define safe_sprintf(dst, dst_max, format, ...) _snprintf_s(dst, dst_max, _TRUNCATE, format, __VA_ARGS__)
      #define safe_vsprintf(dst, dst_max, format, argptr) _vsnprintf_s(dst, dst_max, _TRUNCATE, format, argptr)
      //#define safe_vsprintf _vsnprintf
    #else
   /*   #define safe_min(a,b) (((a) < (b)) ? (a) : (b)) <*>
      #define safe_strcp(dst, dst_max, src, count) do {memcpy(dst, src, safe_min(count, dst_max)); \
  	  ((char*)dst)[safe_min(count, dst_max)-1] = 0;} while(0)
      #define safe_strcpy(dst, dst_max, src) do { \
        if (!src) \
          *dst = '\0'; \
        else \
          safe_strcp(dst, dst_max, src, strlen(src)+1); \
        } while(0) */
      #define safe_sprintf _snprintf
      #define safe_vsprintf _vsnprintf  
    #endif
  #endif  
#endif


#ifdef __cplusplus
  }
#endif

#endif 
