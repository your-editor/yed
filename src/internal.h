#ifndef __INTERNAL_H__
#define __INTERNAL_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>

#include "tree.h"
#include "array.h"
#include "yed.h"
#include "term.h"
#include "key.h"
#include "buffer.h"
#include "frame.h"
#include "command.h"

#define inline
use_tree(yed_command_name_t, yed_command_t);
typedef yed_frame *yed_frame_ptr_t;
use_tree(yed_frame_id_t, yed_frame_ptr_t);
#undef inline


#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#define LIMIT(x, lower, upper) do { \
    if ((x) < (lower)) {            \
        (x) = (lower);              \
    } else if ((x) > (upper)) {     \
        (x) = (upper);              \
    }                               \
} while (0)


#define ERR ; /* @incomplete */


typedef struct {
    char *data;
    int   avail, used;
} output_stream;


#define MAX_BUFFERS (128)

typedef struct yed_state_t {
    output_stream   out_s;
    struct termios  sav_term;
    int             term_cols,
                    term_rows;
    int             cur_x,
                    cur_y,
                    save_cur_x,
                    save_cur_y;
    yed_buffer     *buff_list[MAX_BUFFERS];
    int             n_buffers;
    tree(yed_frame_id_t, yed_frame_ptr_t) frames;
    yed_frame      *active_frame;
    int             accepting_command;
    array_t         cmd_buff;
    int             cmd_cursor_x;
    char            command_buff[128];
    int             status;
    tree(yed_command_name_t, yed_command_t) commands;
} yed_state;

static yed_state *ys;

static void yed_init_output_stream(void);

static void yed_add_new_buff(void);

static void clear_output_buff(void);
static int output_buff_len(void);
static void append_n_to_output_buff(const char *s, int n);
static void append_to_output_buff(const char *s);
static void append_int_to_output_buff(int i);
static void flush_output_buff(void);

static void yed_service_reload(void);

static int s_to_i(const char *s);

#endif
