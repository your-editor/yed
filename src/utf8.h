typedef union {
    char          c;
    unsigned char u_c;
    uint32_t      data;
    unsigned char bytes[4];
} yed_glyph;

#define G(c) ((yed_glyph){(c)})

int yed_get_glyph_width(yed_glyph g);
int yed_get_glyph_len(yed_glyph g);
