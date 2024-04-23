/***************************************************************************
                           com_io.c  -  description
                             -------------------
    begin             : 01.11.2010
    last modify       : 04.05.2019
    copyright         : (C) 2010 - 2019 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef __APPLE__
#include <IOKit/serial/ioss.h>
#include <IOKit/IOBSD.h>
#else
#include <linux/serial.h>
#endif
#include "util.h"
#include "var_obj.h"
#include "com_io.h"
#include "ser_io.h"


static const char ERR_STR_NO_DEVICE_NAME[] = {"Cannot open device with no name"};
static const char ERR_STR_DEVICE_OPEN[] = {"Cannot open %s : %s"};
static const char ERR_STR_DEVICE_NOT_OPEN[] = {"Device not open"};
static const char ERR_STR_READ_DATA[] = {"Error receive data: %s"};
static const char ERR_STR_READ_DATA_NOPARAM[] = {"Error receive data"};
static const char ERR_STR_WRITE_DATA[] = {"Error sending data: %s"};

static const char ERR_STR_NO_DEVICE_NAME_B[] = {"Cannot open \"B\" device with no name"};
static const char ERR_STR_DEVICE_OPEN_B[] = {"\"B\" device, cannot open %s : %s"};

static const int32_t DefVendorId        = 0x0403;
static const int32_t DefProductId       = 0x6001;
static const char DefSerialNr[]         = "";
static const char *DefDeviceName = NULL;
static const uint32_t DefBaudRate   = BR_57600;
static const uint32_t DefRxDTimeout = 500;
static const char DefUsbDescription[]   = "";
static const uint32_t DefUsbDeviceType  = FT_DEVICE_232R;

static const struct TValueDescription SerValues[] = {
// Name            | Alias | Type      | Flags | MaxSize | Access | Default
  {"VendorId",      NULL,   VT_LONG,        0,      0,    0xFFFF,   &DefVendorId},
  {"ProductId",     NULL,   VT_LONG,        0,      0,    0xFFFF,   &DefProductId},
  {"SerialNr",      NULL,   VT_STRING,      0,      0,    0xFFFF,   &DefSerialNr},
  {"DeviceName",    NULL,   VT_STRING,      0,      0,    0xFFFF,   &DefDeviceName},
  {"DeviceNameB",   NULL,   VT_STRING,      0,      0,    0xFFFF,   &DefDeviceName},
  {"BaudRate",      NULL,   VT_ULONG,       0,      0,    0xFFFF,   &DefBaudRate},
  {"RxDTimeout",    NULL,   VT_ULONG,       0,      0,    0xFFFF,   &DefRxDTimeout},
  {"UsbDescription",NULL,   VT_STRING,      0,      0,    0xFFFF,   &DefUsbDescription},
  {"UsbDeviceType", NULL,   VT_ULONG,       0,      0,    0xFFFF,   &DefUsbDeviceType},
  {NULL,            NULL,   0,              0,      0,       0,   NULL}};


struct TComSpeedTab
  {
  uint32_t Baud;
  speed_t SetBaud;
  char *Name;
  };



static const struct TComSpeedTab ComSpeedTab[] = {
  {110,     B110,     "110"},
  {300,     B300,     "300"},
  {600,     B600,     "600"},
  {1200,    B1200,    "1200"},
  {2400,    B2400,    "2400"},
  {4800,    B4800,    "4800"},
  {9600,    B9600,    "9600"},
  {19200,   B19200,   "19200"},
  {38400,   B38400,   "38400"},
  {57600,   B57600,   "57600"},
  {115200,  B115200,  "115200"},
  {230400,  B230400,  "230400"},
#ifdef B460800
  {460800,  B460800,  "460800"},
#else
  {460800,  0,        "460800"},
#endif
#ifdef B921600
  {921600,  B921600,  "921600"},
#else
  {921600,  0,        "921600"},
#endif
#ifdef B3000000
  {3000000, B3000000, "3000000"},
#else
  {3000000, 0,        "3000000"},
#endif
#ifdef B4000000
  {4000000, B4000000, "4000000"},
#else
  {4000000, 0,        "4000000"},
#endif
#ifdef B6000000
  {6000000, B6000000, "6000000"},
#else
  {6000000, 0,        "6000000"},
#endif
#ifdef B8000000
  {8000000, B8000000, "8000000"},
#else
  {8000000, 0,        "8000000"},
#endif
#ifdef B12000000
  {12000000, B12000000, "12000000"},
#else
  {12000000, 0,         "12000000"},
#endif
  {0, 0, NULL}};


#define com_io_return(io, err, format, args...) do { \
  snprintf((io)->ErrorString, sizeof((io)->ErrorString) - 1, format, ## args); \
  (io)->ErrorCode = (int32_t)(err); \
  return((err)); } while(0)

#define com_io_clear_error(io) do { \
  (io)->ErrorCode = 0; \
  (io)->ErrorString[0] = '\0'; } while(0)

#define com_io_set_error(io, err, format, args...) do { \
  snprintf((io)->ErrorString, sizeof((io)->ErrorString) - 1, format, ## args); \
  (io)->ErrorCode = (int32_t)(err); } while(0)

static void SerSetBaudrate(TObjValue *obj, void *user_data);
static void SerSetTimeout(TObjValue *obj, void *user_data);
static void SerClose(struct TIo *io);
static void SerFlushBuffer(struct TIo *io);
static int32_t SerWriteDTR(struct TIo *io, uint32_t index, uint32_t status);
static int32_t SerWriteRTS(struct TIo *io, uint32_t index, uint32_t status);


/***************************************************************/
/*  Treiber erzeugen                                           */
/***************************************************************/
static struct TIo *SerCreate(TMhsObjContext *context)
{
struct TIo *io;
TMhsObj *obj;

if (!context)
  return(NULL);
io = (struct TIo *)mhs_calloc(1, sizeof(struct TIo));
if (!io)
  return(NULL);
com_io_clear_error(io);
io->Context = context;
io->ComIoDriver = &SerDevice;
io->ComPortFd = -1;
io->ComPortFdB = -1;
if (mhs_values_create_from_list(context, SerValues) < 0)
  safe_free(io);
else
  {
  obj = mhs_object_get_by_name("BaudRate", context);
  mhs_object_cmd_event_connect(obj, 1, (TMhsObjectCB)&SerSetBaudrate, (void *)io);
  obj = mhs_object_get_by_name("RxDTimeout", context);
  mhs_object_cmd_event_connect(obj, 1, (TMhsObjectCB)&SerSetTimeout, (void *)io);
  }
return(io);
}


