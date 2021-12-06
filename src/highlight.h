#define HL_KEY     (1)
#define HL_CON     (2)
#define HL_PP      (3)
#define HL_CALL    (4)
#define HL_NUM     (5)
#define HL_STR     (6)
#define HL_CHAR    (7)
#define HL_COMMENT (8)
#define HL_ATTN    (9)
#define HL_CF      (10)
#define HL_TYPE    (11)
#define HL_ASSOC   (12)

#define HL_IGNORE  (100)

typedef struct {
    yed_attrs key,
              con,
              num,
              pp,
              cal,
              st,
              ch,
              com,
              atn,
              cf,
              ty,
              ass;
} attrs_set;

static inline void get_attrs(attrs_set *set) {
    set->key = yed_active_style_get_code_keyword();
    set->con = yed_active_style_get_code_constant();
    set->num = yed_active_style_get_code_number();
    set->pp  = yed_active_style_get_code_preprocessor();
    set->cal = yed_active_style_get_code_fn_call();
    set->st  = yed_active_style_get_code_string();
    set->ch  = yed_active_style_get_code_character();
    set->com = yed_active_style_get_code_comment();
    set->atn = yed_active_style_get_attention();
    set->cf  = yed_active_style_get_code_control_flow();
    set->ty  = yed_active_style_get_code_typename();
    set->ass = yed_active_style_get_associate();
}

typedef struct {
    char *kwd;
    int   kind;
    char  prefix;
} highlight_kwd;

typedef struct {
    array_t kwds_by_len;
} highlight_kwd_set;

#include <yed/tree.h>
use_tree(char, highlight_kwd_set);


#include <yed/internal.h>


static inline void _highlight_kwd_set_make(highlight_kwd_set *set) {
    set->kwds_by_len = array_make(array_t);
}

static inline void _highlight_kwd_set_free(highlight_kwd_set *set) {
    array_t       *kwd_list_it;
    highlight_kwd *kwd_it;

    array_traverse(set->kwds_by_len, kwd_list_it) {
        array_traverse(*kwd_list_it, kwd_it) {
            free(kwd_it->kwd);
        }
        array_free(*kwd_list_it);
    }
    array_free(set->kwds_by_len);
}

static inline void _highlight_kwd_set_ensure_list_for_len(highlight_kwd_set *set, int len) {
    array_t a;

    while (array_len(set->kwds_by_len) < len) {
        a = array_make(highlight_kwd);
        array_push(set->kwds_by_len, a);
    }
}

static inline void _highlight_kwd_set_add(highlight_kwd_set *set, char *kwd, int kind, char prefix) {
    int            len;
    array_t       *kwd_list;
    highlight_kwd  *it, hk;
    int            idx, cmp;

    if (!kwd) { return; }

    len = strlen(kwd);

    if (len == 0) { return; }

    _highlight_kwd_set_ensure_list_for_len(set, len);
    kwd_list = array_item(set->kwds_by_len, len - 1);

    idx = 0;
    array_traverse(*kwd_list, it) {
        cmp = strcmp(kwd, it->kwd);

        if      (cmp == 0) { return; }
        else if (cmp < 0)  { break;  }

        idx += 1;
    }

    hk.kwd    = strdup(kwd);
    hk.kind   = kind;
    hk.prefix = prefix;

    array_insert(*kwd_list, idx, hk);
}

static inline highlight_kwd * _highlight_kwd_set_lookup(highlight_kwd_set *set, char *kwd, int len) {
    array_t       *kwd_list;
    highlight_kwd *it;

    if (!len || len > array_len(set->kwds_by_len)) {
        return NULL;
    }

    kwd_list = array_item(set->kwds_by_len, len - 1);

    array_traverse(*kwd_list, it) {
        if (it->kwd[0] > kwd[0]) {
            continue;
        }

        if (strncmp(it->kwd, kwd, len) == 0) {
            return it;
        }
    }

    return NULL;
}


typedef struct {
    int  kind;
    int  inclusive;
    char prefix;
} prefix_word_hl;

typedef struct {
    int  kind;
    int  inclusive;
    char suffix;
} suffix_word_hl;

typedef struct {
    char *start;
    char *end;
    int   start_len;
    int   end_len;
    int   kind;
    int   max_len;
    int   multiline;
    char  escape;
} within_hl;

typedef struct {
    char *start;
    int   start_len;
    int   kind;
} eol_hl;

