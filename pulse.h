//
// Created by fabio on 1/22/21.
//

#ifndef MYP_PULSE_H
#define MYP_PULSE_H

#include <pulse/pulseaudio.h>
#include <stdio.h>

static pa_context *pa_ctx;

void pa_context_state_cb(pa_context *c, void *userdata);
void pa_source_info_list_cb(pa_context *ctx, const pa_source_info *si, int eol, void *userdata);
void pa_init();

#endif //MYP_PULSE_H
