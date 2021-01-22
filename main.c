#include <stdio.h>
#include <string.h>
#include <pulse/pulseaudio.h>

#define _(x) x

#if defined (_WIN32) || defined (_WIN64)
#define TRAY_WINAPI 1
#elif defined (__linux__) || defined (linux) || defined (__linux)
#define TRAY_APPINDICATOR 1
#elif defined (__APPLE__) || defined (__MACH__)
#define TRAY_APPKIT 1
#endif

#include "tray.h"

#if TRAY_APPINDICATOR
#define TRAY_ICON1 "indicator-messages"
#define TRAY_ICON2 "indicator-messages-new"
#elif TRAY_APPKIT
#define TRAY_ICON1 "icon.png"
#define TRAY_ICON2 "icon.png"
#elif TRAY_WINAPI
#define TRAY_ICON1 "icon.ico"
#define TRAY_ICON2 "icon.ico"
#endif

static struct tray tray;

static void toggle_cb(struct tray_menu *item) {
    printf("toggle cb\n");
    item->checked = !item->checked;
    tray_update(&tray);
}

static void hello_cb(struct tray_menu *item) {
    (void)item;
    printf("hello cb\n");
    if (strcmp(tray.icon, TRAY_ICON1) == 0) {
        tray.icon = TRAY_ICON2;
    } else {
        tray.icon = TRAY_ICON1;
    }
    tray_update(&tray);
}

static void quit_cb(struct tray_menu *item) {
    (void)item;
    printf("quit cb\n");
    tray_exit();
}

static void submenu_cb(struct tray_menu *item) {
    (void)item;
    printf("submenu: clicked on %s\n", item->text);
    tray_update(&tray);
}

// Test tray init
static struct tray tray = {
        .icon = TRAY_ICON1,
        .menu =
        (struct tray_menu[]){
                {.text = "Hello", .cb = hello_cb},
                {.text = "Checked", .checked = 1, .cb = toggle_cb},
                {.text = "Disabled", .disabled = 1},
                {.text = "-"},
                {.text = "SubMenu",
                        .submenu =
                        (struct tray_menu[]){
                                {.text = "FIRST", .checked = 1, .cb = submenu_cb},
                                {.text = "SECOND",
                                        .submenu =
                                        (struct tray_menu[]){
                                                {.text = "THIRD",
                                                        .submenu =
                                                        (struct tray_menu[]){
                                                                {.text = "7", .cb = submenu_cb},
                                                                {.text = "-"},
                                                                {.text = "8", .cb = submenu_cb},
                                                                {.text = NULL}}},
                                                {.text = "FOUR",
                                                        .submenu =
                                                        (struct tray_menu[]){
                                                                {.text = "5", .cb = submenu_cb},
                                                                {.text = "6", .cb = submenu_cb},
                                                                {.text = NULL}}},
                                                {.text = NULL}}},
                                {.text = NULL}}},
                {.text = "-"},
                {.text = "Quit", .cb = quit_cb},
                {.text = NULL}},
};

void show_error(char *s) {
    fprintf(stderr, "%s\n", s);
}


void source_cb(pa_context *c, const pa_source_info *i, int eol, void *userdata) {
    if (eol > 0) {
        return;
    }

    printf("Source: name %s, description %s\n", i->name, i->description);
    // print_properties(i->proplist);
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

            if (!(o = pa_context_get_source_info_list(c,
                                                      source_cb,
                                                      NULL
            ))) {
                show_error(_("pa_context_subscribe() failed"));
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




int main() {

    int ret;

    // Define our pulse audio loop and connection variables
    pa_mainloop *pa_ml;
    pa_mainloop_api *pa_mlapi;
    pa_context *context;

    // Create a mainloop API and connection to the default server
    pa_ml = pa_threaded_mainloop_new();
    pa_threaded_mainloop_start(pa_ml);
    pa_mlapi = pa_threaded_mainloop_get_api(pa_ml);
    context = pa_context_new(pa_mlapi, "Device list");

    // This function connects to the pulse server
    pa_context_connect(context, NULL, 0, NULL);

    // This function defines a callback so the server will tell us its state.
    pa_context_set_state_callback(context, context_state_cb, NULL);


    if (tray_init(&tray) < 0) {
        printf("failed to create tray\n");
        return 1;
    }
    while (tray_loop(1) == 0) {
        printf("iteration\n");
    }
    return 0;
}