#include <yed/plugin.h>
#include <yed/highlight.h>

#define inline static inline
#include <yed/tree.h>
typedef char *ctags_str_t;
use_tree(ctags_str_t, int);
#undef inline

#define TAG_KIND_MACRO      (1)
#define TAG_KIND_TYPE       (2)
#define TAG_KIND_ENUMERATOR (3)


int                     gen_thread_started;
int                     gen_thread_finished;
int                     gen_thread_exit_status;
int                     should_reparse;

int                     has_parsed;
tree(ctags_str_t, int)  tags;

highlight_info          hinfo;

yed_buffer *get_or_make_buff(void) {
    yed_buffer *buff;

    buff = yed_get_buffer("*ctags-find-list");

    if (buff == NULL) {
        buff = yed_create_buffer("*ctags-find-list");
        buff->flags |= BUFF_RD_ONLY | BUFF_SPECIAL;
    }

    return buff;
}
void ensure_parsed(void);

void ctags_gen(int n_args, char **args);
void ctags_find(int n_args, char **args);
void ctags_jump_to_definition(int n_args, char **args);
void ctags_hl_reparse(int n_args, char **args);

void unload(yed_plugin *self);
void ctags_find_key_pressed_handler(yed_event *event);
void ctags_find_line_handler(yed_event *event);
void ctags_hl_line_handler(yed_event *event);
void ctags_pump_handler(yed_event *event);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler key_pressed;
    yed_event_handler find_line;
    yed_event_handler hl_line;
    yed_event_handler pump;

    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_unload_fn(self, unload);

    key_pressed.kind   = EVENT_KEY_PRESSED;
    key_pressed.fn     = ctags_find_key_pressed_handler;
    find_line.kind     = EVENT_LINE_PRE_DRAW;
    find_line.fn       = ctags_find_line_handler;
    hl_line.kind       = EVENT_LINE_PRE_DRAW;
    hl_line.fn         = ctags_hl_line_handler;
    pump.kind          = EVENT_PRE_PUMP;
    pump.fn            = ctags_pump_handler;
    yed_plugin_add_event_handler(self, key_pressed);
    yed_plugin_add_event_handler(self, find_line);
    yed_plugin_add_event_handler(self, hl_line);
    yed_plugin_add_event_handler(self, pump);

    yed_plugin_set_command(self, "ctags-gen",                ctags_gen);
    yed_plugin_set_command(self, "ctags-find",               ctags_find);
    yed_plugin_set_command(self, "ctags-jump-to-definition", ctags_jump_to_definition);
    yed_plugin_set_command(self, "ctags-hl-reparse",         ctags_hl_reparse);

    if (!yed_get_var("ctags-formatting-limit")) {
        yed_set_var("ctags-formatting-limit", "10000");
    }

    if (yed_var_is_truthy("ctags-enable-extra-highlighting")) {
        ensure_parsed();
        ys->redraw = 1;
    }

    return 0;
}

int parse_tag_line(char *line, char **tag, char **file, char **kind) {
    char *scan;

    if (*line == '!') { return 0; }

    scan = line;
    *tag = scan;

    /* Scan to file start. */
    if (!(scan = strchr(scan, '\t'))) { return 0; }
    *scan  = 0;
    scan  += 1;
    *file  = scan;

    /* Scan to search start. */
    if (!(scan = strchr(scan, '\t'))) { return 0; }
    *scan  = 0;
    scan  += 1;
    /* Scan to kind start. */
    if (!(scan = strchr(scan, '\t'))) { return 0; }
    *scan  = 0;
    scan  += 1;
    *kind  = scan;

    /* If there's more, chop it off. */
    if ((scan = strchr(scan, '\t'))) {
        *scan = 0;
    }

    return 1;
}

