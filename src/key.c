#include "internal.h"

static int esc_timout(char *seq) {
    if (read(0, seq, 1)     == 0)    { return ESC; }
    if (read(0, seq + 1, 1) == 0)    { return ESC; }

    return 0;
}

static int esc_sequence(char *seq) {
    if (seq[0] == '[') { /* ESC [ sequences. */
        if (seq[1] >= '0' && seq[1] <= '9') {
            /* Extended escape, read additional byte. */
            if (read(0, seq + 2, 1) == 0)    { return ESC; }
            if (seq[2] == '~') {
                switch (seq[1]) {
                    case '3':    { return DEL_KEY;   }
                    case '5':    { return PAGE_UP;   }
                    case '6':    { return PAGE_DOWN; }
                }
            }
        } else {
            switch (seq[1]) {
                case 'A':    { return ARROW_UP;    }
                case 'B':    { return ARROW_DOWN;  }
                case 'C':    { return ARROW_RIGHT; }
                case 'D':    { return ARROW_LEFT;  }
                case 'H':    { return HOME_KEY;    }
                case 'F':    { return END_KEY;     }
            }
        }
    } else if (seq[0] == 'O') { /* ESC O sequences. */
        switch (seq[1]) {
            case 'H':    { return HOME_KEY; }
            case 'F':    { return END_KEY;  }
        }
    }

    return 0;
}

static int yed_read_keys(int *input) {
    int  nread;
    char c, seq[3];


    while ((nread = read(0, &c, 1)) == 0);
    if (nread == -1)    { ERR("could not read key"); }

    switch (c) {
        case ESC:    /* escape sequence */
            /* If this is just an ESC, we'll timeout here. */
            if ((*input = esc_timout(seq)))      { return 1; }
            if ((*input = esc_sequence(seq)))    { return 1; }
            break;

        /* Here's an example of how we can do some multi-key bindings */
/*         case 'd': */
/*             if (read(0, seq, 1)) { */
/*                 if (seq[0] == 'd') { */
/*                     *input = FOOZLE; */
/*                     return 1; */
/*                 } else { */
/*                     *(input++) = 'd'; */
/*                     *input     = seq[0]; */
/*                     return 2; */
/*                 } */
/*             } */

        default:
            *input = c;
            return 1;
    }

    return 0;
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
            case 0:             return;
            case ARROW_UP:    yed_execute_command("cursor-up",        0, NULL); break;
            case ARROW_DOWN:  yed_execute_command("cursor-down",      0, NULL); break;
            case ARROW_RIGHT: yed_execute_command("cursor-right",     0, NULL); break;
            case ARROW_LEFT:  yed_execute_command("cursor-left",      0, NULL); break;
            case BACKSPACE:   yed_execute_command("delete-back",      0, NULL); break;
            case CTRL_F:      yed_execute_command("command-prompt",   0, NULL); break;
            case CTRL_L:      yed_execute_command("frame-next",       0, NULL); break;
            case FOOZLE:
            case CTRL_D:      yed_execute_command("delete-line",      0, NULL); break;
            case CTRL_W:      yed_execute_command("cursor-next-word", 0, NULL); break;
            default: {
                if (key == ENTER || key == TAB || !iscntrl(key)) {
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
