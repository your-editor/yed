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

void yed_combine_attrs(yed_attrs *dst, yed_attrs *src) {
    if (!dst || !src)    { return; }

    if (src->fg) {
        dst->flags &= ~(ATTR_16_LIGHT_FG);
        dst->fg = src->fg;
    }
    if (src->bg) {
        dst->flags &= ~(ATTR_16_LIGHT_BG);
        dst->bg = src->bg;
    }

    dst->flags &= ~(ATTR_BOLD);
    dst->flags |= src->flags;
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
    int fr, fg, fb;
    int br, bg, bb;
    int f16, b16;

    *buff_p = 0;

    BUFFCATN(buff_p, "\e[0", 3);

    if (attr.flags & ATTR_BOLD) {
        BUFFCATN(buff_p, ";1", 2);
    }

    if (attr.flags & ATTR_UNDERLINE) {
        BUFFCATN(buff_p, ";4", 2);
    }

    if (attr.flags & ATTR_INVERSE) {
        BUFFCATN(buff_p, ";7", 2);
    }

    if (attr.flags & ATTR_16) {
        if (attr.fg || attr.bg) {
            BUFFCATN(buff_p, ";", 1);
        }

        f16 = attr.fg;
        b16 = attr.bg;

        if (f16 && b16) {
            if (attr.flags & ATTR_16_LIGHT_FG) {
                f16 += 60;
            }
            if (attr.flags & ATTR_16_LIGHT_BG) {
                b16 += 60;
            }
            BUFFCAT(buff_p, u8_to_s(10 + b16));
            BUFFCATN(buff_p, ";", 1);
            BUFFCAT(buff_p, u8_to_s(f16));
        } else if (f16) {
            if (attr.flags & ATTR_16_LIGHT_FG) {
                f16 += 60;
            }
            BUFFCAT(buff_p, u8_to_s(f16));
        } else if (b16) {
            if (attr.flags & ATTR_16_LIGHT_BG) {
                b16 += 60;
            }
            BUFFCAT(buff_p, u8_to_s(10 + b16));
        }
    } else if (attr.flags & ATTR_256) {
        LIMIT(attr.fg, 0, 255);
        LIMIT(attr.bg, 0, 255);

        if (attr.fg || attr.bg) {
            BUFFCATN(buff_p, ";", 1);
        }

        if (attr.fg && attr.bg) {
            BUFFCATN(buff_p, "38;5;", 5);
            BUFFCAT(buff_p, u8_to_s(attr.fg));
            BUFFCATN(buff_p, ";", 1);
            BUFFCATN(buff_p, "48;5;", 5);
            BUFFCAT(buff_p, u8_to_s(attr.bg));
        } else if (attr.fg) {
            BUFFCATN(buff_p, "38;5;", 5);
            BUFFCAT(buff_p, u8_to_s(attr.fg));
        } else if (attr.bg) {
            BUFFCATN(buff_p, "48;5;", 5);
            BUFFCAT(buff_p, u8_to_s(attr.bg));
        }
    } else if (attr.flags & ATTR_RGB) {
        if (attr.fg || attr.bg) {
            BUFFCATN(buff_p, ";", 1);
        }

        fr = RGB_32_r(attr.fg);
        fg = RGB_32_g(attr.fg);
        fb = RGB_32_b(attr.fg);
        br = RGB_32_r(attr.bg);
        bg = RGB_32_g(attr.bg);
        bb = RGB_32_b(attr.bg);

        if (attr.fg && attr.bg) {
            BUFFCATN(buff_p, "38;2;", 5);
            BUFFCAT(buff_p, u8_to_s(fr));
            BUFFCATN(buff_p, ";", 1);
            BUFFCAT(buff_p, u8_to_s(fg));
            BUFFCATN(buff_p, ";", 1);
            BUFFCAT(buff_p, u8_to_s(fb));
            BUFFCATN(buff_p, ";", 1);
            BUFFCATN(buff_p, "48;2;", 5);
            BUFFCAT(buff_p, u8_to_s(br));
            BUFFCATN(buff_p, ";", 1);
            BUFFCAT(buff_p, u8_to_s(bg));
            BUFFCATN(buff_p, ";", 1);
            BUFFCAT(buff_p, u8_to_s(bb));
        } else if (attr.fg) {
            BUFFCATN(buff_p, "38;2;", 5);
            BUFFCAT(buff_p, u8_to_s(fr));
            BUFFCATN(buff_p, ";", 1);
            BUFFCAT(buff_p, u8_to_s(fg));
            BUFFCATN(buff_p, ";", 1);
            BUFFCAT(buff_p, u8_to_s(fb));
        } else if (attr.bg) {
            BUFFCATN(buff_p, "48;2;", 5);
            BUFFCAT(buff_p, u8_to_s(br));
            BUFFCATN(buff_p, ";", 1);
            BUFFCAT(buff_p, u8_to_s(bg));
            BUFFCATN(buff_p, ";", 1);
            BUFFCAT(buff_p, u8_to_s(bb));
        }
    }

    BUFFCATN(buff_p, "m", 1);
    *buff_p = 0;
}

