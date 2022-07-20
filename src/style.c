void yed_init_styles(void) {
    int   i, j;
    char *s;
    int   len;

    ys->scomp_strings = array_make(char*);

    s = strdup(""); array_push(ys->scomp_strings, s);
#define __SCOMP(comp) { s = strdup(#comp); array_push(ys->scomp_strings, s); }
    __STYLE_COMPONENTS
#undef __SCOMP

    for (i = 1; i < N_SCOMPS; i += 1) {
        s = *(char**)array_item(ys->scomp_strings, i);
        len = strlen(s);
        for (j = 0; j < len; j += 1) {
            if (s[j] == '_') {
                s[j] = '-';
            }
        }
    }

    ys->styles = tree_make(yed_style_name_t, yed_style_ptr_t);
    yed_set_default_styles();

}

static void rgb_to_hsv(int r, int g, int b,
                       float *h, float *s, float *v) {
    float max;
    float min;
    float R;
    float G;
    float B;

    R = (float)r / 255.0;
    G = (float)g / 255.0;
    B = (float)b / 255.0;

    max = MAX(r, MAX(g, b));
    min = MIN(r, MIN(g, b));

    *v = max / 255.0;
    *s = max == 0.0 ? 0.0 : 1.0 - (min / max);
    *h = atan2f((sqrtf(3.0) * (G - B)), (2 * R - G - B));
    if (*h < 0.0) { *h += 2.0 * M_PI; }
}

static void hsv_to_rgb(float h, float s, float v,
                       int  *r, int  *g, int  *b) {

    float R;
    float G;
    float B;
    float C;
    float X;
    float m;

    C = v * s;
    X = C * (1 - fabs(fmod(h/(M_PI/3.0), 2.0) - 1.0));
    m = v - C;

    if      (h >= 0.0              && h < (M_PI/3.0))       { R = C; G = X; B = 0; }
    else if (h >= (M_PI/3.0)       && h < (2.0 * M_PI/3.0)) { R = X; G = C; B = 0; }
    else if (h >= (2.0 * M_PI/3.0) && h < (M_PI))           { R = 0; G = C; B = X; }
    else if (h >= (M_PI/2.0)       && h < (4.0 * M_PI/3.0)) { R = 0; G = X; B = C; }
    else if (h >= (4.0 * M_PI/3.0) && h < (5.0 * M_PI/3.0)) { R = X; G = 0; B = C; }
    else if (h >= (5.0 * M_PI/3.0) && h < (2.0*M_PI))       { R = C; G = 0; B = X; }
    else                                                    { R =    G =    B = 0; }

    *r = (R + m) * 255;
    *g = (G + m) * 255;
    *b = (B + m) * 255;
}

