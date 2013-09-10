/* $Id: libxmms_tracking.c,v 1.43 2008/08/05 19:27:20 pez Exp $ */
/* Some Includes */
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Some More Includes */
#include <sys/types.h>
#include <sys/wait.h>

/* GTK/Glib Includes */
#include <gtk/gtk.h>

/* Player Includes */
#include <audacious/plugin.h>
#include <audacious/misc.h>
#include <audacious/playlist.h>
#include <audacious/drct.h>
#include "formatter.h"

/* Local includes */
#include "config.h"

/* Other defines */
#define CFGCAT "xmms_tracking"
#define DEFAULT_PERCENT 50
#define DEFAULT_SECONDS 240
#define DEFAULT_MINIMUM 30

/* Hooks */
static gboolean init(void);
static void cleanup(void);
static void configure(void);

/* Other functions */
static void read_config(void);

/* Our Config Vars */
static gint percent_done = -1;
static gint seconds_past = -1;
static gint minimum_len = -1;
static gchar *cmd_line = NULL;

/* Keep track of the configure window */
static GtkWidget *configure_win = NULL;
static GtkWidget *configure_vbox = NULL;

/* Keep track of our config window input boxes */
static GtkWidget *percent_entry;
static GtkWidget *seconds_entry;
static GtkWidget *minimum_entry;
static GtkWidget *cmd_entry;

/* Our thread prototype and handle */
static void * worker_func(void *);
static pthread_t pt_worker;

/* Control variable for thread */
static int going;

