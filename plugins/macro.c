#include <yed/plugin.h>

typedef struct {
    int  n;
    int  keys[16];
} key_seq_t;

yed_plugin        *Self;
yed_event_handler  rec_key_handler;
yed_event_handler  play_key_handler;
yed_event_handler  pump_rec_handler;
yed_event_handler  pump_play_handler;
int                is_recording;
int                has_recorded;
int                stop_key;
char              *stop_key_str;
array_t            keys;
int                playback_idx;
int                blink_count;
yed_direct_draw_t *blink_dd;
u64                blink_start_time;
int                play_count;
int                has_played_count;
int                key_pressed_is_playback;

void macro_record(int n_args, char **args);
void macro_play(int n_args, char **args);

void macro_rec_key_handler(yed_event *event);
void macro_play_key_handler(yed_event *event);
void macro_pump_rec_handler(yed_event *event);
void macro_pump_play_handler(yed_event *event);

void macro_unload(yed_plugin *self);

int yed_plugin_boot(yed_plugin *self) {
    YED_PLUG_VERSION_CHECK();

    Self = self;

    rec_key_handler.kind   = EVENT_KEY_PRESSED;
    rec_key_handler.fn     = macro_rec_key_handler;
    play_key_handler.kind  = EVENT_KEY_PRESSED;
    play_key_handler.fn    = macro_play_key_handler;
    pump_rec_handler.kind  = EVENT_PRE_PUMP;
    pump_rec_handler.fn    = macro_pump_rec_handler;
    pump_play_handler.kind = EVENT_PRE_PUMP;
    pump_play_handler.fn   = macro_pump_play_handler;

    if (yed_get_var("macro-stop-key") == NULL) {
        yed_set_var("macro-stop-key", "ctrl-e");
    }
    if (yed_get_var("macro-instant-playback") == NULL) {
        yed_set_var("macro-instant-playback", "no");
    }

    keys = array_make(key_seq_t);

    yed_plugin_set_command(self, "macro-record", macro_record);
    yed_plugin_set_command(self, "macro-play",   macro_play);

    return 0;
}

void macro_unload(yed_plugin *self) {
    array_free(keys);
}

void macro_rec_key_handler(yed_event *event) {
    key_seq_t seq;

    if (event->key == stop_key) {
        yed_delete_event_handler(rec_key_handler);
        yed_delete_event_handler(pump_rec_handler);
        if (blink_dd != NULL) {
            yed_kill_direct_draw(blink_dd);
            blink_dd = NULL;
        }
        has_recorded  = 1;
        is_recording  = 0;
        event->cancel = 1;
    } else {
        if (!yed_get_real_keys(event->key, &seq.n, seq.keys)) {
            seq.n       = 1;
            seq.keys[0] = event->key;
        }
        array_push(keys, seq);
    }
}

void macro_play_key_handler(yed_event *event) {
    if (key_pressed_is_playback) { return; }

    if (event->key == CTRL_C) {
        if (blink_dd != NULL) {
            yed_kill_direct_draw(blink_dd);
            blink_dd = NULL;
        }

        yed_delete_event_handler(pump_play_handler);
        yed_delete_event_handler(play_key_handler);
LOG_CMD_ENTER("macro-play");
        yed_cprint("finished playing back macro");
LOG_EXIT();
    }

    event->cancel = 1;
}

void macro_pump_rec_handler(yed_event *event) {
    u64       now;
    yed_attrs attrs;
    char      buff[64];

    if (blink_dd != NULL) {
        yed_kill_direct_draw(blink_dd);
        blink_dd = NULL;
    }

    now = measure_time_now_ms();

    if (now - blink_start_time > 1000) {
        blink_start_time = now;
        blink_count += 1;
    }

    if (blink_count % 2 == 0) {
        snprintf(buff, sizeof(buff), " macro recording    %s to stop ", stop_key_str);
        attrs        = yed_active_style_get_attention();
        attrs.flags |= ATTR_INVERSE;
        blink_dd     = yed_direct_draw(ys->term_rows - 2, 1, attrs, buff);
    }
}

void macro_play_key_seq(key_seq_t *seq) {
    int virt;

    key_pressed_is_playback = 1;
    if (seq->n == 1
    ||  (virt = yed_get_key_sequence(seq->n, seq->keys)) == KEY_NULL) {
        yed_feed_keys(1, seq->keys);
    } else {
        yed_feed_keys(1, &virt);
    }
    key_pressed_is_playback = 0;
}

