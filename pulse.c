#include "pulse.h"


void pa_mute_cb(pa_context *c, int success, void *userdata) {
    if (success) {
        printf("mute success");
    } else {
        printf("mute failure");
    }
}

void pa_source_info_list_cb(pa_context *c, const pa_source_info *si, int eol, void *userdata) {
    if (eol > 0) {
        return;
    }
    printf("\n");
    if (si->active_port != NULL) {
        printf("Found active microphone microphone %s (%s)\n", si->name, si->description);
        pa_context_set_source_mute_by_index(c, si->index, 1, pa_mute_cb, userdata);
    }
}


void pa_context_state_cb(pa_context *c, void *userdata) {

    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;
        case PA_CONTEXT_READY: {
            pa_operation *o;
            o = pa_context_get_source_info_list(c, pa_source_info_list_cb, userdata);
            if (!o) {
                fprintf(stderr, "pa_context_get_source_info_list() failed\n");
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


void pa_init() {
    // Define our pulse audio loop and connection variables
    pa_mainloop *pa_ml;
    pa_mainloop_api *pa_mlapi;

    // Create a mainloop API and connection to the default server
    pa_ml = pa_threaded_mainloop_new();
    pa_threaded_mainloop_start(pa_ml);
    pa_mlapi = pa_threaded_mainloop_get_api(pa_ml);
    pa_ctx = pa_context_new(pa_mlapi, "Device list");

    // This function connects to the pulse server
    pa_context_connect(pa_ctx, NULL, 0, NULL);

    // This function defines a callback so the server will tell us its state.
    pa_context_set_state_callback(pa_ctx, pa_context_state_cb, NULL);
}