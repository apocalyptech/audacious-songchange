# $Id: Makefile.am,v 1.5 2005/11/28 18:07:40 pez Exp $
if MAKE_XMMS
xmmslib_LTLIBRARIES = libxmms_tracking.la
xmmslibdir = @XMMS_GENERAL_PLUGIN_DIR@
endif
if MAKE_BMP
bmplib_LTLIBRARIES = libbmp_tracking.la
bmplibdir = `pkg-config bmp --variable=general_plugin_dir`
endif

AM_CFLAGS = -Wall @CFLAGS@ @GTK_CFLAGS@ -I$(tob_builddir) -I$(top_srcdir) \
         -DMETA_DEBUG=0
AM_LDFLAGS = @LDFLAGS@ -module -avoid-version
LIBS = @GTK_LIBS@

libxmms_tracking_la_CFLAGS = $(AM_CFLAGS) @XMMS_CFLAGS@ -DMAKE_XMMS=1
libxmms_tracking_la_LIBS = $(LIBS) @XMMS_LIBS@
libxmms_tracking_la_LDFLAGS = $(AM_LDFLAGS)
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

libbmp_tracking_la_CFLAGS = $(AM_CFLAGS) @BMP_CFLAGS@ -DMAKE_BMP=1
libbmp_tracking_la_LIBS = $(LIBS) @BMP_LIBS@
libbmp_tracking_la_LDFLAGS = $(AM_LDFLAGS)
libbmp_tracking_la_SOURCES = libxmms_tracking.c \
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
