/* *********** Tiny-CAN Monitor **************                            */
/* Copyright (C) 2005 Klaus Demlehner (klaus@mhs-elektronik.de)           */
/*   Tiny-CAN Project Homepage: http://www.mhs-elektronik.de              */
/*                                                                        */
/* This program is free software; you can redistribute it and/or modify   */
/* it under the terms of the GNU General Public License as published by   */
/* the Free Software Foundation; either version 2 of the License, or      */
/* (at your option) any later version.                                    */
/*                                                                        */
/* This program is distributed in the hope that it will be useful,        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/* GNU General Public License for more details.                           */
/*                                                                        */
/* You should have received a copy of the GNU General Public License      */
/* along with this program; if not, write to the Free Software            */
/* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */
#include "util.h"
#include "mhs_signal.h"


static struct TMhsSignal **MhsSignals;


void mhs_signal_init(struct TMhsSignal **mhs_signals)
{
MhsSignals = mhs_signals;
}


struct TMhsSignal *mhs_signal_create(void)
{
struct TMhsSignal *sig;

if (!MhsSignals)
  return(NULL);
sig = *MhsSignals;
if (!sig)
  {
  // Liste ist leer
  sig = (struct TMhsSignal *)g_malloc0(sizeof(struct TMhsSignal));
  *MhsSignals = sig;
  }
else
  { // Neues Element anhängen
  while (sig->Next != NULL) sig = sig->Next;
  sig->Next = (struct TMhsSignal *)g_malloc0(sizeof(struct TMhsSignal));
  sig = sig->Next;
  if (!sig)
    return(NULL);        // Nicht genügend Arbetsspeicher
  }
sig->Next = NULL;
return(sig);
}


int mhs_signal_destroy(struct TMhsSignal *sig)
{
struct TMhsSignal *l, *prev;
struct TMhsSignalHandler *sig_h, *sig_h_n;

if (!MhsSignals)
  return(-1);
prev = NULL;
for (l = *MhsSignals; l; l = l->Next)
  {
  if (l == sig)
    {
    if (prev)
      prev->Next = l->Next;
    else
      *MhsSignals = l->Next;
    for (sig_h = l->Handlers; sig_h; sig_h = sig_h_n)
      {
      sig_h_n = sig_h->Next;
      g_free(sig_h);
      }
    g_free(l);
    return(0);
    }
  prev = l;
  }
return(-1);
}


struct TMhsSignalHandler *mhs_signal_create_handler(struct TMhsSignal *sig)
{
struct TMhsSignalHandler *sig_h;

if (!sig)
  return(NULL);
sig_h = sig->Handlers;
if (!sig_h)
  {
  // Liste ist leer
  sig_h = (struct TMhsSignalHandler *)g_malloc0(sizeof(struct TMhsSignalHandler));
  sig->Handlers = sig_h;
  }
else
  { // Neues Element anhängen
  while (sig_h->Next != NULL) sig_h = sig_h->Next;
  sig_h->Next = (struct TMhsSignalHandler *)g_malloc0(sizeof(struct TMhsSignalHandler));
  sig_h = sig_h->Next;
  if (!sig_h)
    return(NULL);        // Nicht genügend Arbetsspeicher
  }
sig_h->Next = NULL;
return(sig_h);
}


int mhs_signal_destroy_handler(struct TMhsSignal *sig, struct TMhsSignalHandler *sig_h)
{
struct TMhsSignalHandler *l, *prev;

prev = NULL;
for (l = sig->Handlers; l; l = l->Next)
  {
  if (l == sig_h)
    {
    if (prev)
      prev->Next = l->Next;
    else
      sig->Handlers = l->Next;
    g_free(l);
    return(0);
    }
  prev = l;
  }
return(-1);
}



// Signal auslösen
void mhs_signal_emit(gulong grup, gulong signal, gpointer event_data)
{
struct TMhsSignal *sig;
struct TMhsSignalHandler *sig_h;

if (!MhsSignals)
  return;
for (sig = *MhsSignals; sig; sig = sig->Next)
  {
  if ((sig->SignalNr == signal) && (sig->Grup == grup))
    {
    if (sig->Enable)
      {
      for (sig_h = sig->Handlers; sig_h; sig_h = sig_h->Next)
        {
        if (sig_h->Proc)
          (sig_h->Proc)(signal, event_data, sig_h->UserData);
        }
      }
    break;
    }
  }
}