typedef struct {
    int                            max_scan;
    highlight_kwd_set              kwds;
    tree(char, highlight_kwd_set)  prefix_sets;
    array_t                        general_prefixes;
    array_t                        general_suffixes;
    array_t                        within;
    array_t                        eols;
    int                            hl_numbers;
    int                            has_ml;
    int                            ml_states[1024];
    int                            state_before;
    yed_frame                     *f;
    int                            dirty;
} highlight_info;

static inline void highlight_info_make(highlight_info *info) {
    char *max_scan_str;

    memset(info, 0, sizeof(*info));

    if ((max_scan_str = yed_get_var("highlight-max-scan"))) {
        sscanf(max_scan_str, "%d", &info->max_scan);
    } else {
        info->max_scan = 200;
    }

    _highlight_kwd_set_make(&info->kwds);
    info->prefix_sets      = tree_make(char, highlight_kwd_set);
    info->general_prefixes = array_make(prefix_word_hl);
    info->general_suffixes = array_make(suffix_word_hl);
    info->within           = array_make(within_hl);
    info->eols             = array_make(eol_hl);

    LOG_FN_ENTER();
    yed_log("[!] highlight.h is deprecated and will be removed in version 1400. Use syntax.h instead.");
#warning        "highlight.h is deprecated and will be removed in version 1400. Use syntax.h instead."
    LOG_EXIT();
}

static inline void highlight_info_free(highlight_info *info) {
    eol_hl       *eit;
    within_hl    *wit;

    array_traverse(info->eols, eit) {
        free(eit->start);
    }
    array_free(info->eols);

    array_traverse(info->within, wit) {
        free(wit->end);
        free(wit->start);
    }
    array_free(info->within);

    array_free(info->general_suffixes);
    array_free(info->general_prefixes);
    tree_free(info->prefix_sets);
    _highlight_kwd_set_free(&info->kwds);
}

static inline void highlight_numbers(highlight_info *info) {
    info->hl_numbers = 1;
}

static inline highlight_kwd_set * _highlight_get_prefix_set(highlight_info *info, char pre) {
    tree_it(char, highlight_kwd_set) it;

    it = tree_lookup(info->prefix_sets, pre);

    if (!tree_it_good(it)) { return NULL; }

    return &tree_it_val(it);
}

static inline highlight_kwd_set * _highlight_make_prefix_set(highlight_info *info, char pre) {
    tree_it(char, highlight_kwd_set)  it;
    highlight_kwd_set                *set, new_set;

    set = _highlight_get_prefix_set(info, pre);

    if (set) { return set; }

    _highlight_kwd_set_make(&new_set);
    tree_insert(info->prefix_sets, pre, new_set);
    it = tree_lookup(info->prefix_sets, pre);

    return &tree_it_val(it);
}

static inline prefix_word_hl * _highlight_get_general_prefix(highlight_info *info, char pre) {
    prefix_word_hl *it;

    array_traverse(info->general_prefixes, it) {
        if (it->prefix == pre) {
            return it;
        }
    }

    return NULL;
}

static inline void __highlight_prefixed_words(highlight_info *info, char pre, int kind, int inclusive) {
    prefix_word_hl *it, new;

    array_traverse(info->general_prefixes, it) {
        if (it->prefix == pre) {
            it->kind      = kind;
            it->inclusive = inclusive;
            return;
        }
    }

    new.prefix    = pre;
    new.kind      = kind;
    new.inclusive = inclusive;

    array_push(info->general_prefixes, new);
}

static inline suffix_word_hl * _highlight_get_general_suffix(highlight_info *info, char suf) {
    suffix_word_hl *it;

    array_traverse(info->general_suffixes, it) {
        if (it->suffix == suf) {
            return it;
        }
    }

    return NULL;
}

static inline void __highlight_suffixed_words(highlight_info *info, char suf, int kind, int inclusive) {
    suffix_word_hl *it, new;

    array_traverse(info->general_suffixes, it) {
        if (it->suffix == suf) {
            it->kind      = kind;
            it->inclusive = inclusive;
            return;
        }
    }

    new.suffix    = suf;
    new.kind      = kind;
    new.inclusive = inclusive;

    array_push(info->general_suffixes, new);
}

static inline void highlight_prefixed_words(highlight_info *info, char pre, int kind) {
    __highlight_prefixed_words(info, pre, kind, 0);
}

