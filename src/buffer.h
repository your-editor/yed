#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "internal.h"

typedef struct yed_line_t {
    array_t            chars;
    struct yed_line_t *wrap_next;
} yed_line;

typedef struct {
    array_t lines;
} yed_buffer;

static yed_buffer yed_new_buff(void);
static void yed_append_to_line(yed_line *line, char c);
static void yed_append_to_buff(yed_buffer *buff, char c);

static void yed_fill_buff_from_file(yed_buffer *buff, const char *path);

#endif
