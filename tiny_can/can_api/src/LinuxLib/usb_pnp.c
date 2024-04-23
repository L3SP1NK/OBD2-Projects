/***************************************************************************
                            usb_pnp.c  -  description
                             -------------------
    begin             : 23.03.2008
    last modified     : 05.05.2023    
    copyright         : (C) 2008 - 2023 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <signal.h>
#include "errors.h"
#include "util.h"
#include "sysfs.h"
#include "com_io.h"
#include "index.h"
#include "usb_hlp.h"
#include "usb_scan.h"
#include "mhs_file_event.h"
#include "mhs_thread.h"
#include "mhs_event_list.h"
#include "usb_pnp.h"
#include "pnp_inc.c"


#define MAX_EVENTS 1024 /*Max. number of events to process at one go*/
#define LEN_NAME (NAME_MAX + 1)
#define EVENT_SIZE ( sizeof (struct inotify_event) ) /*size of one event*/
#define BUF_LEN ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME )) /*buffer to store the data of events*/


//static const char USB_SERIAL_DEVICES_PATH[] = {"bus/usb-serial/devices"};


static fd_set RxFdSet;
#define max(a, b) ((a)>(b)) ? (a) : (b)

static void PnPThreadExecute(TMhsThread *thread);

/*
******************** PnPCreate ********************
*/
int32_t PnPCreate(void)
{
if (MhsPnP)
  return(0);
if (!(MhsPnP = (struct TMhsPnP *)mhs_calloc(1, sizeof(struct TMhsPnP))))
  return(-1);
MhsPnP->EventList = MhsEventListCreate();  
UpdateTCanUsbList();
if (!(MhsPnP->FileEvent = create_file_event()))
  {
  PnPDestroy();
  return(-1);
  }
if (!(MhsPnP->Thread = mhs_create_thread(PnPThreadExecute, MhsPnP, 0, 1)))
  {
  PnPDestroy();
  return(-1);
  }
mhs_event_set_event_mask((TMhsEvent *)MhsPnP->Thread, MHS_ALL_EVENTS);
return(0);
}


/*
******************** PnPDestroy *******************
*/
void PnPDestroy(void)
{
if (MhsPnP)
  {
  mhs_exit_thread(MhsPnP->Thread);
  (void)set_file_event(MhsPnP->FileEvent, 0xFF);
  if (MhsPnP->Thread)
    mhs_destroy_thread(&MhsPnP->Thread, 1000);
  DestroyTCanUsbList(&TCanUsbList);
  destroy_file_event(&MhsPnP->FileEvent);
  MhsEventListDestroy(&MhsPnP->EventList);
  safe_free(MhsPnP);
  }
}



static void PnPThreadExecute(TMhsThread *thread)
{
ssize_t rc, i;
struct TMhsPnP *pnp;
char *buffer;
struct inotify_event *event;
int event_fd, maxfd, fd, wd, hit;


pnp = (struct TMhsPnP *)thread->Data;
buffer = (char *)mhs_malloc(BUF_LEN);
wd = 0;
fd = inotify_init();
if (fd)
  {
  wd = inotify_add_watch(fd, "/dev", IN_CREATE | IN_DELETE | IN_DELETE_SELF);
  if (!wd)
    thread->Run = 0;
  }
else
  thread->Run = 0;

maxfd = 0;
event_fd = file_event_get_fd(pnp->FileEvent);
/***********************************************************/
/*                 Thread Loop Schleife                    */
/***********************************************************/
while (thread->Run)
  {
  // **** fd Array initialisieren
  FD_ZERO(&RxFdSet);           // Inhalt leeren
  FD_SET(event_fd, &RxFdSet); // Cancel Pipe hinzufügen
  FD_SET(fd, &RxFdSet); // Den Socket der verbindungen annimmt hinzufügen
  maxfd = max(fd, event_fd);

  if (select(maxfd+1, &RxFdSet, NULL, NULL, NULL) == -1)
    {
    if (errno == EINTR)
      continue;
    else
      thread->Run = 0;
    }
  if (!thread->Run)
    break;
  if (FD_ISSET(event_fd, &RxFdSet))
    {
    if (get_file_event(pnp->FileEvent) == 0x01)
      UpdateListAndSetEvent(1);
    }
  if (FD_ISSET(fd, &RxFdSet))
    {
    hit = 0;
    i = 0;
    do
      rc = read(fd, buffer, BUF_LEN);
    while ((rc == -1) && (errno == EINTR));
    while (i < rc)
      {
      if (rc < (ssize_t)sizeof(struct inotify_event))
        break;
      event = (struct inotify_event *)&buffer[i];
      if (event->len)
        {
        if (event->mask & (IN_CREATE | IN_DELETE | IN_DELETE_SELF))
          //{
          //if (!strncmp(USB_DEVICE_NAME, event->name, strlen(USB_DEVICE_NAME)))
            hit = 1;
          //}
        }
      i += EVENT_SIZE + event->len;
      }
    if (hit)
      UpdateListAndSetEvent(0);
    }
  }
if ((fd) && (wd))
  (void)inotify_rm_watch(fd, wd);
if (fd)
  (void)close(fd);
safe_free(buffer);
}










