/***************************************************************************
                           com_io.c  -  description
                             -------------------
    begin             : 12.03.2012
    last modified     : 05.05.2023     
    copyright         : (C) 2012 - 2023 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include <stdio.h>
#include <string.h>
#include "ser_io.h"
#include "com_io.h"

#define ERR_NO_COM_DRIVER_LOAD -1

/***************************************************************/
/*  Treiber Initialisieren                                     */
/***************************************************************/
void ComIoDrvCreate(void)
{
}


/***************************************************************/
/*  Treiber beenden                                            */
/***************************************************************/
void ComIoDrvDestroy(void)
{
}


struct TIo *ComIoCreate(int32_t driver_index, TMhsObjContext *context)
{
const struct TComIoDriver *com_io_driver;
(void)driver_index;

com_io_driver = &SerDevice;
return((com_io_driver->Create)(context));
}


void ComIoDestroy(struct TIo **io)
{
struct TIo *io_driver;

if (!io)
  return;
io_driver = *io;
if (!io_driver)
  return;
if (io_driver->ComIoDriver)
  (io_driver->ComIoDriver->Destroy)(io);
*io = NULL;
}


int32_t ComOpen(struct TIo *io)
{
if (!io)
  return(-1);
if (io->ComIoDriver)
  return((io->ComIoDriver->Open)(io));
else
  return(ERR_NO_COM_DRIVER_LOAD);
}


void ComClose(struct TIo *io)
{
if (!io)
  return;
if (io->ComIoDriver)
  (io->ComIoDriver->Close)(io);
}


int32_t ComIsOpen(struct TIo *io)
{
if (!io)
  return(-1);
if (io->ComIoDriver)
  return((io->ComIoDriver->IsOpen)(io));
else
  return(ERR_NO_COM_DRIVER_LOAD);
}


void ComFlushBuffer(struct TIo *io)
{
if (!io)
  return;
if (io->ComIoDriver)
  (io->ComIoDriver->FlushBuffer)(io);
}


int32_t ComCountRx(struct TIo *io)
{
if (!io)
  return(-1);
if (io->ComIoDriver)
  return((io->ComIoDriver->CountRx)(io));
else
  return(ERR_NO_COM_DRIVER_LOAD);
}


int32_t ComReadData(struct TIo *io, char *data, uint32_t max)
{
if (!io)
  return(-1);
if (io->ComIoDriver)
  return((io->ComIoDriver->ReadData)(io, data, max));
else
  return(ERR_NO_COM_DRIVER_LOAD);
}


int32_t ComWriteData(struct TIo *io, char *data, int32_t size)
{
if (!io)
  return(-1);
if (io->ComIoDriver)
  return((io->ComIoDriver->WriteData)(io, data, size));
else
  return(ERR_NO_COM_DRIVER_LOAD);
}


uint32_t ComGetLineStatus(struct TIo *io, uint32_t index)
{
if (!io)
  return(0);
if (io->ComIoDriver)
  return((io->ComIoDriver->GetLineStatus)(io, index));
else
  return(0);
}


int32_t ComWriteDTR(struct TIo *io, uint32_t index, uint32_t status)
{
if (!io)
  return(-1);
if (io->ComIoDriver)
  return((io->ComIoDriver->WriteDTR)(io, index, status));
else
  return(ERR_NO_COM_DRIVER_LOAD);
}


int32_t ComWriteRTS(struct TIo *io, uint32_t index, uint32_t status)
{
if (!io)
  return(-1);
if (io->ComIoDriver)
  return((io->ComIoDriver->WriteRTS)(io, index, status));
else
  return(ERR_NO_COM_DRIVER_LOAD);
}


// Sendet ein Byte
int32_t ComWriteByte(struct TIo *io, unsigned char value)
{
return(ComWriteData(io, (char *)&value, 1));
}


// Sendet ein Zeichen
int32_t ComWriteChar(struct TIo *io, char value)
{
return(ComWriteData(io, &value, 1));
}
