/* $Id: libxmms_tracking.c,v 1.9 2005/02/18 21:48:31 pez Exp $ */
/* Some Includes */
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

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
#define DEFAULT_PERCENT 50
#define DEFAULT_SECONDS 240

/* Hooks */
static void init(void);
static void cleanup(void);
static void configure(void);

/* Other functions */
static void read_config(void);

/* Our Config Vars */
static gint percent_done = -1;
static gint seconds_past = -1;
static gchar *cmd_line = NULL;

/* Keep track of the configure window */
static GtkWidget *configure_win = NULL;
static GtkWidget *configure_vbox = NULL;

/* Keep track of our config window input boxes */
static GtkWidget *percent_entry;
static GtkWidget *seconds_entry;
static GtkWidget *cmd_entry;

/* Our thread prototype and handle */
static void * worker_func(void *);
static pthread_t pt_worker;

/* Control variable for thread */
static int going;

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
	going = 1;

	fprintf(stderr, "Plugin init\n");
	if (pthread_create(&pt_worker, NULL, worker_func, NULL))
	{
		return;
	}
}

static void cleanup(void)
{
	fprintf(stderr, "In cleanup\n");
	void *dummy;

	if (cmd_line)
		g_free(cmd_line);
	cmd_line = NULL;

	if (going)
	{
		going = 0;
		pthread_join(pt_worker, &dummy);
	}
}

static void save_and_close(GtkWidget *w, gpointer data)
{
	char *percent;
	char *seconds;
	char *cmd;

	ConfigFile *cfgfile = xmms_cfg_open_default_file();

	percent = gtk_entry_get_text(GTK_ENTRY(percent_entry));
	seconds = gtk_entry_get_text(GTK_ENTRY(seconds_entry));
	cmd = gtk_entry_get_text(GTK_ENTRY(cmd_entry));

	xmms_cfg_write_int(cfgfile, CFGCAT, "percent_done", atoi(percent));
	xmms_cfg_write_int(cfgfile, CFGCAT, "seconds_past", atoi(seconds));
	xmms_cfg_write_string(cfgfile, CFGCAT, "cmd_line", cmd);
	xmms_cfg_write_default_file(cfgfile);
	xmms_cfg_free(cfgfile);

	gtk_widget_destroy(configure_win);
}

static void *worker_func(void *data)
{
	int otime;
	int playing;
	int run = 1;
	int sessid = xmms_tracking.xmms_session;
	int prevpos = -1, prevlen = -1;
	int pos, len;
	int oldtime = 0;
	int docmd;

	otime = xmms_remote_get_output_time(sessid);

	while (run)
	{
		/* Are we playing right now? */
		playing = xmms_remote_is_playing(sessid);
		if (!playing)
		{
			prevpos = -1;
			prevlen = -1;
		}

		/* Grab information about the current track */
		pos = xmms_remote_get_playlist_pos(sessid);
		len = xmms_remote_get_playlist_time(sessid, pos);

		/* Figure out how much time has past */
		if (otime == -1)
			otime = xmms_remote_get_output_time(sessid);
		oldtime = otime;
		otime = xmms_remote_get_output_time(sessid);

		/* Are we supposed to run the command yet? */
		docmd = (otime/1000 > seconds_past) ||
			(((double)otime/((double)len + 1) * 100) >= percent_done);

		/* Sanity check - has the user skipped in the track? */
		if (otime - oldtime > 5000 && (prevpos != pos && prevlen != len && playing))
		{
			fprintf(stderr, "No skipping allowed, discarding song.\n");
			prevpos = pos;
			prevlen = len;
		}
		
		/* Run the command */
		if ((pos != prevpos && len != prevlen) && playing && docmd)
		{
			fprintf(stderr, "Would run the command now, at second %d, pos %d\n", (otime/1000), pos);
		}
		run = going;
		usleep(100000);
	}
	pthread_exit(NULL);
}

static void configure_ok_cb(GtkWidget *w, gpointer data)
{
	char *cmd;

	cmd = gtk_entry_get_text(GTK_ENTRY(cmd_entry));
	/* Theoretically do some checking on cmd here */
	save_and_close(NULL, NULL);
}

