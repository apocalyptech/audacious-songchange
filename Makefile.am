# $Id: Makefile.am,v 1.8 2008/08/05 19:27:20 pez Exp $
audlib_LTLIBRARIES = libaud_tracking.la
audlibdir = `pkg-config audacious --variable=general_plugin_dir`

AM_CFLAGS = -Wall @CFLAGS@ @GTK_CFLAGS@ -I$(tob_builddir) -I$(top_srcdir) \
         -DMETA_DEBUG=0
AM_LDFLAGS = @LDFLAGS@ -module -avoid-version
ACLOCAL_AMFLAGS = -I m4
LIBS = @GTK_LIBS@

libaud_tracking_la_CFLAGS = $(AM_CFLAGS) @AUD_CFLAGS@
libaud_tracking_la_LIBS = $(LIBS) @AUD_LIBS@
libaud_tracking_la_LDFLAGS = $(AM_LDFLAGS)
libaud_tracking_la_SOURCES = formatter.c \
	libxmms_tracking.c \
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