void parse_tags(void) {
    FILE *f;
    char  line[4096];
    char *tag;
    char *file;
    char *kind;
    int   k;

LOG_FN_ENTER();

    tags = tree_make_c(ctags_str_t, int, strcmp);

    f = fopen("tags", "r");
    if (f == NULL) {
        yed_log("[!] (ctags.c:" STR(__LINE__) ") could not open tags file");
        goto out;
    }

    while (fgets(line, sizeof(line), f)) {
        if (parse_tag_line(line, &tag, &file, &kind)) {
            switch (*kind) {
                case 'd':
                    k = TAG_KIND_MACRO;
                    break;
                case 'g':
                case 's':
                case 't':
                case 'u':
                    k = TAG_KIND_TYPE;
                    break;
                case 'e':
                    k = TAG_KIND_ENUMERATOR;
                    break;
                default:
                    k = 0;
            }

            tree_insert(tags, strdup(tag), k);
        }
    }

    fclose(f);

out:;
LOG_EXIT();
}

void ctags_hl_parse(void) {
    tree_it(ctags_str_t, int) it;

    parse_tags();
    highlight_info_make(&hinfo);

    tree_traverse(tags, it) {
        switch (tree_it_val(it)) {
            case TAG_KIND_MACRO:
                highlight_add_kwd(&hinfo, tree_it_key(it), HL_PP);
                break;
            case TAG_KIND_TYPE:
                highlight_add_kwd(&hinfo, tree_it_key(it), HL_TYPE);
                break;
            case TAG_KIND_ENUMERATOR:
                highlight_add_kwd(&hinfo, tree_it_key(it), HL_CON);
                break;
        }
    }

    has_parsed = 1;
}

void ensure_parsed(void) {
    if (!has_parsed) {
        ctags_hl_parse();
    }
}

void ctags_hl_cleanup(void) {
    tree_it(ctags_str_t, int) it;

    if (has_parsed) {
        tree_traverse(tags, it) {
            free(tree_it_key(it));
        }
        tree_free(tags);

        highlight_info_free(&hinfo);
        ys->redraw = 1;
    }

    has_parsed = 0;
}

void unload(yed_plugin *self) {
    ctags_hl_cleanup();
}

int ctag_parse_path_and_search(const char *text, char *path_buff, char *search_buff, int *line_nr) {
    char *path_start;
    char *search_start;

    *line_nr = -1;
    if (!(path_start = strchr(text, '\t'))) {
        return 0;
    }
    path_start += 1;

    if (!(search_start = strchr(path_start, '\t'))) {
        return 0;
    }

    *search_start = 0;
    strcpy(path_buff, path_start);
    *search_start = '\t';

    search_start += 1;

    if (*search_start != '/') {
        if (sscanf(search_start, "%d", line_nr) == 0) {
            return 0;
        }
        return 1;
    }

    search_start += 1;
    if (*search_start == '^') {
        search_start += 1;
    }

    while (*search_start != '/') {
        if (*search_start == '\\') {
            search_start += 1;
        }
        *search_buff  = *search_start;
        search_buff  += 1;
        search_start += 1;
    }

    if (*(search_buff - 1) == '$') {
        *(search_buff - 1) = 0;
    } else {
        *search_buff = 0;
    }

    return 1;
}

