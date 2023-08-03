typedef union {
    char          c;
    unsigned char u_c;
    uint32_t      data;
    unsigned char bytes[4];
} yed_glyph;

#define G_IS_ASCII(g) (!((g).u_c >> 7))

#define G(c) ((yed_glyph){ .data = (c)})

/*
 * This is what the length table would look like:
 */
/* static const unsigned char _utf8_lens[] = { */
/*     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, */
/*     0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0 */
/* }; */

/*
 * But this is what we use instead because we might be
 * editing a binary. In that case, there might be byte
 * sequences that aren't valid UTF-8.
 */
static const unsigned char _utf8_lens[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 4, 1
};

#define yed_get_glyph_len(g)                  \
    (likely(G_IS_ASCII(g))                    \
        ? 1                                   \
        : (int)(_utf8_lens[(g).u_c >> 3ULL]))


__attribute__((always_inline))
static inline int _yed_get_mbyte_width(yed_glyph g) {
    int     len, w;
    wchar_t wch;

    len = yed_get_glyph_len(g);

    /* Reset the shift state. */
    mbtowc(NULL, 0, 0);

    mbtowc(&wch, (const char*)g.bytes, len);
    w = mk_wcwidth(wch);

    if (unlikely(w <= 0)) { return 1; }

    return w;
}

#define yed_get_glyph_width(g)          \
    (likely((g).u_c <= 127)             \
        ? (unlikely((g).c == '\t')      \
            ? (ys->tabw)                \
            : (likely(isprint((g).c))   \
                ? 1                     \
                : 2))                   \
        : (_yed_get_mbyte_width(g)))

void yed_get_string_info(const char *bytes, int len, int *n_glyphs, int *width);
int yed_get_string_width(const char *s);

#define yed_glyph_traverse(_s, _g)                                   \
    u64 _yed_glyph_traverse_len = strlen((_s));                      \
    for ((_g) = (yed_glyph*)(void*)(_s);                             \
         ((char*)(void*)(_g)) < ((_s) + _yed_glyph_traverse_len);    \
         (_g) = (yed_glyph*)(((char*)(_g)) + yed_get_glyph_len(*(_g))))

#define yed_glyph_traverse_n(_s, _n, _g)                             \
    for ((_g) = (yed_glyph*)(void*)(_s);                             \
         ((char*)(void*)(_g)) < ((_s) + (_n));                       \
         (_g) = (yed_glyph*)(((char*)(_g)) + yed_get_glyph_len(*(_g))))
