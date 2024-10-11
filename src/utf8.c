void yed_get_string_info(const char *bytes, int len, int *n_glyphs, int *width) {
    const char *end;
    int         _n_glyphs, _width;
    yed_glyph  *g;

    end       = bytes + len;
    _n_glyphs = _width = 0;

    while (bytes < end) {
        g          = (yed_glyph*)bytes;
        _width    += yed_get_glyph_width(g);
        bytes     += yed_get_glyph_len(g);
        _n_glyphs += 1;
    }

    *n_glyphs = _n_glyphs;
    *width    = _width;
}

int yed_get_string_width(const char *s) {
    int len;
    int n_glyphs;
    int width;

    // if (*s == 0) { return 0; }

    len = strlen(s);

    yed_get_string_info((char*)s, len, &n_glyphs, &width);

    return width;
}