// Signal an Handler binden
void mhs_signal_connect(gpointer owner, gulong grup, gulong signal, TMhsSignalCB proc, gpointer user_data)
{
struct TMhsSignal *sig;
struct TMhsSignalHandler *sig_h;
int hit;

if (!MhsSignals)
  return;
hit = 0;
for (sig = *MhsSignals; sig; sig = sig->Next)
  {
  if ((sig->SignalNr == signal) && (sig->Grup == grup))
    {
    for (sig_h = sig->Handlers; sig_h; sig_h = sig_h->Next)
      {
      if (sig_h->Proc == proc)
        {
        sig_h->UserData = user_data;
        sig->Enable = 1;
        return;
        }
      }
    hit = 1;
    break;
    }
  }
if (!hit)
  {
  if (!(sig = mhs_signal_create()))
    return;
  sig->SignalNr = signal;
  sig->Grup = grup;
  sig->Enable = 1;
  }
sig_h = mhs_signal_create_handler(sig);
if (!sig_h)
  return;
sig_h->Owner = owner;
sig_h->Proc = proc;
sig_h->UserData = user_data;
}


// Signal handler löschen
void mhs_signal_disconnect(gulong grup, gulong signal, TMhsSignalCB proc)
{
struct TMhsSignal *sig, *next_sig;
struct TMhsSignalHandler *sig_h, *next_sig_h;

if (!MhsSignals)
  return;
sig = *MhsSignals;
while (sig)
  {
  next_sig = sig->Next;
  if ((sig->SignalNr == signal) && (sig->Grup == grup))
    {
    sig_h = sig->Handlers;
    while (sig_h)
      {
      next_sig_h = sig_h->Next;
      if (sig_h->Proc == proc)
        {
        mhs_signal_destroy_handler(sig, sig_h);
        if (sig->Handlers == NULL)
          mhs_signal_destroy(sig);
        break;
        }
      sig_h = next_sig_h;
      }
    }
  sig = next_sig;
  }
}


void mhs_signal_disconnect_owner(gpointer owner)
{
struct TMhsSignal *sig, *next_sig;
struct TMhsSignalHandler *sig_h, *next_sig_h;

if (!MhsSignals)
  return;
sig = *MhsSignals;
while (sig)
  {
  next_sig = sig->Next;
  sig_h = sig->Handlers;
  while (sig_h)
    {
    next_sig_h = sig_h->Next;
    if (sig_h->Owner == owner)
      {
      mhs_signal_destroy_handler(sig, sig_h);
      if (sig->Handlers == NULL)
        {
        mhs_signal_destroy(sig);
        break;
        }
      }
    sig_h = next_sig_h;
    }
  sig = next_sig;
  }
}


// Alle Signal handler für ein bestimmtes Signal löschen
void mhs_signal_clear(gulong grup, gulong signal)
{
struct TMhsSignal *sig;

if (!MhsSignals)
  return;
for (sig = *MhsSignals; sig; sig = sig->Next)
  {
  if ((sig->SignalNr == signal) && (sig->Grup == grup))
    mhs_signal_destroy(sig);
  }
}


void mhs_signal_clear_all(void)
{
struct TMhsSignal *sig, *sig_n;
struct TMhsSignalHandler *sig_h, *sig_h_n;

if (!MhsSignals)
  return;
for (sig = *MhsSignals; sig; sig = sig_n)
  {
  sig_n = sig->Next;
  for (sig_h = sig->Handlers; sig_h; sig_h = sig_h_n)
    {
    sig_h_n = sig_h->Next;
    g_free(sig_h);
    }
  g_free(sig);
  }
*MhsSignals = NULL;
}


void mhs_signal_block(gulong grup, gulong signal)
{
struct TMhsSignal *sig;

for (sig = *MhsSignals; sig; sig = sig->Next)
  {
  if ((sig->SignalNr == signal) && (sig->Grup == grup))
    {
    sig->Enable = 0;
    break;
    }
  }
}


void mhs_signal_unblock(gulong grup, gulong signal)
{
struct TMhsSignal *sig;

for (sig = *MhsSignals; sig; sig = sig->Next)
  {
  if ((sig->SignalNr == signal) && (sig->Grup == grup))
    {
    sig->Enable = 1;
    break;
    }
  }
}
