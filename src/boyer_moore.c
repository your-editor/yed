#include "boyer_moore.h"

int yed_boyer_moore(char *text, int text_len, char *pattern, int pattern_len) {
    int bad_char_table[256];
    int i;
    int shift;
    int j;

    if (unlikely(pattern_len > text_len)) { return -1; }

    /* Set up the table. */
    for (i = 0; i < 256; i += 1) {
        bad_char_table[i] = -1;
    }
    for (i = 0; i < pattern_len; i += 1) {
        bad_char_table[(int)pattern[i]] = i;
    }

    shift = 0;
    while (shift <= text_len - pattern_len) {
        j = pattern_len - 1;

        /* March back from pattern suffix. */
        while (j >= 0 && pattern[j] == text[shift + j]) {
            j -= 1;
        }

        if (j < 0) {
            /* Found it. */
            return shift;
        }

        shift += MAX(1, j - bad_char_table[(int)text[shift + j]]);
    }

    return -1;
}
