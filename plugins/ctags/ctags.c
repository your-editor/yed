#include <yed/plugin.h>
#include <yed/syntax.h>

#include <yed/tree.h>
typedef char *ctags_str_t;
use_tree_c(ctags_str_t, int, strcmp);

#define TAG_KIND_MACRO      (1)
#define TAG_KIND_TYPE       (2)
#define TAG_KIND_ENUMERATOR (3)

typedef struct {
    yed_direct_draw_t *dd;
    char              *text;
    int                row;
    int                col;
} ctags_fn_hint;

static int                     gen_thread_started;
static int                     gen_thread_finished;
static int                     gen_thread_exit_status;
static int                     parse_thread_started;
static int                     parse_thread_finished;
static int                     has_parsed;
static pthread_t               gen_pthread;
static pthread_mutex_t         gen_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_t               parse_pthread;
static pthread_mutex_t         parse_mtx = PTHREAD_MUTEX_INITIALIZER;
static int                     using_tmp;
static char                    tmp_tags_file[4096];
static array_t                 tmp_tags_buffers;
static array_t                 hint_stack;
static int                     hint_row;
static tree(ctags_str_t, int)  tags;
static pthread_mutex_t         tags_mtx = PTHREAD_MUTEX_INITIALIZER;
static yed_syntax              syn;
static yed_syntax              syn_tmp;

yed_buffer *get_or_make_buff(void) {
    yed_buffer *buff;

    buff = yed_get_buffer("*ctags-find-list");

    if (buff == NULL) {
        buff = yed_create_buffer("*ctags-find-list");
        buff->flags |= BUFF_RD_ONLY | BUFF_SPECIAL;
    }

    return buff;
}

void ctags_gen(int n_args, char **args);
void ctags_find(int n_args, char **args);
void ctags_jump_to_definition(int n_args, char **args);
void ctags_show_function_hint(int n_args, char **args);
void ctags_dismiss_function_hint(int n_args, char **args);
void ctags_parse(void);
void ctags_reparse(int n_args, char **args);

void unload(yed_plugin *self);
void ctags_find_key_pressed_handler(yed_event *event);
void ctags_find_line_handler(yed_event *event);
void ctags_hl_line_handler(yed_event *event);
void ctags_pump_handler(yed_event *event);
void ctags_buffer_post_load_handler(yed_event *event);
void ctags_buffer_post_write_handler(yed_event *event);
void ctags_buffer_pre_quit_handler(yed_event *event);
void ctags_buffer_post_insert_handler(yed_event *event);
void ctags_cursor_post_move_handler(yed_event *event);

void estyle(yed_event *event) {
    yed_syntax_style_event(&syn,     event);
    yed_syntax_style_event(&syn_tmp, event);
}

static int ctags_compl(char *string, struct yed_completion_results_t *results);