void yed_set_attr(yed_attrs attr) {
    char buff[128];

    yed_get_attr_str(attr, buff);

    append_to_output_buff(buff);
}

int yed_attrs_eq(yed_attrs attr1, yed_attrs attr2) {
    return    (attr1.fg    == attr2.fg)
           && (attr1.bg    == attr2.bg)
           && (attr1.flags == attr2.flags);
}

yed_attrs yed_parse_attrs(char *string) {
    yed_attrs  attrs;
    array_t    words;
    int        idx;
    char      *word;
    int        scomp;
    unsigned   color;
    char       rgb_str[9];

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
            scomp = yed_scomp_nr_by_name(word + 1);
            if (scomp != -1) {
                attrs = yed_get_active_style_scomp(scomp);
            }
        } else if (strcmp(word, "fg") == 0) {
            idx += 1;
            word = WORD_OR_NULL(idx);
            if (word == NULL) { goto out; }

            if (word[0] == '!') {
                if (sscanf(word + 1, "%u", &color)) {
                    attrs.flags |=  ATTR_16;
                    attrs.flags &= ~ATTR_256;
                    attrs.flags &= ~ATTR_RGB;
                    attrs.fg     = ATTR_16_BLACK + color;
                }
            } else if (word[0] == '@') {
                if (sscanf(word + 1, "%u", &color)) {
                    attrs.flags &= ~ATTR_16;
                    attrs.flags |=  ATTR_256;
                    attrs.flags &= ~ATTR_RGB;
                    attrs.fg     = color;
                }
            } else {
                snprintf(rgb_str, sizeof(rgb_str), "0x%s", word);
                if (sscanf(rgb_str, "%x", &color)) {
                    if (color != 0) {
                        attrs.flags &= ~ATTR_16;
                        attrs.flags &= ~ATTR_256;
                        attrs.flags |=  ATTR_RGB;
                    }
                    attrs.fg = color;
                }
            }
        } else if (strcmp(word, "bg") == 0) {
            idx += 1;
            word = WORD_OR_NULL(idx);
            if (word == NULL) { goto out; }

            if (word[0] == '!') {
                if (sscanf(word + 1, "%u", &color)) {
                    attrs.flags |=  ATTR_16;
                    attrs.flags &= ~ATTR_256;
                    attrs.flags &= ~ATTR_RGB;
                    attrs.bg     = ATTR_16_BLACK + color;
                }
            } else if (word[0] == '@') {
                if (sscanf(word + 1, "%u", &color)) {
                    attrs.flags &= ~ATTR_16;
                    attrs.flags |=  ATTR_256;
                    attrs.flags &= ~ATTR_RGB;
                    attrs.bg     = color;
                }
            } else {
                snprintf(rgb_str, sizeof(rgb_str), "0x%s", word);
                if (sscanf(rgb_str, "%x", &color)) {
                    if (color != 0) {
                        attrs.flags &= ~ATTR_16;
                        attrs.flags &= ~ATTR_256;
                        attrs.flags |=  ATTR_RGB;
                    }
                    attrs.bg = color;
                }
            }
        } else if (strcmp(word, "normal") == 0) {
            attrs.flags &= ~ATTR_INVERSE;
            attrs.flags &= ~ATTR_BOLD;
            attrs.flags &= ~ATTR_UNDERLINE;
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
        } else if (strcmp(word, "16-light-fg") == 0) {
            attrs.flags |= ATTR_16_LIGHT_FG;
        } else if (strcmp(word, "no-16-light-fg") == 0) {
            attrs.flags &= ~ATTR_16_LIGHT_FG;
        } else if (strcmp(word, "16-light-bg") == 0) {
            attrs.flags |= ATTR_16_LIGHT_BG;
        } else if (strcmp(word, "no-16-light-bg") == 0) {
            attrs.flags &= ~ATTR_16_LIGHT_BG;
        } else if (strcmp(word, "swap") == 0) {
            attrs.fg ^= attrs.bg;
            attrs.bg ^= attrs.fg;
            attrs.fg ^= attrs.bg;
        }

        idx += 1;
    }

out:;
    free_string_array(words);
    return attrs;
}
