#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "internal.h"

#define ATTR_NORMAL (0x1)
#define ATTR_256    (0x2)
#define ATTR_RGB    (0x4)

typedef struct {
    uint32_t flags;
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

#define RANGE_NORMAL  (0x1)
#define RANGE_LINE    (0x2)
typedef struct {
    int kind;
    int locked;
    int anchor_row, anchor_col, cursor_row, cursor_col;
} yed_range;

#define BUFF_KIND_FILE  (0x1)
#define BUFF_KIND_YANK  (0x2)
#define BUFF_KIND_UNDO  (0x3)

#define BUFF_MODIFIED   (0x1)
#define BUFF_RD_ONLY    (0x2)
#define BUFF_YANK_LINES (0x4)

typedef struct {
    int             kind;
    int             flags;
    yed_file       *file;
    const char     *path;
    bucket_array_t  lines;
    int             has_selection;
    yed_range       selection;
} yed_buffer;

#define YED_NEW_CELL__DATA(c) (c)

void yed_init_attrs(void);

void yed_set_attr(uint8_t attr_idx);

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

int yed_is_in_range(yed_range *range, int row, int col);
void yed_buff_delete_selection(yed_buffer *buff);

#endif
