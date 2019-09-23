#ifndef __FRAME_H__
#define __FRAME_H__

#include "internal.h"

typedef const char *yed_frame_id_t;

typedef struct yed_frame_t {
    yed_frame_id_t      id;
    yed_buffer         *buffer;
    int                 top,
                        left,
                        height,
                        width;
    int                 cursor_line,
                        buffer_offset;
    int                 cur_x,
                        cur_y;
    int                 desired_x;
    int                 dirty;
} yed_frame;

static void yed_init_frames(void);
static yed_frame * yed_add_new_frame(yed_frame_id_t id, int top, int left, int height, int width);
static yed_frame * yed_get_frame(yed_frame_id_t id);
static yed_frame * yed_get_or_add_frame(yed_frame_id_t id);
static yed_frame * yed_new_frame(yed_frame_id_t id, int top, int left, int height, int width);
static void yed_activate_frame(yed_frame *frame);
static void yed_clear_frame(yed_frame *frame);
static void yed_frame_draw_buff(yed_frame *frame, yed_buffer *buff, int offset);
static void yed_frame_set_pos(yed_frame *frame, int top, int left);
static void yed_frame_set_buff(yed_frame *frame, yed_buffer *buff);
static void yed_frame_update(yed_frame *frame);
static void yed_move_cursor_within_frame(yed_frame *f, int col, int row);
static void yed_update_frames(void);
static void yed_frame_update_cursor_line(yed_frame *frame);
static void yed_frame_take_key(yed_frame *frame, int key);


#endif