static const char *tags_file_name(void);
static void setup_tmp_tags(void);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler key_pressed;
    yed_event_handler find_line;
    yed_event_handler hl_line;
    yed_event_handler pump;
    yed_event_handler load;
    yed_event_handler write;
    yed_event_handler quit;
    yed_event_handler insert;
    yed_event_handler move;
    yed_event_handler style;

    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_unload_fn(self, unload);

    yed_syntax_start(&syn);
    yed_syntax_start(&syn_tmp);
    tags       = tree_make(ctags_str_t, int);
    hint_stack = array_make(ctags_fn_hint);

    tmp_tags_buffers = array_make(char*);

    key_pressed.kind   = EVENT_KEY_PRESSED;
    key_pressed.fn     = ctags_find_key_pressed_handler;
    find_line.kind     = EVENT_LINE_PRE_DRAW;
    find_line.fn       = ctags_find_line_handler;
    hl_line.kind       = EVENT_LINE_PRE_DRAW;
    hl_line.fn         = ctags_hl_line_handler;
    pump.kind          = EVENT_PRE_PUMP;
    pump.fn            = ctags_pump_handler;
    load.kind          = EVENT_BUFFER_POST_LOAD;
    load.fn            = ctags_buffer_post_load_handler;
    write.kind         = EVENT_BUFFER_POST_WRITE;
    write.fn           = ctags_buffer_post_write_handler;
    quit.kind          = EVENT_PRE_QUIT;
    quit.fn            = ctags_buffer_pre_quit_handler;
    insert.kind        = EVENT_BUFFER_POST_INSERT;
    insert.fn          = ctags_buffer_post_insert_handler;
    move.kind          = EVENT_CURSOR_POST_MOVE;
    move.fn            = ctags_cursor_post_move_handler;
    style.kind         = EVENT_STYLE_CHANGE;
    style.fn           = estyle;
    yed_plugin_add_event_handler(self, key_pressed);
    yed_plugin_add_event_handler(self, find_line);
    yed_plugin_add_event_handler(self, hl_line);
    yed_plugin_add_event_handler(self, pump);
    yed_plugin_add_event_handler(self, load);
    yed_plugin_add_event_handler(self, write);
    yed_plugin_add_event_handler(self, quit);
    yed_plugin_add_event_handler(self, insert);
    yed_plugin_add_event_handler(self, move);
    yed_plugin_add_event_handler(self, style);

    yed_plugin_set_command(self, "ctags-gen",                   ctags_gen);
    yed_plugin_set_command(self, "ctags-find",                  ctags_find);
    yed_plugin_set_command(self, "ctags-jump-to-definition",    ctags_jump_to_definition);
    yed_plugin_set_command(self, "ctags-show-function-hint",    ctags_show_function_hint);
    yed_plugin_set_command(self, "ctags-dismiss-function-hint", ctags_dismiss_function_hint);
    yed_plugin_set_command(self, "ctags-reparse",               ctags_reparse);

    yed_plugin_set_completion(self, "tags", ctags_compl);

    if (!yed_get_var("ctags-tags-file")) {
        yed_set_var("ctags-tags-file", "tags");
    }
    tags_file_name(); /* Just to create the name in the buffer. */

    if (!yed_get_var("ctags-enable-extra-highlighting")) {
        yed_set_var("ctags-enable-extra-highlighting", "yes");
    }

    if (!yed_get_var("ctags-compl")) {
        yed_set_var("ctags-compl", "yes");
    }

    if (!yed_get_var("ctags-formatting-limit")) {
        yed_set_var("ctags-formatting-limit", "10000");
    }

    if (!yed_get_var("ctags-additional-paths")) {
        yed_set_var("ctags-additional-paths", "");
    }

    if (!yed_get_var("ctags-fallback-autogen")) {
        yed_set_var("ctags-fallback-autogen", "yes");
    }

    if (!yed_get_var("ctags-regen-on-write")) {
        yed_set_var("ctags-regen-on-write", "yes");
    }

    if (!yed_get_var("ctags-auto-show-function-hint")) {
        yed_set_var("ctags-auto-show-function-hint", "yes");
    }

    if (!yed_get_var("ctags-auto-dismiss-function-hint")) {
        yed_set_var("ctags-auto-dismiss-function-hint", "yes");
    }

    if (yed_var_is_truthy("ctags-fallback-autogen")) {
        if (access(yed_get_var("ctags-tags-file"), F_OK) != 0) {
            setup_tmp_tags();
        }
    }

    if (yed_var_is_truthy("ctags-enable-extra-highlighting")
    ||  yed_var_is_truthy("ctags-compl")) {
        ctags_parse();
    }

    return 0;
}

static const char *tags_file_name(void) {
    const char *tags_file;

    if (using_tmp) {
        snprintf(tmp_tags_file, sizeof(tmp_tags_file),
                 "/tmp/yed_ctags_%d", getpid());
        tags_file = tmp_tags_file;
    } else {
        tags_file = yed_get_var("ctags-tags-file");
        if (tags_file == NULL) {
            tags_file = "tags";
        }
    }

    return tags_file;
}

void *ctags_gen_thread(void *arg) {
    char *cmd;

    pthread_mutex_lock(&gen_mtx);

    cmd = arg;

    gen_thread_exit_status = system(cmd);

    free(cmd);
    gen_pthread         = 0;
    gen_thread_finished = 1;

    pthread_mutex_unlock(&gen_mtx);

    return NULL;
}

