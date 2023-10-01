#ifndef __BUFFER_H__
#define __BUFFER_H__


typedef struct yed_line_t {
    array_t chars;
    int     visual_width;
    int     n_glyphs;
} yed_line;

#define RANGE_NORMAL  (0x1)
#define RANGE_LINE    (0x2)
#define RANGE_RECT    (0x3)
typedef struct {
    int kind;
    int locked;
    int anchor_row, anchor_col, cursor_row, cursor_col;
} yed_range;

#define BUFF_KIND_UNKNOWN         (0x0)
#define BUFF_KIND_FILE            (0x1)
#define BUFF_KIND_YANK            (0x2)
#define BUFF_KIND_LOG             (0x3)

#define BUFF_MODIFIED             (0x1)
#define BUFF_RD_ONLY              (0x2)
#define BUFF_YANK_LINES           (0x4)
#define BUFF_SPECIAL              (0x8)
#define BUFF_YANK_RECT            (0x10)
#define BUFF_NO_MOD_EVENTS        (0x20)

#define BUFF_FILL_STATUS_SUCCESS  (0)
#define BUFF_FILL_STATUS_ERR_NOF  (1)
#define BUFF_FILL_STATUS_ERR_DIR  (2)
#define BUFF_FILL_STATUS_ERR_PER  (3)
#define BUFF_FILL_STATUS_ERR_MAP  (4)
#define BUFF_FILL_STATUS_ERR_UNK  (5)

#define BUFF_WRITE_STATUS_SUCCESS (0)
#define BUFF_WRITE_STATUS_ERR_DIR (1)
#define BUFF_WRITE_STATUS_ERR_PER (2)
#define BUFF_WRITE_STATUS_ERR_UNK (3)

typedef struct yed_buffer_t {
    int               kind;
    int               flags;
    int               ft;
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
    char             *underlying_buff;
} yed_buffer;

void yed_init_buffers(void);

yed_line yed_new_line(void);
yed_line yed_new_line_with_cap(int len);
void yed_free_line(yed_line *line);

yed_line * yed_copy_line(yed_line *line);
void yed_line_add_glyph(yed_line *line, yed_glyph g, int idx);
void yed_line_append_glyph(yed_line *line, yed_glyph g);
void yed_line_delete_glyph(yed_line *line, int idx);
void yed_line_pop_glyph(yed_line *line);
void yed_clear_line(yed_line *line);

yed_buffer yed_new_buff(void);
yed_buffer * yed_create_buffer(char *name);
yed_buffer * yed_get_buffer(char *name);
yed_buffer * yed_get_or_create_special_rdonly_buffer(char *name);
yed_buffer * yed_get_buffer_by_path(char *path);
void yed_free_buffer(yed_buffer *buffer);
void yed_destroy_buffer(yed_buffer *buffer);


yed_buffer *yed_get_log_buffer(void);
yed_buffer *yed_get_yank_buffer(void);
yed_buffer *yed_get_bindings_buffer(void);
yed_buffer *yed_get_vars_buffer(void);


void yed_buffer_set_ft(yed_buffer *buffer, int ft);

int yed_line_idx_to_col(yed_line *line, int idx);
int yed_line_col_to_idx(yed_line *line, int col);
yed_line * yed_buff_get_line(yed_buffer *buff, int row);
yed_glyph * yed_line_col_to_glyph(yed_line *line, int col);
yed_glyph * yed_line_last_glyph(yed_line *line);
int yed_line_normalize_col(yed_line *line, int col);
yed_glyph * yed_buff_get_glyph(yed_buffer *buff, int row, int col);

void yed_buff_insert_string_no_undo(yed_buffer *buff, const char *str, int row, int col);
void yed_append_to_line_no_undo(yed_buffer *buff, int row, yed_glyph g);
void yed_pop_from_line_no_undo(yed_buffer *buff, int row);
void yed_line_clear_no_undo(yed_buffer *buff, int row);
int yed_buffer_add_line_no_undo(yed_buffer *buff);
void yed_buff_set_line_no_undo(yed_buffer *buff, int row, yed_line *line);
yed_line * yed_buff_insert_line_no_undo(yed_buffer *buff, int row);
void yed_buff_delete_line_no_undo(yed_buffer *buff, int row);
void yed_insert_into_line_no_undo(yed_buffer *buff, int row, int col, yed_glyph g);
void yed_delete_from_line_no_undo(yed_buffer *buff, int row, int col);
void yed_buff_clear_no_undo(yed_buffer *buff);
/*
 * The following functions are the interface by which everything
 * else should modify buffers.
 * This is meant to preserve undo/redo behavior.
 */
void yed_buff_insert_string(yed_buffer *buff, const char *str, int row, int col);
void yed_append_to_line(yed_buffer *buff, int row, yed_glyph g);
void yed_pop_from_line(yed_buffer *buff, int row);
void yed_line_clear(yed_buffer *buff, int row);
int yed_buffer_add_line(yed_buffer *buff);
void yed_buff_set_line(yed_buffer *buff, int row, yed_line *line);
yed_line * yed_buff_insert_line(yed_buffer *buff, int row);
void yed_buff_delete_line(yed_buffer *buff, int row);
void yed_insert_into_line(yed_buffer *buff, int row, int col, yed_glyph g);
void yed_delete_from_line(yed_buffer *buff, int row, int col);
void yed_buff_clear(yed_buffer *buff);


int yed_buff_n_lines(yed_buffer *buff);


int yed_fill_buff_from_file(yed_buffer *buff, char *path);
int yed_fill_buff_from_file_map(yed_buffer *buff, int fd, unsigned long long file_size);
int yed_fill_buff_from_file_stream(yed_buffer *buff, FILE *f);
int yed_fill_buff_from_string(yed_buffer *buff, const char *s, unsigned long long len);
int yed_write_buff_to_file(yed_buffer *buff, char *path);

void yed_range_sorted_points(yed_range *range, int *r1, int *c1, int *r2, int *c2);
int yed_is_in_range(yed_range *range, int row, int col);
void yed_buff_delete_selection(yed_buffer *buff);


int yed_buff_is_visible(yed_buffer *buff);

void yed_update_line_visual_widths(void);


#define yed_line_glyph_traverse(array, it)                                                  \
    for (it = (array).chars.data;                                                           \
         (void*)it < ((array).chars.data + ((array).chars.used * (array).chars.elem_size)); \
         it = ((void*)it) + yed_get_glyph_len(*it))

#define yed_line_glyph_traverse_from(array, it, starting_idx)                               \
    for (it = (array).chars.data + ((starting_idx) * (array).chars.elem_size);              \
         (void*)it < ((array).chars.data + ((array).chars.used * (array).chars.elem_size)); \
         it = ((void*)it) + yed_get_glyph_len(*it))

/*
 * NOTE: This is an O(n^2) traversal, so avoid if possible.
 */
#define yed_line_glyph_rtraverse(array, it)                                               \
    for (it = yed_line_last_glyph(&(array));                                              \
         (it) != NULL && (void*)it >= (array).chars.data;                                 \
         it = ((((void*)it) - (array).chars.data - 1) >= 0                                \
                 ? yed_line_col_to_glyph(&(array),                                        \
                     yed_line_idx_to_col(&(array), ((void*)it) - (array).chars.data - 1)) \
                 : NULL))

/* free() the results of these functions. May return NULL. */
char *yed_get_selection_text(yed_buffer *buffer);
char *yed_get_line_text(yed_buffer *buffer, int row);
char *yed_get_buffer_text(yed_buffer *buffer);


#endif
