int yed_get_glyph_width(char *g) {
    if (*g == '\t') {
        return yed_get_tab_width();
    }

    /*
     * @todo
     * Determine width for control characters such as NULL ('\0')
     * and any unicode characters that occupy more than one column.
     */

    return 1;
}

int yed_get_glyph_len(char *g) {
    return 1;
}
