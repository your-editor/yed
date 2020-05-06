void yed_init_styles(void) {
    ys->styles = tree_make_c(yed_style_name_t, yed_style_ptr_t, strcmp);
    yed_set_default_styles();
}

void yed_set_style(char *name, yed_style *style) {
    tree_it(yed_style_name_t,
            yed_style_ptr_t)   it;
    yed_style                 *new_style,
                              *old_style;

    new_style  = malloc(sizeof(*new_style));
    memcpy(new_style, style, sizeof(*new_style));

    it = tree_lookup(ys->styles, name);
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

void yed_remove_style(char *name) {
    tree_it(yed_style_name_t,
            yed_style_ptr_t)   it;
    yed_style                 *style;

    if (ys->active_style
    &&  strcmp(name, ys->active_style) == 0) {
        ys->active_style = NULL;
        ys->redraw       = 1;
    }

    it = tree_lookup(ys->styles, name);
    if (tree_it_good(it)) {
        style = tree_it_val(it);
        tree_delete(ys->styles, name);
        free(style->_name);
        free(style);
    }
}

yed_style * yed_get_style(char *name) {
    tree_it(yed_style_name_t,
            yed_style_ptr_t)   it;

    it = tree_lookup(ys->styles, name);

    if (!tree_it_good(it)) {
        return NULL;
    }

    return tree_it_val(it);
}

int yed_activate_style(char *name) {
    yed_style *style;

    if (name) {
        style = yed_get_style(name);

        if (!style) {
            return 0;
        }

        ys->active_style = style->_name;
    } else {
        ys->active_style = NULL;
    }

    ys->redraw = ys->redraw_cls = 1;

    return 1;
}

yed_style * yed_get_active_style(void) {
    if (!ys->active_style) {
        return NULL;
    }

    return yed_get_style(ys->active_style);
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


void yed_set_default_styles(void) {
    yed_style s;

    /* style default */
    memset(&s, 0, sizeof(s));

    s.active.flags        = ATTR_16;
    s.active.fg           = ATTR_16_GREY;
    s.active.bg           = ATTR_16_BLACK;

    s.inactive            = s.active;
/*     s.inactive.flags      = ATTR_16 | ATTR_16_LIGHT_BG; */
/*     s.inactive.fg         = ATTR_16_BLACK; */
/*     s.inactive.bg         = ATTR_16_BLACK; */

    s.active_border       = s.active;
    s.active_border.fg    = ATTR_16_BLUE;

    s.inactive_border     = s.inactive;

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
