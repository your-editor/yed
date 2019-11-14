#include "internal.h"



#ifdef YED_DO_ASSERTIONS
void yed_assert_fail(const char *msg, const char *fname, int line, const char *cond_str) {
    volatile int *trap;

    yed_term_exit();

    fprintf(stderr, "Assertion failed -- %s\n"
                    "at  %s :: line %d\n"
                    "    Condition: '%s'\n",
                    msg, fname, line, cond_str);

    trap = 0;
    (void)*trap;
}
#endif




uint64_t next_power_of_2(uint64_t x) {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    x++;
    return x;
}



char * pretty_bytes(uint64_t n_bytes) {
    uint64_t    s;
    double      count;
    char       *r;
    const char *suffixes[]
        = { " B", " KB", " MB", " GB", " TB", " PB", " EB" };

    s     = 0;
    count = (double)n_bytes;

    while (count >= 1024 && s < 7) {
        s     += 1;
        count /= 1024;
    }

    r = calloc(64, 1);

    if (count - floor(count) == 0.0) {
        sprintf(r, "%d", (int)count);
    } else {
        sprintf(r, "%.2f", count);
    }

    strcat(r, suffixes[s]);

    return r;
}


void yed_init_output_stream(void) {
    ys->output_buffer = array_make_with_cap(char, 4 * ys->term_cols * ys->term_rows);
    ys->writer_buffer = array_make_with_cap(char, 4 * ys->term_cols * ys->term_rows);
}

int output_buff_len(void) { return array_len(ys->output_buffer); }

void append_n_to_output_buff(char *s, int n) {
    array_push_n(ys->output_buffer, s, n);
}

void append_to_output_buff(char *s) {
    append_n_to_output_buff(s, strlen(s));
}

static char *itoa(char *p, unsigned x) {
    p += 3*sizeof(int);
    *--p = 0;
    do {
        *--p = '0' + x % 10;
        x /= 10;
    } while (x);
    return p;
}

void append_int_to_output_buff(int i) {
    char  s[16],
         *p;

    p = itoa(s, i);

    append_to_output_buff(p);
}

void flush_writer_buff(void) {
    if (!array_len(ys->writer_buffer)) {
        return;
    }
    write(1, array_data(ys->writer_buffer), array_len(ys->writer_buffer));
    array_clear(ys->writer_buffer);
}

void flush_output_buff(void) {
    if (!array_len(ys->output_buffer)) {
        return;
    }
    write(1, array_data(ys->output_buffer), array_len(ys->output_buffer));
    array_clear(ys->output_buffer);
}

void yed_set_small_message(char *msg) {
    if (ys->small_message) {
        free(ys->small_message);
    }

    if (msg) {
        ys->small_message = strdup(msg);
    } else {
        ys->small_message = NULL;
    }
}

void yed_service_reload(void) {
    tree_it(yed_command_name_t, yed_command)    cmd_it;
    tree_it(yed_var_name_t, yed_var_val_t)      var_it;
    tree_it(yed_style_name_t, yed_style_ptr_t)  style_it;
    char                                       *key,
                                               *val;
    yed_style                                  *style;

    tree_reset_fns(yed_style_name_t,   yed_style_ptr_t,       ys->styles,           strcmp);
    tree_reset_fns(yed_var_name_t,     yed_var_val_t,         ys->vars,             strcmp);
    tree_reset_fns(yed_buffer_name_t,  yed_buffer_ptr_t,      ys->buffers,          strcmp);
    tree_reset_fns(int,                yed_key_binding_ptr_t, ys->key_seq_map,      NULL);
    tree_reset_fns(yed_command_name_t, yed_command,           ys->commands,         strcmp);
    tree_reset_fns(yed_command_name_t, yed_command,           ys->default_commands, strcmp);
    tree_reset_fns(yed_plugin_name_t,  yed_plugin_ptr_t,      ys->plugins,          strcmp);

    /*
     * Clear out all of the old vars.
     */
     (void)var_it;
     (void)val;
/*     while (tree_len(ys->vars)) { */
/*         var_it = tree_begin(ys->vars); */
/*         key = tree_it_key(var_it); */
/*         val = tree_it_val(var_it); */
/*         tree_delete(ys->vars, key); */
/*         free(key); */
/*         free(val); */
/*     } */
    /*
     * Reset the defaults.
     */
/*     yed_set_default_vars(); */

    /*
     * Clear out all of the old styles.
     */
    while (tree_len(ys->styles)) {
        style_it = tree_begin(ys->styles);
        key   = tree_it_key(style_it);
        style = tree_it_val(style_it);
        tree_delete(ys->styles, key);
        free(key);
        free(style);
    }

    ys->active_style = NULL;
    /*
     * Reset the defaults.
     */
    yed_set_default_styles();

    /*
     * Clear out all of the old commands.
     */
    while (tree_len(ys->commands)) {
        cmd_it = tree_begin(ys->commands);
        key = tree_it_key(cmd_it);
        tree_delete(ys->commands, key);
        free(key);
    }
    /*
     * Reset the defaults.
     */
    yed_set_default_commands();

    yed_reload_default_event_handlers();
    yed_reload_plugins();

    yed_register_sigwinch_handler();

    yed_set_small_message("* reload serviced *");

    ys->redraw = 1;
    memset(ys->written_cells, 0, ys->term_rows * ys->term_cols);
    yed_update_frames();

    if (ys->interactive_command) {
        yed_set_cursor(ys->cmd_cursor_x, ys->term_rows);
        append_to_output_buff(TERM_CURSOR_SHOW);
    } else if (ys->active_frame) {
        append_to_output_buff(TERM_CURSOR_SHOW);
    }
}

int s_to_i(const char *s) {
    int i;

    sscanf(s, "%d", &i);

    return i;
}