/***************************************************************/
/*  Treiber beenden                                            */
/***************************************************************/
static void SerDestroy(struct TIo **io)
{
struct TIo *io_dev;
TMhsObj *obj;
TMhsObjContext *context;

if (!io)
  return;
io_dev = *io;
if (!io_dev)
  return;
context = io_dev->Context;
obj = mhs_object_get_by_name("BaudRate", context);
mhs_object_cmd_event_disconnect(obj, (TMhsObjectCB)&SerSetBaudrate);
obj = mhs_object_get_by_name("RxDTimeout", context);
mhs_object_cmd_event_disconnect(obj, (TMhsObjectCB)&SerSetTimeout);
SerClose(io_dev);
mhs_free(io_dev);
*io = NULL;
}


/***************************************************************/
/*  COM Treiber Einstellungen vornehmen                        */
/***************************************************************/
static void ser_set_settings(struct TIo *io, uint32_t baudrate, uint32_t rxd_timeout)
{
struct termios tios;
const struct TComSpeedTab *l;
#ifdef __APPLE__
speed_t b, baud;
#else
struct serial_struct serial;
uint32_t b;
#endif

if (io->ComPortFd < 0)           // nur ausführen wenn Connectet
  return;

tcgetattr(io->ComPortFd, &tios);
tios.c_iflag = IGNBRK | IGNPAR;
tios.c_oflag = 0;
tios.c_cflag = CS8 | CREAD | CLOCAL;
tios.c_lflag = 0;

#ifdef __APPLE__
baud = 0;
#else
ioctl(io->ComPortFd, TIOCGSERIAL, &serial);
serial.flags &= ~ASYNC_SPD_MASK; //ASYNC_USR_MASK;
serial.flags |= ASYNC_LOW_LATENCY;
#endif
// ******** Baudrate **********************************
for (l = ComSpeedTab; (b = l->Baud); l++)
  {
  if (b == baudrate)
    {
    if (l->SetBaud)
      {
      cfsetspeed(&tios, l->SetBaud);
      }
    else
      {
#ifdef __APPLE__
      cfsetspeed(&tios, B19200);
      baud = b;
#else
      serial.flags |= ASYNC_SPD_CUST;
      serial.custom_divisor = serial.baud_base / b;
      cfsetspeed(&tios, B38400);
#endif
      }
    break;
    }
  }
#ifndef __APPLE__
ioctl(io->ComPortFd, TIOCSSERIAL, &serial);
#endif
if (rxd_timeout > 200)
  b = (rxd_timeout / 200) + 1;
else if (rxd_timeout)
  b = 1;
else
  b = 0;
tios.c_cc[VTIME] = b;
tios.c_cc[VMIN] = 0;

tcsetattr(io->ComPortFd, TCSANOW | TCSAFLUSH, &tios);
#ifdef __APPLE__
if (baud)
  ioctl(io->ComPortFd, IOSSIOSPEED, &baud);
#endif
tcflush(io->ComPortFd, TCIOFLUSH);
}


