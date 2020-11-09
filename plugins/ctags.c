#include <yed/plugin.h>

static yed_frame  *frame;
static yed_buffer *find_buff;
static char        prompt_buff[512];

void ctags_gen(int n_args, char **args);
void ctags_find(int n_args, char **args);
void ctags_jump_to_definition(int n_args, char **args);

void ctags_find_frame_delete_handler(yed_event *event);
void ctags_find_buffer_delete_handler(yed_event *event);
void ctags_find_key_pressed_handler(yed_event *event);
void ctags_find_line_handler(yed_event *event);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler frame_delete;
    yed_event_handler buffer_delete;
    yed_event_handler key_pressed;
    yed_event_handler line;

    frame_delete.kind  = EVENT_FRAME_PRE_DELETE;
    frame_delete.fn    = ctags_find_frame_delete_handler;
    buffer_delete.kind = EVENT_BUFFER_PRE_DELETE;
    buffer_delete.fn   = ctags_find_buffer_delete_handler;
    key_pressed.kind   = EVENT_KEY_PRESSED;
    key_pressed.fn     = ctags_find_key_pressed_handler;
    line.kind          = EVENT_LINE_PRE_DRAW;
    line.fn            = ctags_find_line_handler;
    yed_plugin_add_event_handler(self, frame_delete);
    yed_plugin_add_event_handler(self, buffer_delete);
    yed_plugin_add_event_handler(self, key_pressed);
    yed_plugin_add_event_handler(self, line);

    yed_plugin_set_command(self, "ctags-gen",  ctags_gen);
    yed_plugin_set_command(self, "ctags-find", ctags_find);
    yed_plugin_set_command(self, "ctags-jump-to-definition", ctags_jump_to_definition);

    if (!yed_get_var("ctags-formatting-limit")) {
        yed_set_var("ctags-formatting-limit", "10000");
    }

    return 0;
}

void ctags_find_frame_delete_handler(yed_event *event) {
    if (event->frame == frame) {
        if (find_buff) {
            yed_free_buffer(find_buff);
        }

        frame = NULL;
    }
}

void ctags_find_buffer_delete_handler(yed_event *event) {
    if (event->buffer == find_buff) {
        find_buff = NULL;
    }
}

void ctags_find_cleanup(void) {
    if (frame) {
        yed_delete_frame(frame);
    }
    if (find_buff) {
        yed_free_buffer(find_buff);
    }
}

int ctag_parse_path_and_search(const char *text, char *path_buff, char *search_buff) {
    char *path_start;
    char *search_start;

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
        return 0;
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

    line = yed_buff_get_line(find_buff, frame->cursor_line);
    array_zero_term(line->chars);

    if (!ctag_parse_path_and_search(line->chars.data, path, search)) {
        ctags_find_cleanup();
LOG_CMD_ENTER("ctags-find");
        yed_cerr("unable to parse tag location");
LOG_EXIT();
        return;
    }

    ctags_find_cleanup();

    YEXE("buffer", path);

    /* This will help keep the destination near the top of the buffer. */
    YEXE("cursor-buffer-end");

    save_cur_search = ys->current_search;

    ys->current_search = search;
    if (yed_find_next(1, 1, &row, &col)) {
        yed_set_cursor_within_frame(ys->active_frame, col, row);
    } else {
        YEXE("cursor-buffer-begin");
        yed_cerr("could not find pattern from tag file");
    }

    ys->current_search = save_cur_search;
}

