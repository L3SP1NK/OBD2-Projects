/***************************************************************************
                          mhs_event.c  -  description
                             -------------------
    begin             : 17.06.2009
    copyright         : (C) 2009 by MHS-Elektronik GmbH & Co. KG, Germany
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/

/**
    This file is a part of Tiny-CAN-View. You find the latest versions at
       http://www.tiny-can.com/
**/
#include "mhs_event.h"


typedef struct _MHSEventSource MHSEventSource;

struct _MHSEventSource
  {
  GSource     source;
  volatile gint event;
  volatile gint event_enable;
  };


static gboolean mhs_event_prepare(GSource *source, gint *timeout);
static gboolean mhs_event_check(GSource *source);
static gboolean mhs_event_dispatch(GSource *source, GSourceFunc callback, gpointer user_data);


GSourceFuncs mhs_event_funcs =
  {
  mhs_event_prepare,
  mhs_event_check,
  mhs_event_dispatch,
  NULL
  };

/* Timeouts */
static gboolean mhs_event_prepare(GSource *source, gint *timeout)
{
MHSEventSource *event_source;

event_source = (MHSEventSource *)source;
*timeout = -1;
if ((event_source->event) && (event_source->event_enable))
    return(TRUE);
else
  return(FALSE);
}


static gboolean mhs_event_check(GSource *source)
{
MHSEventSource *event_source;

event_source = (MHSEventSource *)source;
if ((event_source->event) && (event_source->event_enable))
  return(TRUE);
else
  return(FALSE);
}


static gboolean mhs_event_dispatch(GSource *source, GSourceFunc callback, gpointer user_data)
{
MHSEventSource *event_source;

event_source = (MHSEventSource *)source;
if (!callback)
  {
  g_warning ("Event source dispatched without callback\n"
             "You must call g_source_set_callback().");
  return(FALSE);
  }

if ((event_source->event) && (event_source->event_enable))
  {
  if (g_atomic_int_dec_and_test(&event_source->event))
    return(((callback)(user_data)));
  else
    return(TRUE);
  }
else
  return(TRUE);
}



GSource *mhs_event_source_new(void)
{
GSource *source;
MHSEventSource *event_source;

source = g_source_new(&mhs_event_funcs, sizeof(MHSEventSource));
event_source = (MHSEventSource *)source;
g_atomic_int_set(&event_source->event, 0);
event_source->event_enable = 1;
return(source);
}


guint mhs_event_add_full(gint priority, GSourceFunc function, gpointer data, GDestroyNotify notify)
{
GSource *source;
guint id;

g_return_val_if_fail (function != NULL, 0);

source = mhs_event_source_new();

if (priority != G_PRIORITY_DEFAULT)
  g_source_set_priority (source, priority);

g_source_set_callback(source, function, data, notify);
id = g_source_attach(source, NULL);
g_source_unref(source);

return(id);
}


guint mhs_event_add(GSourceFunc function, gpointer data)
{
return(mhs_event_add_full(G_PRIORITY_DEFAULT, function, data, NULL));
}


gint mhs_event_enable(guint id, gint enable)
{
MHSEventSource *event_source;

if (!id)
  return(-1);
event_source = (MHSEventSource *)g_main_context_find_source_by_id(NULL, id);
if (!event_source)
  return(-1);
event_source->event_enable = enable;
return(0);
}


gint mhs_reset_event(guint id)
{
MHSEventSource *event_source;

if (!id)
  return(-1);
event_source = (MHSEventSource *)g_main_context_find_source_by_id(NULL, id);
if (!event_source)
  return(-1);
g_atomic_int_set(&event_source->event, 0);
return(0);
}


gint mhs_set_event(guint id)
{
MHSEventSource *event_source;

if (!id)
  return(-1);
event_source = (MHSEventSource *)g_main_context_find_source_by_id(NULL, id);
if (!event_source)
  return(-1);
g_atomic_int_set(&event_source->event, 1);
g_main_context_wakeup(NULL);
return(0);
}