static inline void highlight_suffixed_words(highlight_info *info, char suf, int kind) {
    __highlight_suffixed_words(info, suf, kind, 0);
}

static inline void highlight_prefixed_words_inclusive(highlight_info *info, char pre, int kind) {
    __highlight_prefixed_words(info, pre, kind, 1);
}

static inline void highlight_suffixed_words_inclusive(highlight_info *info, char suf, int kind) {
    __highlight_suffixed_words(info, suf, kind, 1);
}

static inline void highlight_add_kwd(highlight_info *info, char *kwd, int kind) {
    _highlight_kwd_set_add(&info->kwds, kwd, kind, 0);
}

static inline void highlight_add_prefixed_kwd(highlight_info *info, char pre, char *kwd, int kind) {
    highlight_kwd_set *set;

    set = _highlight_make_prefix_set(info, pre);
    _highlight_kwd_set_add(set, kwd, kind, pre);
}

static inline void _highlight_add_within(highlight_info *info, char *start, char *end, char escape, int kind, int max_len, int multiline) {
    within_hl w;

    w.start     = strdup(start);
    w.end       = strdup(end);
    w.start_len = strlen(start);
    w.end_len   = strlen(end);
    w.multiline = multiline;
    w.escape    = escape;
    w.kind      = kind;
    w.max_len   = max_len;

    array_push(info->within, w);
}

static inline void highlight_within(highlight_info *info, char *start, char *end, char escape, int max_len, int kind) {
    _highlight_add_within(info, start, end, escape, kind, max_len, 0);
}

static inline void highlight_within_multiline(highlight_info *info, char *start, char *end, char escape, int kind) {
    _highlight_add_within(info, start, end, escape, kind, -1, 1);
    info->has_ml = 1;
}

static inline within_hl * _highlight_get_within(highlight_info *info, char *start) {
    within_hl *it;

    array_traverse(info->within, it) {
        if (strncmp(start, it->start, it->start_len) == 0) {
            return it;
        }
    }

    return NULL;
}

static inline void _highlight_add_eol(highlight_info *info, char *start, int kind) {
    eol_hl e;

    e.start     = strdup(start);
    e.start_len = strlen(start);
    e.kind      = kind;

    array_push(info->eols, e);
}

static inline void highlight_to_eol_from(highlight_info *info, char *start, int kind) {
    _highlight_add_eol(info, start, kind);
}

static inline eol_hl * _highlight_get_eol(highlight_info *info, char *start) {
    eol_hl *it;

    array_traverse(info->eols, it) {
        if (strncmp(start, it->start, it->start_len) == 0) {
            return it;
        }
    }

    return NULL;
}

static inline void _highlight_range_with_attrs(int start, int end, array_t line_attrs, yed_attrs *attrs) {
    int        col;
    yed_attrs *dst_attrs;

    if (!attrs) { return; }

    for (col = start; col <= end; col += 1) {
        dst_attrs = array_item(line_attrs, col - 1);
        yed_combine_attrs(dst_attrs, attrs);
    }
}

static inline yed_attrs * _kind_to_attrs(attrs_set *set, int kind) {
    switch (kind) {
        case HL_KEY:     return &set->key;
        case HL_NUM:     return &set->num;
        case HL_STR:     return &set->st;
        case HL_CHAR:    return &set->ch;
        case HL_COMMENT: return &set->com;
        case HL_CON:     return &set->con;
        case HL_PP:      return &set->pp;
        case HL_CALL:    return &set->cal;
        case HL_ATTN:    return &set->atn;
        case HL_CF:      return &set->cf;
        case HL_TYPE:    return &set->ty;
        case HL_ASSOC:   return &set->ass;
        case HL_IGNORE:  return NULL;
    }

    return NULL;
}

static inline int _highlight_line_eol(highlight_info *info, yed_line *line, array_t line_attrs, attrs_set *set, int col) {
    yed_glyph *g;
    char      *str;
    eol_hl    *eol;
    yed_attrs *attrs;

    str = (char*)(g = yed_line_col_to_glyph(line, col));

    eol = _highlight_get_eol(info, str);
    if (!eol) { return 0; }

    attrs = _kind_to_attrs(set, eol->kind);

    _highlight_range_with_attrs(col, line->visual_width, line_attrs, attrs);

    return 1;
}