static void SerSetBaudrate(TObjValue *obj, void *user_data)
{
struct TIo *io;
uint32_t rxd_timeout;

if ((!obj) || (!user_data))
  return;
io = (struct TIo *)user_data;
if (io->ComPortFd > -1)
  {
  SerFlushBuffer(io);
  rxd_timeout = mhs_value_get_as_ulong("RxDTimeout", ACCESS_INIT, io->Context);
  ser_set_settings(io, obj->Value.U32, rxd_timeout);
  mhs_sleep(50);
  SerFlushBuffer(io);
  }
}


static void SerSetTimeout(TObjValue *obj, void *user_data)
{
struct TIo *io;
uint32_t baud_rate;

if ((!obj) || (!user_data))
  return;
io = (struct TIo *)user_data;
baud_rate = mhs_value_get_as_ulong("BaudRate", ACCESS_INIT, io->Context);
ser_set_settings(io, baud_rate, obj->Value.U32);
}


/***************************************************************/
/*  Connect                                                    */
/***************************************************************/
static int32_t SerOpen(struct TIo *io)
{
TMhsObjContext *context;
uint32_t baud_rate, rxd_timeout, device_type;
int32_t fd, fd_b, res;
char *device_name, *device_name_b;
#ifdef __APPLE__
uint32_t mics;
#else
int32_t rc;
#endif

res = 1;
com_io_clear_error(io);
SerClose(io);                            // Port schließen
context = io->Context;
baud_rate = mhs_value_get_as_ulong("BaudRate", ACCESS_INIT, context);
rxd_timeout = mhs_value_get_as_ulong("RxDTimeout", ACCESS_INIT, context);
device_name = mhs_value_get_as_string("DeviceName", ACCESS_INIT, context);
device_name_b = mhs_value_get_as_string("DeviceNameB", ACCESS_INIT, context);
device_type = mhs_value_get_as_ulong("UsbDeviceType", ACCESS_INIT, context);
// **** Com Port öffnen
if ((!device_name) || (!strlen(device_name)))
  com_io_return(io, -1, ERR_STR_NO_DEVICE_NAME);
fd = open(device_name, O_RDWR | O_NOCTTY | O_NONBLOCK);
io->ComPortFd = fd;
if (fd < 0)                   // Com Port geöffnet ?
  com_io_return(io, -1, ERR_STR_DEVICE_OPEN, device_name, strerror(errno));

if (ioctl(fd, TIOCEXCL) == -1)
  com_io_return(io, -1, ERR_STR_DEVICE_OPEN, device_name, strerror(errno));
ser_set_settings(io, baud_rate, rxd_timeout);              // Port Configurieren
#ifdef __APPLE__
if (fcntl(fd, F_SETFL, 0) < 0)
  com_io_return(io, -1, ERR_STR_DEVICE_OPEN, device_name, strerror(errno));

mics = 1UL;
if (ioctl(fd, IOSSDATALAT, &mics) < 0)
  com_io_return(io, -1, ERR_STR_DEVICE_OPEN, device_name, strerror(errno));
#else
rc = fcntl(fd, F_GETFL, 0);
if (rc != -1)
  {
  if (fcntl(fd, F_SETFL, rc & ~O_NONBLOCK) < 0)
    com_io_return(io, -1, ERR_STR_DEVICE_OPEN, device_name, strerror(errno));
  }
#endif

// ***** B Port öffnen
if (device_type == FT_DEVICE_2232H)
  {
  if ((!device_name_b) || (!strlen(device_name_b)))
    {
    res = -1;
    com_io_set_error(io, -1, ERR_STR_NO_DEVICE_NAME_B);
    }
  fd_b = open(device_name_b, O_RDWR | O_NOCTTY| O_NONBLOCK);
  io->ComPortFdB = fd_b;

  if (res > -1)
    {
    if (fd_b < 0)                   // Com Port geöffnet ?
      {
      res = -1;
      com_io_set_error(io, -1, ERR_STR_DEVICE_OPEN_B, device_name_b, strerror(errno));
      }
    else
      {
      if (ioctl(fd_b, TIOCEXCL) == -1)
        {
        res = -1;
        com_io_set_error(io, -1, ERR_STR_DEVICE_OPEN_B, device_name_b, strerror(errno));
        }
      else
        ser_set_settings(io, baud_rate, rxd_timeout);              // Port Configurieren
      }
    }
  if (res > -1)
    {
#ifdef __APPLE__
    if (fcntl(fd_b, F_SETFL, 0) < 0)
      {
      res = -1;
      com_io_set_error(io, -1, ERR_STR_DEVICE_OPEN_B, device_name_b, strerror(errno));
      }
    else
      {
      mics = 1UL;
      if (ioctl(fd_b, IOSSDATALAT, &mics) < 0)
        {
        res = -1;
        com_io_set_error(io, -1, ERR_STR_DEVICE_OPEN_B, device_name_b, strerror(errno));
        }
      }
#else
    rc = fcntl(fd_b, F_GETFL, 0);
    if (rc != -1)
      {
      if (fcntl(fd_b, F_SETFL, rc & ~O_NONBLOCK) < 0)
        {
        res = -1;
        com_io_set_error(io, -1, ERR_STR_DEVICE_OPEN_B, device_name_b, strerror(errno));
        }
      }
#endif
    }

  if (res > -1)
    {
    // Reset / Prog Setup
    if (SerWriteRTS(io, 1, 0) < 0)
      {
      res = -1;
      com_io_set_error(io, -1, ERR_STR_DEVICE_OPEN_B, device_name_b, strerror(errno));
      }
    else
      {
      mhs_sleep(10);
      if (SerWriteDTR(io, 1, 0) < 0)
        {
        res = -1;
        com_io_set_error(io, -1, ERR_STR_DEVICE_OPEN_B, device_name_b, strerror(errno));
        }
      else
        mhs_sleep(700);
      }
    }

  if (res < 0)
    {
    close(io->ComPortFd);
    io->ComPortFd = -1;
    }
  }
return(res);
}


