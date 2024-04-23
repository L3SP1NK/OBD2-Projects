/***************************************************************************
                          rs485.c  -  description
                             -------------------
    begin             : 11.07.2022
    last modify       : 23.07.2022    
    copyright         : (C) 2022 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include "uc_fifo.h"
#include "log.h"
#include "mhs_event.h"
#include "errors.h"
#include "var_obj.h"
#include "tcan_com2.h"
#include "rs485.h"

#ifndef ENABLE_RS485_SUPPORT
  #error define ENABLE_RS485_SUPPORT when link rs485.c
#endif

#ifndef ENABLE_EX_IO_SUPPORT
  #error define ENABLE_EX_IO_SUPPORT when link rs485.c
#endif



/*

#######################################################################################################################
#                                                     K O M M A N D O                                                 #
#######################################################################################################################

RS485 Data Write - Short Frame
==============================
 Byte |    Bit 7    |    Bit 6    |    Bit 5    |    Bit 4    |    Bit 3    |    Bit 2    |    Bit 1    |    Bit 0    |
      +=============+=============+=============+=============+=============+=============+=============+=============+
   1  |No CAN Frame |                                     Kommando                                                    |
      |      1      |                                  0x08 = RS485 I/O                                               |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+      
   2  |           RS485 Commando = 0x1 (Short Frame)          |                Reserved = 0x0                         |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+
   3  |                                                Data Bytes in Frame                                            |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+
   4  |                                                                                                               |
      +-----------                                       Rx Data Size                                      -----------+
   5  |                                                                                                               |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+
   6  |                                               CAN Daten Bytes                                                 |
      +-----------                                                                                         -----------+
   .. |                                                    . . . .                                                    |
      +-----------                                                                                         -----------+
   n  |                                                    . . . .                                                    |
      +=============+=============+=============+=============+=============+=============+=============+=============+


RS485 Data Write - Start of Frame
================================= 
 Byte |    Bit 7    |    Bit 6    |    Bit 5    |    Bit 4    |    Bit 3    |    Bit 2    |    Bit 1    |    Bit 0    |
      +=============+=============+=============+=============+=============+=============+=============+=============+
   1  |No CAN Frame |                                     Kommando                                                    |
      |      1      |                                  0x08 = RS485 I/O                                               |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+      
   2  |         RS485 Commando = 0x2 (Start of Frame)         |                Reserved = 0x0                         |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+
   3  |                                                Data Bytes in Frame                                            |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+
   4  |                                                                                                               |
      +-----------                                      Total Data Size                                    -----------+
   5  |                                                                                                               |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+
   6  |                                                                                                               |
      +-----------                                       Rx Data Size                                      -----------+
   7  |                                                                                                               |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+
   8  |                                               CAN Daten Bytes                                                 |
      +-----------                                                                                         -----------+
   .. |                                                    . . . .                                                    |
      +-----------                                                                                         -----------+
   n  |                                                    . . . .                                                    |
      +=============+=============+=============+=============+=============+=============+=============+=============+


RS485 Data Write - Continues Frame
==================================
 Byte |    Bit 7    |    Bit 6    |    Bit 5    |    Bit 4    |    Bit 3    |    Bit 2    |    Bit 1    |    Bit 0    |
      +=============+=============+=============+=============+=============+=============+=============+=============+
   1  |No CAN Frame |                                     Kommando                                                    |
      |      1      |                                  0x08 = RS485 I/O                                               |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+      
   2  |         RS485 Commando = 0x3 (Continuous Frame)       |                Reserved = 0x0                         |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+
   3  |                                                Data Bytes in Frame                                            |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+
   4  |                                               CAN Daten Bytes                                                 |
      +-----------                                                                                         -----------+
   .. |                                                    . . . .                                                    |
      +-----------                                                                                         -----------+
   n  |                                                    . . . .                                                    |
      +=============+=============+=============+=============+=============+=============+=============+=============+


RS485 Schnittstelle öffnen
==========================
 Byte |    Bit 7    |    Bit 6    |    Bit 5    |    Bit 4    |    Bit 3    |    Bit 2    |    Bit 1    |    Bit 0    |
      +=============+=============+=============+=============+=============+=============+=============+=============+
   1  |No CAN Frame |                                     Kommando                                                    |
      |      1      |                                  0x08 = RS485 I/O                                               |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+      
   2  |       RS485 Commando = 0xF (RS485_CMD_INTERFACE)      |                      Open = 0x1                       |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+
   3  |                                                    Baudrate                                                   |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+
   4  |                                       Flags (Parity, Daten Bits, Stop Bits)                                   |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+
   5  |                                                                                                               |
      +-----------                                                                                         -----------+
   6  |                                                                                                               |
      +-----------                                    RX Timeout in µS                                     -----------+
   7  |                                                                                                               |
      +-----------                                                                                         -----------+
   8  |                                                                                                               |   
      +=============+=============+=============+=============+=============+=============+=============+=============+


RS485 Schnittstelle schließen
=============================
 Byte |    Bit 7    |    Bit 6    |    Bit 5    |    Bit 4    |    Bit 3    |    Bit 2    |    Bit 1    |    Bit 0    |
      +=============+=============+=============+=============+=============+=============+=============+=============+
   1  |No CAN Frame |                                     Kommando                                                    |
      |      1      |                                  0x08 = RS485 I/O                                               |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+      
   2  |       RS485 Commando = 0xF (RS485_CMD_INTERFACE)      |                      Close = 0x0                      |   
      +=============+=============+=============+=============+=============+=============+=============+=============+


#######################################################################################################################
#                                                          A C K                                                      #
####################################################################################################################### 

RS485 Data Read
===============
 Byte |    Bit 7    |    Bit 6    |    Bit 5    |    Bit 4    |    Bit 3    |    Bit 2    |    Bit 1    |    Bit 0    |
      +=============+=============+=============+=============+=============+=============+=============+=============+
   1  |No CAN Frame |                                     Kommando                                                    |
      |      1      |                                  0x08 = RS485 I/O                                               |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+      
   2  |                                                 Response Type                                                 |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+
   3  |                                                  Data Bytes                                                   |
      +-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+
   4  |                                               CAN Daten Bytes                                                 |
      +-----------                                                                                         -----------+
   .. |                                                    . . . .                                                    |
      +-----------                                                                                         -----------+
   n  |                                                    . . . .                                                    |
      +=============+=============+=============+=============+=============+=============+=============+=============+
*/

