#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "internal.h"

typedef struct {
    uint32_t fg;
    uint32_t bg;
} yed_cell_attrs;

typedef struct {
    union {
        struct {
            char    c;
            uint8_t attr_idx;
        };
        uint16_t __data;
    };
} yed_cell;

typedef struct yed_line_t {
    array_t            cells;
    int                visual_width;
} yed_line;

typedef struct {
	const char     *path;
    bucket_array_t  lines;
} yed_buffer;

#define YED_NEW_CELL__DATA(c) (c)

yed_line yed_new_line(void);
yed_line yed_new_line_with_cap(int len);

yed_buffer yed_new_buff(void);
void yed_append_to_line(yed_line *line, char c);
void yed_append_to_buff(yed_buffer *buff, char c);

int yed_line_col_to_cell_idx(yed_line *line, int col);
yed_cell * yed_line_col_to_cell(yed_line *line, int col);

void yed_line_clear(yed_line *line);

yed_line * yed_buff_get_line(yed_buffer *buff, int row);
yed_line * yed_buff_insert_line(yed_buffer *buff, int row);
void yed_buff_delete_line(yed_buffer *buff, int row);
void yed_insert_into_line(yed_buffer *buff, int row, int col, char c);
void yed_delete_from_line(yed_buffer *buff, int row, int col);

void yed_fill_buff_from_file(yed_buffer *buff, const char *path);
void yed_write_buff_to_file(yed_buffer *buff, const char *path);

#endif
