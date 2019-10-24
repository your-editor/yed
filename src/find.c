#include "internal.h"

void yed_init_search(void) { }

void yed_search_line_handler(yed_event *event) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_line   *line;
    yed_attrs  *attr;
    char       *line_data,
               *line_data_start,
               *line_data_end;
    int         i,
                search_len,
                data_len;

    if (!ys->current_search) {
        return;
    }

    if (!event->frame) {
        return;
    }

    frame = event->frame;

    if (!frame->buffer) {
        return;
    }

    buff       = frame->buffer;
    line       = yed_buff_get_line(buff, event->row);
    data_len   = array_len(line->chars);
    search_len = strlen(ys->current_search);

    if (!data_len)    { return; }

    line_data     = line_data_start = array_data(line->chars);
    line_data_end = line_data + data_len;

    for (; line_data != line_data_end; line_data += 1) {
        if (strncmp(ys->current_search, line_data, search_len) == 0) {
            if (event->row == frame->cursor_line
            && (line_data - line_data_start) + 1 == frame->cursor_col) {
                for (i = 0; i < search_len; i += 1) {
                    attr         = array_item(event->line_attrs, (line_data - line_data_start) + i);
                    attr->flags  = ATTR_RGB;
                    attr->bg     = RGB_32(255, 150, 0);
                    attr->fg     = RGB_32(0, 0, 255);
                }
            } else {
                for (i = 0; i < search_len; i += 1) {
                    attr         = array_item(event->line_attrs, (line_data - line_data_start) + i);
                    attr->flags  = ATTR_RGB;
                    attr->bg     = RGB_32(255, 255, 0);
                    attr->fg     = RGB_32(0, 0, 255);
                }
            }
        }
    }
}

int yed_find_next(int row, int col, int *row_out, int *col_out) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_line   *line;
    char       *line_data,
               *line_data_start,
               *line_data_end;
    int         i,
                r,
                search_len,
                data_len;

    if (!ys->current_search)    { return 0; }
    if (!ys->active_frame)      { return 0; }

    frame = ys->active_frame;

    if (!frame->buffer)    { return 0; }

    buff       = frame->buffer;
    search_len = strlen(ys->current_search);

    r = row;
    bucket_array_traverse_from(buff->lines, line, row - 1) {
        data_len = array_len(line->chars);

        if (!data_len) {
            r += 1;
            continue;
        }

        line_data     = line_data_start = array_data(line->chars);
        line_data_end = line_data + data_len;

        for (i = 0; i < data_len; i += 1) {
            if (strncmp(ys->current_search, line_data + i, search_len) == 0) {
                *row_out = r;
                *col_out = i + 1;
                return 1;
            }
        }

        r += 1;
    }

    r = 1;
    bucket_array_traverse(buff->lines, line) {
        if (r == row)    { break; }

        data_len = array_len(line->chars);

        if (!data_len) {
            r += 1;
            continue;
        }

        line_data     = line_data_start = array_data(line->chars);
        line_data_end = line_data + data_len;

        for (i = 0; i < data_len; i += 1) {
            if (strncmp(ys->current_search, line_data + i, search_len) == 0) {
                *row_out = r;
                *col_out = i + 1;
                return 1;
            }
        }

        r += 1;
    }

    return 0;
}