static void fixup_missing(yed_style *style) {
    int sample_scomps[] = {
        STYLE_code_comment,
        STYLE_code_keyword,
        STYLE_code_control_flow,
        STYLE_code_typename,
        STYLE_code_preprocessor,
        STYLE_code_fn_call,
        STYLE_code_number,
        STYLE_code_constant,
        STYLE_code_field,
        STYLE_code_variable,
        STYLE_code_string,
        STYLE_code_character,
        STYLE_code_escape,
    };
    int       N;
    int       n;
    float     avg_sat;
    float     avg_val;
    float     avg_off;
    yed_attrs attrs;
    int       i;
    int       r;
    int       g;
    int       b;
    float     h;
    float     s;
    float     v;
    int set_scomps[] = {
        STYLE_red,
        STYLE_orange,
        STYLE_yellow,
        STYLE_lime,
        STYLE_green,
        STYLE_turquoise,
        STYLE_cyan,
        STYLE_blue,
        STYLE_purple,
        STYLE_magenta,
        STYLE_pink,
    };

    N       = sizeof(sample_scomps) / sizeof(int);
    n       = 0;
    avg_sat = 0.0;
    avg_val = 0.0;
    avg_off = 0.0;

    if (style->good.flags == 0) {
        if (style->active.flags & ATTR_16) {
            style->good.flags = ATTR_16;
            style->good.fg    = ATTR_16_GREEN;
        } else if (style->active.flags & ATTR_256) {
            style->good.flags = ATTR_256;
            style->good.fg    = 2;
        } else if (style->active.flags & ATTR_RGB) {
            style->good.flags = ATTR_RGB;
            style->good.fg    = RGB_32(0, 127, 0);
        }
    }

    if (style->bad.flags == 0) {
        if (style->active.flags & ATTR_16) {
            style->bad.flags = ATTR_16;
            style->bad.fg    = ATTR_16_RED;
        } else if (style->active.flags & ATTR_256) {
            style->bad.flags = ATTR_256;
            style->bad.fg    = 1;
        } else if (style->active.flags & ATTR_RGB) {
            style->bad.flags = ATTR_RGB;
            style->bad.fg    = RGB_32(127, 0, 0);
        }
    }

    if (style->active.flags == 0 || style->active.flags & ATTR_16) {
        style->white.flags     = ATTR_16 | ATTR_16_LIGHT_FG;
        style->white.fg        = ATTR_16_GREY;
        style->grey.flags      = ATTR_16;
        style->grey.fg         = ATTR_16_GREY;
        style->black.flags     = ATTR_16;
        style->black.fg        = ATTR_16_BLACK;
        style->red.flags       = ATTR_16;
        style->red.fg          = ATTR_16_RED;
        style->orange.flags    = ATTR_16 | ATTR_16_LIGHT_FG;
        style->orange.fg       = ATTR_16_RED;
        style->yellow.flags    = ATTR_16;
        style->yellow.fg       = ATTR_16_YELLOW;
        style->lime.flags      = ATTR_16 | ATTR_16_LIGHT_FG;
        style->lime.fg         = ATTR_16_YELLOW;
        style->green.flags     = ATTR_16;
        style->green.fg        = ATTR_16_GREEN;
        style->turquoise.flags = ATTR_16 | ATTR_16_LIGHT_FG;
        style->turquoise.fg    = ATTR_16_GREEN;
        style->cyan.flags      = ATTR_16;
        style->cyan.fg         = ATTR_16_CYAN;
        style->blue.flags      = ATTR_16;
        style->blue.fg         = ATTR_16_BLUE;
        style->purple.flags    = ATTR_16 | ATTR_16_LIGHT_FG;
        style->purple.fg       = ATTR_16_BLUE;
        style->magenta.flags   = ATTR_16;
        style->magenta.fg      = ATTR_16_MAGENTA;
        style->pink.flags      = ATTR_16 | ATTR_16_LIGHT_FG;
        style->pink.fg         = ATTR_16_MAGENTA;

        return;
    } else if (style->active.flags & ATTR_256) {
        style->white.flags     = ATTR_256;
        style->white.fg        = 231;
        style->grey.flags      = ATTR_256;
        style->grey.fg         = 244;
        style->black.flags     = ATTR_256;
        style->black.fg        = 16;
        style->red.flags       = ATTR_256;
        style->red.fg          = 196;
        style->orange.flags    = ATTR_256;
        style->orange.fg       = 214;
        style->yellow.flags    = ATTR_256;
        style->yellow.fg       = 226;
        style->lime.flags      = ATTR_256;
        style->lime.fg         = 154;
        style->green.flags     = ATTR_256;
        style->green.fg        = 46;
        style->turquoise.flags = ATTR_256;
        style->turquoise.fg    = 43;
        style->cyan.flags      = ATTR_256;
        style->cyan.fg         = 51;
        style->blue.flags      = ATTR_256;
        style->blue.fg         = 21;
        style->purple.flags    = ATTR_256;
        style->purple.fg       = 93;
        style->magenta.flags   = ATTR_256;
        style->magenta.fg      = 127;
        style->pink.flags      = ATTR_256;
        style->pink.fg         = 169;

        return;
    }

    for (i = 0; i < N; i += 1) {
        attrs = yed_get_style_scomp(style, sample_scomps[i]);
        if (attrs.flags == 0 || !(attrs.flags & ATTR_RGB)) { continue; }

        if (   (sample_scomps[i] == STYLE_status_line
            ||  sample_scomps[i] == STYLE_popup
            ||  sample_scomps[i] == STYLE_popup_alt)
        && !(attrs.flags & ATTR_INVERSE)) {

            r = RGB_32_r(attrs.bg);
            g = RGB_32_g(attrs.bg);
            b = RGB_32_b(attrs.bg);
        } else {
            r = RGB_32_r(attrs.fg);
            g = RGB_32_g(attrs.fg);
            b = RGB_32_b(attrs.fg);
        }

        rgb_to_hsv(r, g, b, &h, &s, &v);

        avg_sat += s;
        avg_val += v;
        avg_off += fmod(h + (M_PI/6.0), (M_PI/3.0)) - (M_PI/6.0);

        n += 1;
    }

    avg_sat /= (float)n;
    avg_val /= (float)n;
    avg_off /= (float)n;

    N = sizeof(set_scomps) / sizeof(int);

    for (i = 0; i < N; i += 1) {
        if (yed_get_style_scomp(style, set_scomps[i]).flags != 0) { continue; }

        h = i * (M_PI/6.0) + avg_off;
        if (h < 0.0) { h += 2.0 * M_PI; }

        hsv_to_rgb(h, avg_sat, avg_val,
                   &r, &g, &b);

        switch (set_scomps[i]) {
            #define __SCOMP(comp)                        \
                case STYLE_##comp:                       \
                    style->comp.flags = ATTR_RGB;        \
                    style->comp.fg    = RGB_32(r, g, b); \
                    break;
            __STYLE_COMPONENTS
            #undef __SCOMP
        }
    }

    if (style->white.flags == 0) {
        style->white.flags = ATTR_RGB | ATTR_BOLD;
        style->white.fg    = RGB_32(255, 255, 255);
    }
    if (style->grey.flags == 0) {
        style->grey.flags = ATTR_RGB | ATTR_BOLD;
        style->grey.fg    = RGB_32(127, 127, 127);
    }
    if (style->black.flags == 0) {
        style->black.flags = ATTR_RGB | ATTR_BOLD;
        style->black.fg    = RGB_32(1, 1, 1);
    }
}

