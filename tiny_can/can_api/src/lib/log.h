#ifndef __LOG_H__
#define __LOG_H__

#include "global.h"
#include "can_types.h"
#ifdef MHSTCAN_DRV
  #include "tar_drv.h"
  #include "usb_hlp.h"
#endif

#ifdef __cplusplus
  extern "C" {
#endif

#define LOG_FUNCTION_ERROR(function, res) do {  \
  if ((res < 0) && (LogEnable)) \
    LogPrintf(LOG_ERROR , "[ERROR] function \"%s\" returns with error: %d", function, res); \
  } while(0)
  
#define LOG_FUNCTION_ERROR_STR(function, res, str) do {  \
  if ((res < 0) && (LogEnable)) \
    LogPrintf(LOG_ERROR , "[ERROR] function \"%s\" returns with error: %d [%s]", function, res, str); \
  } while(0)  

#define LOG_API(function, res) do { \
  if (LogEnable) \
    { \
    if (res < 0) \
      LogPrintf(LOG_ERROR | LOG_API_CALL, "API-Call: %s, result: %s", function, GetErrorString(res)); \
    else \
      LogPrintf(LOG_API_CALL, "API-Call: %s, result: Ok", function); \
    } \
  } while(0)

#define LOG_API_VOID(function) do { \
  if (LogEnable) \
    LogPrintf(LOG_API_CALL, "API-Call: %s", function); \
  } while(0)

#define LOG_API_ENTER(function) do { \
  if (LogEnable) \
    LogPrintf(LOG_API_CALL, "API-Call Enter: %s", function); \
  } while(0)


#define LOG_API_ENTER_IDX(function, idx) do { \
  if (LogEnable) \
    LogPrintf(LOG_API_CALL, "API-Call Enter: %s, index: %#08X", function, idx); \
  } while(0)


#define LOG_API_EXIT_COUNT(function, res) do { \
  if (LogEnable) \
    { \
    if (res < 0) \
      LogPrintf(LOG_ERROR | LOG_API_CALL, "API-Call Exit: %s, result: %s", function, GetErrorString(res)); \
    else \
      LogPrintf(LOG_API_CALL, "API-Call Exit: %s, count: %d", function, res); \
    } \
  } while(0)

#define LOG_API_EXIT_VALUE(function, res, value) do { \
  if (LogEnable) \
    { \
    if (res < 0) \
      LogPrintf(LOG_ERROR | LOG_API_CALL, "API-Call Exit: %s, result: %s", function, GetErrorString(res)); \
    else \
      LogPrintf(LOG_API_CALL, "API-Call Exit: %s, value: %u", function, value); \
    } \
  } while(0)

#define LOG_API_EXIT_STR_VALUE(function, res, value) do { \
  if (LogEnable) \
    { \
    if (res < 0) \
      LogPrintf(LOG_ERROR | LOG_API_CALL, "API-Call Exit: %s, result: %s", function, GetErrorString(res)); \
    else \
      LogPrintf(LOG_API_CALL, "API-Call Exit: %s, value: %s", function, value); \
    } \
  } while(0)

#define LOG_API_ENTER_STR_LIST(function, desc, strlist) do { \
  if (LogEnable) \
    LogStringList(LOG_API_CALL, strlist, "API-Call Enter: %s, %s", function, desc); \
 } while(0)


#define LOG_API_EXIT(function, res) do { \
  if (LogEnable) \
    { \
    if (res < 0) \
      LogPrintf(LOG_ERROR | LOG_API_CALL, "API-Call Exit: %s, result: %s", function, GetErrorString(res)); \
    else \
      LogPrintf(LOG_API_CALL, "API-Call Exit: %s, result: Ok", function); \
    } \
  } while(0)


#define LOG_API_STR_LIST(name, list) do { \
  if (LogEnable) \
    LogStringList(LOG_API_CALL, list, "%s", name); \
  } while(0)


#ifdef ENABLE_LOG_SUPPORT
extern uint32_t LogEnable ATTRIBUTE_INTERNAL;
#else
#define LogEnable 0
#endif

#define LOG_FLAG_TYPE      0x01
#define LOG_FLAG_FD        0x02
#define LOG_FLAG_TIMESTAMP 0x04
#define LOG_FLAG_SOURCE    0x08

#define LOG_FMT_TYPE_SOURCE     (LOG_FLAG_SOURCE | LOG_FLAG_TYPE)
#define LOG_FMT_FD_TYPE         (LOG_FLAG_TYPE | LOG_FLAG_FD)
#define LOG_FMT_FD_TYPE_SOURCE  (LOG_FLAG_SOURCE | LOG_FLAG_TYPE | LOG_FLAG_FD)


void LogInit(void) ATTRIBUTE_INTERNAL;

void LogDestroy(void) ATTRIBUTE_INTERNAL;
int32_t LogCreate(char *filename, uint32_t flags) ATTRIBUTE_INTERNAL;

void LogPrintf(uint32_t log_type, const char *fmt, ...) ATTRIBUTE_INTERNAL;
void LogPrintfUnlocked(const char *fmt, ...) ATTRIBUTE_INTERNAL;

void LogCanMessage(uint32_t log_type, struct TCanMsg *msg, uint32_t format, const char *fmt, ...) ATTRIBUTE_INTERNAL;
void LogCanMessages(uint32_t log_type, struct TCanMsg *msg, int32_t count, uint32_t format, const char *fmt, ...) ATTRIBUTE_INTERNAL;
void LogCanFdMessage(uint32_t log_type, struct TCanFdMsg *msg, uint32_t format, const char *fmt, ...) ATTRIBUTE_INTERNAL;
void LogCanFdMessages(uint32_t log_type, struct TCanFdMsg *msg, int32_t count, uint32_t format, const char *fmt, ...) ATTRIBUTE_INTERNAL;

void LogStringList(uint32_t log_type, const char *str_list, const char *fmt, ...) ATTRIBUTE_INTERNAL;

/*void LogSysError(char *file_name, char *func_name, int32_t error_no) ATTRIBUTE_INTERNAL; <*>
void LogHexDump(uint32_t log_type, uint32_t log_flags, char *data, int32_t count) ATTRIBUTE_INTERNAL;*/
void LogPrintDrvStatus(uint32_t log_type, uint32_t channel, struct TDeviceStatus *dev_status) ATTRIBUTE_INTERNAL;
void LogPrintUsbDevices(uint32_t log_type, struct TCanDevicesList *list, int32_t num_devs) ATTRIBUTE_INTERNAL;

void LogPrintHwInfo(uint32_t log_type, struct TCanInfoVar *info, uint32_t size) ATTRIBUTE_INTERNAL;
void LogPrintDeviceInfo(uint32_t log_type, struct TCanDeviceInfo *info) ATTRIBUTE_INTERNAL;                            

#ifdef MHSTCAN_DRV
void LogProtokollInfo(uint32_t log_type, TCanDevice *dev, struct TBiosVersionInfo *version_info) ATTRIBUTE_INTERNAL;
void LogPrintTCanUsbList(uint32_t log_type, struct TTCanUsbList *list, const char *fmt, ...);
#endif



#ifdef __cplusplus
  }
#endif

#endif
