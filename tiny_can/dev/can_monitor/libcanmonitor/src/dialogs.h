#ifndef __DIALOGS_H__
#define __DIALOGS_H__

#include <stdlib.h>
#include <string.h>

#define MSG_TYPE_MESSAGE  0x00
#define MSG_TYPE_WARNING  0x01
#define MSG_TYPE_FRAGE    0x02
#define MSG_TYPE_ERROR    0x03

#define DLG_YES_NO_BUTTONS 0x04

int msg_box(guint flags, const gchar *title, const gchar *message, ...);

#endif