/***************************************************************/
/*  Disconnect                                                 */
/***************************************************************/
static void SerClose(struct TIo *io)
{
if (!io)
  return;
if (io->ComPortFd > -1)
  {
  tcflush(io->ComPortFd, TCOFLUSH);
  tcflush(io->ComPortFd, TCIFLUSH);
  close(io->ComPortFd);
  io->ComPortFd = -1;
  }
if (io->ComPortFdB > -1)
  {
  close(io->ComPortFdB);
  io->ComPortFdB = -1;
  }
}


/***************************************************************/
/*  Treiber Status abfragen                                    */
/***************************************************************/
static int32_t SerIsOpen(struct TIo *io)
{
if (!io)
  return(0);
if (io->ComPortFd < 0)
  return(0);  // ungültig
else
  return(1);  // gültig
}


/***************************************************************/
/*  Empfangs- und Sendepuffer löschen                          */
/***************************************************************/
static void SerFlushBuffer(struct TIo *io)
{
if (!io)
  return;
if (io->ComPortFd > -1)        // nur ausfhren wenn Connectet
  {
  tcflush(io->ComPortFd, TCOFLUSH);
  tcflush(io->ComPortFd, TCIFLUSH);
  }
}


/***************************************************************/
/*  Anzahl Zeichen im Empfangs Puffer                          */
/***************************************************************/
static int32_t SerCountRx(struct TIo *io)
{
int32_t cnt;

if (!io)
  return(-1);
com_io_clear_error(io);
if (io->ComPortFd < 0)
  com_io_return(io, -1, ERR_STR_DEVICE_NOT_OPEN);

if (ioctl(io->ComPortFd, FIONREAD, &cnt) < 0)
  com_io_return(io, -1, ERR_STR_DEVICE_NOT_OPEN);
return(cnt);
}