#define RS485_CMD_MASK              0xF0

#define RS485_CMD_SHORT_FRAME       0x10
#define RS485_CMD_START_OF_FRAME    0x20
#define RS485_CMD_CONTINUOUS_FRAME  0x30
#define RS485_CMD_INTERFACE         0xF0

#define RS485_RX_STATUS_EVENT         0x01
#define RS485_TX_STATUS_ERROR_EVENT   0x02

static const uint32_t RS485_BADRATE_INIT = {9600};
static const uint8_t RS485_DATA_BITS_INIT = {1};
static const uint8_t RS485_STOP_BITS_INIT = {1};
static const uint8_t RS485_PARITY_INIT = {0};
static const uint32_t RS485_RX_TIMEOUT_INIT = {100000};  // 100 ms

static const struct TValueDescription RS485PortDeviceValues[] = {
// Name                    | Alias | Type     | Flags |MaxSize| Access               | Default
  {"RS485Baudrate",          NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,   &RS485_BADRATE_INIT},
  {"RS485DataBits",          NULL,   VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,   &RS485_DATA_BITS_INIT},
  {"RS485StopBits",          NULL,   VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,   &RS485_STOP_BITS_INIT},
  {"RS485Parity",            NULL,   VT_UBYTE,     0,      0,   M_RD_ALL | M_WR_ALL,   &RS485_PARITY_INIT},
  {"RS485RxTimeout",         NULL,   VT_ULONG,     0,      0,   M_RD_ALL | M_WR_ALL,   &RS485_RX_TIMEOUT_INIT},
  {NULL,                     NULL,   0,            0,      0,   0,                      NULL}};


const uint32_t TrueSpeedValuesTab[] = {1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200,
                                       10400, 125000, 153600, 230400, 250000, 460800, 500000, 921600, 
                                       1000000, 2000000, 3000000, 4000000, 6000000, 8000000, 12000000, 0};


static uint8_t RS485GetBaudrate(TCanDevice *dev)
{
uint8_t idx;
uint32_t b, baud;

baud = mhs_value_get_as_ulong("RS485Baudrate", ACCESS_PORT_OPEN, dev->Context);
for (idx = 0; (b = TrueSpeedValuesTab[idx]); idx++)
  {
  if (b == baud)
    return(idx + 1);
  }
return(4); // Default idx=4 -> 9600 Baud  
}


