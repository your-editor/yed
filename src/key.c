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
