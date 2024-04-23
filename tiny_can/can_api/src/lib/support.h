#ifndef __SUPPORT_H__
#define __SUPPORT_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* Gettext package. */
#define GETTEXT_PACKAGE "mhsudsflash"
/* Name of package */
#define PACKAGE "mhsudsflash"

/*
 * Standard gettext macros.
 */
#ifdef ENABLE_NLS
  #include <libintl.h>
  #undef _
  #define _(String) dgettext(PACKAGE, String)
  #ifdef gettext_noop
    #define N_(String) gettext_noop (String)
  #else
    #define N_(String) (String)
  #endif
#else
  #define textdomain(String) (String)
  #define gettext(String) (String)
  #define dgettext(Domain,Message) (Message)
  #define dcgettext(Domain,Message,Type) (Message)
  #define bindtextdomain(Domain,Directory) (Domain)
  #define _(String) (String)
  #define N_(String) (String)
#endif

#ifdef __cplusplus
  }
#endif

#endif
