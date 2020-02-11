#include <yed/plugin.h>

void align(int n_args, char **args);

int yed_plugin_boot(yed_plugin *self) {
    yed_plugin_set_command(self, "align", align);

    return 0;
}

void align(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;
    char       *pattern;
    int         pattern_len;
    int         save_col, r1, c1, r2, c2, row, col, max_col, i;
    char       *line_data;
    yed_line   *line;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument, a pattern string");
        return;
    }

    pattern     = args[0];
    pattern_len = strlen(pattern);

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    if (!buff->has_selection
    ||  buff->selection.anchor_row == buff->selection.cursor_row) {
        yed_append_text_to_cmd_buff("[!] multiple lines must be selected");
        return;
    }

    yed_range_sorted_points(&buff->selection, &r1, &c1, &r2, &c2);

    max_col = 0;

    for (row = r1; row <= r2; row += 1) {
        line = yed_buff_get_line(buff, row);
        array_zero_term(line->chars);

        for (col = 1; col <= line->visual_width; col += 1) {
            line_data = array_data(line->chars) + col - 1;
            if (strncmp(line_data, pattern, pattern_len) == 0) {
                max_col  = MAX(max_col, col);
                col     += pattern_len;
                break;
            }
        }
    }

    if (max_col) {
        save_col = frame->cursor_col;

        yed_start_undo_record(frame, buff);

        for (row = r1; row <= r2; row += 1) {
            line = yed_buff_get_line(buff, row);

            for (col = 1; col <= line->visual_width; col += 1) {
                line_data = array_data(line->chars) + col - 1;
                if (strncmp(line_data, pattern, pattern_len) == 0) {
                    for (i = 0; i < max_col - col; i += 1) {
                        yed_insert_into_line(buff, row, col, G(' '));
                    }
                    break;
                }
            }
        }

        yed_set_cursor_within_frame(frame, save_col, frame->cursor_line);
        yed_end_undo_record(frame, buff);
        frame->dirty = 1;
    }

    YEXE("select-off");
}
