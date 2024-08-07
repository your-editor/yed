int rgb_to_256(unsigned rgb) {
    int         r,  g,  b;
    int         ri, gi, bi;
    int         smaller,    bigger;
    int         small_dist, big_dist;
    static int  cutoff_norm[] = { 0x00, 0x5f, 0x87, 0xaf, 0xd7, 0xff };
    static int  cutoff_gs[]   = { 0x08, 0x12, 0x1c, 0x26, 0x30, 0x3a,
                                  0x44, 0x4e, 0x58, 0x62, 0x6c, 0x76,
                                  0x80, 0x8a, 0x94, 0x9e, 0xa8, 0xb2,
                                  0xbc, 0xc6, 0xd0, 0xda, 0xe4 };
    int        *cutoff, cutoff_len;

    r = RGB_32_r(rgb);
    g = RGB_32_g(rgb);
    b = RGB_32_b(rgb);

    if (r == b && b == g) {
        cutoff     = cutoff_gs;
        cutoff_len = sizeof(cutoff_gs) / sizeof(cutoff_gs[0]);
    } else {
        cutoff     = cutoff_norm;
        cutoff_len = sizeof(cutoff_norm) / sizeof(cutoff_norm[0]);
    }

    for (ri = 0; ri < cutoff_len - 1; ri += 1) {
        smaller = cutoff[ri];
        bigger  = cutoff[ri + 1];

        if (smaller <= r && r <= bigger) {
            small_dist = abs(smaller - r);
            big_dist   = abs(bigger  - r);

            if (big_dist <= small_dist) {
                ri = ri + 1;
            }

            break;
        }
    }

    /* Check for greyscale. */
    if (cutoff == cutoff_gs) {
        if (r < 5) { return 16; }
        if (r < cutoff[0]) {
            ri = 0;
        }
        return 232 + ri;
    }

    for (gi = 0; gi < cutoff_len - 1; gi += 1) {
        smaller = cutoff[gi];
        bigger  = cutoff[gi + 1];

        if (smaller <= g && g <= bigger) {
            small_dist = abs(smaller - g);
            big_dist   = abs(bigger  - g);

            if (big_dist <= small_dist) {
                gi = gi + 1;
            }

            break;
        }
    }

    for (bi = 0; bi < cutoff_len - 1; bi += 1) {
        smaller = cutoff[bi];
        bigger  = cutoff[bi + 1];

        if (smaller <= b && b <= bigger) {
            small_dist = abs(smaller - b);
            big_dist   = abs(bigger  - b);

            if (big_dist <= small_dist) {
                bi = bi + 1;
            }

            break;
        }
    }

    return 16 + (36 * ri) + (6 * gi) + (bi);
}

#define BUFFCATN(buff_p, str, n) \
do { memcpy((buff_p), (str), (n)); (buff_p) += (n); } while (0)

#define BUFFCAT(buff_p, str)                \
do {                                        \
    int __BUFFCAT_len;                      \
    __BUFFCAT_len = strlen(str);            \
    memcpy((buff_p), (str), __BUFFCAT_len); \
    (buff_p) += __BUFFCAT_len;              \
} while (0)




