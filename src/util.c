#include "internal.h"

char *yed_word_under_cursor(void) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_line   *line;
    int         col, word_len;
    char        c, *word_start, *ret;

    if (!ys->active_frame)    { return NULL; }
    frame = ys->active_frame;

    if (!frame->buffer)       { return NULL; }
    buff = frame->buffer;

    line = yed_buff_get_line(buff, frame->cursor_line);
    if (!line)                { return NULL; }

    col = frame->cursor_col;

    if (col == line->visual_width + 1)     { return NULL; }

    c   = yed_line_col_to_char(line, col);

    if (isspace(c))           { return NULL; }

    word_len = 0;

    if (isalnum(c) || c == '_') {
        while (col > 1) {
            c = yed_line_col_to_char(line, col - 1);

            if (!isalnum(c) && c != '_') {
                break;
            }

            col -= 1;
        }
        word_start = array_item(line->chars, yed_line_col_to_idx(line, col));
        c = yed_line_col_to_char(line, col);
        while (col <= line->visual_width
        &&    (isalnum(c) || c == '_')) {

            word_len += 1;
            col      += 1;
            c         = yed_line_col_to_char(line, col);
        }
    } else {
        while (col > 1) {
            c = yed_line_col_to_char(line, col - 1);

            if (isalnum(c) || c == '_' || isspace(c)) {
                break;
            }

            col -= 1;
        }
        word_start = array_item(line->chars, yed_line_col_to_idx(line, col));
        c = yed_line_col_to_char(line, col);
        while (col <= line->visual_width
        &&    (!isalnum(c) && c != '_' && !isspace(c))) {

            word_len += 1;
            col      += 1;
            c         = yed_line_col_to_char(line, col);
        }
    }

    ret = malloc(word_len + 1);
    memcpy(ret, word_start, word_len);
    ret[word_len] = 0;

    return ret;
}

char * get_path_ext(char *path) {
    char *ext;

    ext = strrchr(path, '.');

    if (!ext) {
        /* no extension */
    } else {
        ext += 1;
    }

    return ext;
}

char * path_without_ext(char *path) {
    char *ext;
    char *cpy;
    int   len;

    cpy = strdup(path);
    len = strlen(cpy);

    ext = get_path_ext(cpy);

    if (ext) {
        cpy[len - strlen(ext) - 1] = 0;
    }

    return cpy;
}

char *exe_path(char *prg) {
    char  cmd_buff[256];
    char *path;

    cmd_buff[0] = 0;

    strcat(cmd_buff, "which ");
    strcat(cmd_buff, prg);
    strcat(cmd_buff, " 2>/dev/null");

    path = yed_run_subproc(cmd_buff);

    if (path) {
        if (strlen(path) == 0) {
            free(path);
            return NULL;
        }
    }

    return path;
}
