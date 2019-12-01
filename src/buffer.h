#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "internal.h"

#define ATTR_NORMAL   (0x0)
#define ATTR_INVERSE  (0x1)
#define ATTR_BOLD     (0x2)
#define ATTR_16_LIGHT (0x2)
#define ATTR_16       (0x4)
#define ATTR_256      (0x8)
#define ATTR_RGB      (0x10)

#define ATTR_16_BLACK   (30)
#define ATTR_16_BLUE    (34)
#define ATTR_16_GREEN   (32)
#define ATTR_16_CYAN    (36)
#define ATTR_16_RED     (31)
#define ATTR_16_MAGENTA (35)
#define ATTR_16_YELLOW  (33)
#define ATTR_16_BROWN   (33)
#define ATTR_16_GREY    (37)
#define ATTR_16_WHITE   (37)

#define RGB_32(r, g, b) (((r) << 16) | ((g) << 8) | (b))
#define RGB_32_r(rgb)   ((rgb) >> 16)
#define RGB_32_g(rgb)   (((rgb) >> 8) & 0xFF)
#define RGB_32_b(rgb)   ((rgb) & 0xFF)
#define RGB_32_hex(x)   (0x##x)

int rgb_to_256(unsigned rgb);

typedef struct {
    uint32_t flags;
    uint32_t fg;
    uint32_t bg;
} yed_attrs;

#define ZERO_ATTR    ((yed_attrs){ 0, 0, 0 })

typedef struct __attribute__((packed)) yed_line_t {
    array_t chars;
    int     visual_width;
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

typedef struct yed_buffer_t {
    int               kind;
    int               flags;
    yed_file          file;
    char             *name;
    char             *path;
    bucket_array_t    lines;
    yed_line         *get_line_cache;
    int               get_line_cache_row;
    int               has_selection;
    yed_range         selection;
    yed_undo_history  undo_history;
    int               last_cursor_row,
                      last_cursor_col;
} yed_buffer;

void yed_init_buffers(void);

void yed_get_attr_str(yed_attrs attr, char *buff_p);
void yed_set_attr(yed_attrs attr);
int  yed_attrs_eq(yed_attrs attr1, yed_attrs attr2);

yed_line yed_new_line(void);
yed_line yed_new_line_with_cap(int len);
void yed_free_line(yed_line *line);

yed_line * yed_copy_line(yed_line *line);

yed_buffer yed_new_buff(void);
yed_buffer * yed_create_buffer(char *name);
yed_buffer * yed_get_buffer(char *name);
void yed_free_buffer(yed_buffer *buffer);

int yed_line_col_to_idx(yed_line *line, int col);
char yed_line_col_to_char(yed_line *line, int col);
yed_line * yed_buff_get_line(yed_buffer *buff, int row);


void yed_append_to_line_no_undo(yed_buffer *buff, int row, char c);
void yed_pop_from_line_no_undo(yed_buffer *buff, int row);
void yed_line_clear_no_undo(yed_buffer *buff, int row);
int yed_buffer_add_line_no_undo(yed_buffer *buff);
void yed_buff_set_line_no_undo(yed_buffer *buff, int row, yed_line *line);
yed_line * yed_buff_insert_line_no_undo(yed_buffer *buff, int row);
void yed_buff_delete_line_no_undo(yed_buffer *buff, int row);
void yed_insert_into_line_no_undo(yed_buffer *buff, int row, int col, char c);
void yed_delete_from_line_no_undo(yed_buffer *buff, int row, int col);
void yed_buff_clear_no_undo(yed_buffer *buff);
/*
 * The following functions are the interface by which everything
 * else should modify buffers.
 * This is meant to preserve undo/redo behavior.
 */
void yed_append_to_line(yed_buffer *buff, int row, char c);
void yed_pop_from_line(yed_buffer *buff, int row);
void yed_line_clear(yed_buffer *buff, int row);
int yed_buffer_add_line(yed_buffer *buff);
void yed_buff_set_line(yed_buffer *buff, int row, yed_line *line);
yed_line * yed_buff_insert_line(yed_buffer *buff, int row);
void yed_buff_delete_line(yed_buffer *buff, int row);
void yed_insert_into_line(yed_buffer *buff, int row, int col, char c);
void yed_delete_from_line(yed_buffer *buff, int row, int col);
void yed_buff_clear(yed_buffer *buff);


int yed_buff_n_lines(yed_buffer *buff);


int yed_fill_buff_from_file(yed_buffer *buff, char *path);
int yed_fill_buff_from_file_map(yed_buffer *buff, FILE *f);
int yed_fill_buff_from_file_stream(yed_buffer *buff, FILE *f);
void yed_write_buff_to_file(yed_buffer *buff, char *path);

void yed_range_sorted_points(yed_range *range, int *r1, int *c1, int *r2, int *c2);
int yed_is_in_range(yed_range *range, int row, int col);
void yed_buff_delete_selection(yed_buffer *buff);


int yed_buff_is_visible(yed_buffer *buff);

#endif
