#ifndef __FRAME_H__
#define __FRAME_H__


typedef struct yed_frame_t {
    struct yed_frame_t *v_link, *h_link;
    yed_buffer         *buffer;
    float               top_f,
                        left_f,
                        height_f,
                        width_f;
    int                 top,
                        left,
                        height,
                        width;
    int                 btop, bleft, bheight, bwidth;
    int                 cursor_line,
                        cursor_col,
                        last_cursor_line,
                        dirty_line,
                        buffer_y_offset,
                        buffer_x_offset;
    int                 cur_x,
                        cur_y;
    int                 desired_x;
    int                 dirty;
    int                 scroll_off;
    array_t             line_attrs;
} yed_frame;

void yed_init_frames(void);
yed_frame * yed_add_new_frame(float top_f, float left_f, float height_f, float width_f);
yed_frame * yed_add_new_frame_full(void);
yed_frame * yed_new_frame(float top_f, float left_f, float height_f, float width_f);
void yed_delete_frame(yed_frame *frame);
yed_frame * yed_vsplit_frame(yed_frame *frame);
yed_frame * yed_hsplit_frame(yed_frame *frame);
void yed_activate_frame(yed_frame *frame);
void yed_clear_frame(yed_frame *frame);
void yed_undraw_frame(yed_frame *frame);
void yed_frame_draw_buff(yed_frame *frame, yed_buffer *buff, int y_offset, int x_offset);
void yed_frame_set_pos(yed_frame *frame, float top_f, float left_f);
void yed_frame_set_buff(yed_frame *frame, yed_buffer *buff);
void yed_frame_update(yed_frame *frame);
void yed_move_cursor_within_frame(yed_frame *f, int col, int row);
void yed_move_cursor_within_active_frame(int col, int row);
void yed_set_cursor_within_frame(yed_frame *frame, int dst_x, int dst_y);
void yed_set_cursor_far_within_frame(yed_frame *frame, int dst_x, int dst_y);
void yed_frame_reset_cursor(yed_frame *frame);
void yed_frame_hard_reset_cursor_x(yed_frame *frame);
void yed_update_frames(void);
void yed_frame_update_dirty_line(yed_frame *frame);
void yed_frame_update_cursor_line(yed_frame *frame);
void yed_frames_remove_buffer(yed_buffer *buff);
void yed_mark_dirty_frames(yed_buffer *dirty_buff);
void yed_mark_dirty_frames_line(yed_buffer *dirty_buff, int row);

void frame_get_rect(yed_frame *frame, int *top,  int *left,  int *height,  int *width,
                                      int *btop, int *bleft, int *bheight, int *bwidth);

#define FRAME_RESET_RECT(f) frame_get_rect((f), &(f)->top,  &(f)->left,  &(f)->height,  &(f)->width,\
                                                &(f)->btop, &(f)->bleft, &(f)->bheight, &(f)->bwidth)

#endif
