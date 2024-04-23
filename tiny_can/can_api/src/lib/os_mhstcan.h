#ifndef __OS_MHSTCAN_H__
#define __OS_MHSTCAN_H__

#ifdef __cplusplus
  extern "C" {
#endif

#define TCAN_DRV_NOT_INIT 0
#define TCAN_DRV_STD_INIT 1
#define TCAN_DRV_EX_INIT  2
#define TCAN_DRV_FD_INIT  3


extern int32_t DriverInit ATTRIBUTE_INTERNAL; // <*> 05.04.21 extern ergänzt

#ifdef __cplusplus
  }
#endif

#endif