/*
Flags Bits
       0-1 : 0 = Parity keine
             1 = Parity Gerade
             2 = Parity Ungerade
        2  : 0 = 8 Daten Bits
             1 = 7 Daten Bits
        3  : 0 = 1 Stop Bit
             1 = 2 Stop Bits
*/             
static uint8_t RS485GetCfgFlags(TCanDevice *dev)  
{
TMhsObjContext *context;
uint8_t flags, data_bits, stop_bits, parity;

context = dev->Context;
data_bits = mhs_value_get_as_ubyte("RS485DataBits", ACCESS_PORT_OPEN, context);
stop_bits = mhs_value_get_as_ubyte("RS485StopBits", ACCESS_PORT_OPEN, context); 
parity = mhs_value_get_as_ubyte("RS485Parity", ACCESS_PORT_OPEN, context);
if (parity < 3)
  flags = parity & 0x03;
else
  flags = 0;
if (data_bits == 7)
  flags |= 0x04;
if (stop_bits == 2)
  flags |= 0x08;
return(flags);      
}


int32_t RS485Create(TCanDevice *dev)
{
int32_t err;
TRS485Port *rs485port;

rs485port = (TRS485Port *)mhs_malloc0(sizeof(TRS485Port));
dev->RS485Port = rs485port;
err = mhs_values_create_from_list(dev->Context, RS485PortDeviceValues);
rs485port->Event = mhs_event_create();
mhs_event_set_event_mask(rs485port->Event, 0xFFFFFFFF);
rs485port->RxFifo = char_fifo_create(4096);
return(err);
}


void RS485Destroy(TCanDevice *dev)
{
TRS485Port *rs485port;

rs485port = dev->RS485Port;
char_fifo_destroy(rs485port->RxFifo);
mhs_event_destroy(&rs485port->Event);
safe_free(dev->RS485Port);
}


int32_t ProcessRS485Cmd(uint8_t **dst, uint32_t available_size, TCanDevice *dev)
{
TRS485Port *rs485port;
uint32_t tx_size, ofs;
uint8_t *data;
uint8_t cmd, n;

if (!(rs485port = dev->RS485Port))
  return(0);  
if ((cmd = rs485port->Command))
  {
  rs485port->Command = 0;
  data = *dst;
  *data++ = DATA_STREAM_CMD_RS485;
  switch (cmd)
    {
    case RS485_COMMAND_OPEN :
        {     
        tx_size = 8;   
        *data++ = RS485_CMD_INTERFACE | RS485_COMMAND_OPEN;
        *data++ = RS485GetBaudrate(dev);  // Baudrate 
        *data++ = RS485GetCfgFlags(dev);  // Flags
        *(uint32_t *)data = mhs_value_get_as_ulong("RS485RxTimeout", ACCESS_PORT_OPEN, dev->Context);         
        data += 4;                        
        rs485port->IsOpen = 1;
        break;
        }
    case RS485_COMMAND_CLOSE :
        {
        tx_size = 2;
        *data++ = RS485_CMD_INTERFACE | RS485_COMMAND_CLOSE;
        rs485port->IsOpen = 0;                        
        break;
        }
    }
  *dst = data;  
  return(tx_size);  
  }    
if (!rs485port->IsOpen)
  return(0);      
if (!(tx_size = rs485port->TxSize))
  return(0);
data = *dst;  
if (ofs = rs485port->TxOfs)
  {
  if (available_size < 4)
    return(0);
  available_size -= 3;  
  if (tx_size > available_size)
    n = (uint8_t)available_size;
  else
    n = (uint8_t)tx_size;   
  *data++ = DATA_STREAM_CMD_RS485;
  *data++ = RS485_CMD_CONTINUOUS_FRAME;
  *data++ = n;
  memcpy(data, &rs485port->TxBuffer[ofs], n);
  data += n;
  if ((rs485port->TxSize -= n))
    rs485port->TxOfs += n;
  else
    rs485port->TxOfs = 0;  
  *dst = data;               
  return(3 + n);  
  }
else
  {  
  if ((tx_size <= 255) && (available_size >= (tx_size + 5)))
    {
    // Short Frame
    *data++ = DATA_STREAM_CMD_RS485;
    *data++ = RS485_CMD_SHORT_FRAME;
    *data++ = (uint8_t)tx_size;
    // Rx Data Size
    *(uint16_t *)data = 0;
    data += 2;    
    memcpy(data, rs485port->TxBuffer, tx_size);
    data += tx_size;
    rs485port->TxSize = 0;
    rs485port->TxOfs = 0;
    *dst = data;
    return(5 + tx_size);
    }
  else
    {      
    // Start Frame
    if (available_size < 8)
      return(0);
    available_size -= 7;  
    *data++ = DATA_STREAM_CMD_RS485;
    *data++ = RS485_CMD_START_OF_FRAME;
    *data++ = (uint8_t)available_size;      
    // Total Data Size  
    *(uint16_t *)data = (uint16_t)tx_size;
    data += 2;
    // Rx Data Size
    *(uint16_t *)data = 0;
    data += 2;
    memcpy(data, rs485port->TxBuffer, available_size);
    data += available_size;
    rs485port->TxSize = tx_size - available_size;
    rs485port->TxOfs = available_size;
    *dst = data;
    return(7 + tx_size);
    }
  }  
return(0);    
}


