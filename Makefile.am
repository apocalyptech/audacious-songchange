# $Id: Makefile.am,v 1.3 2005/02/18 23:53:36 pez Exp $
lib_LTLIBRARIES = libxmms_tracking.la

libdir = @XMMS_GENERAL_PLUGIN_DIR@

CFLAGS = -Wall @XMMS_CFLAGS@ @CFLAGS@ -I$(tob_builddir) -I$(top_srcdir) \
	 @GTK_CFLAGS@ \
	 -DMETA_DEBUG=1
	
LIBS = @XMMS_LIBS@ @GTK_LIBS@
libxmms_tracking_la_LDFLAGS = -module -avoid-version
libxmms_tracking_la_SOURCES = libxmms_tracking.c \
	tags/ape.c \
	tags/fmt.c \
	tags/id3genres.c \
	tags/id3v1.c \
	tags/id3v2.c \
	tags/itunes.c \
	tags/tags.c \
	tags/unicode.c \
	tags/vorbis.c \
	tags/wma.c