/***************************************************************/
/*  Routinen zum Empfangen von Zeichen                         */
/***************************************************************/
static int32_t SerReadData(struct TIo *io, char *data, uint32_t to_read)
{
int32_t res;
uint32_t read_count, timeout_cnt;

if (!io)
  return(-1);
com_io_clear_error(io);
if (io->ComPortFd < 0)
  com_io_return(io, -1, ERR_STR_DEVICE_NOT_OPEN);
if (!to_read)
  return(0);
read_count = 0;
timeout_cnt = 0;
do
  {
  do
    res = read(io->ComPortFd, data, to_read);
  while ((res < 0) && (errno == EINTR));
  if (res < 0)
    com_io_return(io, -1, ERR_STR_READ_DATA, strerror(errno));
  if (res == 0)
    {
    if (timeout_cnt > 1)
      break;
    timeout_cnt++;
    continue;
    }
  data += res;
  read_count += res;
  to_read -= res;
  }
while (to_read);
// keine Zeichen gelesen, eventuell USB-Gerät abgesteckt
if (!read_count)
  {
  if (ioctl(io->ComPortFd, FIONREAD, &res) < 0)
    com_io_return(io, -1, ERR_STR_READ_DATA_NOPARAM);
  else
    return(0);
  }
return(read_count);
}


// Sendet ein Byte
/*int32_t ComWriteByte(struct TIo *io, unsigned char value)
{
return(SerWriteData(io, (char *)&value, 1));
} */

/***************************************************************/
/*  Routinen zum Senden von Zeichen                            */
/***************************************************************/
static int32_t SerWriteData(struct TIo *io, char *data, int32_t size)
{
int32_t res;

if (!io)
  return(-1);
com_io_clear_error(io);
if (io->ComPortFd < 0)
  com_io_return(io, -1, ERR_STR_DEVICE_NOT_OPEN);
res = write(io->ComPortFd, data, size);
/*for (i = 0; i < 10000; i++)
  {
  res = write(io->ComPortFd, data, size);
  if (res < 0)
    {
    if ((errno == EINTR) || (errnor == EAGAIN))
      {
      usleep(1);
      continue;
      }
    }
  break;
  }*/
if (res < 0)
  com_io_return(io, -1, ERR_STR_WRITE_DATA, strerror(errno));
else
  return(res);
}


