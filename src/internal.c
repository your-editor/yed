#include "internal.h"



#ifdef YED_DO_ASSERTIONS
void yed_assert_fail(const char *msg, const char *fname, int line, const char *cond_str) {
    volatile int *trap;

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



void yed_add_new_buff(void) {
    yed_buffer *new_buff;

    new_buff  = malloc(sizeof(*new_buff));
    *new_buff = yed_new_buff();
    array_push(ys->buff_list, new_buff);
}

void clear_output_buff(void) {
    ys->out_s.data[0] = 0;
    ys->out_s.used    = 1; /* 1 for NULL */
}

void yed_init_output_stream(void) {
    ys->out_s.avail = 4096;
    ys->out_s.used  = 1; /* 1 for NULL */
    ys->out_s.data  = calloc(ys->out_s.avail, 1);
}

int output_buff_len(void) { return ys->out_s.used; }

void append_n_to_output_buff(const char *s, int n) {
    char *data_save;

    if (n) {
        if (ys->out_s.used + n > ys->out_s.avail) {
            ys->out_s.avail <<= 1;
            data_save         = ys->out_s.data;
            ys->out_s.data    = calloc(ys->out_s.avail, 1);

            memcpy(ys->out_s.data, data_save, ys->out_s.used);
            free(data_save);
        }

        strncat(ys->out_s.data, s, n);
        ys->out_s.used += n;
    }
}

void append_to_output_buff(const char *s) {
    append_n_to_output_buff(s, strlen(s));
}

void append_int_to_output_buff(int i) {
    char s[16];

    sprintf(s, "%d", i);

    append_to_output_buff(s);
}

void flush_output_buff(void) {
    write(1, ys->out_s.data, ys->out_s.used);
    clear_output_buff();
}

void yed_service_reload(void) {
    tree_reset_fns(yed_frame_id_t,     yed_frame_ptr_t,  ys->frames,           strcmp);
    tree_reset_fns(yed_command_name_t, yed_command,      ys->commands,         strcmp);
    tree_reset_fns(yed_command_name_t, yed_command,      ys->default_commands, strcmp);
    tree_reset_fns(yed_plugin_name_t,  yed_plugin_ptr_t, ys->plugins,          strcmp);

    yed_set_default_commands();
    yed_reload_plugins();

    ys->small_message = "* reload serviced *";
}

int s_to_i(const char *s) {
    int i;

    sscanf(s, "%d", &i);

    return i;
}