static void show_fn_hint(ctags_fn_hint *hint) {
    yed_attrs attrs;
    yed_attrs assoc_attrs;

    attrs       = yed_active_style_get_active();
    assoc_attrs = yed_active_style_get_associate();

    yed_combine_attrs(&attrs, &assoc_attrs);
    hint->dd = yed_direct_draw(hint->row, hint->col, attrs, hint->text);
}

static void hide_fn_hint(ctags_fn_hint *hint) {
    yed_kill_direct_draw(hint->dd);
    hint->dd = NULL;
}

static void pop_fn_hint(void) {
    ctags_fn_hint *hit;

    hit = array_last(hint_stack);
    if (hit == NULL) { return; }
    hide_fn_hint(hit);
    free(hit->text);
    array_pop(hint_stack);

    hit = array_last(hint_stack);
    if (hit == NULL) {
        hint_row = 0;
    } else {
        show_fn_hint(hit);
    }
}

static void push_fn_hint(char *hint, int row, int col) {
    ctags_fn_hint     *last_hint;
    int                abs_y;
    int                abs_x;
    ctags_fn_hint      new_hint;

    if (ys->active_frame == NULL) { return; }

    last_hint = array_last(hint_stack);
    if (last_hint != NULL) {
        hide_fn_hint(last_hint);
    }

    hint_row = row;

    abs_y = ys->active_frame->cur_y - (ys->active_frame->cursor_line - row);
    abs_x = ys->active_frame->cur_x - (ys->active_frame->cursor_col  - col);

    abs_y = abs_y < ys->active_frame->top + ys->active_frame->height - 1
            ? abs_y + 1
            : abs_y - 1;

    new_hint.text = strdup(hint);
    new_hint.row  = abs_y;
    new_hint.col  = abs_x;

    array_push(hint_stack, new_hint);

    show_fn_hint(array_last(hint_stack));
}

static void clear_all_hints(void) {
    while (array_len(hint_stack)) {
        pop_fn_hint();
    }
}

static void launch_tmp_tags_gen(void) {
    char            cmd_buff[8192];
    char          **it;
    char           *add_paths;
    DIR            *d;
    struct dirent  *dent;
    char            check_path[4096];
    const char     *base;
    char            rel_path[2048];

    cmd_buff[0] = 0;

    strcat(cmd_buff, "ctags -f");
    strcat(cmd_buff, tags_file_name());
    array_traverse(tmp_tags_buffers, it) {
        if (  strlen(cmd_buff)
            + 1 /* NULL byte */
            + strlen(" ")
            + strlen(*it)
            + strlen(" > /dev/null")
            > sizeof(cmd_buff)) { goto trunc; }

        strcat(cmd_buff, " ");
        strcat(cmd_buff, *it);
    }

    if ((add_paths = yed_get_var("ctags-additional-paths"))) {
        if (  strlen(cmd_buff)
            + 1 /* NULL byte */
            + strlen(" ")
            + strlen(add_paths)
            + strlen(" > /dev/null")
            > sizeof(cmd_buff)) { goto trunc; }

        strcat(cmd_buff, " ");
        strcat(cmd_buff, add_paths);
    }

    if ((d = opendir(".")) != NULL) {
        while ((dent = readdir(d)) != NULL) {
            if (dent->d_type != DT_REG) { goto next1; }

            abs_path(dent->d_name, check_path);

            base = get_path_basename(check_path);

            if (strncmp(base, ".yed_ctags", strlen(".yed_ctags")) == 0) { goto next1; }

            array_traverse(tmp_tags_buffers, it) {
                if (strcmp(*it, check_path) == 0) { goto next1; }
            }

            if (  strlen(cmd_buff)
                + 1 /* NULL byte */
                + strlen(" ")
                + strlen(dent->d_name)
                + strlen(" > /dev/null")
                > sizeof(cmd_buff)) { closedir(d); goto trunc; }

            strcat(cmd_buff, " ");
            strcat(cmd_buff, dent->d_name);
next1:;
        }
        closedir(d);
    }

    if ((d = opendir("src")) != NULL) {
        while ((dent = readdir(d)) != NULL) {
            if (dent->d_type != DT_REG) { goto next2; }

            snprintf(rel_path, sizeof(rel_path), "src/%s", dent->d_name);

            abs_path(rel_path, check_path);

            base = get_path_basename(check_path);

            if (strncmp(base, ".yed_ctags", strlen(".yed_ctags")) == 0) { goto next2; }

            array_traverse(tmp_tags_buffers, it) {
                if (strcmp(*it, check_path) == 0) { goto next2; }
            }

            if (  strlen(cmd_buff)
                + 1 /* NULL byte */
                + strlen(" ")
                + strlen(rel_path)
                + strlen(" > /dev/null")
                > sizeof(cmd_buff)) { closedir(d); goto trunc; }

            strcat(cmd_buff, " ");
            strcat(cmd_buff, rel_path);
next2:;
        }
        closedir(d);
    }

trunc:;
    strcat(cmd_buff, " > /dev/null");

LOG_CMD_ENTER("ctags");
    yed_cprint("running ctags in background...");
LOG_EXIT();
    gen_thread_finished = 0;
    gen_thread_started  = 1;
    pthread_create(&gen_pthread, NULL, ctags_gen_thread, strdup(cmd_buff));
}

