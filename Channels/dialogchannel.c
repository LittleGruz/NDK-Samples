/*
 * Copyright (c) 2011-2012 Research In Motion Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>

#include <screen/screen.h>
#include <bps/bps.h>
#include <bps/dialog.h>
#include <bps/navigator.h>
#include "dialogchannel.h"

static screen_context_t screen_ctx = 0;
static screen_window_t screen_win = 0;
static dialog_instance_t geolocation_dialog = 0;
static dialog_instance_t accelerometer_dialog = 0;


/**
 * Use the PID to set the window group id.
 */
static const char *
get_window_group_id()
{
    static char s_window_group_id[16] = "";

    if (s_window_group_id[0] == '\0') {
        snprintf(s_window_group_id, sizeof(s_window_group_id), "%d", getpid());
    }

    return s_window_group_id;
}

int
setup_screen()
{
    if (screen_create_context(&screen_ctx, SCREEN_APPLICATION_CONTEXT) != 0) {
        return EXIT_FAILURE;
    }

    if (screen_create_window(&screen_win, screen_ctx) != 0) {
        screen_destroy_context(screen_ctx);
        return EXIT_FAILURE;
    }

    if (screen_create_window_group(screen_win, get_window_group_id()) != 0) goto fail;

    int usage = SCREEN_USAGE_NATIVE;
    if (screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_USAGE, &usage) != 0) goto fail;

    const char *env = getenv("WIDTH");
    if (0 == env) goto fail;
    int width = atoi(env);

    env = getenv("HEIGHT");
    if (0 == env) goto fail;
    int height = atoi(env);
    int size[2] = { width, height };

    if (screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_BUFFER_SIZE, size) != 0) goto fail;

    if (screen_create_window_buffers(screen_win, 1) != 0) goto fail;

    screen_buffer_t buff;
    if (screen_get_window_property_pv(screen_win, SCREEN_PROPERTY_RENDER_BUFFERS, (void*)&buff) != 0) goto fail;

    int attribs[1] = {SCREEN_BLIT_END};
    if (screen_fill(screen_ctx, buff, attribs) != 0) goto fail;

    int dirty_rects[4] = {0, 0, width, height};
    if (screen_post_window(screen_win, buff, 1, (const int*)dirty_rects, 0) != 0) goto fail;

    return EXIT_SUCCESS;

fail:
    perror(NULL);
    cleanup_screen();
    return EXIT_FAILURE;
}


void cleanup_screen() {
    screen_destroy_window(screen_win);
    screen_destroy_context(screen_ctx);
    screen_win = 0;
    screen_ctx = 0;
}



void
create_geolocation_dialog()
{
    if (accelerometer_dialog == NULL) {
        dialog_create_alert(&geolocation_dialog);
        dialog_set_alert_message_text(geolocation_dialog, "\n");
        dialog_set_group_id(geolocation_dialog, get_window_group_id());
        dialog_set_cancel_required(geolocation_dialog, true);
        dialog_show(geolocation_dialog);
    }
}

void
create_accelerometer_dialog()
{
    if (accelerometer_dialog == NULL) {
        dialog_create_toast(&accelerometer_dialog);
        dialog_set_toast_message_text(accelerometer_dialog, "\n");
        dialog_set_toast_position(accelerometer_dialog, DIALOG_POSITION_BOTTOM_CENTER);
        dialog_set_group_id(accelerometer_dialog, get_window_group_id());
        dialog_add_button(accelerometer_dialog, "OK", true, NULL, true);
        dialog_show(accelerometer_dialog);
    }

}

void
destroy_geolocation_dialog() {
    if (geolocation_dialog) {
        dialog_destroy(geolocation_dialog);
    }
    geolocation_dialog = 0;
}

void
destroy_accelerometer_dialog() {

    if (accelerometer_dialog) {
        dialog_destroy(accelerometer_dialog);
    }
    accelerometer_dialog = 0;

}

void
show_geolocation_dialog_message(const char * msg) {
    dialog_set_alert_message_text(geolocation_dialog, msg);
    dialog_update(geolocation_dialog);
    fprintf(stderr, "%s\n", msg);
}

void
show_accelerometer_dialog_message(const char * msg) {
    dialog_set_alert_message_text(accelerometer_dialog, msg);
    dialog_update(accelerometer_dialog);
    fprintf(stderr, "%s\n", msg);

}