void ctag_find_select(void) {
    yed_line *line;
    char      path[4096];
    char      search[4096];
    char     *save_cur_search;
    int       row;
    int       col;

    line = yed_buff_get_line(get_or_make_buff(), ys->active_frame->cursor_line);
    array_zero_term(line->chars);

    if (!ctag_parse_path_and_search(line->chars.data, path, search, &row)) {
        YEXE("special-buffer-prepare-unfocus", "*ctags-find-list");
LOG_CMD_ENTER("ctags-find");
        yed_cerr("unable to parse tag location");
LOG_EXIT();
        return;
    }

    YEXE("special-buffer-prepare-jump-focus", "*ctags-find-list");
    YEXE("buffer", path);

    if (ys->active_frame         == NULL
    ||  ys->active_frame->buffer == NULL) {
LOG_CMD_ENTER("ctags-find");
        yed_cerr("unable to open buffer for tag path '%s'", path);
LOG_EXIT();
        return;
    }

    /* This will help keep the destination near the top of the buffer. */
    YEXE("cursor-buffer-end");

    if (row > 0) {
        yed_set_cursor_within_frame(ys->active_frame, row, 1);
    } else {
        save_cur_search = ys->current_search;

        ys->current_search = search;
        if (yed_find_next(1, 1, &row, &col)) {
            yed_set_cursor_within_frame(ys->active_frame, row, col);
        } else {
            YEXE("cursor-buffer-begin");
LOG_CMD_ENTER("ctags-find");
            yed_cerr("could not find pattern from tag file");
LOG_EXIT();
        }

        ys->current_search = save_cur_search;
    }
}

void ctags_find_key_pressed_handler(yed_event *event) {
    yed_frame *eframe;

    eframe = ys->active_frame;

    if (event->key != ENTER                     /* not the key we want */
    ||  ys->interactive_command                 /* still typing        */
    ||  !eframe                                 /* no frame            */
    ||  !eframe->buffer                         /* no buffer           */
    ||  eframe->buffer != get_or_make_buff()) { /* not our buffer      */
        return;
    }

    ctag_find_select();

    event->cancel = 1;
}

void ctags_hl_line_handler(yed_event *event) {
    if (!yed_var_is_truthy("ctags-enable-extra-highlighting")) { return; }

    ensure_parsed();

    highlight_line(&hinfo, event);
}

void ctags_pump_handler(yed_event *event) {
LOG_CMD_ENTER("ctags-gen");

    if (gen_thread_started && gen_thread_finished) {
        if (gen_thread_exit_status) {
            yed_cerr("ctags failed with exit status %d", gen_thread_exit_status);
        } else {
            yed_cprint("ctags has completed");
        }
        gen_thread_started = gen_thread_finished = gen_thread_exit_status = 0;
    }

    if (should_reparse && !gen_thread_started) {
        ctags_hl_cleanup();
        ctags_hl_parse();
        ys->redraw     = 1;
        should_reparse = 0;
    }

LOG_EXIT();
}

void ctags_find_line_handler(yed_event *event) {
    yed_buffer *buff;
    yed_line   *line;
    yed_glyph  *git;
    int         col;
    int         width;
    int         i;
    yed_attrs   attrs[4];
    int         attr_pos;
    yed_attrs  *dst_attrs;

    if (event->frame->buffer != get_or_make_buff()) {
        return;
    }

    buff = event->frame->buffer;
    line = yed_buff_get_line(buff, event->row);

    attrs[0] = yed_active_style_get_code_keyword();
    attrs[1] = yed_active_style_get_code_string();
    attrs[2] = yed_active_style_get_code_number();
    attrs[3] = yed_active_style_get_code_comment();

    attr_pos = 0;
    col      = 0;
    yed_line_glyph_traverse(*line, git) {
        if (git->c == '\t' && attr_pos < 3) {
            attr_pos += 1;
        }
        width = yed_get_glyph_width(*git);
        for (i = 0; i < width; i += 1) {
            dst_attrs = array_item(event->line_attrs, col + i);
            yed_combine_attrs(dst_attrs, attrs + attr_pos);
        }
        col += width;
    }
}

void *ctags_gen_thread(void *arg) {
    char *cmd;

    cmd = arg;

    gen_thread_exit_status = system(cmd);

    free(cmd);
    gen_thread_finished = 1;

    return NULL;
}

