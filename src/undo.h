#ifndef __UNDO_H__
#define __UNDO_H__


#define UNDO_GLYPH_ADD   (1)
#define UNDO_GLYPH_PUSH  (2)
#define UNDO_GLYPH_DEL   (3)
#define UNDO_GLYPH_POP   (4)
#define UNDO_LINE_ADD    (6)
#define UNDO_LINE_DEL    (7)
#define UNDO_CONTENT     (11)
#define UNDO_DIFF        (12)

struct yed_line_t;

typedef struct {
    int       kind;
    int       col;
    int       row;
    yed_glyph g;
} yed_undo_action;

typedef struct {
    int start_cursor_row, start_cursor_col;
    int end_cursor_row,   end_cursor_col;
    array_t actions;
} yed_undo_record;

typedef struct {
    array_t          undo;
    array_t          redo;
    yed_undo_record *current_record;
} yed_undo_history;


struct yed_buffer_t;
struct yed_frame_t;

yed_undo_history yed_new_undo_history(void);
void yed_free_undo_history(yed_undo_history *history);
void yed_reset_undo_history(yed_undo_history *history);
int yed_num_undo_records(struct yed_buffer_t *buffer);
void yed_start_undo_record(struct yed_frame_t *frame, struct yed_buffer_t *buffer);
void yed_end_undo_record(struct yed_frame_t *frame, struct yed_buffer_t *buffer);
void yed_cancel_undo_record(struct yed_frame_t *frame, struct yed_buffer_t *buffer);
int yed_get_undo_num_records(struct yed_buffer_t *buffer);
void yed_merge_undo_records(struct yed_buffer_t *buffer);
int yed_push_undo_action(struct yed_buffer_t *buffer, yed_undo_action *action);
int yed_undo(struct yed_frame_t *frame, struct yed_buffer_t *buffer);
int yed_redo(struct yed_frame_t *frame, struct yed_buffer_t *buffer);

#endif
