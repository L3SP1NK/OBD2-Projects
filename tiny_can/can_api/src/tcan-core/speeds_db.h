#ifndef __SPEEDS_DB_H__
#define __SPEEDS_DB_H__

#include "global.h"

#ifdef __cplusplus
  extern "C" {
#endif

struct TSpeedDesc
  {
  uint32_t NomSpeed;
  uint32_t Clock;
  uint32_t Ranking;
  uint32_t NBTR;
  };
  
struct TFdSpeedDesc
  {
  uint32_t NomSpeed;
  uint32_t DataSpeed;
  uint32_t Clock;
  uint32_t Ranking;
  uint32_t NBTR;
  uint32_t DBTR;
  };
  
extern const struct TSpeedDesc DefSpeedTab[] ATTRIBUTE_INTERNAL; 
extern const struct TFdSpeedDesc DefFdSpeedTab[] ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif