/* $Id: libxmms_tracking.c,v 1.5 2005/02/18 06:00:52 pez Exp $ */
/* Some Includes */
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

/* Some More Includes */
#include <sys/types.h>
#include <sys/wait.h>

/* GTK Includes */
#include <gtk/gtk.h>

/* XMMS Includes */
#include <xmms/plugin.h>
#include <xmms/configfile.h>
#include <xmms/xmmsctrl.h>
#include <xmms/formatter.h>

/* Local includes */
#include "config.h"

/* Other defines */
#define CFGCAT "xmms_tracking"

/* Hooks */
static void init(void);
static void cleanup(void);
static void configure(void);

/* Other functions */
static void read_config(void);

/* Static Vars */
static gint percent_done;
static gint seconds_past;
static gchar *cmd_line = NULL;

static GeneralPlugin xmms_tracking =
{
	NULL,		/* handle */
	NULL,		/* filename */
	-1,		/* xmms_session */
	NULL,		/* description */
	init,
	NULL,
	configure,
	cleanup,
};

GeneralPlugin *get_gplugin_info(void)
{
	xmms_tracking.description = g_strdup_printf("XMMS-Tracking %s", VERSION);
	return &xmms_tracking;
}

static void init(void)
{
	read_config();

	fprintf(stderr, "Plugin init\n");
}

static void cleanup(void)
{
	fprintf(stderr, "In cleanup\n");
}

static void configure(void)
{
	fprintf(stderr, "In configure\n");
}

static void read_config(void)
{
	ConfigFile *cfgfile;

	g_free(cmd_line);
	cmd_line = NULL;

	if ((cfgfile = xmms_cfg_open_default_file()) != NULL)
	{
		xmms_cfg_read_int(cfgfile, CFGCAT, "percent_done", &percent_done);
		xmms_cfg_read_int(cfgfile, CFGCAT, "seconds_past", &seconds_past);
		xmms_cfg_read_string(cfgfile, CFGCAT, "cmd_line", &cmd_line);
		xmms_cfg_free(cfgfile);
	}
}