static inline int _highlight_line_keyword(highlight_info *info, yed_line *line, array_t line_attrs, attrs_set *set, int col, int word_len) {
    char          *kwd;
    highlight_kwd *lookup;
    yed_attrs     *attrs;

    kwd = (char*)yed_line_col_to_glyph(line, col);

    lookup = _highlight_kwd_set_lookup(&info->kwds, kwd, word_len);
    if (!lookup) { return 0; }

    attrs = _kind_to_attrs(set, lookup->kind);

    _highlight_range_with_attrs(col, col + word_len - 1, line_attrs, attrs);

    return word_len;
}

static inline int _highlight_line_prefixed_keyword(highlight_info *info, yed_line *line, array_t line_attrs, attrs_set *set, int col, int word_len, char last_c) {
    int                used_last;
    highlight_kwd_set *kwds;
    int                hl_start_col, kwd_start_col;
    yed_glyph         *g;
    char              *kwd_start;
    highlight_kwd     *lookup;
    yed_attrs         *attrs;

    used_last = 0;
    kwds      = _highlight_get_prefix_set(info, last_c);

    if (kwds) {
        used_last     = 1;
        hl_start_col  = col - 1;
        kwd_start_col = col;
    } else {
        g    = yed_line_col_to_glyph(line, col);
        kwds = _highlight_get_prefix_set(info, g->c);
        if (!kwds) { return 0; }
        used_last     = 0;
        hl_start_col  = col;
        kwd_start_col = col;
    }

    kwd_start = (char*)yed_line_col_to_glyph(line, kwd_start_col);

    lookup = _highlight_kwd_set_lookup(kwds, kwd_start, word_len - !used_last);
    if (!lookup) { return 0; }

    attrs = _kind_to_attrs(set, lookup->kind);

    _highlight_range_with_attrs(hl_start_col, hl_start_col + word_len + used_last - 1, line_attrs, attrs);

    return word_len + used_last;
}

static inline int _highlight_line_prefix(highlight_info *info, yed_line *line, array_t line_attrs, attrs_set *set, int col, int word_len, char last_c) {
    prefix_word_hl *hl;
    int             used_last;
    int             hl_start_col;
    yed_glyph      *g;
    yed_attrs      *attrs;

    hl = _highlight_get_general_prefix(info, last_c);

    if (hl) {
        g  = yed_line_col_to_glyph(line, col);
        if (!isalnum(g->c) && g->c != '_') {
            return 0;
        }
        used_last     = 1;
        hl_start_col  = col - !!hl->inclusive;
    } else if (word_len > 1) {
        g  = yed_line_col_to_glyph(line, col);
        hl = _highlight_get_general_prefix(info, g->c);
        if (!hl || col == line->visual_width) { return 0; }
        g  = yed_line_col_to_glyph(line, col + 1);
        if (!isalnum(g->c) && g->c != '_') {
            return 0;
        }
        used_last     = 0;
        hl_start_col  = col + !hl->inclusive;
    } else {
        return 0;
    }

    attrs = _kind_to_attrs(set, hl->kind);

    _highlight_range_with_attrs(hl_start_col, hl_start_col + word_len + used_last - 1, line_attrs, attrs);

    return word_len + used_last;
}

static inline int _highlight_line_suffix(highlight_info *info, yed_line *line, array_t line_attrs, attrs_set *set, int col, int word_len) {
    suffix_word_hl *hl;
    int             suff_col;
    char            suff_c;
    yed_attrs      *attrs;

    suff_col = col + word_len;

    if (word_len < 1 || suff_col > line->visual_width) {
        return 0;
    }

    suff_c = yed_line_col_to_glyph(line, suff_col)->c;
    hl     = _highlight_get_general_suffix(info, suff_c);

    if (!hl) { return 0; }

    attrs = _kind_to_attrs(set, hl->kind);

    _highlight_range_with_attrs(col, col + word_len + !!hl->inclusive - 1, line_attrs, attrs);

    return word_len + !!hl->inclusive;
}

