#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "internal.h"

typedef struct yed_line_t {
    array_t            chars;
    struct yed_line_t *wrap_next;
} yed_line;

typedef struct {
	const char *path;
    array_t     lines;
} yed_buffer;

static yed_buffer yed_new_buff(void);
static void yed_append_to_line(yed_line *line, char c);
static void yed_append_to_buff(yed_buffer *buff, char c);

static yed_line * yed_buff_get_line(yed_buffer *buff, int row);
static yed_line * yed_buff_insert_line(yed_buffer *buff, int row);
static void yed_buff_delete_line(yed_buffer *buff, int row);
static void yed_insert_into_line(yed_buffer *buff, yed_line *line, int col, char c);
static void yed_delete_from_line(yed_buffer *buff, yed_line *line, int col);

static void yed_fill_buff_from_file(yed_buffer *buff, const char *path);
static void yed_write_buff_to_file(yed_buffer *buff, const char *path);

static int yed_buff_get_line_number(yed_buffer *buff, yed_line *line);

#endif
