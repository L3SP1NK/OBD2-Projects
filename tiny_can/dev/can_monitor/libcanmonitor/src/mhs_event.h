#ifndef __MHS_EVENT_H__
#define __MHS_EVENT_H__

#include <glib.h>

GSource *mhs_event_source_new(void);
guint mhs_event_add_full(gint priority, GSourceFunc function, gpointer data, GDestroyNotify notify);
guint mhs_event_add(GSourceFunc function, gpointer data);

gint mhs_event_enable(guint id, gint enable);
gint mhs_reset_event(guint id);
gint mhs_set_event(guint id);


#endif
