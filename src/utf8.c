#if 0
int yed_get_glyph_width(yed_glyph g) {
    /*
     * @todo
     * Determine width for control characters such as NULL ('\0')
     * and any unicode characters that occupy more than one column.
     */

    if (likely(g.u_c <= 127)) {
        return unlikely(g.c == '\t') ? ys->tabw : 1;
    }

    return mk_wcwidth(g.data);
}
#endif

/*
 * @bad @todo
 * I think this is _technically_ incorrect code.
 * The issue is that we're taking a pointer to arbitrary
 * bytes in a string and casting the pointer into a yed_glyph*.
 * Then we dereference it.
 * Imagine that our pointer is pointing at the last byte of
 * some memory region.
 * When we cast, we're allowed to touch (and may be implicitly
 * touching) 3 extra bytes (yed_glyph is 4 bytes).
 * This probably won't introduce any problems because we're only
 * ever explicitly reading the first byte of this memory, but if
 * the compiler decides that it needs to touch the other three bytes,
 * then we might segfault.
 *
 * I'm going to leave it for now so that it'll be fast, but just be
 * aware that this could induce crashes.
 *
 *                                   - Brandon Kammerdiener, Feb 2020
 *
 * UPDATE: A workaround for this issue is to make sure that the string
 * has at least 3 bytes of padding. This is implemented in
 * yed_fill_buff_from_file_map().
 *
 *                                   - Brandon Kammerdiener, Feb 2020
 */
void yed_get_string_info(char *bytes, int len, int *n_glyphs, int *width) {
    char      *end;
    int        _n_glyphs, _width;
    yed_glyph *g;

    end       = bytes + len;
    _n_glyphs = _width = 0;

    while (bytes < end) {
        g          = (yed_glyph*)bytes;
        _width    += yed_get_glyph_width(*g);
        bytes     += yed_get_glyph_len(*g);
        _n_glyphs += 1;
    }

    *n_glyphs = _n_glyphs;
    *width    = _width;
}