static void configure(void)
{
	GtkWidget *percent_hbox, *percent_label, *percent_frame, *percent_vbox, *percent_desc;
	GtkWidget *seconds_hbox, *seconds_label, *seconds_frame, *seconds_vbox, *seconds_desc;
	GtkWidget *cmd_hbox, *cmd_label, *cmd_frame, *cmd_vbox, *cmd_desc;
	GtkWidget *configure_bbox, *configure_ok, *configure_cancel;
	gchar *temp;

	if (configure_win)
		return;

	read_config();

	/* Set up the initial window */
	configure_win = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_signal_connect(GTK_OBJECT(configure_win), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &configure_win);
	gtk_window_set_title(GTK_WINDOW(configure_win), "Tracking Information");
	gtk_container_set_border_width(GTK_CONTAINER(configure_win), 10);

	/* Set up the vbox to put things in */
	configure_vbox = gtk_vbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(configure_win), configure_vbox);

	/* Container for Percent Done indicator */
	percent_frame = gtk_frame_new("Percent Done");
	gtk_box_pack_start(GTK_BOX(configure_vbox), percent_frame, FALSE, FALSE, 0);
	percent_vbox = gtk_vbox_new(FALSE, 10);
	gtk_container_set_border_width(GTK_CONTAINER(percent_vbox), 5);
	gtk_container_add(GTK_CONTAINER(percent_frame), percent_vbox);

	/* Container for Seconds Past indicator */
	seconds_frame = gtk_frame_new("Seconds Past");
	gtk_box_pack_start(GTK_BOX(configure_vbox), seconds_frame, FALSE, FALSE, 0);
	seconds_vbox = gtk_vbox_new(FALSE, 10);
	gtk_container_set_border_width(GTK_CONTAINER(seconds_vbox), 5);
	gtk_container_add(GTK_CONTAINER(seconds_frame), seconds_vbox);

	/* Container for Command indicator */
	cmd_frame = gtk_frame_new("Seconds Past");
	gtk_box_pack_start(GTK_BOX(configure_vbox), cmd_frame, FALSE, FALSE, 0);
	cmd_vbox = gtk_vbox_new(FALSE, 10);
	gtk_container_set_border_width(GTK_CONTAINER(cmd_vbox), 5);
	gtk_container_add(GTK_CONTAINER(cmd_frame), cmd_vbox);

	/* Description for Percent Done */
	temp = g_strdup_printf("Run the command when the file is x%% done.");
	percent_desc = gtk_label_new(temp);
	g_free(temp);
	gtk_label_set_justify(GTK_LABEL(percent_desc), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC(percent_desc), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(percent_vbox), percent_desc, FALSE, FALSE, 0);
	gtk_label_set_line_wrap(GTK_LABEL(percent_desc), TRUE);

	/* Label for Percent Done */
	percent_hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(percent_vbox), percent_hbox, FALSE, FALSE, 0);
	percent_label = gtk_label_new("Percent Done:");
	gtk_box_pack_start(GTK_BOX(percent_hbox), percent_label, FALSE, FALSE, 0);

	/* Entry box for Percent Done */
	percent_entry = gtk_entry_new();
	if (percent_done)
		gtk_entry_set_text(GTK_ENTRY(percent_entry), g_strdup_printf("%d", percent_done));
	gtk_widget_set_usize(percent_entry, 200, -1);
	gtk_box_pack_start(GTK_BOX(percent_hbox), percent_entry, TRUE, TRUE, 0);

	/* Description for Seconds Past */
	temp = g_strdup_printf("Run the command if the file passes x seconds.");
	seconds_desc = gtk_label_new(temp);
	g_free(temp);
	gtk_label_set_justify(GTK_LABEL(seconds_desc), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC(seconds_desc), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(seconds_vbox), seconds_desc, FALSE, FALSE, 0);
	gtk_label_set_line_wrap(GTK_LABEL(seconds_desc), TRUE);

	/* Label for Seconds Past */
	seconds_hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(seconds_vbox), seconds_hbox, FALSE, FALSE, 0);
	seconds_label = gtk_label_new("Seconds Past:");
	gtk_box_pack_start(GTK_BOX(seconds_hbox), seconds_label, FALSE, FALSE, 0);

	/* Entry box for Seconds Past */
	seconds_entry = gtk_entry_new();
	if (seconds_past)
		gtk_entry_set_text(GTK_ENTRY(seconds_entry), g_strdup_printf("%d", seconds_past));
	gtk_widget_set_usize(seconds_entry, 200, -1);
	gtk_box_pack_start(GTK_BOX(seconds_hbox), seconds_entry, TRUE, TRUE, 0);

	/* Description for Command */
	temp = g_strdup_printf("Run this command.");
	cmd_desc = gtk_label_new(temp);
	g_free(temp);
	gtk_label_set_justify(GTK_LABEL(cmd_desc), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC(cmd_desc), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(cmd_vbox), cmd_desc, FALSE, FALSE, 0);
	gtk_label_set_line_wrap(GTK_LABEL(cmd_desc), TRUE);

	/* Label for Command */
	cmd_hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(cmd_vbox), cmd_hbox, FALSE, FALSE, 0);
	cmd_label = gtk_label_new("Command Line:");
	gtk_box_pack_start(GTK_BOX(cmd_hbox), cmd_label, FALSE, FALSE, 0);

	/* Entry box for Command */
	cmd_entry = gtk_entry_new();
	if (cmd_line)
		gtk_entry_set_text(GTK_ENTRY(cmd_entry), cmd_line);
	gtk_widget_set_usize(cmd_entry, 200, -1);
	gtk_box_pack_start(GTK_BOX(cmd_hbox), cmd_entry, TRUE, TRUE, 0);

	/*** GOOD FUCKING GOD ***/

	/* Set up area for buttons at the bottom */
	configure_bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(configure_bbox), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(configure_bbox), 5);
	gtk_box_pack_start(GTK_BOX(configure_vbox), configure_bbox, FALSE, FALSE, 0);

	/* OK button */
	configure_ok = gtk_button_new_with_label("Ok");
	gtk_signal_connect(GTK_OBJECT(configure_ok), "clicked", GTK_SIGNAL_FUNC(configure_ok_cb), NULL);
	GTK_WIDGET_SET_FLAGS(configure_ok, GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(configure_bbox), configure_ok, TRUE, TRUE, 0);
	gtk_widget_grab_default(configure_ok);

	/* Cancel button */
	configure_cancel = gtk_button_new_with_label("Cancel");
	gtk_signal_connect_object(GTK_OBJECT(configure_cancel), "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT(configure_win));
	GTK_WIDGET_SET_FLAGS(configure_cancel, GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(configure_bbox), configure_cancel, TRUE, TRUE, 0);

	/* ... aaaand we're done */
	gtk_widget_show_all(configure_win);
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

	if (percent_done == -1)
	{
		percent_done = DEFAULT_PERCENT;
	}

	if (seconds_past == -1)
	{
		seconds_past = DEFAULT_SECONDS;
	}
}
