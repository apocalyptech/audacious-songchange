# $Id: Makefile.am,v 1.2 2005/02/18 05:48:38 pez Exp $
lib_LTLIBRARIES = libxmms_tracking.la

libdir = @XMMS_GENERAL_PLUGIN_DIR@

CFLAGS = -Wall @XMMS_CFLAGS@ @CFLAGS@ -I$(tob_builddir) -I$(top_srcdir) \
	 @GTK_CFLAGS@
	
LIBS = @XMMS_LIBS@ @GTK_LIBS@
libxmms_tracking_la_LDFLAGS = -module -avoid-version
libxmms_tracking_la_SOURCES = libxmms_tracking.c
