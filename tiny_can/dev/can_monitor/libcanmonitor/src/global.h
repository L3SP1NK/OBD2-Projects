#ifndef __GLOBAL_H__
#define __GLOBAL_H__

// #include "err_codes.h"

// GTK+ only (export callbacks for Glade linking at runtime)
#if defined(__WIN32__) && !defined(HAVE_FVISIBILITY)
# define GLADE_CB __declspec(dllexport)
#elif defined(HAVE_FVISIBILITY)
# define GLADE_CB __attribute__ ((visibility("default")))
#else
# define GLADE_CB
#endif

#define USE_EVENT_MODE

#endif