static gboolean init(void)
{
    read_config();
    going = 1;

    fprintf(stderr, "Plugin init\n");
    if (pthread_create(&pt_worker, NULL, worker_func, NULL) == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
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
    char *minlen;
    char *cmd;

    percent = gtk_entry_get_text(GTK_ENTRY(percent_entry));
    seconds = gtk_entry_get_text(GTK_ENTRY(seconds_entry));
    minlen = gtk_entry_get_text(GTK_ENTRY(minimum_entry));
    cmd = gtk_entry_get_text(GTK_ENTRY(cmd_entry));

    aud_set_int(CFGCAT, "percent_done", atoi(percent));
    aud_set_int(CFGCAT, "seconds_past", atoi(seconds));
    aud_set_int(CFGCAT, "minimum_len", atoi(minlen));
    aud_set_string(CFGCAT, "cmd_line", cmd);

    gtk_widget_destroy(configure_win);
}

/*
 * TODO: FIX THIS, IT'S TOTALLY LAME
 */
static char *wtfescape(char *string)
{
    const gchar *special = "$`"; /* chars to escape */
    char *in = string, *out;
    char *escaped;
    int num = 0;

    while (*in != '\0')
        if (strchr(special, *in++))
            num++;

    escaped = g_malloc(strlen(string) + num + 1);

    in = string;
    out = escaped;
    
    while (*in != '\0')
    {
        if (strchr(special, *in))
            *out++ = '\\';
        *out++ = *in++;
    }
    *out = '\0';

    return escaped;
}

static char *escape_shell_chars(char *string)
{
    const gchar *special = "$`\"\\"; /* chars to escape */
    char *in = string, *out;
    char *escaped;
    int num = 0;

    while (*in != '\0')
        if (strchr(special, *in++))
            num++;

    escaped = g_malloc(strlen(string) + num + 1);

    in = string;
    out = escaped;
    
    while (*in != '\0')
    {
        if (strchr(special, *in))
            *out++ = '\\';
        *out++ = *in++;
    }
    *out = '\0';

    return wtfescape(escaped);
}

static void associate(Formatter *formatter, char letter, char *data)
{
    char *tmp;
    if (data == NULL)
    {
        formatter_associate(formatter, letter, "");
    }
    else
    {
        tmp = escape_shell_chars(data);
        formatter_associate(formatter, letter, tmp);
        g_free(tmp);
    }
}

static void bury_child(int signal)
{
    waitpid(-1, NULL, WNOHANG);
}

static void execute_command(gchar *cmd)
{
    gchar *argv[4] = {"/bin/sh", "-c", NULL, NULL};
    gint i;
    argv[2] = cmd;
    signal(SIGCHLD, bury_child);
    if (fork() == 0)
    {
        /* We don't want this process to hog the audio device etc */
        for (i=3; i<255; i++)
            close(i);
        execv("/bin/sh", argv);
    }
}

static void *worker_func(void *data)
{
    int otime = -1;
    gboolean playing;
    int run = 1;
    int prevpos = -1;
    int pos = -1;
    int len = -1;
    int oldtime = 0;
    int docmd;
    //char *fname;
    Formatter *formatter;
    char *cmdstring = NULL;
    gchar *temp;
    gchar *filename = NULL;
    gchar *filenamecomp = NULL;
    gchar *tempfilepath = NULL;
    gchar *tempfilename = NULL;
    gchar *tempfilefull = NULL;
    int processtrack = 0;
    int checkskip = 0;
    int glitchcount = 0;
    gboolean glitched = 0;
    gint playlist;
    Tuple *tuple;
    gboolean fast_call = 0;

    while (run)
    {
        /* See if we're playing */
        playing = aud_drct_get_playing();

        /* Don't really do *anything* unless we're actually playing */
        if (playing)
        {
            /* NOW grab this info */
            playlist = aud_playlist_get_active();
            pos = aud_playlist_get_position(playlist);
            tuple = aud_playlist_entry_get_tuple(playlist, pos, fast_call);
            if (tuple)
            {
                len = tuple_get_int(tuple, FIELD_LENGTH, NULL);
                // TODO: Calling aud_drct_get_time() when aud_drct_get_ready() returns FALSE
                // can lead to the Audacious GUI freezing; this happens sometimes when the
                // check happens inbetween track plays, when the decoder is still warming up
                // (though the freeze does not happen in every case).  This is still technically
                // not threadsafe, since we could technically get into a non-ready state
                // inbetween the ready check and the get_time call, but it should be better than
                // what we were doing before, at least, which would tend to fail reasonably
                // often.
                if (aud_drct_get_ready())
                {
                    otime = aud_drct_get_time();
                }
                else
                {
                    otime = 0;
                }

                /* Check to see if we were skipping (so we can recover if we skip back to the beginning) */
                if (checkskip)
                {
                    if (otime <= 1000)
                    {
                        /* Doing this will let us get picked up by the next block */
                        fprintf(stderr, "pos %d, len %d (%ds): Skip-to-beginning detected, allowing track (at %d)\n", pos+1, len, (len/1000), otime);
                        prevpos = -1;
                        checkskip = 0;
                    }
                }

                /* Check to see if we should start processing */
                if (pos != prevpos)
                {
                    /* Also check for a glitch */
                    if (otime > 1000)
                    {
                        filenamecomp = g_strdup((gchar *) tuple_get_str(tuple, FIELD_FILE_NAME, NULL));
                        if (filename && filenamecomp && strcmp(filename, filenamecomp) == 0)
                        {
                            if (otime >= oldtime-1000 && otime <= oldtime+1000)
                            {
                                /* Apparently our playlist entry just got moved - continue as per usual */
                                fprintf(stderr, "pos %d, len %d (%ds): Position change detected at %d\n", pos+1, len, (len/1000), otime);
                                prevpos = pos;
                            }
                            else
                            {
                                /* Handle this just like a skip */
                                fprintf(stderr, "pos %d, len %d (%ds): Position change weirdness, handling as skip, at %d\n", pos+1, len, (len/1000), otime);
                                processtrack = 0;
                                checkskip = 1;
                            }
                        }
                        else
                        {
                            /* Now into our usual glitching code. */

                            if (!glitched)
                            {
                                /* We should look for skips here... */
                                checkskip = 1;

                                if (glitchcount >= 10)
                                {
                                    fprintf(stderr, "pos %d, len %d (%ds): Discarding track, though it's probably just a playlist move\n",
                                            pos+1, len, (len/1000));
                                    prevpos = pos;
                                    processtrack = 0;
                                    glitched = 1;
                                }
                                else
                                {
                                    glitchcount++;
                                    if (glitchcount == 1)
                                    {
                                        fprintf(stderr, "pos %d, len %d (%ds): Glitching, counter at %d\n", pos+1, len, (len/1000), otime);
                                    }
                                    processtrack = 0;
                                    pos = -1;
                                    len = -1;
                                }
                            }
                        }
                        g_free(filenamecomp);
                    }
                    else
                    {
                        /* Our previous skip-check, if any, is no longer valid. */
                        checkskip = 0;

                        fprintf(stderr, "pos %d, len %d (%ds): Starting track processing, counter at %d\n", pos+1, len, (len/1000), otime);
                        glitchcount = 0;
                        glitched = 0;
                        processtrack = 1;
                        oldtime = otime;
                        prevpos = pos;

                        /* Load our filename, for checking later on */
                        g_free(filename);
                        filename = g_strdup((gchar *) tuple_get_str(tuple, FIELD_FILE_NAME, NULL));
                    }
                }

                /* And now process if we're supposed to */
                if (processtrack)
                {

                    /* Sanity check - minimum length */
                    if (len < (minimum_len*1000))
                    {
                        fprintf(stderr, "pos %d, len %d (%ds): Song shorter than %d seconds, discarding song.\n", pos+1, len, (len/1000), minimum_len);
                        processtrack = 0;
                    }
                    /* Sanity check - has the user skipped in the track? */
                    else if (otime - oldtime > 5000)
                    {
                        fprintf(stderr, "pos %d, len %d (%ds): No skipping allowed, discarding song (%d -> %d)\n", pos+1, len, (len/1000), oldtime, otime);
                        processtrack = 0;
                        checkskip = 1;
                    }
                    /* Finally we're ready to see if we should run the command or not */
                    else
                    {
                        /* Are we supposed to run the command yet? */
                        docmd = (otime/1000 > seconds_past) ||
                            (((double)otime/((double)len + 1) * 100) >= percent_done);
                        
                        /* Run the command if needed */
                        if (docmd)
                        {
                            /* This'll make sure we only call it once per track */
                            processtrack = 0;

                            /* Run the command */
                            if (cmd_line && strlen(cmd_line) > 0)
                            {
                                /* Get meta information */
                                fprintf(stderr, "About to get tuple\n");
                                tempfilepath = g_strdup((gchar *) tuple_get_str(tuple, FIELD_FILE_PATH, NULL));
                                tempfilename = g_strdup((gchar *) tuple_get_str(tuple, FIELD_FILE_NAME, NULL));
                                tempfilefull = g_strconcat(tempfilepath, tempfilename, NULL);
                                fprintf(stderr, "Got tuple\n");
                                //fname = g_filename_from_uri(tempfilefull, NULL, NULL);
                                //fprintf(stderr, "Got URI: %s\n", fname);
                                fprintf(stderr, "Got path: %s\n", tempfilefull);
                                g_free(tempfilepath);
                                g_free(tempfilename);
                                //g_free(tempfilefull);
                                fprintf(stderr, "Freed\n");

                                /* Get our commandline */
                                formatter = formatter_new();

                                associate(formatter, 'a', (char *)tuple_get_str(tuple, FIELD_ARTIST, NULL));
                                associate(formatter, 't', (char *)tuple_get_str(tuple, FIELD_TITLE, NULL));
                                associate(formatter, 'l', (char *)tuple_get_str(tuple, FIELD_ALBUM, NULL));
                                // TODO: is FIELD_YEAR an int as well?
                                associate(formatter, 'y', (char *)tuple_get_str(tuple, FIELD_YEAR, NULL));
                                associate(formatter, 'g', (char *)tuple_get_str(tuple, FIELD_GENRE, NULL));
                                temp = g_strdup_printf("%d", (int)tuple_get_int(tuple, FIELD_TRACK_NUMBER, NULL));
                                associate(formatter, 'n', (char *)temp);
                                g_free(temp);
                                temp = g_strdup_printf("%d", len/1000);
                                associate(formatter, 's', (char *)temp);
                                g_free(temp);
                                cmdstring = formatter_format(formatter, cmd_line);
                                formatter_destroy(formatter);

                                /* Run the command */
                                fprintf(stderr, "pos %d, len %d (%ds): Running command at %d secs: %s\n", pos+1, len, (len/1000), (otime/1000), cmdstring);
                                execute_command(cmdstring);
                                g_free(cmdstring);  /* according to song_change.c, this could get freed too early */
                            }
                            else
                            {
                                fprintf(stderr, "Would run the command now, but no command present.\n");
                            }
                        }
                    }
                }

                /* Update prev vars */
                prevpos = pos;
                oldtime = otime;
            }
        }
        else
        {
            /* reset prev vars if we're not playing */
            prevpos = -1;
            oldtime = -1;

            /* Notify on stderr if we were processing */
            if (processtrack)
            {
                processtrack = 0;
                fprintf(stderr, "pos %d, len %d (%ds): Stopped track processing\n", pos+1, len, (len/1000));
            }
        }

        /* Make sure we don't keep going if we've been called off */
        run = going;

        /* Sleep a bit so we don't bog the system down */
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

static GtkWidget *labelbox_int(GtkWidget *container_box, gchar *text, gint data, GtkWidget *entrybox)
{
    GtkWidget *myhbox;
    GtkWidget *mylabel;
    gchar *temp;

    /* Label */
    myhbox = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(container_box), myhbox, FALSE, FALSE, 0);
    mylabel = gtk_label_new(text);
    gtk_box_pack_start(GTK_BOX(myhbox), mylabel, FALSE, FALSE, 0);

    /* Entry box */
    entrybox = gtk_entry_new();
    if (percent_done)
    {
        temp = g_strdup_printf("%d", data);
        gtk_entry_set_text(GTK_ENTRY(entrybox), temp);
        g_free(temp);
    }
    gtk_widget_set_usize(entrybox, 200, -1);
    gtk_box_pack_start(GTK_BOX(myhbox), entrybox, FALSE, FALSE, 0);

    /* Return the new entrybox */
    return entrybox;
}

static void configure(void)
{
    GtkWidget *condition_frame, *condition_vbox, *condition_desc;
    GtkWidget *cmd_hbox, *cmd_label, *cmd_frame, *cmd_vbox, *cmd_desc;
    GtkWidget *configure_bbox, *configure_ok, *configure_cancel;
    gchar *temp;

    if (configure_win)
        return;

    read_config();

    /* Set up the initial window */
    configure_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_type_hint(GTK_WINDOW(configure_win), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_window_set_title(GTK_WINDOW(configure_win), "Tracking Information");
    gtk_signal_connect(GTK_OBJECT(configure_win), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &configure_win);
    gtk_container_set_border_width(GTK_CONTAINER(configure_win), 10);
    gtk_window_set_default_size(GTK_WINDOW(configure_win), 400, 300);

    /* Set up the vbox to put things in */
    configure_vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_add(GTK_CONTAINER(configure_win), configure_vbox);

    /* Container for Conditional Parameters */
    condition_frame = gtk_frame_new("Conditions");
    gtk_box_pack_start(GTK_BOX(configure_vbox), condition_frame, FALSE, FALSE, 0);
    condition_vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(condition_vbox), 5);
    gtk_container_add(GTK_CONTAINER(condition_frame), condition_vbox);

    /* Container for Command indicator */
    cmd_frame = gtk_frame_new("Command");
    gtk_box_pack_start(GTK_BOX(configure_vbox), cmd_frame, FALSE, FALSE, 0);
    cmd_vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(cmd_vbox), 5);
    gtk_container_add(GTK_CONTAINER(cmd_frame), cmd_vbox);

    /* Description for Conditional Parameters */
    temp = g_strdup_printf("The command will be run when one of the following "
            "conditions are met, whichever triggers first.  Defaults are "
            "to trigger at either 50%% done or 240 seconds past.  Also, "
            "the command will not trigger unless the song is at least the "
            "specified number of seconds long.");
    condition_desc = gtk_label_new(temp);
    g_free(temp);
    gtk_label_set_justify(GTK_LABEL(condition_desc), GTK_JUSTIFY_FILL);
    gtk_misc_set_alignment(GTK_MISC(condition_desc), 0, 0.5);
    gtk_box_pack_start(GTK_BOX(condition_vbox), condition_desc, TRUE, TRUE, 0);
    gtk_label_set_line_wrap(GTK_LABEL(condition_desc), TRUE);

    /* Various Label Boxes */
    percent_entry = labelbox_int(condition_vbox, "Percent Done:", percent_done, percent_entry);
    seconds_entry = labelbox_int(condition_vbox, "Seconds Past:", seconds_past, seconds_entry);
    minimum_entry = labelbox_int(condition_vbox, "Minimum Length:", minimum_len, minimum_entry);

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
    g_free(cmd_line);
    cmd_line = NULL;

    percent_done = aud_get_int(CFGCAT, "percent_done");
    seconds_past = aud_get_int(CFGCAT, "seconds_past");
    minimum_len = aud_get_int(CFGCAT, "minimum_len");
    cmd_line = aud_get_string(CFGCAT, "cmd_line");

    if (percent_done == -1)
    {
        percent_done = DEFAULT_PERCENT;
    }

    if (seconds_past == -1)
    {
        seconds_past = DEFAULT_SECONDS;
    }

    if (minimum_len == -1)
    {
        minimum_len = DEFAULT_MINIMUM;
    }
}

AUD_GENERAL_PLUGIN
(
    .name = "Audacious-Tracking " VERSION,
    .init = init,
    .configure = configure,
    .cleanup = cleanup
)
