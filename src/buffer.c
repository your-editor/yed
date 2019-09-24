#include "internal.h"

static yed_line * yed_buffer_add_line(yed_buffer *buff) {
    yed_line *line;

    line = array_next_elem(buff->lines);
    memset(line, 0, sizeof(*line));
    line->chars = array_make(char);

    yed_mark_dirty_frames(buff);

    return line;
}

static yed_buffer yed_new_buff(void) {
    yed_buffer  buff;

    buff.lines = array_make(yed_line);
	buff.path  = NULL;

    yed_buffer_add_line(&buff);

    return buff;
}

static void yed_append_to_line(yed_line *line, char c) {
    array_push(line->chars, c);
}

static void yed_append_to_buff(yed_buffer *buff, char c) {
    yed_line *line;

    if (c == '\n') {
        yed_buffer_add_line(buff);
    } else {
        if (array_len(buff->lines) == 0) {
            line = yed_buffer_add_line(buff);
        } else {
            line = array_last(buff->lines);
        }

        yed_append_to_line(line, c);
    }
}


static yed_line * yed_buff_get_line(yed_buffer *buff, int row) {
    int idx;

    idx = row - 1;

    if (idx < 0 || idx >= array_len(buff->lines)) {
        return NULL;
    }

    return array_item(buff->lines, idx);
}

static yed_line * yed_buff_insert_line(yed_buffer *buff, int row) {
    int       idx;
    yed_line  empty_line,
             *line;

    idx = row - 1;

    if (idx < 0 || idx > array_len(buff->lines)) {
        return NULL;
    }

    memset(&empty_line, 0, sizeof(yed_line));
    empty_line.chars = array_make(char);
    line = array_insert(buff->lines, idx, empty_line);

    yed_mark_dirty_frames(buff);

    return line;
}

static void yed_buff_delete_line(yed_buffer *buff, int row) {
    int idx;

    idx = row - 1;

    LIMIT(idx, 0, array_len(buff->lines));

    array_delete(buff->lines, idx);

    yed_mark_dirty_frames(buff);
}

static void yed_insert_into_line(yed_buffer *buff, yed_line *line, int col, char c) {
    int idx;

    idx = col - 1;

    LIMIT(idx, 0, array_len(line->chars));

    array_insert(line->chars, idx, c);

    yed_mark_dirty_frames_line(buff, yed_buff_get_line_number(buff, line));
}

static void yed_delete_from_line(yed_buffer *buff, yed_line *line, int col) {
    int idx;

    idx = col - 1;

    LIMIT(idx, 0, array_len(line->chars));

    array_delete(line->chars, idx);

    yed_mark_dirty_frames_line(buff, yed_buff_get_line_number(buff, line));
}


static void yed_fill_buff_from_file(yed_buffer *buff, const char *path) {
    FILE *f;
    char  c;

    f = fopen(path, "r");
    if (!f) {
        ERR;
        return;
    }

    while ((c = fgetc(f)) != EOF) {
        yed_append_to_buff(buff, c);
    }

	buff->path = strdup(path);

    fclose(f);

    yed_mark_dirty_frames(buff);
}

static void yed_write_buff_to_file(yed_buffer *buff, const char *path) {
    FILE       *f;
    yed_line   *line;
    const char *nl;

    f = fopen(path, "w");
    if (!f) {
        ERR;
        return;
    }

    nl = "";

    array_traverse(buff->lines, line) {
        fprintf(f, "%s", nl);
        fwrite(array_data(line->chars), 1, array_len(line->chars), f);
        nl = "\n";
    }

    fclose(f);
}

static int yed_buff_get_line_number(yed_buffer *buff, yed_line *line) {
    int row;

    row = 1 + ((((void*)line) - array_data(buff->lines)) / sizeof(*line));

    return row;
}
