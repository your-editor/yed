typedef union {
    char          c;
    unsigned char u_c;
    uint32_t      data;
    unsigned char bytes[4];
} yed_glyph;

#define G_IS_ASCII(g) (!((g).u_c >> 7))

#define G(c) ((yed_glyph){(c)})

int _yed_get_mbyte_width(yed_glyph g);

#define yed_get_glyph_width(g)          \
    (unlikely((g).c == '\t')            \
        ? ys->tabw                      \
        : (likely((g).u_c <= 127)       \
            ? 1                         \
            : _yed_get_mbyte_width(g))) \

static const unsigned char _utf8_lens[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
};

#define yed_get_glyph_len(g)                  \
    (likely(G_IS_ASCII(g))                    \
        ? 1                                   \
        : (int)(_utf8_lens[(g).u_c >> 3ULL]))

/* #define yed_get_glyph_len(g)               \ */
/*         (int)(_utf8_lens[(g).u_c >> 3ULL]) */

void yed_get_string_info(char *bytes, int len, int *n_glyphs, int *width);
