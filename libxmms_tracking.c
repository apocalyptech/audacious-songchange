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

static void init(void);
static void cleanup(void);
static void configure(void);

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

static void init(void)
{
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
