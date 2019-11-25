#ifndef __INTERNAL_H__
#define __INTERNAL_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/mman.h>
#if defined(__linux__)
#include <linux/mman.h> /* linux mmap flags */
#endif
#include <sys/ioctl.h>
#include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <stdint.h>
#include <math.h>
#include <pthread.h>

#define _GNU_SOURCE
#include <dlfcn.h>
#include <unistd.h>
#include <libgen.h>



#include "tree.h"

#define inline
typedef struct yed_key_binding_t *yed_key_binding_ptr_t;
use_tree(int, yed_key_binding_ptr_t);

typedef char *yed_buffer_name_t;
typedef struct yed_buffer_t *yed_buffer_ptr_t;
use_tree(yed_buffer_name_t, yed_buffer_ptr_t);

typedef char *yed_command_name_t;
typedef void (*yed_command)(int, char**);
use_tree(yed_command_name_t, yed_command);

typedef char *yed_plugin_name_t;
struct yed_plugin_t;
typedef struct yed_plugin_t *yed_plugin_ptr_t;
use_tree(yed_plugin_name_t, yed_plugin_ptr_t);

typedef char *yed_var_name_t;
typedef char *yed_var_val_t;
use_tree(yed_var_name_t, yed_var_val_t);

typedef char *yed_style_name_t;
struct yed_style_t;
typedef struct yed_style_t *yed_style_ptr_t;
use_tree(yed_style_name_t, yed_style_ptr_t);
#undef inline

#include "array.h"
#include "bucket_array.h"
#include "yed.h"
#include "term.h"
#include "key.h"
#include "fs.h"
#include "undo.h"
#include "buffer.h"
#include "frame.h"
#include "command.h"
#include "getRSS.h"
#include "event.h"
#include "plugin.h"
#include "find.h"
#include "var.h"
#include "util.h"
#include "style.h"
#include "subproc.h"

#define likely(x)   (__builtin_expect(!!(x), 1))
#define unlikely(x) (__builtin_expect(!!(x), 0))

#ifdef YED_DO_ASSERTIONS
void yed_assert_fail(const char *msg, const char *fname, int line, const char *cond_str);
#define ASSERT(cond, msg)                            \
do { if (unlikely(!(cond))) {                        \
    yed_assert_fail(msg, __FILE__, __LINE__, #cond); \
} } while (0)
#else
#define ASSERT(cond, mst) ;
#endif

#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#define LIMIT(x, lower, upper) do { \
    if ((x) < (lower)) {            \
        (x) = (lower);              \
    } else if ((x) > (upper)) {     \
        (x) = (upper);              \
    }                               \
} while (0)

/* @incomplete */
#define ERR(msg) ASSERT(0, #msg)


#define UINT(w) uint##w##_t
#define SINT(w) int##w##_t

#define u8  UINT(8 )
#define u16 UINT(16)
#define u32 UINT(32)
#define u64 UINT(64)

#define i8  SINT(8 )
#define i16 SINT(16)
#define i32 SINT(32)
#define i64 SINT(64)

#define XOR_SWAP_64(a, b) do {   \
    a = ((u64)(a)) ^ ((u64)(b)); \
    b = ((u64)(b)) ^ ((u64)(a)); \
    a = ((u64)(a)) ^ ((u64)(b)); \
} while (0);

#define XOR_SWAP_PTR(a, b) do {           \
    a = (void*)(((u64)(a)) ^ ((u64)(b))); \
    b = (void*)(((u64)(b)) ^ ((u64)(a))); \
    a = (void*)(((u64)(a)) ^ ((u64)(b))); \
} while (0);

#define ALIGN(x, align)         ((__typeof(x))((((u64)(x)) + (((u64)align) - 1ULL)) & ~(((u64)align) - 1ULL)))
#define IS_ALIGNED(x, align)    (!(((u64)(x)) & (((u64)align) - 1ULL)))
#define IS_ALIGNED_PP(x, align) (!((x) & ((align) - 1ULL)))
#define IS_POWER_OF_TWO(x)      ((x) != 0 && IS_ALIGNED((x), (x)))
#define IS_POWER_OF_TWO_PP(x)   ((x) != 0 && IS_ALIGNED_PP((x), (x)))

#define LOG2_8BIT(v)  (8 - 90/(((v)/4+14)|1) - 2/((v)/2+1))
#define LOG2_16BIT(v) (8*((v)>255) + LOG2_8BIT((v) >>8*((v)>255)))
#define LOG2_32BIT(v)                                        \
    (16*((v)>65535L) + LOG2_16BIT((v)*1L >>16*((v)>65535L)))
#define LOG2_64BIT(v)                                        \
    (32*((v)/2L>>31 > 0)                                     \
     + LOG2_32BIT((v)*1L >>16*((v)/2L>>31 > 0)               \
                         >>16*((v)/2L>>31 > 0)))

uint64_t next_power_of_2(uint64_t x);

#define KiB(x) ((x) * 1024ULL)
#define MiB(x) ((x) * 1024ULL * KiB(1ULL))
#define GiB(x) ((x) * 1024ULL * MiB(1ULL))
#define TiB(x) ((x) * 1024ULL * GiB(1ULL))
char *pretty_bytes(uint64_t n_bytes);

