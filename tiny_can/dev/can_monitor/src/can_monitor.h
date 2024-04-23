#ifndef __CAN_MONITOR__
#define __CAN_MONITOR__

#include "global.h"
#include "paths.h"
#include <glib.h>
#include <gtk/gtk.h>
#include "can_types.h"
#include "can_drv.h"
#include "mhs_signal.h"
#include "mhs_signals.h"
#include "util.h"
#include "gtk_util.h"
#include "main.h"

extern struct TCanMsg DefaultCanMsg;
extern struct TCanMsg LastTransmit;

#define SetLastTransmit(can_msg) memcpy(&LastTransmit, can_msg, sizeof(struct TCanMsg))

#endif
