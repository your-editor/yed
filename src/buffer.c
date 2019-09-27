#include "internal.h"

static yed_cell yed_new_cell(char c) {
    yed_cell cell;

    memset(&cell, 0, sizeof(cell));
    cell.bytes[0] = c;
    cell.width    = 1;

    return cell;
}

static yed_line yed_new_line(void) {
    yed_line line;

    memset(&line, 0, sizeof(line));

    line.chars = array_make(char);
    line.cells = array_make(yed_cell);

    return line;
}

static void yed_free_line(yed_line *line) {
    array_free(line->chars);
    array_free(line->cells);
}

static int yed_cell_n_bytes(yed_cell *cell) {
	if      (cell->__u32 == 0)             { return 0; }
	else if (cell->__u32 <= 0xFF000000)    { return 1; }
	else if (cell->__u32 <= 0xFFFF0000)    { return 2; }
	else if (cell->__u32 <= 0xFFFFFF00)    { return 3; }
	return 4;
}

static void yed_line_add_cell(yed_line *line, yed_cell *cell, int idx) {
    array_insert(line->cells, idx, *cell);
    line->visual_width += cell->width;
}

static void yed_line_append_cell(yed_line *line, yed_cell *cell) {
    array_push(line->cells, *cell);
    line->visual_width += cell->width;
}

static void yed_line_delete_cell(yed_line *line, int idx) {
    yed_cell *cell;

    cell = array_item(line->cells, idx);
    line->visual_width -= cell->width;
    array_delete(line->cells, idx);
}

static void yed_line_pop_cell(yed_line *line) {
    yed_line_delete_cell(line, array_len(line->cells) - 1);
}

static yed_line * yed_buffer_add_line(yed_buffer *buff) {
    yed_line *line;

    line  = array_next_elem(buff->lines);
    *line = yed_new_line();

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
    yed_cell cell;

    array_push(line->chars, c);

    cell = yed_new_cell(c);
    yed_line_append_cell(line, &cell);
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


static int yed_line_col_to_cell_idx(yed_line *line, int col) {
    int       found;
    int       cell_idx;
    yed_cell *cell_it;

    if (col == array_len(line->cells) + 1) {
        return col - 1;
    } else if (col == 1 && array_len(line->cells) == 0) {
        return 0;
    }

    cell_idx = 0;
    found    = 0;

    array_traverse(line->cells, cell_it) {
        if (col - cell_it->width <= 0) {
            found = 1;
            break;
        }
        col      -= cell_it->width;
        cell_idx += 1;
    }

    if (!found) {
        ASSERT(0, "didn't compute a good cell idx");
        return -1;
    }

    return cell_idx;
}

static yed_cell * yed_line_col_to_cell(yed_line *line, int col) {
    int idx;

    idx = yed_line_col_to_cell_idx(line, col);

    if (idx == -1) {
        return NULL;
    }

    return array_item(line->cells, idx);
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
    int      idx;
    yed_line new_line, *line;

    idx = row - 1;

    if (idx < 0 || idx > array_len(buff->lines)) {
        return NULL;
    }

    new_line = yed_new_line();
    line     = array_insert(buff->lines, idx, new_line);

    yed_mark_dirty_frames(buff);

    return line;
}

static void yed_buff_delete_line(yed_buffer *buff, int row) {
    int       idx;
    yed_line *line;

    idx = row - 1;

    LIMIT(idx, 0, array_len(buff->lines));

    line = yed_buff_get_line(buff, row);
    yed_free_line(line);
    array_delete(buff->lines, idx);

    yed_mark_dirty_frames(buff);
}

static void yed_insert_into_line(yed_buffer *buff, yed_line *line, int col, char c) {
    int      idx;
    yed_cell cell;

    idx = col - 1;

    LIMIT(idx, 0, array_len(line->chars));

    array_insert(line->chars, idx, c);

    cell = yed_new_cell(c);
    idx  = yed_line_col_to_cell_idx(line, col);
    yed_line_add_cell(line, &cell, idx);

    yed_mark_dirty_frames_line(buff, yed_buff_get_line_number(buff, line));
}

static void yed_delete_from_line(yed_buffer *buff, yed_line *line, int col) {
    int idx;

    idx = col - 1;

    LIMIT(idx, 0, array_len(line->chars));

    array_delete(line->chars, idx);

    idx = yed_line_col_to_cell_idx(line, col);
    yed_line_delete_cell(line, idx);

    yed_mark_dirty_frames_line(buff, yed_buff_get_line_number(buff, line));
}


static void yed_fill_buff_from_file(yed_buffer *buff, const char *path) {
    FILE     *f;
    char      c;
    yed_line *last_line;

    f = fopen(path, "r");
    if (!f) {
        ERR;
        return;
    }

    while ((c = fgetc(f)) != EOF) {
        yed_append_to_buff(buff, c);
    }

    if (array_len(buff->lines)) {
        last_line = array_last(buff->lines);
        if (array_len(last_line->cells) == 0) {
            array_pop(buff->lines);
        }
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