static inline int _highlight_line_within(highlight_info *info, int state_idx, yed_line *line, array_t line_attrs, attrs_set *set, int col) {
    int        start_col;
    yed_glyph *g;
    char      *str;
    within_hl *within;
    yed_attrs *attrs;
    int        len;
    int        within_idx;

    start_col = col;
    g         = yed_line_col_to_glyph(line, col);
    str       = (char*)g;

    within_idx = 0;
    array_traverse(info->within, within) {
        attrs = _kind_to_attrs(set, within->kind);

        if (strncmp(str, within->start, within->start_len) != 0) {
            within_idx += 1;
            continue;
        }

        len = 0;
        col = start_col + within->start_len;

        while (col <= line->visual_width) {
            g   = yed_line_col_to_glyph(line, col);
            str = (char*)g;

            if (strncmp(str, within->end, within->end_len) == 0) {
                if (within->max_len == -1 || len <= within->max_len) {
                    _highlight_range_with_attrs(start_col, col + within->end_len - 1, line_attrs, attrs);
                    return (col - start_col) + within->end_len;
                } else {
                    return 0;
                }
            } else if (g->c == within->escape && col < line->visual_width) {
                col += yed_get_glyph_width(*g);
                g    = yed_line_col_to_glyph(line, col);
            }

            col += yed_get_glyph_width(*g);
            len += 1;
        }

        if (within->multiline) {
            info->ml_states[state_idx] = within_idx + 1;
        }

        _highlight_range_with_attrs(start_col, line->visual_width, line_attrs, attrs);
        return line->visual_width - start_col + 1;
    }

    return 0;
}

static inline int _highlight_scan_for_prev_ml_state(highlight_info *info, int row, yed_frame *frame) {
    int        state, r;
    yed_line  *line;
    char      *s, *end, *first, *tmp;
    within_hl *within, *found_within;
    int        within_idx, found_within_idx;

#define SENTINAL ((char*)(void*)UINT64_MAX)

    state = 0;
    r     = MAX(1, row - info->max_scan);

    bucket_array_traverse_from(frame->buffer->lines, line, r - 1) {
        if (r == row) { break; }

        array_zero_term(line->chars);

        s   = line->chars.data;
        end = s + array_len(line->chars);

        while (s && s < end) {
            if (state) {
                /*
                 * We are looking for a closing delimiter.
                 * If we find it, the state resets.
                 * Then proceed.
                 */
                s = strstr(s, found_within->end);
                if (s) {
                    state  = 0;
                    s     += found_within->end_len;
                }
            } else {
                /*
                 * We are looking to see if there is an opening
                 * delimiter somewhere.
                 */
                first = SENTINAL;

                /*
                 * Look for the starts of every multiline within.
                 */
                within_idx = 0;
                array_traverse(info->within, within) {
                    /*
                     * If we find some, keep track of which one
                     * comes first.
                     */
                    tmp = strstr(s, within->start);
                    if (tmp && tmp < first) {
                        first            = tmp;
                        found_within     = within;
                        found_within_idx = within_idx;
                    }

                    within_idx += 1;
                }

                if (first == SENTINAL) {
                    /* Nothing. Go to the next line. */
                    s = NULL;
                } else {
                    /*
                     * We found one.
                     * Update the state and the scan pointer.
                     */
                    state = found_within_idx + 1;
                    s     = first + found_within->start_len;
                }
            }
        }

        r += 1;
    }

    if (state && !found_within->multiline) {
        state = 0;
    }

    return state;

#undef SENTINAL
}

static inline int _highlight_get_next_ml_state(highlight_info *info, int row, yed_frame *frame, int prev_state) {
    int        state;
    yed_line  *line;
    char      *s, *end, *first, *tmp;
    within_hl *within, *found_within;
    int        within_idx, found_within_idx;

#define SENTINAL ((char*)(void*)UINT64_MAX)

    state        = prev_state;
    found_within = NULL;

    line = yed_buff_get_line(frame->buffer, row);

    if (line == NULL || line->visual_width == 0) {
        return state;
    }

    array_zero_term(line->chars);

    s   = line->chars.data;
    end = s + array_len(line->chars);

    while (s && s < end) {
        if (state) {
            found_within = array_item(info->within, state - 1);

            /*
             * We are looking for a closing delimiter.
             * If we find it, the state resets.
             * Then proceed.
             */
            s = strstr(s, found_within->end);
            if (s) {
                state  = 0;
                s     += found_within->end_len;
            }
        } else {
            /*
             * We are looking to see if there is an opening
             * delimiter somewhere.
             */
            first = SENTINAL;

            /*
             * Look for the starts of every multiline within.
             */
            within_idx = 0;
            array_traverse(info->within, within) {
                /*
                 * If we find some, keep track of which one
                 * comes first.
                 */
                tmp = strstr(s, within->start);
                if (tmp && tmp < first) {
                    first            = tmp;
                    found_within     = within;
                    found_within_idx = within_idx;
                }

                within_idx += 1;
            }

            if (first == SENTINAL) {
                /* Nothing. Go to the next line. */
                s = NULL;
            } else {
                /*
                 * We found one.
                 * Update the state and the scan pointer.
                 */
                state = found_within_idx + 1;
                s     = first + found_within->start_len;
            }
        }
    }

    if (state && !found_within->multiline) {
        state = 0;
    }

    return state;

#undef SENTINAL
}

