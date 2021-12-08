#include <yed/plugin.h>

static yed_plugin *Self;

static void unload(yed_plugin *self);

static void fstyle(int n_args, char **args);

static void maybe_change_ft(yed_buffer *buff);
static void maybe_change_ft_event(yed_event *event);
static void syntax_fstyle_line_handler(yed_event *event);
static void syntax_fstyle_row_handler(yed_event *event);
static void syntax_fstyle_mod_handler(yed_event *event);

static yed_attrs parse_attr_line(char *line, int *scomp);

int yed_plugin_boot(yed_plugin *self) {
    tree_it(yed_buffer_name_t, yed_buffer_ptr_t) bit;
    yed_event_handler                            buff_post_load_handler;
    yed_event_handler                            buff_pre_write_handler;
    yed_event_handler                            line_handler;
    yed_event_handler                            row_handler;
    yed_event_handler                            mod_handler;

    YED_PLUG_VERSION_CHECK();

    Self = self;

    yed_plugin_set_unload_fn(self, unload);

    if (yed_plugin_make_ft(self, "fstyle") == FT_ERR_TAKEN) {
        LOG_CMD_ENTER("fstyle");
        yed_cerr("lang/yedrc: unable to create file type name");
        LOG_EXIT();
        return 1;
    }

    yed_plugin_set_command(self, "fstyle", fstyle);

    buff_post_load_handler.kind = EVENT_BUFFER_POST_LOAD;
    buff_post_load_handler.fn   = maybe_change_ft_event;
    buff_pre_write_handler.kind = EVENT_BUFFER_PRE_WRITE;
    buff_pre_write_handler.fn   = maybe_change_ft_event;
    line_handler.kind           = EVENT_LINE_PRE_DRAW;
    line_handler.fn             = syntax_fstyle_line_handler;
    row_handler.kind            = EVENT_ROW_PRE_CLEAR;
    row_handler.fn              = syntax_fstyle_row_handler;
    mod_handler.kind            = EVENT_BUFFER_POST_MOD;
    mod_handler.fn              = syntax_fstyle_mod_handler;

    yed_plugin_add_event_handler(self, buff_post_load_handler);
    yed_plugin_add_event_handler(self, buff_pre_write_handler);
    yed_plugin_add_event_handler(self, line_handler);
    yed_plugin_add_event_handler(self, row_handler);
    yed_plugin_add_event_handler(self, mod_handler);

    tree_traverse(ys->buffers, bit) {
        maybe_change_ft(tree_it_val(bit));
    }

    return 0;
}

static void unload(yed_plugin *self) {}

static void fstyle(int n_args, char **args) {
    char       *path;
    char        full_path[4096];
    FILE       *f;
    const char *base_ext;
    const char *base;
    yed_style   s;
    char        line[512];
    yed_attrs   attr;
    int         scomp;

    if (n_args == 0) {
        if (ys->active_frame == NULL) {
            yed_cerr("no active frame");
            return;
        }
        if (ys->active_frame->buffer == NULL) {
            yed_cerr("active frame has no buffer");
            return;
        }

        if (ys->active_frame->buffer->ft != yed_get_ft("fstyle")) {
            yed_cerr("buffer's ft is not 'fstyle'");
            return;
        }

        path = ys->active_frame->buffer->path;
        if (path == NULL) {
            yed_cerr("buffer has not been written");
            return;
        }
    } else if (n_args == 1) {
        path = args[0];
    } else {
        yed_cerr("expected 0 or 1 arguments, but got %d", n_args);
        return;
    }

    abs_path(path, full_path);

    f = fopen(full_path, "r");
    if (f == NULL) {
        yed_cerr("unable to open '%s'", full_path);
        return;
    }

    base_ext = get_path_basename(path);
    base     = path_without_ext(base_ext);

    memset(&s, 0, sizeof(s));

    while (fgets(line, sizeof(line), f)) {
        if (*line && line[strlen(line) - 1] == '\n') { line[strlen(line) - 1] = 0; }
        if (strlen(line) == 0) { continue; }

        attr = parse_attr_line(line, &scomp);

        switch (scomp) {
            #define __SCOMP(comp) case STYLE_##comp: s.comp = attr; break;
            __STYLE_COMPONENTS
            #undef __SCOMP
        }
    }

    yed_plugin_set_style(Self, (char*)base, &s);
    YEXE("style", (char*)base);

    free((void*)base);

    fclose(f);
}

static void maybe_change_ft(yed_buffer *buff) {
    const char *ext;

    if (buff->ft != FT_UNKNOWN) {
        return;
    }
    if (buff->path == NULL) {
        return;
    }
    if ((ext = get_path_ext(buff->path)) != NULL) {
        if (strcmp(ext, "fstyle") == 0) {
            yed_buffer_set_ft(buff, yed_get_ft("fstyle"));
        }
    }
}

static void maybe_change_ft_event(yed_event *event) {
    if (event->buffer) {
        maybe_change_ft(event->buffer);
    }
}


static yed_attrs known_active;

static void syntax_fstyle_line_handler(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;
    yed_attrs  attrs;
    yed_attrs *ait;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->ft != yed_get_ft("fstyle")) {
        return;
    }

    line = yed_buff_get_line(frame->buffer, event->row);
    if (line == NULL) { return; }

    array_zero_term(line->chars);
    attrs = parse_attr_line(array_data(line->chars), NULL);

    if (attrs.flags != 0) {
        array_traverse(event->line_attrs, ait) {
            *ait = known_active;
            yed_combine_attrs(ait, &attrs);
        }
    }
}

static void syntax_fstyle_row_handler(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;
    int        scomp;
    yed_attrs  attrs;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->ft != yed_get_ft("fstyle")) {
        return;
    }

    line = yed_buff_get_line(frame->buffer, event->row);
    if (line == NULL) { return; }

    scomp = -1;

    array_zero_term(line->chars);
    attrs = parse_attr_line(array_data(line->chars), &scomp);

    if (scomp == STYLE_active) {
        if (memcmp(&attrs, &known_active, sizeof(attrs))) {
            known_active = attrs;
        }
    }

    event->row_base_attr = known_active;
    if (attrs.flags != 0) {
        yed_combine_attrs(&event->row_base_attr, &attrs);
    }
}

static void syntax_fstyle_mod_handler(yed_event *event) {
    if (event->buffer->ft == yed_get_ft("fstyle")) {
        yed_mark_dirty_frames(event->buffer);
    }
}

static yed_attrs parse_attr_line(char *line, int *scomp) {
    char      *scomp_str;
    yed_attrs  attrs;
    array_t    words;
    int        idx;
    char      *word;
    unsigned   color;
    char       rgb_str[9];

    memset(&attrs, 0, sizeof(attrs));

    words = sh_split(line);

    if (array_len(words) == 0) { goto out; }

    scomp_str = *(char**)array_item(words, 0);
    if (scomp != NULL) {
        *scomp = yed_scomp_nr_by_name(scomp_str);
    }

    if (array_len(words) < 2) { goto out; }

    attrs = yed_parse_attrs(line + strlen(scomp_str));

out:;
    free_string_array(words);
    return attrs;
}
