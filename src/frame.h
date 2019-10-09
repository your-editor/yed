#ifndef __FRAME_H__
#define __FRAME_H__

#include "internal.h"

typedef char *yed_frame_id_t;

typedef struct yed_frame_t {
    yed_frame_id_t      id;
    yed_buffer         *buffer;
    int                 top,
                        left,
                        height,
                        width;
    int                 cursor_line,
                        cursor_col,
                        dirty_line,
                        buffer_y_offset,
                        buffer_x_offset;
    int                 cur_x,
                        cur_y;
    int                 desired_x;
    int                 dirty;
    int                 scroll_off;
} yed_frame;

void yed_init_frames(void);
yed_frame * yed_add_new_frame(yed_frame_id_t id, int top, int left, int height, int width);
yed_frame * yed_get_frame(yed_frame_id_t id);
yed_frame * yed_get_or_add_frame(yed_frame_id_t id);
yed_frame * yed_new_frame(yed_frame_id_t id, int top, int left, int height, int width);
void yed_activate_frame(yed_frame *frame);
void yed_clear_frame(yed_frame *frame);
void yed_frame_draw_buff(yed_frame *frame, yed_buffer *buff, int y_offset, int x_offset);
void yed_frame_set_pos(yed_frame *frame, int top, int left);
void yed_frame_set_buff(yed_frame *frame, yed_buffer *buff);
void yed_frame_update(yed_frame *frame);
void yed_move_cursor_within_frame(yed_frame *f, int col, int row);
void yed_move_cursor_within_active_frame(int col, int row);
void yed_set_cursor_within_frame(yed_frame *frame, int dst_x, int dst_y);
void yed_set_cursor_far_within_frame(yed_frame *frame, int dst_x, int dst_y);
void yed_frame_reset_cursor(yed_frame *frame);
void yed_update_frames(void);
void yed_frame_update_dirty_line(yed_frame *frame);
void yed_frame_update_cursor_line(yed_frame *frame);
void yed_mark_dirty_frames(yed_buffer *dirty_buff);
void yed_mark_dirty_frames_line(yed_buffer *dirty_buff, int row);

#endif