void ctags_find_key_pressed_handler(yed_event *event) {
    yed_frame *eframe;

    eframe = ys->active_frame;

    if (event->key != ENTER            /* not the key we want */
    ||  ys->interactive_command        /* still typing        */
    ||  !eframe                        /* no frame            */
    ||  eframe != frame                /* not our frame       */
    ||  !eframe->buffer                /* no buffer           */
    ||  eframe->buffer != find_buff) { /* not our buffer      */
        return;
    }

    ctag_find_select();

    event->cancel = 1;
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

    if (event->frame->buffer != find_buff) {
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

void ctags_gen(int n_args, char **args) {
    char cmd_buff[1024];
    int  i;
    int  status;

    if (n_args == 0) {
        yed_cerr("expected 1 or more arguments");
        return;
    }

    cmd_buff[0] = 0;
    strcat(cmd_buff, "ctags ");
    for (i = 0; i < n_args; i += 1) {
        strcat(cmd_buff, " ");
        strcat(cmd_buff, args[i]);
    }
    strcat(cmd_buff, " > /dev/null 2>&1");

    status = system(cmd_buff);
    if (status) {
        yed_cerr("ctags failed with exit status %d", status);
        return;
    }
    yed_cprint("ctags has completed");
}

void ctags_find_set_prompt(char *p, char *attr) {
    prompt_buff[0] = 0;

    strcat(prompt_buff, p);

    if (attr) {
        strcat(prompt_buff, attr);
    }

    ys->cmd_prompt = prompt_buff;
}

int ctags_find_make_buffers(void) {
    if (!find_buff) {
        find_buff = yed_create_buffer("*ctags-find-list");
        find_buff->flags |= BUFF_RD_ONLY;
    } else {
        yed_buff_clear_no_undo(find_buff);
    }

    return 1;
}

void ctags_find_make_frame(void) {
    frame = yed_add_new_frame(0.15, 0.15, 0.7, 0.7);
    yed_clear_frame(frame);
    yed_activate_frame(frame);
}

void ctags_find_filter(void) {
    char      *tag_start;
    char       cmd_buff[1024];
    FILE      *stream;
    int        status;
    int        exit_code;
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

    yed_buff_clear_no_undo(find_buff);

    if (strlen(tag_start) == 0) { return; }

    /* Try with binary search flag first. */
    sprintf(cmd_buff, "look -b '%s' tags 2>/dev/null", tag_start);
    stream = popen(cmd_buff, "r");
    if (stream == NULL) { return; }
    status = yed_fill_buff_from_file_stream(find_buff, stream);
    if (status != BUFF_FILL_STATUS_SUCCESS) { return; }
    exit_code = pclose(stream);

    if (exit_code) {
        /* Failed.. so try without the flag. */
        sprintf(cmd_buff, "look '%s' tags 2>/dev/null", tag_start);
        stream = popen(cmd_buff, "r");
        if (stream == NULL) { return; }
        status = yed_fill_buff_from_file_stream(find_buff, stream);
        if (status != BUFF_FILL_STATUS_SUCCESS) { return; }
        exit_code = pclose(stream);
        if (exit_code) { return; }
    }

    formatting_limit = INT32_MAX;
    if ((ctags_formatting_limit = yed_get_var("ctags-formatting-limit"))) {
        sscanf(ctags_formatting_limit, "%d", &formatting_limit);
        if (formatting_limit < 0) {
            formatting_limit = INT32_MAX;
        }
    }

    if (yed_buff_n_lines(find_buff) > formatting_limit) { goto out; }

    /* Do some formatting. */
    max_tag_len = 0;
    bucket_array_traverse(find_buff->lines, line) {
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
    bucket_array_traverse(find_buff->lines, line) {
        array_zero_term(line->chars);

        line = yed_buff_get_line(find_buff, row);

        for (col = 1; col <= line->visual_width;) {
            git = yed_line_col_to_glyph(line, col);
            if (git->c == '\t') {
                for (i = 0; i < max_tag_len - col + 1; i += 1) {
                    yed_insert_into_line_no_undo(find_buff, row, col, G(' '));
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
    ctags_find_set_prompt("(ctags-find) ", NULL);
    ctags_find_make_frame();
    if (!ctags_find_make_buffers()) {
        return 0;
    }
    yed_frame_set_buff(frame, find_buff);
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
    if (key == CTRL_C) {
        ys->interactive_command = NULL;
        yed_clear_cmd_buff();
        ctags_find_cleanup();
    } else if (key == ENTER) {
        ys->interactive_command = NULL;
        frame->dirty            = 1;
        yed_clear_cmd_buff();
        if (yed_buff_n_lines(find_buff) == 1) {
            ctag_find_select();
        }
    } else {
        if (key == BACKSPACE) {
            if (array_len(ys->cmd_buff)) {
                yed_cmd_buff_pop();
            }
        } else if (!iscntrl(key)) {
            yed_cmd_buff_push(key);
        }

        ctags_find_filter();
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
            frame->dirty            = 1;
            yed_clear_cmd_buff();
            if (yed_buff_n_lines(find_buff) == 1) {
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

    if (!ctag_parse_path_and_search(text, path, search)) {
        yed_cerr("error parsing location from tag line");
        goto out;
    }

    YEXE("buffer", path);

    /* This will help keep the destination near the top of the buffer. */
    YEXE("cursor-buffer-end");

    save_cur_search = ys->current_search;

    ys->current_search = search;
    if (yed_find_next(1, 1, &row, &col)) {
        yed_set_cursor_within_frame(ys->active_frame, col, row);
    } else {
        YEXE("cursor-buffer-begin");
        yed_cerr("could not find pattern from tag file");
    }

    ys->current_search = save_cur_search;

out:
    if (text) { free(text); }
}
