typedef union {
    char          c;
    unsigned char u_c;
    uint32_t      data;
    unsigned char bytes[4];
} yed_glyph;

#define G(c) ((yed_glyph){(c)})

/* int yed_get_glyph_width(yed_glyph g); */

#define yed_get_glyph_width(g)       \
    (unlikely((g).c == '\t')         \
        ? ys->tabw                   \
        : (likely((g).u_c <= 127)    \
            ? 1                      \
            : mk_wcwidth((g).data)))

static const unsigned char utf8_lens[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
};

#define yed_get_glyph_len(g)                                      \
    (likely((g).u_c <= 127) ? 1 : (int)(utf8_lens[(g).u_c >> 3]))

void yed_get_string_info(char *bytes, int len, int *n_glyphs, int *width);