void yed_set_style(const char *name, yed_style *style) {
    tree_it(yed_style_name_t,
            yed_style_ptr_t)   it;
    yed_style                 *new_style,
                              *old_style;

    new_style  = malloc(sizeof(*new_style));
    memcpy(new_style, style, sizeof(*new_style));

    fixup_missing(new_style);

    it = tree_lookup(ys->styles, (char*)name);
    if (tree_it_good(it)) {
        old_style        = tree_it_val(it);
        new_style->_name = old_style->_name;
        tree_insert(ys->styles, new_style->_name, new_style);
        free(old_style);
    } else {
        new_style->_name = strdup(name);
        tree_insert(ys->styles, new_style->_name, new_style);
    }
}

void yed_remove_style(const char *name) {
    tree_it(yed_style_name_t,
            yed_style_ptr_t)   it;
    yed_style                 *style;

    if (ys->active_style
    &&  strcmp(name, ys->active_style->_name) == 0) {
        free(ys->active_style);
        ys->active_style = NULL;
    }

    it = tree_lookup(ys->styles, (char*)name);
    if (tree_it_good(it)) {
        style = tree_it_val(it);
        tree_delete(ys->styles, (char*)name);
        free(style->_name);
        free(style);
    }
}

yed_style * yed_get_style(const char *name) {
    tree_it(yed_style_name_t,
            yed_style_ptr_t)   it;

    it = tree_lookup(ys->styles, (char*)name);

    if (!tree_it_good(it)) {
        return NULL;
    }

    return tree_it_val(it);
}

int yed_activate_style(const char *name) {
    yed_style *style;
    yed_event  event;

    if (name) {
        style = yed_get_style(name);

        if (!style) {
            return 0;
        }

        if (ys->active_style) {
            free(ys->active_style);
            ys->active_style = NULL;
        }

        ys->active_style = malloc(sizeof(*ys->active_style));
        memcpy(ys->active_style, style, sizeof(*style));
    } else {
        if (ys->active_style) {
            free(ys->active_style);
            ys->active_style = NULL;
        }
    }

    memset(&event, 0, sizeof(event));
    event.kind = EVENT_STYLE_CHANGE;
    yed_trigger_event(&event);

    return 1;
}

yed_style * yed_get_active_style(void) {
    return ys->active_style;
}

#define __SCOMP(comp)                           \
yed_attrs yed_active_style_get_##comp(void) {   \
    yed_style *style;                           \
                                                \
    style = yed_get_active_style();             \
    if (!style) {                               \
        return ZERO_ATTR;                       \
    }                                           \
                                                \
    return style->comp;                         \
}

