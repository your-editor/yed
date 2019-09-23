#include "internal.h"

static int yed_read_key(void) {
	char c;

	read(0, &c, 1);

	if (c == 27) {
		read(0, &c, 1);
		if (c == 91) {
			read(0, &c, 1);
			switch (c) {
				case 65: return KEY_UP;
				case 66: return KEY_DOWN;
				case 67: return KEY_RIGHT;
				case 68: return KEY_LEFT;
			}
		}
	}

	return c;
}
