dnl Process this file with autoconf to produce a configure script.
dnl $Id: configure.in,v 1.6 2008/02/25 16:49:43 pez Exp $
AC_PREREQ([2.59])
AC_INIT([libxmms-tracking], [0.2.0])

AC_CONFIG_SRCDIR([libxmms_tracking.cc])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX
LT_INIT

AUD_COMMON_PROGS

BUILDSYS_SHARED_LIB

dnl Check for gtk stuff
dnl ===================

PKG_CHECK_MODULES(GTK, [glib-2.0 >= 2.4.0 gtk+-2.0 >= 2.4.0],
  [],
  [ AC_MSG_ERROR([*** GLIB >= 2.4.0 or GTK+ >= 2.4.0 not installed - please install first *** ])]
)
CPPFLAGS="$CPPFLAGS $GTK_CFLAGS"
LIBS="$LIBS $GTK_LIBS"

dnl Check for Audacious
dnl ===================

PKG_CHECK_MODULES(AUDACIOUS, [audacious >= 3.7],
    [],
    [AC_MSG_ERROR([Cannot find Audacious 3.7; have you installed Audacious yet?])]
)
CPPFLAGS="$CPPFLAGS $AUDACIOUS_CFLAGS"
LIBS="$LIBS $AUDACIOUS_LIBS"

plugindir=`pkg-config audacious --variable=plugin_dir`
AC_SUBST(plugindir)

dnl Reliably #include "config.h" (for large file support)
dnl =====================================================
CPPFLAGS="$CPPFLAGS -include config.h"

dnl Generate config files
dnl =====================

#AC_CONFIG_FILES([
#    buildsys.mk
#    extra.mk
#])

AC_OUTPUT(Makefile)