int32_t ExtractRS485Data(TCanDevice *dev, uint8_t **src, int32_t *size)
{
TRS485Port *rs485port;
uint8_t *data;
int32_t count;
uint8_t n, resp_type;

if ((!size) || (!src) || (!(rs485port = dev->RS485Port)))
  return(ERR_FATAL_INTERNEL_ERROR);
count = *size;
data = *src;
data++;                       // Header überspringen
if (count < 3)
  return(ERR_COMM_STREAM_SIZE);  
resp_type = *data++;

if (resp_type & RS485_REQ_TX_ERROR)
  rs485port->TxStatus = RS485_TX_STATUS_ERROR; 
else if (resp_type & RS485_REQ_TX_FINISH)
  rs485port->TxStatus = RS485_TX_STATUS_FINISH;
  
if (resp_type & RS485_REQ_RX_FINISH) 
  rs485port->RxStatus = RS485_RX_STATUS_FINISH;
else if (resp_type & RS485_REQ_RX_TIMEOUT) 
  rs485port->RxStatus = RS485_RX_STATUS_TIMEOUT;
else if (resp_type & RS485_REQ_RX_ERROR) 
  rs485port->RxStatus = RS485_RX_STATUS_ERROR; 
else if (resp_type & RS485_REQ_RX_OV)  
  rs485port->RxStatus = RS485_RX_STATUS_OV;
  
n = *data++;
count -= 3;
if (count < n)
  return(ERR_COMM_STREAM_SIZE);
if (char_fifo_write(rs485port->RxFifo, data, n) != n)
  rs485port->RxStatus = RS485_RX_STATUS_OV;
 
data += n;  
count -= n;    
*src = data;   
*size = count;  
if (rs485port->TxStatus == RS485_TX_STATUS_ERROR)
  mhs_event_set(rs485port->Event, RS485_TX_STATUS_ERROR_EVENT);
else if (rs485port->RxStatus)  
  mhs_event_set(rs485port->Event, RS485_RX_STATUS_EVENT);
return(0);
}



int32_t RS485MsIoRw(TCanDevice *dev, TRS485MsData *ms_data, uint32_t size, uint32_t timeout)
{
TRS485Port *rs485port;
uint16_t data_size;
uint8_t rx_status;
 
if (!(rs485port = dev->RS485Port))
  return(-1); // <*>
if (!timeout)
  return(-1);    
if (size != sizeof(TRS485MsData))
  return(-1);
if (rs485port->Status)
  return(-1);
if (!(data_size = ms_data->TxSize))
  return(-1);    
mhs_event_clear(rs485port->Event, 0x7FFFFFFF);         
rs485port->Status = RS485_STATUS_TX;  
rs485port->TxStatus = 0;
rs485port->RxStatus = 0;
ms_data->TxStatus = 0;
ms_data->RxStatus = 0; 
char_fifo_clear(rs485port->RxFifo);
// **** Daten senden          
memcpy(rs485port->TxBuffer, ms_data->TxData, data_size);
rs485port->TxSize = data_size;  
// **** Daten Empfangen           
(void)mhs_wait_for_event(rs485port->Event, timeout); // <*> Events auswerten ?
rx_status = rs485port->RxStatus; 
data_size = (uint16_t)char_fifo_read(rs485port->RxFifo, ms_data->RxData, 4096);
ms_data->TxStatus = rs485port->TxStatus;
if (rs485port->TxStatus != RS485_TX_STATUS_FINISH)
  {
  rs485port->Status = RS485_STATUS_IDLE;
  return(-1);
  }
ms_data->RxStatus = rx_status;
ms_data->RxSize = data_size;
rs485port->Status = RS485_STATUS_IDLE;  
if ((rx_status == RS485_RX_STATUS_FINISH) || (rx_status == RS485_REQ_RX_TIMEOUT))  
  return(0);
else
  return(-1);
}

