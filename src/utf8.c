int yed_get_glyph_width(yed_glyph g) {
    if (g.c == '\t') { return ys->tabw; }

    /*
     * @todo
     * Determine width for control characters such as NULL ('\0')
     * and any unicode characters that occupy more than one column.
     */

    if (g.c <= 127) {
        return 1;
    }

    return mk_wcwidth(g.data);
}

/* Each of these masks detects characters of zero to four bytes */
#define UTF8_TWO_BYTE_VAL   0x03
#define UTF8_THREE_BYTE_VAL 0X07
#define UTF8_FOUR_BYTE_VAL  0x0F

int yed_get_glyph_len(yed_glyph g) {
    if (!(g.u_c >> 7)) {
        return 1;
    } else if ((g.u_c >> 4) == UTF8_FOUR_BYTE_VAL) {
        /* The byte looks like: 11110xxx */
        return 4;
    } else if ((g.u_c >> 5) == UTF8_THREE_BYTE_VAL) {
        /* The byte looks like: 1110xxxx */
        return 3;
    } else if ((g.u_c >> 6) == UTF8_TWO_BYTE_VAL) {
        /* The byte looks like: 110xxxxx */
        return 2;
    } else {
        ASSERT(0, "could not determine glyph length in bytes");
    }

    return 1;
}
