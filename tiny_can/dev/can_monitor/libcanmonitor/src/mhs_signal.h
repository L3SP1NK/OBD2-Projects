#ifndef __MHS_SIGNAL__
#define __MHS_SIGNAL__

#include <glib.h>


typedef void(*TMhsSignalCB)(gulong signal, gpointer event_data, gpointer user_data);

struct TMhsSignalHandler
  {
  struct TMhsSignalHandler *Next;
  gpointer Owner;
  TMhsSignalCB Proc;
  gpointer UserData;
  };


struct TMhsSignal
  {
  struct TMhsSignal *Next;
  gulong SignalNr;
  gulong Grup;
  unsigned int Enable;
  struct TMhsSignalHandler *Handlers;
  };

void mhs_signal_init(struct TMhsSignal **mhs_signals);

void mhs_signal_emit(gulong grup, gulong signal, gpointer event_data);
void mhs_signal_connect(gpointer owner, gulong grup, gulong signal, TMhsSignalCB proc, gpointer user_data);
void mhs_signal_disconnect(gulong grup, gulong signal, TMhsSignalCB proc);
void mhs_signal_disconnect_owner(gpointer owner);
void mhs_signal_clear(gulong grup, gulong signal);
void mhs_signal_clear_all(void);
void mhs_signal_block(gulong grup, gulong signal);
void mhs_signal_unblock(gulong grup, gulong signal);

#endif
