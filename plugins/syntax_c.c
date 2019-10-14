#include "plugin.h"

void syntax_c_handler(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;
    yed_cell  *cell_it;
    int        col, old_col, i, j, match;
    char       c, buff[1024],
              *kwds[] = {
                  "unsigned",
                  "long",
                  "const",
                  "static",
                  "int",
                  "size_t",
                  "short",
                  "char",
                  "void",
                  "float",
                  "double",
                  "struct",
                  "enum",
                  "typedef",
                  "if",
                  "else",
                  "while",
                  "do",
                  "for",
                  "switch",
                  "case",
                  "default",
                  "break",
                  "continue",
                  "return",
                  "goto",
              };

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE) {
        return;
    }

    line = yed_buff_get_line(frame->buffer, event->row);
    col  = 1;

    while (col <= line->visual_width) {
        old_col  = col;
        buff[0]  = 0;
        cell_it  = yed_line_col_to_cell(line, col);
        c        = cell_it->c;

        if (isalnum(c) || c == '_') {
            while (col <= line->visual_width) {
                strncat(buff, &c, 1);

                col += 1;
                cell_it  = yed_line_col_to_cell(line, col);
                c        = cell_it->c;

                if (!isalnum(c) && c != '_') {
                    break;
                }
            }
        } else {
            while (col <= line->visual_width) {
                col += 1;
                cell_it  = yed_line_col_to_cell(line, col);
                c        = cell_it->c;

                if (isalnum(c) || c == '_' || isspace(c)) {
                    break;
                }
            }
        }

        if (isspace(c)) {
            while (col <= line->visual_width) {
                col += 1;
                cell_it  = yed_line_col_to_cell(line, col);
                c        = cell_it->c;

                if (!isspace(c)) {
                    break;
                }
            }
        }

        for (i = 0; i < sizeof(kwds) / sizeof(const char*); i += 1) {
            match = (strcmp(buff, kwds[i]) == 0);
            if (match) {
                break;
            }
        }
        for (j = 0; j < strlen(buff); j += 1) {
            cell_it = yed_line_col_to_cell(line, old_col + j);
            cell_it->attr_idx = match;
        }
    }
}

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    h.kind = EVENT_LINE_PRE_DRAW;
    h.fn   = syntax_c_handler;

    yed_plugin_add_event_handler(self, h);

    return 0;
}