static inline int _highlight_get_ml_state(highlight_info *info, yed_frame *frame, int row, int *prev_state) {
    int state_idx;

    state_idx = row - (frame->buffer_y_offset + 1);

    if (info->dirty || row == frame->buffer_y_offset + 1) {
        *prev_state = _highlight_scan_for_prev_ml_state(info, row, frame);
        if (state_idx > 0) {
            info->ml_states[state_idx - 1] = *prev_state;
        }
        return _highlight_get_next_ml_state(info, row, frame, *prev_state);
    }

    *prev_state = info->ml_states[state_idx - 1];
    return _highlight_get_next_ml_state(info, row, frame, *prev_state);
}

static inline int _highlight_line_ml_within(highlight_info *info, int row, yed_frame *frame, array_t line_attrs, attrs_set *set) {
    int        state, prev_state, state_idx;
    within_hl *within;
    yed_line  *line;
    yed_attrs *attrs;
    char      *s, *scan;
    int        col;

    state = prev_state = 0;

    state_idx = row - (frame->buffer_y_offset + 1);

    state = _highlight_get_ml_state(info, frame, row, &prev_state);

    info->ml_states[state_idx] = state;

    if (state > 0) {
        within = array_item(info->within, state - 1);
        line   = yed_buff_get_line(frame->buffer, row);
        attrs  = _kind_to_attrs(set, within->kind);
        if (state == prev_state) {
            _highlight_range_with_attrs(1, line->visual_width, line_attrs, attrs);
            return line->visual_width;
        }
    } else if (prev_state) {
        within = array_item(info->within, prev_state - 1);
        line   = yed_buff_get_line(frame->buffer, row);
        s      = line->chars.data;
        scan   = strstr(s, within->end);
        if (scan) {
            info->ml_states[state_idx] = 0;
            attrs = _kind_to_attrs(set, within->kind);
            col   = yed_line_idx_to_col(line, scan - s + within->end_len - 1);
            _highlight_range_with_attrs(1, col, line_attrs, attrs);

            return col;
        }
    }

    return 0;
}

static inline void _highlight_line_get_word_info(highlight_info *info, yed_line *line, int col, int *word_len, int *all_num, int *is_hex) {
    yed_glyph *g;
    int        save_col;

    save_col = col;
    *all_num = 1;
    *is_hex  = 0;

    if (col > line->visual_width) { return; }

    g = yed_line_col_to_glyph(line, col);

    if (isalnum(g->c) || g->c == '_') {
        do {
            col      += yed_get_glyph_width(*g);
            *all_num &= !!isdigit(g->c);
            switch (*is_hex) {
                case 0: if (g->c == '0') { *is_hex = 1; } break;
                case 1: if (g->c == 'x') { *is_hex = 2; } break;
                case 2: if (!(
                             (g->c >= 'a' && g->c <= 'f')
                        ||   (g->c >= 'A' && g->c <= 'F')
                        ||   isdigit(g->c))) {

                            *is_hex = 3;
                        }
                        break;
            }
            if (col > line->visual_width) { break; }
            g = yed_line_col_to_glyph(line, col);
        } while (isalnum(g->c) || g->c == '_');
    } else if (!isspace(g->c) && !isalnum(g->c) && g->c != '_') {
        *all_num = *is_hex = 0;
        do {
            col += yed_get_glyph_width(*g);
            if (col > line->visual_width) { break; }
            g = yed_line_col_to_glyph(line, col);
        } while (!isspace(g->c) && !isalnum(g->c) && g->c != '_');
    } else if (isspace(g->c)) {
        *all_num = *is_hex = 0;
        do {
            col += yed_get_glyph_width(*g);
            if (col > line->visual_width) { break; }
            g = yed_line_col_to_glyph(line, col);
        } while (isspace(g->c));
    } else {
        *all_num = *is_hex  = 0;
        col                += yed_get_glyph_width(*g);
    }

    *word_len = col - save_col;
    *is_hex   = *is_hex == 2;
}