void yed_get_attr_str(yed_attrs attr, char *buff_p) {
    int c16;
    int r;
    int g;
    int b;

    *buff_p = 0;

    BUFFCATN(buff_p, "\e[", 2);

    if (attr.flags & ATTR_BOLD) {
        BUFFCATN(buff_p, ";1", 2);
    }

    if (attr.flags & ATTR_ITALIC) {
        BUFFCATN(buff_p, ";3", 2);
    }

    if (attr.flags & ATTR_UNDERLINE) {
        BUFFCATN(buff_p, ";4", 2);
    }

    if (attr.flags & ATTR_INVERSE) {
        BUFFCATN(buff_p, ";7", 2);
    }

    if (ATTR_FG_KIND(attr.flags) != ATTR_KIND_NONE) {
        BUFFCATN(buff_p, ";", 1);
        switch(ATTR_FG_KIND(attr.flags)) {
            case ATTR_KIND_16:
                c16 = attr.fg;
                if (attr.flags & ATTR_16_LIGHT_FG) { c16 += 60; }
                BUFFCAT(buff_p, u8_to_s(c16));
                break;
            case ATTR_KIND_256:
                LIMIT(attr.fg, 0, 255);
                BUFFCATN(buff_p, "38;5;", 5);
                BUFFCAT(buff_p, u8_to_s(attr.fg));
                break;
            case ATTR_KIND_RGB:
                r = RGB_32_r(attr.fg);
                g = RGB_32_g(attr.fg);
                b = RGB_32_b(attr.fg);
                BUFFCATN(buff_p, "38;2;", 5);
                BUFFCAT(buff_p, u8_to_s(r));
                BUFFCATN(buff_p, ";", 1);
                BUFFCAT(buff_p, u8_to_s(g));
                BUFFCATN(buff_p, ";", 1);
                BUFFCAT(buff_p, u8_to_s(b));
                break;
        }
    }

    if (ATTR_BG_KIND(attr.flags) != ATTR_KIND_NONE) {
        BUFFCATN(buff_p, ";", 1);
        switch(ATTR_BG_KIND(attr.flags)) {
            case ATTR_KIND_16:
                c16 = attr.bg + 10;
                if (attr.flags & ATTR_16_LIGHT_BG) { c16 += 60; }
                BUFFCAT(buff_p, u8_to_s(c16));
                break;
            case ATTR_KIND_256:
                LIMIT(attr.bg, 0, 255);
                BUFFCATN(buff_p, "48;5;", 5);
                BUFFCAT(buff_p, u8_to_s(attr.bg));
                break;
            case ATTR_KIND_RGB:
                r = RGB_32_r(attr.bg);
                g = RGB_32_g(attr.bg);
                b = RGB_32_b(attr.bg);
                BUFFCATN(buff_p, "48;2;", 5);
                BUFFCAT(buff_p, u8_to_s(r));
                BUFFCATN(buff_p, ";", 1);
                BUFFCAT(buff_p, u8_to_s(g));
                BUFFCATN(buff_p, ";", 1);
                BUFFCAT(buff_p, u8_to_s(b));
                break;
        }
    }

    BUFFCATN(buff_p, "m", 1);
    *buff_p = 0;
}

int yed_attrs_eq(yed_attrs attr1, yed_attrs attr2) {
    return ATTRS_EQ(attr1, attr2);
}