__STYLE_COMPONENTS

#undef __SCOMP

yed_attrs yed_get_style_scomp(yed_style *style, int scomp) {
    if (!style) { goto fail; }

    switch (scomp) {
        #define __SCOMP(comp) case STYLE_##comp: return style->comp;
        __STYLE_COMPONENTS
        #undef __SCOMP
    }

fail:
    return ZERO_ATTR;
}

yed_attrs yed_get_active_style_scomp(int scomp) {
    return yed_get_style_scomp(yed_get_active_style(), scomp);
}

int yed_scomp_nr_by_name(const char *name) {
    int    i;
    char **it;

    if (name == NULL) { return -1; }

    i = 0;
    array_traverse(ys->scomp_strings, it) {
        if (strcmp(*it, name) == 0) {
            return i;
        }
        i += 1;
    }

    return -1;
}

void yed_set_default_styles(void) {
    yed_style s;

    /* style default */
    memset(&s, 0, sizeof(s));

    s.active.flags        = ATTR_16 | ATTR_16_LIGHT_FG;
    s.active.fg           = ATTR_16_GREY;
    s.active.bg           = ATTR_16_BLACK;

    s.inactive            = s.active;
/*     s.inactive.flags      = ATTR_16 | ATTR_16_LIGHT_BG; */
/*     s.inactive.fg         = ATTR_16_BLACK; */
/*     s.inactive.bg         = ATTR_16_BLACK; */

    s.active_border.flags = ATTR_16;
    s.active_border.fg    = ATTR_16_BLUE;
    s.active_border.bg    = ATTR_16_BLACK;

    s.inactive_border     = s.inactive;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.cursor_line         = s.active;
/*     s.cursor_line.flags     = ATTR_16 | ATTR_16_LIGHT_FG | ATTR_16_LIGHT_BG; */
/*     s.cursor_line.fg        = ATTR_16_GREY; */
/*     s.cursor_line.bg        = ATTR_16_BLACK; */

    s.selection.flags     = ATTR_16 | ATTR_16_LIGHT_FG | ATTR_16_LIGHT_BG;
    s.selection.fg        = ATTR_16_GREY;
    s.selection.bg        = ATTR_16_BLACK;

    s.search.flags        = ATTR_16;
    s.search.fg           = ATTR_16_BLACK;
    s.search.bg           = ATTR_16_YELLOW;

    s.search_cursor.flags = ATTR_16;
    s.search_cursor.fg    = ATTR_16_BLACK;
    s.search_cursor.bg    = ATTR_16_MAGENTA;

    s.attention.flags     = ATTR_16;
    s.attention.fg        = ATTR_16_RED;

    s.associate.flags     = ATTR_16 | ATTR_16_LIGHT_FG | ATTR_16_LIGHT_BG;
    s.associate.fg        = ATTR_16_BLUE;
    s.associate.bg        = ATTR_16_BLACK;

    s.command_line        = s.active;

    s.status_line.flags   = ATTR_16 | ATTR_BOLD;
    s.status_line.fg      = ATTR_16_BLACK;
    s.status_line.bg      = ATTR_16_BLUE;


    s.code_comment.flags  = ATTR_16 | ATTR_16_LIGHT_FG | ATTR_BOLD;
    s.code_comment.fg     = ATTR_16_YELLOW;

    s.code_keyword.flags  = ATTR_16 | ATTR_BOLD;
    s.code_keyword.fg     = ATTR_16_RED;

    s.code_control_flow   =
    s.code_typename       = s.code_keyword;

    s.code_preprocessor   = s.code_keyword;

    s.code_fn_call.flags  = ATTR_16;
    s.code_fn_call.fg     = ATTR_16_MAGENTA;

    s.code_number.flags   = ATTR_16 | ATTR_16_LIGHT_FG;
    s.code_number.fg      = ATTR_16_CYAN;

    s.code_constant       = s.code_number;

    s.code_string.flags   = ATTR_16;
    s.code_string.fg      = ATTR_16_GREEN;

    s.code_character      = s.code_string;

    yed_set_style("default", &s);

    yed_activate_style("default");
}
