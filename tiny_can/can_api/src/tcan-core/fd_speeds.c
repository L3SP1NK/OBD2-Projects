/***************************************************************************
                        tcan_drv.c  -  description
                             -------------------
    begin             : 18.02.2020
    last modify       : 28.10.2022
    copyright         : (C) 2020 - 2022 by MHS-Elektronik GmbH & Co. KG, Germany
    author            : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/

/**
    Library to talk to Tiny-CAN devices. You find the latest versions at
       http://www.tiny-can.com/
**/
#include "global.h"
#include "util.h"
#include "info.h"
#include "speeds_db.h"
#include "fd_speeds.h"


static const struct TSpeedDesc *GetSpeedDbEntry(uint32_t *clocks, uint16_t speed, uint8_t *clock_idx)
{
const struct TSpeedDesc *db, *hit;
uint8_t i, clk_idx;
uint32_t clock, db_speed, max_rank;

hit = NULL;
max_rank = 0;
clk_idx = 0;
for (i = 0; ((i < 10) && (clock = *clocks++)); i++)
  {
  for (db = &DefSpeedTab[0]; (db_speed = db->NomSpeed); db++)
    {
    if ((db_speed == speed) && (db->Clock == clock) && (db->Ranking > max_rank))
      {
      max_rank = db->Ranking;
      clk_idx = i;
      hit = db;
      break;
      }
    }  
  }
if (clock_idx)  
  *clock_idx = clk_idx;  
return(hit);  
}


static const struct TFdSpeedDesc *GetFdSpeedDbEntry(uint32_t *clocks, uint16_t speed, uint16_t fd_speed, uint8_t *clock_idx)
{
const struct TFdSpeedDesc *db, *hit;
uint8_t i, clk_idx;
uint32_t clock, db_speed, max_rank;

hit = NULL;
max_rank = 0;
clk_idx = 0;
for (i = 0; ((i < 10) && (clock = *clocks++)); i++)
  {
  for (db = &DefFdSpeedTab[0]; (db_speed = db->NomSpeed); db++)
    {
    if ((db_speed == speed) && ( db->DataSpeed == fd_speed) && (db->Clock == clock) && (db->Ranking > max_rank))
      {
      max_rank = db->Ranking;
      clk_idx = i;
      hit = db;
      break;
      }
    }  
  }
if (clock_idx)  
  *clock_idx = clk_idx;  
return(hit);  
}
  
 
static int32_t GetCanClocks(TCanDevice *dev, uint32_t *clocks)
{ 
struct TInfoVarList *info;
uint8_t idx, can_clock_cnt;
 
if (!(info = GetInfoByIndex(dev, TCAN_INFO_KEY_CAN_CLOCKS)))
  return(-1);  
can_clock_cnt = (uint8_t)GetInfoValueAsULong(info);
if (can_clock_cnt > 10)
  return(-1);
for (idx = 0; idx < can_clock_cnt; idx++)
  {
  //if (!(info = GetInfoByIndex(dev, TCAN_INFO_KEY_FW_CAN_CLOCK1 + idx))) <*>
  if (!(info = GetInfoByIndex(dev, TCAN_INFO_KEY_CAN_CLOCK1 + idx)))
    break;
  if (!(clocks[idx] = GetInfoValueAsULong(info)))
    break;  
  }  
for (; idx < 10; idx++)
  clocks[idx] = 0;
return(can_clock_cnt);
}

/* <*>
int32_t GetNBtrValueFromSpeed(TCanDevice *dev, uint8_t *clk_idx, uint32_t *nbtr, uint16_t speed)
{
uint32_t clocks[10];
uint8_t idx;
const struct TSpeedDesc *speed_entry;

if ((!clk_idx) || (!nbtr))
  return(-1);
if (GetCanClocks(dev, &clocks) < 0)
  return(-1);  
if ((speed_entry = GetSpeedDbEntry(&clocks, speed, &idx)))
  {
  *clk_idx = idx;
  *nbtr = speed_entry->NBTR;
  return(0);
  }
return(-1);  
} */


int32_t GetNbtrDBtrValueFromSpeeds(TCanDevice *dev, uint8_t *clk_idx, uint32_t *nbtr, uint32_t *dbtr,
    uint16_t speed, uint16_t fd_speed)
{
uint32_t clocks[10];
uint8_t idx;
const struct TFdSpeedDesc *fd_speed_entry;
const struct TSpeedDesc *speed_entry;

if ((!clk_idx) || (!nbtr) || (!dbtr))
  return(-1);
if (GetCanClocks(dev, &clocks[0]) < 0)
  return(-1);  
if (fd_speed)
  {  
  if ((fd_speed_entry = GetFdSpeedDbEntry(&clocks[0], speed, fd_speed, &idx)))
    {  
    *clk_idx = idx;
    *nbtr = fd_speed_entry->NBTR;        
    *dbtr = fd_speed_entry->DBTR;
    return(0);
    }
  }
else
  {
  if ((speed_entry = GetSpeedDbEntry(&clocks[0], speed, &idx)))
    {
    *clk_idx = idx;
    *nbtr = speed_entry->NBTR;
    return(0);
    }
  }  
        
return(-1);
}