yed_attrs yed_parse_attrs(const char *string) {
    yed_attrs  attrs;
    array_t    words;
    int        idx;
    char      *word;
    int        scomp;
    yed_attrs  ref_attrs;
    char      *field_start;
    unsigned   color;
    char       rgb_str[9];
    int        tmp;

#define WORD_OR_NULL(_idx) \
    (((_idx) >= array_len(words)) ? NULL : *(char**)array_item(words, (_idx)))

    memset(&attrs, 0, sizeof(attrs));

    words = sh_split(string);

    if (array_len(words) == 0) { goto out; }

    idx = 0;

    while (idx < array_len(words)) {
        word = WORD_OR_NULL(idx);
        if (word == NULL) { goto out; }

        if (word[0] == '&') {
            field_start = word;
            while (*field_start && *field_start != '.') { field_start += 1; }
            if (*field_start) {
                *field_start  = 0;
                field_start  += 1;
            } else {
                field_start = NULL;
            }
            scomp = yed_scomp_nr_by_name(word + 1);
            if (scomp != -1) {
                ref_attrs = yed_get_active_style_scomp(scomp);
                if (field_start == NULL) {
                    yed_combine_attrs(&attrs, &ref_attrs);
                } else {
                    if (strcmp(field_start, "fg") == 0) {
                        ATTR_SET_FG_KIND(attrs.flags, ATTR_FG_KIND(ref_attrs.flags));
                        attrs.fg = ref_attrs.fg;
                    } else if (strcmp(field_start, "bg") == 0) {
                        ATTR_SET_BG_KIND(attrs.flags, ATTR_BG_KIND(ref_attrs.flags));
                        attrs.bg = ref_attrs.bg;
                    } else if (strcmp(word, "inverse") == 0) {
                        attrs.flags &= ~(ATTR_INVERSE);
                        attrs.flags |= ref_attrs.flags & ATTR_INVERSE;
                    } else if (strcmp(word, "bold") == 0) {
                        attrs.flags &= ~(ATTR_BOLD);
                        attrs.flags |= ref_attrs.flags & ATTR_BOLD;
                    } else if (strcmp(word, "underline") == 0) {
                        attrs.flags &= ~(ATTR_UNDERLINE);
                        attrs.flags |= ref_attrs.flags & ATTR_UNDERLINE;
                    } else if (strcmp(word, "italic") == 0) {
                        attrs.flags &= ~(ATTR_ITALIC);
                        attrs.flags |= ref_attrs.flags & ATTR_ITALIC;
                    } else if (strcmp(word, "16-light-fg") == 0) {
                        attrs.flags &= ~(ATTR_16_LIGHT_FG);
                        attrs.flags |= ref_attrs.flags & ATTR_16_LIGHT_FG;
                    } else if (strcmp(word, "16-light-bg") == 0) {
                        attrs.flags &= ~(ATTR_16_LIGHT_BG);
                        attrs.flags |= ref_attrs.flags & ATTR_16_LIGHT_BG;
                    }
                }
            }
        } else if (strcmp(word, "fg") == 0) {
            idx += 1;
            word = WORD_OR_NULL(idx);
            if (word == NULL) { goto out; }

            if (word[0] == '!') {
                if (sscanf(word + 1, "%u", &color)) {
                    ATTR_SET_FG_KIND(attrs.flags, ATTR_KIND_16);
                    attrs.fg = ATTR_16_BLACK + color;
                }
            } else if (word[0] == '@') {
                if (sscanf(word + 1, "%u", &color)) {
                    ATTR_SET_FG_KIND(attrs.flags, ATTR_KIND_256);
                    attrs.fg = color;
                }
            } else {
                snprintf(rgb_str, sizeof(rgb_str), "0x%s", word);
                if (sscanf(rgb_str, "%x", &color)) {
                    ATTR_SET_FG_KIND(attrs.flags, ATTR_KIND_RGB);
                    attrs.fg = color;
                }
            }
        } else if (strcmp(word, "bg") == 0) {
            idx += 1;
            word = WORD_OR_NULL(idx);
            if (word == NULL) { goto out; }

            if (word[0] == '!') {
                if (sscanf(word + 1, "%u", &color)) {
                    ATTR_SET_BG_KIND(attrs.flags, ATTR_KIND_16);
                    attrs.bg = ATTR_16_BLACK + color;
                }
            } else if (word[0] == '@') {
                if (sscanf(word + 1, "%u", &color)) {
                    ATTR_SET_BG_KIND(attrs.flags, ATTR_KIND_256);
                    attrs.bg = color;
                }
            } else {
                snprintf(rgb_str, sizeof(rgb_str), "0x%s", word);
                if (sscanf(rgb_str, "%x", &color)) {
                    ATTR_SET_BG_KIND(attrs.flags, ATTR_KIND_RGB);
                    attrs.bg = color;
                }
            }
        } else if (strcmp(word, "normal") == 0) {
            attrs.flags &= ~ATTR_INVERSE;
            attrs.flags &= ~ATTR_BOLD;
            attrs.flags &= ~ATTR_UNDERLINE;
            attrs.flags &= ~ATTR_ITALIC;
            attrs.flags &= ~ATTR_16_LIGHT_FG;
            attrs.flags &= ~ATTR_16_LIGHT_BG;
        } else if (strcmp(word, "inverse") == 0) {
            attrs.flags |= ATTR_INVERSE;
        } else if (strcmp(word, "no-inverse") == 0) {
            attrs.flags &= ~ATTR_INVERSE;
        } else if (strcmp(word, "bold") == 0) {
            attrs.flags |= ATTR_BOLD;
        } else if (strcmp(word, "no-bold") == 0) {
            attrs.flags &= ~ATTR_BOLD;
        } else if (strcmp(word, "underline") == 0) {
            attrs.flags |= ATTR_UNDERLINE;
        } else if (strcmp(word, "no-underline") == 0) {
            attrs.flags &= ~ATTR_UNDERLINE;
        } else if (strcmp(word, "italic") == 0) {
            attrs.flags |= ATTR_ITALIC;
        } else if (strcmp(word, "no-italic") == 0) {
            attrs.flags &= ~ATTR_ITALIC;
        } else if (strcmp(word, "16-light-fg") == 0) {
            attrs.flags |= ATTR_16_LIGHT_FG;
        } else if (strcmp(word, "no-16-light-fg") == 0) {
            attrs.flags &= ~ATTR_16_LIGHT_FG;
        } else if (strcmp(word, "16-light-bg") == 0) {
            attrs.flags |= ATTR_16_LIGHT_BG;
        } else if (strcmp(word, "no-16-light-bg") == 0) {
            attrs.flags &= ~ATTR_16_LIGHT_BG;
        } else if (strcmp(word, "swap") == 0) {
            tmp = ATTR_FG_KIND(attrs.flags);
            ATTR_SET_FG_KIND(attrs.flags, ATTR_BG_KIND(attrs.flags));
            ATTR_SET_BG_KIND(attrs.flags, tmp);

            tmp = attrs.fg;
            attrs.fg = attrs.bg;
            attrs.bg = tmp;
        }

        idx += 1;
    }

out:;
    free_string_array(words);
    return attrs;
}