void macro_pump_play_handler(yed_event *event) {
    u64       now;
    yed_attrs attrs;

    if (blink_dd != NULL) {
        yed_kill_direct_draw(blink_dd);
        blink_dd = NULL;
    }

    now = measure_time_now_ms();

    if (blink_count % 2 == 0) {
        attrs        = yed_active_style_get_associate();
        attrs.flags |= ATTR_INVERSE;
        blink_dd     = yed_direct_draw(ys->term_rows - 2, 1, attrs,
                                       " playing back macro    ctrl-c to cancel ");
    }

    if (now - blink_start_time > 1000) {
        blink_start_time = now;
        blink_count += 1;
    }

    if (playback_idx == array_len(keys)) {
        has_played_count += 1;
        if (has_played_count == play_count) {
            if (blink_dd != NULL) {
                yed_kill_direct_draw(blink_dd);
                blink_dd = NULL;
            }

            yed_delete_event_handler(pump_play_handler);
            yed_delete_event_handler(play_key_handler);
LOG_CMD_ENTER("macro-play");
            yed_cprint("finished playing back macro");
LOG_EXIT();
        } else {
            playback_idx = 0;
        }
    } else {
        macro_play_key_seq(array_item(keys, playback_idx));
        playback_idx += 1;
    }
}

void macro_record(int n_args, char **args) {
    char *key_str;
    char  key_c;
    int   key_i;

    if (is_recording) {
        yed_cerr("a macro is currently being recorded");
        return;
    }

    if (n_args > 1) {
        yed_cerr("expected 0 or 1 arguments, but got %d", n_args);
        return;
    }

    if (n_args == 1) {
        key_str = args[0];
    } else {
        key_str = yed_get_var("macro-stop-key");
    }

    if (key_str == NULL) {
        yed_set_var("macro-stop-key", "ctrl-e");
    }

    key_c = key_i = -1;

    if (strlen(key_str) == 1) {
        sscanf(key_str, "%c", &key_c);
        key_i = key_c;
    } else if (strcmp(key_str, "tab") == 0) {
        key_i = TAB;
    } else if (strcmp(key_str, "enter") == 0) {
        key_i = ENTER;
    } else if (strcmp(key_str, "esc") == 0) {
        key_i = ESC;
    } else if (strcmp(key_str, "spc") == 0) {
        key_i = ' ';
    } else if (strcmp(key_str, "bsp") == 0) {
        key_i = BACKSPACE;
    } else if (sscanf(key_str, "ctrl-%c", &key_c)) {
        if (key_c != -1) {
            if (key_c == '/') {
                key_i = CTRL_FS;
            } else {
                key_i = CTRL_KEY(key_c);
            }
        }
    }

    if (key_i == -1) {
        yed_cerr("invalid macro-stop-key '%s'", key_str);
        return;
    }

    stop_key = key_i;
    if (has_recorded) { free(stop_key_str); }
    stop_key_str = strdup(key_str);
    is_recording = 1;
    blink_count  = 0;
    blink_start_time = measure_time_now_ms();
    array_clear(keys);
    yed_plugin_add_event_handler(Self, rec_key_handler);
    yed_plugin_add_event_handler(Self, pump_rec_handler);
}

void macro_play(int n_args, char **args) {
    int        i;
    key_seq_t *seq_it;
    yed_range *sel;

    if (is_recording) {
        yed_cerr("a macro is currently being recorded");
        return;
    }

    if (n_args > 1) {
        yed_cerr("expected 0 or 1 arguments, but got %d", n_args);
        return;
    }

    if (n_args == 0
    ||  sscanf(args[0], "%d", &play_count) != 1
    ||  play_count < 0) {

        if (ys->active_frame
        &&  ys->active_frame->buffer
        &&  ys->active_frame->buffer->has_selection) {
            sel = &(ys->active_frame->buffer->selection);
            play_count = abs(sel->anchor_row - sel->cursor_row) + 1;
            yed_set_cursor_within_frame(ys->active_frame,
                                         sel->anchor_row, sel->anchor_col);
            ys->active_frame->buffer->has_selection = 0;
        } else {
            play_count = 1;
        }

    }

    blink_count = 0;

    if (yed_var_is_truthy("macro-instant-playback")) {
        for (i = 0; i < play_count; i += 1) {
            array_traverse(keys, seq_it) {
                macro_play_key_seq(seq_it);
            }
        }
    } else {
        playback_idx     = 0;
        has_played_count = 0;
        yed_plugin_add_event_handler(Self, pump_play_handler);
        yed_plugin_add_event_handler(Self, play_key_handler);
    }
}