void ctags_gen(int n_args, char **args) {
    char       cmd_buff[1024];
    char      *ctags_flags;
    pthread_t  p;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments but got %d -- set cflags options in 'ctags-flags'", n_args);
        return;
    }

    if (gen_thread_started) {
        yed_cerr("ctags is currently running");
        return;
    }

    ctags_flags = yed_get_var("ctags-flags");
    if (ctags_flags == NULL) {
        yed_cerr("'ctags-flags' is unset");
        return;
    }

    snprintf(cmd_buff, sizeof(cmd_buff), "ctags %s > /dev/null", ctags_flags);

    yed_cprint("running 'ctags %s' in background...", ctags_flags);
    gen_thread_finished = 0;
    gen_thread_started  = 1;
    pthread_create(&p, NULL, ctags_gen_thread, strdup(cmd_buff));
}

void ctags_find_filter(void) {
    char      *tag_start;
    char       cmd_buff[1024];
    int        status;
    yed_line  *line;
    yed_glyph *git;
    int        formatting_limit;
    char      *ctags_formatting_limit;
    int        max_tag_len;
    int        tag_len;
    int        row;
    int        col;
    int        i;

    array_zero_term(ys->cmd_buff);
    tag_start = array_data(ys->cmd_buff);

    if (strlen(tag_start) == 0) { return; }

    /* Try with binary search flag first. */
    sprintf(cmd_buff, "look -b '%s' tags", tag_start);

    if (yed_read_subproc_into_buffer(cmd_buff, get_or_make_buff(), &status) != 0) {
        return;
    }

    if (status != 0) {
        /* Failed.. so try without the flag. */
        sprintf(cmd_buff, "look '%s' tags", tag_start);
        if (yed_read_subproc_into_buffer(cmd_buff, get_or_make_buff(), &status) != 0) {
            return;
        }
        if (status != 0) {
            return;
        }
    }

    formatting_limit = INT32_MAX;
    if ((ctags_formatting_limit = yed_get_var("ctags-formatting-limit"))) {
        sscanf(ctags_formatting_limit, "%d", &formatting_limit);
        if (formatting_limit < 0) {
            formatting_limit = INT32_MAX;
        }
    }

    if (yed_buff_n_lines(get_or_make_buff()) > formatting_limit) { goto out; }

    /* Do some formatting. */
    max_tag_len = 0;
    bucket_array_traverse(get_or_make_buff()->lines, line) {
        tag_len = 0;
        yed_line_glyph_traverse(*line, git) {
            if (git->c == '\t') { break; }
            tag_len += 1;
        }
        if (tag_len > max_tag_len) {
            max_tag_len = tag_len;
        }
    }

    row = 1;
    bucket_array_traverse(get_or_make_buff()->lines, line) {
        array_zero_term(line->chars);

        line = yed_buff_get_line(get_or_make_buff(), row);

        for (col = 1; col <= line->visual_width;) {
            git = yed_line_col_to_glyph(line, col);
            if (git->c == '\t') {
                for (i = 0; i < max_tag_len - col + 1; i += 1) {
                    yed_insert_into_line_no_undo(get_or_make_buff(), row, col, G(' '));
                }
                break;
            }
            col += yed_get_glyph_width(*git);
        }

        row += 1;
    }

out:;
}

int ctags_find_start(char *start) {
    int i;

    ys->interactive_command = "ctags-find";
    ys->cmd_prompt          = "(ctags-find) ";

    yed_buff_clear_no_undo(get_or_make_buff());
    YEXE("special-buffer-prepare-focus", "*ctags-find-list");
    yed_frame_set_buff(ys->active_frame, get_or_make_buff());
    yed_set_cursor_far_within_frame(ys->active_frame, 1, 1);
    yed_clear_cmd_buff();

    if (start != NULL) {
        for (i = 0; i < strlen(start); i += 1) {
            yed_cmd_buff_push(start[i]);
        }
    }

    ctags_find_filter();

    return 1;
}

