#include "internal.h"

static int yed_read_key(void) {
    int  r;
	char c;
    int sav_x, sav_y;

    sav_x = ys->cur_x;
    sav_y = ys->cur_y;

	read(0, &c, 1);

	if (c == 27) {
		read(0, &c, 1);
		if (c == 91) {
			read(0, &c, 1);
			switch (c) {
				case 65: r = KEY_UP;    break;
				case 66: r = KEY_DOWN;  break;
				case 67: r = KEY_RIGHT; break;
				case 68: r = KEY_LEFT;  break;
                default: r = 0;
			}
		} else {
            r = 0;
        }
	} else {
        r = c;
    }

    yed_set_cursor(ys->term_cols - 5, ys->term_rows);
    append_n_to_output_buff("     ", 5);
    yed_set_cursor(ys->term_cols - 5, ys->term_rows);
    append_int_to_output_buff(r);
    yed_set_cursor(sav_x, sav_y);

	return r;
}

static void yed_take_key(int key) {
    char *key_str;
    char  key_str_buff[2];

    key_str_buff[0] = (char)key;
    key_str_buff[1] = 0;
    key_str         = key_str_buff;

    if (ys->accepting_command) {
        yed_execute_command("command-prompt", 1, &key_str);
    } else {
        switch (key) {
            case KEY_UP:        yed_execute_command("cursor-up",      0, NULL); break;
            case KEY_DOWN:      yed_execute_command("cursor-down",    0, NULL); break;
            case KEY_RIGHT:     yed_execute_command("cursor-right",   0, NULL); break;
            case KEY_LEFT:      yed_execute_command("cursor-left",    0, NULL); break;
            case KEY_BACKSPACE: yed_execute_command("delete-back",    0, NULL); break;
            case CTRL('f'):     yed_execute_command("command-prompt", 0, NULL); break;
            case CTRL('l'):     yed_execute_command("frame-next",     0, NULL); break;
            case CTRL('d'):     yed_execute_command("delete-line",    0, NULL); break;
            default: {
                if (key == '\n' || !iscntrl(key)) {
                    yed_execute_command("insert", 1, &key_str);
                }
            }
        }
    }

    if (ys->active_frame) {
        yed_set_cursor(ys->term_cols - 20, ys->term_rows);
        append_n_to_output_buff("                    ", 20);
        yed_set_cursor(ys->term_cols - 20, ys->term_rows);
        append_int_to_output_buff(ys->active_frame->cursor_line);
        append_to_output_buff(" :: ");
        append_int_to_output_buff(ys->active_frame->cursor_col);
    }
}