/* <*> alt, raus
int32_t RS485MsIoRw(TCanDevice *dev, TRS485MsData *ms_data, uint32_t size, uint32_t timeout)
{
TRS485Port *rs485port;
uint16_t data_size;
uint32_t n, buf_size;
uint8_t rx_status;
uint8_t *rx_data;
 
if (!(rs485port = dev->RS485Port))
  return(-1); // <*>
if (!timeout)
  return(-1);    
if (size != sizeof(TRS485MsData))
  return(-1);
if (rs485port->Status)
  return(-1);
if (!(data_size = ms_data->TxSize))
  return(-1);           
rs485port->Status = RS485_STATUS_TX;  
rs485port->TxStatus = 0;
rs485port->RxStatus = 0;
ms_data->TxStatus = 0;
ms_data->RxStatus = 0; 
char_fifo_clear(rs485port->RxFifo);
// **** Daten senden          
memcpy(rs485port->TxBuffer, ms_data->TxData, data_size);
rs485port->TxSize = data_size;  
while (timeout)    
  {
  mhs_sleep(1); // <*>
  if (rs485port->TxStatus)
    break;
  timeout--;
  }
ms_data->TxStatus = rs485port->TxStatus;
if (rs485port->TxStatus != RS485_TX_STATUS_FINISH)
  {
  rs485port->Status = RS485_STATUS_IDLE;
  return(-1);
  }
// **** Daten Empfangen           
buf_size = 4096;
rx_data = ms_data->RxData;
data_size = 0; 
do
  {
  rx_status = rs485port->RxStatus; 
  n = char_fifo_read(rs485port->RxFifo, rx_data, buf_size);
  rx_data += n;
  buf_size -= n;
  data_size += (uint16_t)n;
  if (!buf_size)
    break;
  if (rx_status)
    break;    
  if (timeout--)
    mhs_sleep(0);               
    //mhs_sleep(1);    
  }
while (timeout);
ms_data->RxStatus = rx_status;
ms_data->RxSize = data_size;
rs485port->Status = RS485_STATUS_IDLE;  
if ((rx_status == RS485_RX_STATUS_FINISH) || (rx_status == RS485_REQ_RX_TIMEOUT))  
  return(0);
else
  return(-1);
}*/

/* <*>
int32_t RS485IoWrite(TCanDevice *dev, TRS485MsData *tx_data, uint32_t *size, uint32_t timeout)
{
TRS485Port *rs485port;
uint32_t tx_size, true_write;

true_write = 0; 
if (!(rs485port = dev->RS485Port))
  return(-1); // <*>      
if (!rs485port->TxSize)
  {
  rs485port->TxStatus = 0;
  rs485port->RxStatus = 0;
  char_fifo_clear(rs485port->RxFifo);
  tx_size = tx_data->DataSize;        
  memcpy(rs485port->TxBuffer, tx_data->Data, tx_size);
  rs485port->TxSize = tx_size;  
  if (timeout)
    {
    if (timeout < 5)
      timeout = 1;
    else
      timeout /= 5;
    while (timeout)    
      {
      mhs_sleep(5); // <*>
      if (rs485port->TxStatus)
        break;
      timeout--;
      }
    if (rs485port->TxStatus == RS485_TX_STATUS_FINISH)
      true_write = tx_size;        
    }
  else
    true_write = tx_size;           
  }
tx_data->DataSize = (uint16_t)true_write;
if (size)
  *size = true_write;
return((int32_t)true_write);
}*/ 


int32_t RS485IoRead(TCanDevice *dev, uint8_t *rx_data, uint32_t *size, uint32_t timeout)
{
TRS485Port *rs485port;
int32_t err;
uint32_t rx_status, buf_size, read_size;

if (!(rs485port = dev->RS485Port))
  return(-1); // <*>
buf_size = *size;
read_size = 0;
/*if (timeout) <*>
  {
  if (timeout < 5)
    timeout = 2;
  else
    timeout /= 5;
  }*/  
do
  {
  rx_status = rs485port->RxStatus; 
  read_size += char_fifo_read(rs485port->RxFifo, &rx_data[read_size], buf_size);
  buf_size -= read_size;
  if (!buf_size)
    break;
  if (rx_status)
    break;
  if (timeout)
    {
    if (--timeout)               
      mhs_sleep(1); // 5 <*>
    }
  }
while (timeout);  
if (rx_status == RS485_RX_STATUS_FINISH)
  err = 2;
else if (rx_status == RS485_RX_STATUS_TIMEOUT)
  err = 1;
else if (rx_status == RS485_RX_STATUS_ERROR)
  err = -1;
else if (rx_status == RS485_RX_STATUS_OV)
  err = -2;
*size = read_size;  
return(err);   
}