void ctags_find_take_key(int key) {
    switch (key) {
        case ESC:
        case CTRL_C:
            ys->interactive_command = NULL;
            yed_clear_cmd_buff();
            YEXE("special-buffer-prepare-unfocus", "*ctags-find-list");
            break;
        case ENTER:
            ys->interactive_command = NULL;
            ys->active_frame->dirty = 1;
            yed_clear_cmd_buff();
            if (yed_buff_n_lines(get_or_make_buff()) == 1) {
                ctag_find_select();
            }
            break;
        default:
            yed_cmd_line_readline_take_key(NULL, key);
            ctags_find_filter();
            break;
    }
}

void ctags_find(int n_args, char **args) {
    FILE *check;
    int   key;

    if (!ys->interactive_command) {
        if ((check = fopen("tags", "r"))) {
            fclose(check);
        } else {
            yed_cerr("error opening tags file (have you run 'ctags-gen'?)");
            return;
        }

        if (!ctags_find_start(n_args ? args[0] : NULL)) {
            ys->interactive_command = NULL;
            yed_cerr("error opening tags file (have you run 'ctags-gen'?)");
        }

        if (n_args) {
            ys->interactive_command = NULL;
            ys->active_frame->dirty = 1;
            yed_clear_cmd_buff();
            if (yed_buff_n_lines(get_or_make_buff()) == 1) {
                ctag_find_select();
            }
        }
    } else {
        sscanf(args[0], "%d", &key);
        ctags_find_take_key(key);
    }
}

void ctags_jump_to_definition(int n_args, char **args) {
    FILE       *check;
    char       *word;
    int         word_len;
    char        cmd_buff[1024];
    int         output_len;
    int         exit_code;
    char       *text;
    char        path[4096];
    char        search[4096];
    char       *save_cur_search;
    int         row;
    int         col;

    text = NULL;

    if ((check = fopen("tags", "r"))) {
        fclose(check);
    } else {
        yed_cerr("error opening tags file (have you run 'ctags-gen'?)");
        goto out;
    }

    if (!(word = yed_word_under_cursor())) {
        yed_cerr("cursor is not on a word");
        goto out;
    }

    word_len = strlen(word);

    snprintf(cmd_buff, sizeof(cmd_buff),
             "look -b '%s' tags 2>&1", word);
    text = yed_run_subproc(cmd_buff, &output_len, &exit_code);
    if (text) {
        if (exit_code == 0) {
            goto select;
        }

        if (strlen(text) != 0) {
            /* Failed because -b not supported. Try without. */
            free(text);
        } else {
            yed_cerr("tag '%s' not found", word);
            goto out;
        }
    }

    snprintf(cmd_buff, sizeof(cmd_buff),
             "look '%s' tags 2>/dev/null", word);
    text = yed_run_subproc(cmd_buff, &output_len, &exit_code);
    if (text) {
        if (exit_code == 0) {
            goto select;
        }
        yed_cerr("tag '%s' not found", word);
        goto out;
    }

    yed_cerr("failed to run 'look'");
    goto out;

select:
    if (strlen(text) <= word_len
    ||  strncmp(text, word, word_len)
    ||  *(char*)(text + word_len) != '\t')  {

        yed_cerr("tag '%s' not found", word);
        goto out;
    }

    if (!ctag_parse_path_and_search(text, path, search, &row)) {
        yed_cerr("error parsing location from tag line");
        goto out;
    }

    YEXE("buffer", path);

    /* This will help keep the destination near the top of the buffer. */
    YEXE("cursor-buffer-end");

    if (row > 0) {
        yed_set_cursor_within_frame(ys->active_frame, row, 1);
    } else {
        save_cur_search = ys->current_search;

        ys->current_search = search;
        if (yed_find_next(1, 1, &row, &col)) {
            yed_set_cursor_within_frame(ys->active_frame, row, col);
        } else {
            YEXE("cursor-buffer-begin");
            yed_cerr("could not find pattern from tag file");
        }

        ys->current_search = save_cur_search;
    }

out:
    if (text) { free(text); }
}

void ctags_hl_reparse(int n_args, char **args) {
    should_reparse = 1;
}