static inline void highlight_line(highlight_info *info, yed_event *event) {
    yed_line          *line;
    array_t            line_attrs;
    int                col, bump;
    yed_glyph         *g;
    char               last_c;
    int                word_len;
    int                all_num;
    int                is_hex;
    attrs_set          attrs;
    int                last_is_word_boundary;
    int                prev_state, state_idx;

    if (event->frame != info->f) {
        info->dirty = 1;
        info->f     = event->frame;
    }

    state_idx = event->row - (event->frame->buffer_y_offset + 1);

    if (info->dirty) {
        memset(info->ml_states, 0, sizeof(info->ml_states));
    }

    line = yed_buff_get_line(event->frame->buffer, event->row);

    if (line->visual_width == 0) {
        if (info->has_ml) {
            info->ml_states[state_idx] = _highlight_get_ml_state(info, event->frame, event->row, &prev_state);
        }
        goto out;
    }

    line_attrs = event->line_attrs;

    array_zero_term(line->chars);

    get_attrs(&attrs);

    col = 1;

    if (info->has_ml) {
        col += _highlight_line_ml_within(info, event->row, event->frame, line_attrs, &attrs);
    }

    last_c = 0;

    while (col <= line->visual_width) {
        word_len = 0;
        all_num  = 0;
        bump     = 0;

        g = yed_line_col_to_glyph(line, col);

        if (isspace(g->c)) {
            _highlight_line_get_word_info(info, line, col, &word_len, &all_num, &is_hex);
            bump = word_len;
            goto next;
        } else {
            last_is_word_boundary = (!isalnum(last_c) && last_c != '_');
        }

        if (!(bump = _highlight_line_within(info, state_idx, line, line_attrs, &attrs, col))) {
            if (_highlight_line_eol(info, line, line_attrs, &attrs, col)) {
                goto out;
            }

            _highlight_line_get_word_info(info, line, col, &word_len, &all_num, &is_hex);

            if (!last_is_word_boundary) {
                bump = 1;
                goto next;
            }

            if (info->hl_numbers) {
                if (all_num) {
                    if (last_c == '-' || last_c == '.') {
                        _highlight_range_with_attrs(col - 1, col + word_len - 1, line_attrs, &attrs.num);
                    } else {
                        _highlight_range_with_attrs(col, col + word_len - 1, line_attrs, &attrs.num);
                    }
                    bump = word_len;
                    goto next;
                } else if (is_hex) {
                    _highlight_range_with_attrs(col, col + word_len - 1, line_attrs, &attrs.num);
                    bump = word_len;
                    goto next;
                }
            }

            if (!(bump = _highlight_line_prefixed_keyword(info, line, line_attrs, &attrs, col, word_len, last_c))) {
            if (!(bump = _highlight_line_keyword(info, line, line_attrs, &attrs, col, word_len))) {
            if (!(bump = _highlight_line_suffix(info, line, line_attrs, &attrs, col, word_len))) {
            if (!(bump = _highlight_line_prefix(info, line, line_attrs, &attrs, col, word_len, last_c))) {
                bump = yed_get_glyph_width(*g);
            }}}}
        }

next:
        col    += bump;
        last_c  = col > 1 ? yed_line_col_to_glyph(line, col - 1)->c : 0;
    }

out:
    info->dirty = 0;
}

static inline void highlight_frame_pre_draw_update(highlight_info *info, yed_event *event) {
    info->dirty = 1;
}

static inline void highlight_buffer_pre_mod_update(highlight_info *info, yed_event *event) {
    int prev_state;

    if (ys->active_frame         == NULL
    ||  ys->active_frame->buffer == NULL
    ||  ys->active_frame->buffer != event->buffer) {
        return;
    }

    info->state_before = _highlight_get_ml_state(info, ys->active_frame, ys->active_frame->cursor_line, &prev_state);
}

static inline void highlight_buffer_post_mod_update(highlight_info *info, yed_event *event) {
    int state, prev_state;

    if (ys->active_frame         == NULL
    ||  ys->active_frame->buffer == NULL
    ||  ys->active_frame->buffer != event->buffer) {
        return;
    }

    state = _highlight_get_ml_state(info, ys->active_frame, ys->active_frame->cursor_line, &prev_state);

    if (state != info->state_before) {
        ys->active_frame->dirty = 1;
    }
}
