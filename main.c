#include <stdlib.h>

#include <glib.h>
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>
#include <libappindicator/app-indicator.h>
#include <X11/Xlib.h>
#include <xcb/record.h>

#define KEYBOARD_POLL_INTERVAL_USEC  100000
#define MUTE_TIMEOUT_MSEC            250
static GMainLoop *mainloop;
static pa_glib_mainloop *pa_main;
static pa_mainloop_api *pa_api;
static pa_context *pa_ctx;
static AppIndicator *indicator = NULL;
static GtkWidget *indicator_menu;
static GArray *sources;
static GSList *sourcesRadio;
static uint32_t selected_source;
static int keys_pressed;
gboolean muted;
gboolean paused;


static void
switch_source(GtkRadioMenuItem *item,
              gpointer data) {
    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(item))) {
        GSList *group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM (item));
        pa_source_info i = g_array_index(sources, pa_source_info, g_slist_index(group, item));
        selected_source = i.index;
    }
}


void set_mute_cb(pa_context *c, int success, void *userdata) {
    if (success) {
        g_print("muted callback\n");
    } else {
        g_print("error setting mute\n");
    }
}


void get_source_cb(pa_context *c, const pa_source_info *i, int eol, void *userdata) {
    if (eol > 0) {
        gtk_widget_show_all(indicator_menu);
        return;
    }
    if (i->monitor_of_sink == PA_INVALID_INDEX) {
        printf("Found mic %s (%s) [%d]\n", i->description, i->name, i->index);
        g_array_append_val(sources, *i);
        GtkWidget *source = gtk_radio_menu_item_new_with_label(sourcesRadio, i->description);
        sourcesRadio = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM (source));
        if (i->active_port) {
            selected_source = i->index;
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM (source), TRUE);
        }
        g_signal_connect(GTK_RADIO_MENU_ITEM(source), "toggled", G_CALLBACK(switch_source), (gpointer) i);
        gtk_menu_shell_append(GTK_MENU_SHELL (indicator_menu), source);
    }
}


void context_state_cb(pa_context *c, void *userdata) {
    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;

        case PA_CONTEXT_READY: {
            pa_operation *o;
            if (!(o = pa_context_get_source_info_list(c, get_source_cb, NULL))) {
                printf("pa_context_subscribe() failed");
                return;
            }
            pa_operation_unref(o);
            break;
        }
        case PA_CONTEXT_FAILED:
        case PA_CONTEXT_TERMINATED:
        default:
            return;
    }
}


gboolean should_remain_muted(gpointer data) {
    if (!muted) {
        return FALSE;
    }

    g_print("checking if I should stay muted... \n");
    if (keys_pressed > 0 && muted) {
        g_print("yes\n");
        keys_pressed = 0;
        return TRUE;
    }

    g_print("no, unmuting\n");
    muted = FALSE;
    app_indicator_set_icon(indicator, "microphone-sensitivity-high");
    pa_context_set_source_mute_by_index(pa_ctx, selected_source, 0, set_mute_cb, NULL);
    return FALSE;
}


_Noreturn void *listener(gpointer data) {
    Display *display;
    display = XOpenDisplay(NULL);
    char keys_return[32];
    while (1) {
        XQueryKeymap(display, keys_return);
        for (int i = 0; i < 32; i++) {
            if (keys_return[i] != 0) {
                int pos = 0;
                int num = keys_return[i];
                while (pos < 8) {
                    if ((num & 0x01) == 1) {
                        keys_pressed++;
                    }
                    pos++;
                    num /= 2;
                }
                printf("\n");
            }
        }
        if (keys_pressed > 0 && !muted) {
            g_print("%d key pressed in the last %d usecs, muting\n", keys_pressed, KEYBOARD_POLL_INTERVAL_USEC);
            muted = TRUE;
            pa_context_set_source_mute_by_index(pa_ctx, selected_source, 1, set_mute_cb, NULL);
            app_indicator_set_icon(indicator, "microphone-sensitivity-muted");
            g_timeout_add(MUTE_TIMEOUT_MSEC, should_remain_muted, NULL);
        }
        usleep(KEYBOARD_POLL_INTERVAL_USEC);
    }
}


gint main(gint argc G_GNUC_UNUSED, gchar *argv[] G_GNUC_UNUSED) {
    GMainContext *context = NULL;

    mainloop = g_main_loop_new(context, FALSE);

    gtk_init(&argc, &argv);

    indicator = app_indicator_new("myp", "indicator-messages", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
    indicator_menu = gtk_menu_new();

    app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
    app_indicator_set_icon(indicator, "microphone-sensitivity-high");

    GtkWidget *pause = gtk_menu_item_new_with_label("Pause");
    GtkWidget *quit = gtk_menu_item_new_with_label("Quit");

    gtk_menu_shell_append(GTK_MENU_SHELL (indicator_menu), pause);
    gtk_menu_shell_append(GTK_MENU_SHELL (indicator_menu), quit);

    sources = g_array_new(FALSE, FALSE, sizeof(pa_source_info));
    pa_main = pa_glib_mainloop_new(context);
    pa_api = pa_glib_mainloop_get_api(pa_main);
    pa_ctx = pa_context_new(pa_api, "myp");

    pa_context_set_state_callback(pa_ctx, context_state_cb, NULL);

    if (pa_context_connect(pa_ctx, NULL, 0, NULL) < 0)
        fprintf(stderr, "pa_context_connect() failed: %s\n", pa_strerror(pa_context_errno(pa_ctx)));

    app_indicator_set_menu(indicator, GTK_MENU(indicator_menu));
    g_thread_new("keyboardlistener", listener, NULL);

    g_main_loop_run(mainloop);

    return EXIT_SUCCESS;
}

