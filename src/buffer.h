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

static yed_cell yed_new_cell(char c);
static yed_line yed_new_line(void);

static yed_buffer yed_new_buff(void);
static void yed_append_to_line(yed_line *line, char c);
static void yed_append_to_buff(yed_buffer *buff, char c);

static int yed_line_col_to_cell_idx(yed_line *line, int col);
static yed_cell * yed_line_col_to_cell(yed_line *line, int col);

static void yed_line_clear(yed_line *line);

static yed_line * yed_buff_get_line(yed_buffer *buff, int row);
static yed_line * yed_buff_insert_line(yed_buffer *buff, int row);
static void yed_buff_delete_line(yed_buffer *buff, int row);
static void yed_insert_into_line(yed_buffer *buff, int row, int col, char c);
static void yed_delete_from_line(yed_buffer *buff, int row, int col);

static void yed_fill_buff_from_file(yed_buffer *buff, const char *path);
static void yed_write_buff_to_file(yed_buffer *buff, const char *path);

#endif
