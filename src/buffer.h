#ifndef __BUFFER_H__
#define __BUFFER_H__


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
#define BUFF_KIND_CMD   (0x3)

#define BUFF_MODIFIED   (0x1)
#define BUFF_RD_ONLY    (0x2)
#define BUFF_YANK_LINES (0x4)
#define BUFF_SPECIAL    (0x8)

typedef struct yed_buffer_t {
    int                   kind;
    int                   flags;
    yed_file              file;
    char                 *name;
    char                 *path;
    bucket_array_t        lines;
    yed_line             *get_line_cache;
    int                   get_line_cache_row;
    int                   has_selection;
    yed_range             selection;
    yed_undo_history      undo_history;
    int                   last_cursor_row,
                          last_cursor_col;
} yed_buffer;

void yed_init_buffers(void);

yed_line yed_new_line(void);
yed_line yed_new_line_with_cap(int len);
void yed_free_line(yed_line *line);

yed_line * yed_copy_line(yed_line *line);

yed_buffer yed_new_buff(void);
yed_buffer * yed_create_buffer(char *name);
yed_buffer * yed_get_buffer(char *name);
void yed_free_buffer(yed_buffer *buffer);

int yed_line_idx_to_col(yed_line *line, int idx);
int yed_line_col_to_idx(yed_line *line, int col);
char yed_line_col_to_char(yed_line *line, int col);
yed_line * yed_buff_get_line(yed_buffer *buff, int row);
char *yed_get_glyph(yed_buffer *buff, int row, int col);

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