typedef struct yed_state_t {
    yed_lib_t                   *yed_lib;
    array_t                      output_buffer;
    array_t                      writer_buffer;
    pthread_mutex_t              write_mtx, write_ready_mtx;
    pthread_cond_t               write_signal;
    pthread_t                    writer_id;
    int                          writer_done;
    char                         _4096_spaces[4096];
    struct termios               sav_term;
    int                          term_cols,
                                 term_rows;
    char                        * written_cells;
    int                          cur_x,
                                 cur_y,
                                 save_cur_x,
                                 save_cur_y;
    tree(yed_buffer_name_t,
         yed_buffer_ptr_t)       buffers;
    int                          unnamed_buff_counter;
    yed_buffer                  *yank_buff;
    array_t                      frames;
    yed_frame                   *active_frame,
                                *prev_active_frame;
    yed_command_name_t           interactive_command;
    char                        *cmd_prompt;
    char                        *current_search;
    char                        *save_search;
    int                          search_save_row,
                                 search_save_col;
    array_t                      replace_save_lines;
    array_t                      replace_working_lines;
    array_t                      replace_markers;
    int                          replace_count;
    array_t                      cmd_buff;
    int                          cmd_cursor_x;
    int                          status;
    int                          redraw;
    int                          redraw_cls;
    tree(yed_command_name_t,
         yed_command)            commands;
    tree(yed_command_name_t,
         yed_command)            default_commands;
    char                        *small_message;
    tree(yed_plugin_name_t,
         yed_plugin_ptr_t)       plugins;
    array_t                      plugin_dirs;
    yed_key_binding             *real_key_map[REAL_KEY_MAX];
    tree(int,
         yed_key_binding_ptr_t)  key_seq_map;
    array_t                      key_sequences;
    int                          seq_key_counter;
    array_t                      released_seq_keys;
    array_t                      event_handlers[N_EVENTS];
    tree(yed_var_name_t,
         yed_var_val_t)          vars;
    tree(yed_style_name_t,
         yed_style_ptr_t)        styles;
    yed_style_name_t             active_style;
} yed_state;

extern yed_state *ys;

void yed_init_output_stream(void);

void clear_output_buff(void);
int output_buff_len(void);
void append_n_to_output_buff(char *s, int n);
void append_to_output_buff(char *s);
void append_int_to_output_buff(int i);
void flush_output_buff(void);

void yed_set_small_message(char *msg);
void yed_write_status_bar(int key);

void yed_service_reload(void);

int s_to_i(const char *s);

const char * u8_to_s[] = {
"0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",   "8",   "9",   "10",   "11", "12",  "13",  "14",  "15",
"16",  "17",  "18",  "19",  "20",  "21",  "22",  "23",  "24",  "25",  "26",  "27",  "28",  "29",  "30",  "31",
"32",  "33",  "34",  "35",  "36",  "37",  "38",  "39",  "40",  "41",  "42",  "43",  "44",  "45",  "46",  "47",
"48",  "49",  "50",  "51",  "52",  "53",  "54",  "55",  "56",  "57",  "58",  "59",  "60",  "61",  "62",  "63",
"64",  "65",  "66",  "67",  "68",  "69",  "70",  "71",  "72",  "73",  "74",  "75",  "76",  "77",  "78",  "79",
"80",  "81",  "82",  "83",  "84",  "85",  "86",  "87",  "88",  "89",  "90",  "91",  "92",  "93",  "94",  "95",
"96",  "97",  "98",  "99",  "100", "101", "102", "103", "104", "105", "106", "107", "108", "109", "110", "111",
"112", "113", "114", "115", "116", "117", "118", "119", "120", "121", "122", "123", "124", "125", "126", "127",
"128", "129", "130", "131", "132", "133", "134", "135", "136", "137", "138", "139", "140", "141", "142", "143",
"144", "145", "146", "147", "148", "149", "150", "151", "152", "153", "154", "155", "156", "157", "158", "159",
"160", "161", "162", "163", "164", "165", "166", "167", "168", "169", "170", "171", "172", "173", "174", "175",
"176", "177", "178", "179", "180", "181", "182", "183", "184", "185", "186", "187", "188", "189", "190", "191",
"192", "193", "194", "195", "196", "197", "198", "199", "200", "201", "202", "203", "204", "205", "206", "207",
"208", "209", "210", "211", "212", "213", "214", "215", "216", "217", "218", "219", "220", "221", "222", "223",
"224", "225", "226", "227", "228", "229", "230", "231", "232", "233", "234", "235", "236", "237", "238", "239",
"240", "241", "242", "243", "244", "245", "246", "247", "248", "249", "250", "251", "252", "253", "254", "255"
};

#define YEXE(cmd_name, ...)                                  \
do {                                                         \
    char *__YEXE_args[] = { __VA_ARGS__ };                   \
    yed_execute_command((cmd_name),                          \
                        sizeof(__YEXE_args) / sizeof(char*), \
                        __YEXE_args);                        \
} while (0)

#define YBIND(key, cmd, ...)                          \
do {                                                  \
    char *__YEXE_args[] = { __VA_ARGS__ };            \
    yed_bind_key((key), (cmd),                        \
                 sizeof(__YEXE_args) / sizeof(char*), \
                 __YEXE_args);                        \
} while (0)

#define YPBIND(plugin, key, cmd, ...)                 \
do {                                                  \
    char *__YEXE_args[] = { __VA_ARGS__ };            \
    yed_plugin_bind_key((plugin), (key), (cmd),       \
                 sizeof(__YEXE_args) / sizeof(char*), \
                 __YEXE_args);                        \
} while (0)


#endif