/***************************************************************/
/*  COM Port Status Leitungen abfragen                         */
/***************************************************************/
static uint32_t SerGetLineStatus(struct TIo *io, uint32_t index)
{
int32_t res, stat, fd;

if (!io)
  return(0);
res = 0;
if (index)
  {
  if ((fd = io->ComPortFdB) < 0)
    return(0);
  }
else  
  {
  if ((fd = io->ComPortFd) < 0)
    return(0);
  }
// Status leitungen abfragen
if (ioctl(fd, TIOCMGET, &stat) < 0)
  return(0);
if (stat & TIOCM_CTS) res |= ST_CTS;
if (stat & TIOCM_DSR) res |= ST_DSR;
if (stat & TIOCM_RNG) res |= ST_RNG;
if (stat & TIOCM_CAR) res |= ST_CAR;
return(res);
}


/***************************************************************/
/*  Status Leitungen setzen                                    */
/***************************************************************/

// Zustand der DTR Leitung setzen
static int32_t SerWriteDTR(struct TIo *io, uint32_t index, uint32_t status)
{
int32_t stat, fd;

if (!io)
  return(-1);
com_io_clear_error(io);
if (index)
  {
  if ((fd = io->ComPortFdB) < 0)
    com_io_return(io, -1, ERR_STR_DEVICE_NOT_OPEN);
  }
else  
  {
  if ((fd = io->ComPortFd) < 0)
    com_io_return(io, -1, ERR_STR_DEVICE_NOT_OPEN);
  }
// Status leitungen abfragen
if (ioctl(fd, TIOCMGET, &stat) < 0)
  com_io_return(io, -1, ERR_STR_DEVICE_NOT_OPEN);
if (status)
  stat |= TIOCM_DTR;
else
  stat &= ~TIOCM_DTR;
// Status leitungen setzen
if(ioctl(fd, TIOCMSET, &stat) < 0)
  com_io_return(io, -1, ERR_STR_DEVICE_NOT_OPEN);
return(0);
}


// Zustand der RTS Leitung setzen
static int32_t SerWriteRTS(struct TIo *io, uint32_t index, uint32_t status)
{
int32_t stat, fd;

com_io_clear_error(io);
if (index)
  {
  if ((fd = io->ComPortFdB) < 0)
    com_io_return(io, -1, ERR_STR_DEVICE_NOT_OPEN);
  }
else  
  {
  if ((fd = io->ComPortFd) < 0)
    com_io_return(io, -1, ERR_STR_DEVICE_NOT_OPEN);
  }
// Status leitungen abfragen
if (ioctl(fd, TIOCMGET, &stat) < 0)
  com_io_return(io, -1, ERR_STR_DEVICE_NOT_OPEN);
if (status)
  stat |= TIOCM_RTS;
else
  stat &= ~TIOCM_RTS;
// Status leitungen setzen
if(ioctl(fd, TIOCMSET, &stat) < 0)
  com_io_return(io, -1, ERR_STR_DEVICE_NOT_OPEN);
return(0);
}


const struct TComIoDriver SerDevice =
  {
  /* .Create        = */ SerCreate,
  /* .Destroy       = */ SerDestroy,
  /* .Open          = */ SerOpen,
  /* .Close         = */ SerClose,
  /* .IsOpen        = */ SerIsOpen,
  /* .FlushBuffer   = */ SerFlushBuffer,
  /* .CountRx       = */ SerCountRx,
  /* .ReadData      = */ SerReadData,
  /* .WriteData     = */ SerWriteData,
  /* .GetLineStatus = */ SerGetLineStatus,
  /* .WriteDTR      = */ SerWriteDTR,
  /* .WriteRTS      = */ SerWriteRTS
  };