static void setup_tmp_tags(void) {
    tree_it(yed_buffer_name_t, yed_buffer_ptr_t)  bit;
    yed_buffer                                   *buff;
    char                                         *cpy;

    tree_traverse(ys->buffers, bit) {
        buff = tree_it_val(bit);
        if (buff->flags & BUFF_SPECIAL
        ||  buff->path == NULL) { continue; }
        cpy = strdup(buff->path);
        array_push(tmp_tags_buffers, cpy);
    }

    using_tmp = 1;

    launch_tmp_tags_gen();

    if (yed_var_is_truthy("ctags-enable-extra-highlighting")
    ||  yed_var_is_truthy("ctags-compl")) {
        ctags_parse();
    }
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

void * ctags_parse_thread(void *arg) {
    FILE                      *f;
    char                       line[4096];
    char                      *tag;
    char                      *file;
    char                      *kind;
    int                        k;
    int                        do_hl;
    tree_it(ctags_str_t, int)  it;
    char                      *key;

    pthread_mutex_lock(&parse_mtx);

    /* Wait on the gen thread if it's running so that we parse up-to-date tags. */
    while (gen_thread_started && !gen_thread_finished) { usleep(1000); }

    pthread_mutex_lock(&tags_mtx);

    tags = tree_make(ctags_str_t, int);

    f = fopen(tags_file_name(), "r");
    if (f == NULL) { goto out; }

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

    do_hl = !!arg;

    if (do_hl) {
        yed_syntax_free(&syn_tmp);
        yed_syntax_start(&syn_tmp);

        k = 0;
        yed_syntax_attr_push(&syn_tmp, "");

        tree_traverse(tags, it) {
            if (tree_it_val(it) != k) {
                yed_syntax_attr_pop(&syn_tmp);
                switch (tree_it_val(it)) {
                    case TAG_KIND_MACRO:      yed_syntax_attr_push(&syn_tmp, "&code-preprocessor"); break;
                    case TAG_KIND_TYPE:       yed_syntax_attr_push(&syn_tmp, "&code-typename");     break;
                    case TAG_KIND_ENUMERATOR: yed_syntax_attr_push(&syn_tmp, "&code-constant");     break;
                    default:                  yed_syntax_attr_push(&syn_tmp, "");                   break;
                }
                k = tree_it_val(it);
            }
            yed_syntax_kwd(&syn_tmp, tree_it_key(it));
        }

        yed_syntax_attr_pop(&syn_tmp);

        yed_syntax_end(&syn_tmp);
    }

out:;
    pthread_mutex_unlock(&tags_mtx);

    parse_pthread         = 0;
    parse_thread_finished = 1;

    pthread_mutex_unlock(&parse_mtx);

    return NULL;
}

void ctags_hl_cleanup(void) {
    yed_syntax_free(&syn);
}

void ctags_parse_cleanup(void) {
    tree_it(ctags_str_t, int)  it;
    char                      *key;

    pthread_mutex_lock(&tags_mtx);

    if (tags->_root == NULL) { goto out; }

    while (tree_len(tags)) {
        it  = tree_begin(tags);
        key = tree_it_key(it);
        tree_delete(tags, key);
        free(key);
    }
    tree_free(tags);

out:;
    pthread_mutex_unlock(&tags_mtx);
}

void ctags_finish_parse(void) {
    yed_syntax syn_swap;

LOG_CMD_ENTER("ctags");
    parse_thread_started = parse_thread_finished = 0;

    pthread_mutex_lock(&tags_mtx);
    memcpy(&syn_swap, &syn, sizeof(syn));
    memcpy(&syn, &syn_tmp, sizeof(syn_tmp));
    memcpy(&syn_tmp, &syn_swap, sizeof(syn_swap));
    pthread_mutex_unlock(&tags_mtx);

    yed_cprint("tags have been parsed for highlighting/completion");

    has_parsed = 1;
LOG_EXIT();
}

void ctags_parse(void) {
    if (parse_thread_started) { return; }

    parse_thread_finished = 0;
    parse_thread_started  = 1;
    ctags_parse_cleanup();
    pthread_create(&parse_pthread, NULL, ctags_parse_thread, (void*)(u64)yed_var_is_truthy("ctags-enable-extra-highlighting"));

    /* See if the thread can finish in 15ms... if so, we can avoid the pump delay. */
    usleep(15000);

    if (parse_thread_finished) { ctags_finish_parse(); }

}

static void delete_tmp_tags_file(void) {
    unlink(tmp_tags_file);
}

void unload(yed_plugin *self) {
    int cancelled;

    cancelled = 0;

    pthread_mutex_lock(&gen_mtx);
    pthread_mutex_lock(&parse_mtx);

    ctags_parse_cleanup();
    ctags_hl_cleanup();

    free_string_array(tmp_tags_buffers);

    delete_tmp_tags_file();

    clear_all_hints();
    array_free(hint_stack);
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

static char *lookup_tag_get_hint(char *tag) {
    char *line;
    char *output;
    char *output_cpy;
    char  cmd_buff[1024];
    int   output_len;
    int   status;
    char *found_tag;
    char *found_file;
    char *found_kind;
    char  path_buff[4096];
    char  search_buff[4096];
    int   line_nr;
    char *start;
    char *end;

    line       = NULL;
    output     = NULL;
    output_cpy = NULL;

    /* Try with binary search flag first. */
    snprintf(cmd_buff, sizeof(cmd_buff), "look -b '%s' %s", tag, tags_file_name());

    output = yed_run_subproc(cmd_buff, &output_len, &status);

    if (output == NULL || status != 0) {
        if (output != NULL) { free(output); }

        /* Failed.. so try without the flag. */
        snprintf(cmd_buff, sizeof(cmd_buff), "look '%s' %s", tag, tags_file_name());

        output = yed_run_subproc(cmd_buff, &output_len, &status);
        if (output == NULL || status != 0) { goto out; }
    }

    output_cpy = strdup(output);
    parse_tag_line(output_cpy, &found_tag, &found_file, &found_kind);

    if (strcmp(tag, found_tag) != 0)              { goto out; }
/*     if (*found_kind != 'd' && *found_kind != 'f') { goto out; } */

    if (!ctag_parse_path_and_search(output, path_buff, search_buff, &line_nr)) { goto out; }

    start = search_buff;
    while (*start && *start != '(') { start += 1; }
    if (!*start) { start = search_buff; }

    end = search_buff + strlen(search_buff) - 1;
    while (*end && *end != ')') { *end = 0; end -= 1; }

    line = strdup(start);

out:;
    if (output_cpy != NULL) { free(output_cpy); }
    if (output     != NULL) { free(output);     }

    return line;
}

static void do_show_function_hint(void) {
    int        col;
    int        last_open_paren_col;
    int        balance;
    yed_glyph *g;
    char      *word;
    char      *hint;

    if (ys->active_frame == NULL || ys->active_frame->buffer == NULL) { return; }

    col     = last_open_paren_col = ys->active_frame->cursor_col - 1;
    balance = 1;

    while (col > 1) {
        g = yed_buff_get_glyph(ys->active_frame->buffer, ys->active_frame->cursor_line, col);
        if (g->c == ')') {
            balance += 1;
        } else if (g->c == '(') {
            balance -= 1;
            last_open_paren_col = col;
        } else if (balance == 0 && (isalnum(g->c) || g->c == '_')) {
            break;
        }
        col -= 1;
    }

    if (col == 1) { return; }

    word = yed_word_at_point(ys->active_frame, ys->active_frame->cursor_line, col);
    if (word == NULL) { return; }

    hint = NULL;

    hint = lookup_tag_get_hint(word);
    if (hint == NULL) { goto cleanup; }

    push_fn_hint(hint, ys->active_frame->cursor_line, last_open_paren_col);

cleanup:;
    if (hint != NULL) { free(hint); }
    if (word != NULL) { free(word); }
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
    yed_frame *frame;

    if (!yed_var_is_truthy("ctags-enable-extra-highlighting")) { return; }

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->flags & BUFF_SPECIAL) {
        return;
    }

    if (!has_parsed) { ctags_parse(); }

    yed_syntax_line_event(&syn, event);
}

void ctags_pump_handler(yed_event *event) {
    int did_gen_thread;

LOG_CMD_ENTER("ctags");

    did_gen_thread = 0;

    if (gen_thread_started && gen_thread_finished) {
        if (gen_thread_exit_status) {
            yed_cerr("ctags failed with exit status %d", gen_thread_exit_status);
        } else {
            yed_cprint("ctags-gen has completed");
        }
        gen_thread_started = gen_thread_finished = gen_thread_exit_status = 0;
        did_gen_thread = 1;
    }

    if (!gen_thread_started && parse_thread_started && parse_thread_finished) {
        if (did_gen_thread) { yed_cprint("\n"); }
        ctags_finish_parse();
    }

LOG_EXIT();
}

void ctags_buffer_post_load_handler(yed_event *event) {
    const char  *base;
    char       **it;
    char        *cpy;

    if (!using_tmp)                  { return; }
    if (event->buffer == NULL)       { return; }
    if (event->buffer->path == NULL) { return; }
    if (event->buffer_is_new_file)   { return; }

    base = get_path_basename(event->buffer->path);
    if (strncmp(base, ".yed_ctags", strlen(".yed_ctags")) == 0) { return; }

    array_traverse(tmp_tags_buffers, it) {
        if (strcmp(*it, event->buffer->path) == 0) { return; }
    }

    cpy = strdup(event->buffer->path);
    array_push(tmp_tags_buffers, cpy);

    launch_tmp_tags_gen();

    if (yed_var_is_truthy("ctags-enable-extra-highlighting")
    ||  yed_var_is_truthy("ctags-compl")) {
        ctags_parse();
    }
}

void ctags_buffer_post_write_handler(yed_event *event) {
    if (!yed_var_is_truthy("ctags-regen-on-write")) { return; }

    if (using_tmp) {
        launch_tmp_tags_gen();
    } else {
        YEXE("ctags-gen");
    }

    if (yed_var_is_truthy("ctags-enable-extra-highlighting")
    ||  yed_var_is_truthy("ctags-compl")) {
        ctags_parse();
    }
}

void ctags_buffer_pre_quit_handler(yed_event *event) {
    delete_tmp_tags_file();
}

void ctags_buffer_post_insert_handler(yed_event *event) {
    if (event->key != '(')                                   { return; }
    if (!yed_var_is_truthy("ctags-auto-show-function-hint")) { return; }

    do_show_function_hint();
}

void ctags_cursor_post_move_handler(yed_event *event) {
    ctags_fn_hint *hit;
    int            balance;
    int            col;
    int            q;       /* 1: single quote, 2: double quote */
    char           last;
    yed_glyph     *g;

    if (array_len(hint_stack) == 0) { return; }

    if (ys->active_frame->cursor_line != hint_row) {
        clear_all_hints();
        return;
    }

    if (!yed_var_is_truthy("ctags-auto-dismiss-function-hint")) { return; }
    if (ys->active_frame->buffer == NULL)                       { return; }

again:
    hit = array_last(hint_stack);
    if (hit == NULL) { return; }

    if (ys->active_frame->cursor_col <= hit->col) {
        pop_fn_hint();
        goto again;
    } else {
        balance = 1;
        col     = hit->col + 1;
        q       = 0;
        last    = 0;

        while (col < ys->active_frame->cursor_col) {
            g = yed_buff_get_glyph(ys->active_frame->buffer, ys->active_frame->cursor_line, col);

            if (q == 1) {
                if (g->c == '\'' && last != '\\') { q = 0; }
            } else if (q == 2) {
                if (g->c == '"'  && last != '\\') { q = 0; }
            } else {
                switch (g->c) {
                    case '\'': q        = 1; break;
                    case '"':  q        = 2; break;
                    case '(':  balance += 1; break;
                    case ')':  balance -= 1; break;
                }
            }

            if (balance == 0) { break; }

            last  = g->c;
            col  += 1;
        }

        if (balance == 0) {
            pop_fn_hint();
            goto again;
        }
    }
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

static int ctags_compl(char *string, struct yed_completion_results_t *results) {
    int                       status;
    tree_it(ctags_str_t, int) it;

    status = 0;

    if (!has_parsed) { ctags_parse(); }

    FN_BODY_FOR_COMPLETE_FROM_TREE(string, tags, it, results, status);

    return status;
}

void ctags_gen(int n_args, char **args) {
    char       cmd_buff[4096];
    char      *ctags_flags;
    char      *additional_paths;

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

    additional_paths = yed_get_var("ctags-additional-paths");
    if (additional_paths == NULL) {
        additional_paths = "";
    }

    snprintf(cmd_buff, sizeof(cmd_buff), "ctags %s %s > /dev/null", ctags_flags, additional_paths);

    yed_cprint("running 'ctags %s' in background...", ctags_flags);
    gen_thread_finished = 0;
    gen_thread_started  = 1;
    pthread_create(&gen_pthread, NULL, ctags_gen_thread, strdup(cmd_buff));

    using_tmp = 0;
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

    get_or_make_buff()->flags &= ~BUFF_RD_ONLY;

    yed_buff_clear_no_undo(get_or_make_buff());

    if (strlen(tag_start) == 0) { goto out; }


    /* Try with binary search flag first. */
    snprintf(cmd_buff, sizeof(cmd_buff), "look -b '%s' %s", tag_start, tags_file_name());

    if (yed_read_subproc_into_buffer(cmd_buff, get_or_make_buff(), &status) != 0) {
        goto out;
    }

    if (status != 0) {
        /* Failed.. so try without the flag. */
        snprintf(cmd_buff, sizeof(cmd_buff), "look '%s' %s", tag_start, tags_file_name());
        if (yed_read_subproc_into_buffer(cmd_buff, get_or_make_buff(), &status) != 0) {
            goto out;
        }
        if (status != 0) {
            goto out;
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
    get_or_make_buff()->flags |= BUFF_RD_ONLY;
}

int ctags_find_start(char *start) {
    int i;

    ys->interactive_command = "ctags-find";
    ys->cmd_prompt          = "(ctags-find) ";

    get_or_make_buff()->flags &= ~BUFF_RD_ONLY;
    yed_buff_clear_no_undo(get_or_make_buff());
    get_or_make_buff()->flags |= BUFF_RD_ONLY;
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
        if ((check = fopen(tags_file_name(), "r"))) {
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

    if ((check = fopen(tags_file_name(), "r"))) {
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
             "look -b '%s' %s 2>&1", word, tags_file_name());
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
             "look '%s' %s 2>/dev/null", word, tags_file_name());
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

void ctags_show_function_hint(int n_args, char **args) {
    do_show_function_hint();
}

void ctags_dismiss_function_hint(int n_args, char **args) {
    pop_fn_hint();
}

void ctags_reparse(int n_args, char **args) {
    ctags_parse();
}
