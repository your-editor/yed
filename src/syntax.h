/*
 * === What this file provides:
 *
 * Fast highlighting of ASCII keywords matching [a-Z_]+[0-9a-Z_]
 *
 *     Keywords are stored in length buckets so that if we know the length
 *     of the next word in the line, we can search for matches only in one
 *     bucket and cap the string comparisons at word_lenth characters.
 *
 *     The main advantage of this is that most keywords are somewhere in the
 *     range of 2-8 characters for most languages, but other words that may
 *     appear in the buffer are typically longer (like identifiers).
 *     In that case, we don't have to search for a match for a 16 character
 *     word because we won't have a 16 character keyword bucket.
 *
 *     The other optimization that can be made for simple keywords is that
 *     we can highlight them all in a single traversal of the line/string.
 *
 * Matching of regular expressions on a single line
 *     Submatches can be specified.
 *
 * Single/multi-line ranges defined by start/end regular expressions.
 *     Can include regular expression ranges to skip (e.g. skip \" in a string literal).
 *     Regular expressions and keywords can be specified for highlighting within a specific range.
 *
 * A smart and fast caching strategy that stores states of lines with respect to multi-line range state.
 *     Self-balances the distribution of cache entries so that buffers get good coverage and random access
 *     patterns don't cause huge slowdown.
 *     Always keeps the most recently drawn line in cache so that scrolling and full frame redraws are very quick.
 *     Must parse whole buffer at least once so that the cache can always be correct (if highlighting is to be correct
 *     100% of the time, this is an unfortunate necessity). :(
 *
 * A declarative interface for defining syntax.
 *
 *
 * === How to use this file:
 *
 *     #include <yed/syntax.h>
 *
 * Create a yed_syntax structure:
 *
 *     static yed_syntax syn;
 *
 * Use the declarative interface to build up the syntax (this example doesn't check for errors):
 *
 *     yed_syntax_start(&syn);
 *
 *         // Highlight string literals with escape sequences.
 *         yed_syntax_attr_push(&syn, "&code-string");
 *             yed_syntax_range_start(&syn, "\"");
 *                 yed_syntax_range_one_line(&syn);
 *                 yed_syntax_range_skip(&syn, "\\\\\"");
 *
 *                 yed_syntax_attr_push(&syn, "&code-escape-sequence");
 *                     yed_syntax_regex(&syn, "\\\\.");
 *                 yed_syntax_attr_pop();
 *             yed_syntax_range_end(&syn, "\"");
 *         yed_syntax_attr_pop(&syn);
 *
 *         // Highlight numbers.
 *         yed_syntax_regex_sub(&syn, "(^|[^[:alnum:]_])(-?[[:digit:]]+)[[:>:]]", 2);
 *
 *         // Highlight the keyword "if".
 *         yed_syntax_kwd(&syn, "if");
 *
 *     yed_syntax_end(&syn);
 *
 * Register event handlers for for these events that call the respective functions in this file:
 *
 *     event                     function call                           reason
 *     ----------------------------------------------------------------------------------------------------------
 *     EVENT_STYLE_CHANGE        yed_syntax_style_event(&syn);           Update attr structs for syntax elements.
 *     EVENT_BUFFER_PRE_DELETE   yed_syntax_buffer_delete_event(&syn);   Remove state cache for the buffer.
 *     EVENT_BUFFER_POST_MOD     yed_syntax_buffer_mod_event(&syn);      Update state cache for the buffer.
 *     EVENT_LINE_PRE_DRAW       yed_syntax_line_event(&syn);            Highlight the line about to be drawn.
 *
 * You may choose to call yed_syntax_line_event() selectively if, for example, the buffer's ft matches the kind of
 * buffer you're trying to highlight:
 *
 *     void eline(yed_event *event)  {
 *         yed_frame *frame;
 *
 *         frame = event->frame;
 *
 *         if (!frame
 *         ||  !frame->buffer
 *         ||  frame->buffer->kind != BUFF_KIND_FILE
 *         ||  frame->buffer->ft != yed_get_ft("C")) {
 *             return;
 *         }
 *
 *         yed_syntax_line_event(&syn, event);
 *     }
 *
 * Free up the syntax structure when you're finished with it:
 *
 *     yed_syntax_free(&syn);
 */


#include <yed/internal.h>
#include <yed/tree.h>
#include <regex.h>

#define YED_SYN_CACHE_SIZE         (8192)
#define YED_SYN_N_EVICTION_BUCKETS (4096)

#ifdef YED_DEBUG

#define DBG(...)                      \
do {                                  \
    LOG_FN_ENTER();                   \
    yed_log("syntax.h: "__VA_ARGS__); \
    LOG_EXIT();                       \
} while (0)

#else

#define DBG(...) ;

#endif

typedef struct {
    char      *str;
    yed_attrs  attr;
} _yed_syntax_attr;

typedef struct {
    _yed_syntax_attr *attr;
    char             *kwd;
} _yed_syntax_kwd;

typedef struct {
    array_t kwds_by_len;
} _yed_syntax_kwd_set;

use_tree(char, _yed_syntax_kwd_set);

typedef struct {
    _yed_syntax_kwd_set kwds;
    array_t             regs;
} _yed_syntax_items;

typedef struct {
    _yed_syntax_attr *attr;
    regex_t           reg;
    int               group;
} _yed_syntax_regex;

typedef struct {
    _yed_syntax_attr  *attr;
    regex_t            start;
    regex_t            end;
    array_t            skips;
    int                one_line;
    _yed_syntax_items  items;
} _yed_syntax_range;

typedef struct {
    u32 row;
    u32 range_idx;
} _yed_syntax_cache_entry;

typedef struct {
    array_t entries;
    u32     size;
} _yed_syntax_cache;

typedef yed_buffer *_yed_syntax_bp;

use_tree(_yed_syntax_bp, _yed_syntax_cache);

#define CACHE_TREE         tree(_yed_syntax_bp, _yed_syntax_cache)
#define CACHE_IT           tree_it(_yed_syntax_bp, _yed_syntax_cache)
#define CACHE_TREE_MAKE()  tree_make(_yed_syntax_bp, _yed_syntax_cache)
#define CACHE_TREE_FREE(c) tree_free(c)

typedef struct {
    array_t            attrs;
    array_t            attr_stack;
    array_t            ranges;
    _yed_syntax_range *global;
    _yed_syntax_range *range;
    char              *regex_err_str;
    int                max_group;
    regmatch_t        *matches;
    CACHE_TREE         caches;
    int                needs_state;
    int                finalized;
} yed_syntax;


/************************************************************************************/
/*                                 Data management                                  */
/************************************************************************************/

static inline void _yed_syntax_make_kwd_set(_yed_syntax_kwd_set *set) {
    set->kwds_by_len = array_make(array_t);
}

static inline void _yed_syntax_free_kwd_set(_yed_syntax_kwd_set *set) {
    array_t         *kwd_list_it;
    _yed_syntax_kwd *kwd_it;

    array_traverse(set->kwds_by_len, kwd_list_it) {
        array_traverse(*kwd_list_it, kwd_it) {
            free(kwd_it->kwd);
        }
        array_free(*kwd_list_it);
    }
    array_free(set->kwds_by_len);
}

static inline void _yed_syntax_kwd_set_ensure_list_for_len(_yed_syntax_kwd_set *set, int len) {
    array_t a;

    while (array_len(set->kwds_by_len) < len) {
        a = array_make(_yed_syntax_kwd);
        array_push(set->kwds_by_len, a);
    }
}

static inline _yed_syntax_kwd * _yed_syntax_kwd_set_add(_yed_syntax_kwd_set *set, const char *kwd) {
    int              len;
    array_t         *kwd_list;
    _yed_syntax_kwd *it, k;
    int              idx, cmp;

    if (!kwd) { return NULL; }

    len = strlen(kwd);

    if (len == 0) { return NULL; }

    _yed_syntax_kwd_set_ensure_list_for_len(set, len);
    kwd_list = array_item(set->kwds_by_len, len - 1);

    idx = 0;
    array_traverse(*kwd_list, it) {
        cmp = strcmp(kwd, it->kwd);

        if      (cmp == 0) { return NULL; }
        else if (cmp < 0)  { break;       }

        idx += 1;
    }

    k.kwd = strdup(kwd);

    return array_insert(*kwd_list, idx, k);
}

static inline _yed_syntax_kwd * _yed_syntax_kwd_set_lookup(_yed_syntax_kwd_set *set, const char *kwd, int len) {
    array_t         *kwd_list;
    _yed_syntax_kwd *it;

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

static inline int _yed_syntax_line_get_word_len(yed_line *line, int col) {
    yed_glyph *g;
    yed_glyph *end;
    int        len;
    int        glen;

    if (col > line->visual_width) { return 0; }

    g    = yed_line_col_to_glyph(line, col);
    end  = (yed_glyph*)(void*)(array_data(line->chars) + array_len(line->chars));
    len  = 0;
    glen = yed_get_glyph_len(*g);

    if (glen == 1) {
        if (is_alnum(g->c) || g->c == '_') {
            do {
                len += (glen = yed_get_glyph_len(*g));
                g    = (yed_glyph*)(((void*)g) + glen);
            } while (g < end && (is_alnum(g->c) || g->c == '_'));
        } else if (!is_space(g->c) && !is_alnum(g->c) && g->c != '_') {
            do {
                len += (glen = yed_get_glyph_len(*g));
                g    = (yed_glyph*)(((void*)g) + glen);
            } while (g < end && !is_space(g->c) && !is_alnum(g->c) && g->c != '_');
        } else if (is_space(g->c)) {
            do {
                len += (glen = yed_get_glyph_len(*g));
                g    = (yed_glyph*)(((void*)g) + glen);
            } while (g < end && is_space(g->c));
        } else {
            len = yed_get_glyph_len(*g);
        }
    } else {
        len = glen;
    }

    return len;
}

static inline void _yed_syntax_free_items(_yed_syntax_items *items);

static inline void _yed_syntax_free_regex(_yed_syntax_regex *regex) {
    regfree(&regex->reg);
}

static inline void _yed_syntax_make_empty_items(_yed_syntax_items *items) {
    _yed_syntax_make_kwd_set(&items->kwds);
    items->regs = array_make(_yed_syntax_regex);
}

static inline void _yed_syntax_free_items(_yed_syntax_items *items) {
    _yed_syntax_regex *rit;

    if (array_data(items->regs) != NULL) {
        array_traverse(items->regs, rit) {
            _yed_syntax_free_regex(rit);
        }
        array_free(items->regs);
    }
    _yed_syntax_free_kwd_set(&items->kwds);
}

static inline void _yed_syntax_make_range(_yed_syntax_range *range) {
    memset(range, 0, sizeof(*range));

    _yed_syntax_make_empty_items(&range->items);
    range->skips = array_make(regex_t);
}

static inline void _yed_syntax_free_range(_yed_syntax_range *range) {
    regex_t *sit;

    array_traverse(range->skips, sit) {
        regfree(sit);
    }
    array_free(range->skips);

    regfree(&range->end);
    regfree(&range->start);

    _yed_syntax_free_items(&range->items);

    free(range);
}

static inline void _yed_syntax_make_cache(_yed_syntax_cache *cache, u32 size) {
    cache->entries = array_make_with_cap(_yed_syntax_cache_entry, size);
    cache->size    = size;
}

static inline void _yed_syntax_free_cache(_yed_syntax_cache *cache) {
    array_free(cache->entries);
}

static inline _yed_syntax_attr *_yed_syntax_top_attr(yed_syntax *syntax) {
    if (array_len(syntax->attr_stack) == 0) { return NULL; }

    return *(_yed_syntax_attr**)array_last(syntax->attr_stack);
}

static inline _yed_syntax_range *_yed_syntax_top_range(yed_syntax *syntax) {
    if (syntax->range != NULL) { return syntax->range; }

    return syntax->global;
}

static inline int _yed_syntax_get_range_idx(yed_syntax *syntax, _yed_syntax_range *range) {
    int range_idx;

    for (range_idx = 0; range_idx < array_len(syntax->ranges); range_idx += 1) {
        if (range == *(_yed_syntax_range**)array_item(syntax->ranges, range_idx)) {
            return range_idx;
        }
    }

    return -1;
}



/************************************************************************************/
/*                                      cache                                       */
/************************************************************************************/

static inline void _yed_syntax_build_cache(yed_syntax *syntax, yed_buffer *buffer);
static inline _yed_syntax_range *_yed_syntax_get_line_end_state(yed_syntax *syntax, yed_buffer *buffer, yed_line *line, _yed_syntax_range *start_range);

static inline _yed_syntax_cache *_yed_syntax_get_cache(yed_syntax *syntax, yed_buffer *buffer) {
    CACHE_IT it;

    it = tree_lookup(syntax->caches, buffer);

    if (!tree_it_good(it)) {
        _yed_syntax_build_cache(syntax, buffer);
        it = tree_lookup(syntax->caches, buffer);
    }

    return &tree_it_val(it);
}

static inline void _yed_syntax_remove_cache(yed_syntax *syntax, yed_buffer *buffer) {
    CACHE_IT it;

    if (!syntax->finalized || buffer == NULL) { return; }

    it = tree_lookup(syntax->caches, buffer);

    if (tree_it_good(it)) {
        _yed_syntax_free_cache(&tree_it_val(it));
        tree_delete(syntax->caches, buffer);
    }
}

static inline void _yed_syntax_cache_evict_one(yed_syntax *syntax, _yed_syntax_cache *cache) {
    _yed_syntax_cache_entry *it;
    int                      min_row;
    int                      max_row;
    int                      distance;
    int                      buckets[YED_SYN_N_EVICTION_BUCKETS];
    int                      bump;
    u32                      lim;
    int                      max_bucket_idx;
    int                      i;
    int                      idx;

    if (array_len(cache->entries) > 0) {
        it       = array_item(cache->entries, 0);
        min_row  = it->row;
        it       = array_last(cache->entries);
        max_row  = it->row;
        distance = max_row - min_row;

        memset(buckets, 0, sizeof(buckets[0]) * YED_SYN_N_EVICTION_BUCKETS);

        bump = MAX(distance / YED_SYN_N_EVICTION_BUCKETS, 1);
        lim  = 1 + bump;
        idx  = 0;

        array_traverse(cache->entries, it) {
            if (it->row < lim) {
                buckets[idx] += 1;
            } else {
                lim += bump;
                idx += 1;

                /* Things don't always divide nicely, but that's okay. Just keep this in check. */
                idx = MIN(idx, YED_SYN_N_EVICTION_BUCKETS - 1);
            }
        }

        max_bucket_idx = YED_SYN_N_EVICTION_BUCKETS - 1;
        for (i = YED_SYN_N_EVICTION_BUCKETS - 2; i >= 0; i -= 1) {
            if (buckets[i] > buckets[max_bucket_idx]) { max_bucket_idx = i; }
        }

        idx = (array_len(cache->entries) * max_bucket_idx) / YED_SYN_N_EVICTION_BUCKETS;
        array_delete(cache->entries, idx);
    }
}

static inline int _yed_syntax_cache_is_full(yed_syntax *syntax, _yed_syntax_cache *cache) {
    return array_len(cache->entries) == cache->size;
}

static inline void _yed_syntax_cache_evict_if_full(yed_syntax *syntax, _yed_syntax_cache *cache) {
    if (_yed_syntax_cache_is_full(syntax, cache)) {
        _yed_syntax_cache_evict_one(syntax, cache);
    }
}

static inline _yed_syntax_cache_entry *_yed_syntax_cache_lookup_exact(yed_syntax *syntax, _yed_syntax_cache *cache, u32 row) {
    int                      l;
    int                      r;
    int                      m;
    _yed_syntax_cache_entry *it;

    l = 0;
    r = array_len(cache->entries) - 1;

    while (l <= r) {
        m  = (l + r) / 2;
        it = array_item(cache->entries, m);

        if (it->row < row) {
            l = m + 1;
        } else if (it->row > row) {
            r = m - 1;
        } else {
            return it;
        }
    }

    return NULL;
}

static inline _yed_syntax_cache_entry *_yed_syntax_cache_lookup_nearest(yed_syntax *syntax, _yed_syntax_cache *cache, u32 row) {
    int                      len;
    _yed_syntax_cache_entry *it;
    int                      l;
    int                      r;
    int                      m;

    len = array_len(cache->entries);
    if (len == 0) { return NULL; }

    it = array_item(cache->entries, 0);
    if (it->row > row) { return NULL; }

    it = array_last(cache->entries);
    if (it->row < row) { return array_last(cache->entries); }

    l = 0;
    r = len - 1;

    while (l <= r) {
        m  = (l + r) / 2;
        it = array_item(cache->entries, m);

        if (it->row < row) {
            l = m + 1;
        } else if (it->row > row) {
            r = m - 1;
        } else {
            return it;
        }
    }

    return array_item(cache->entries, r);
}

static inline int _yed_syntax_cache_insert_point(yed_syntax *syntax, _yed_syntax_cache *cache, u32 row) {
    int                      len;
    _yed_syntax_cache_entry *it;
    int                      l;
    int                      r;
    int                      m;

    len = array_len(cache->entries);
    if (len == 0) { return 0; }

    it = array_item(cache->entries, 0);
    if (it->row > row) { return 0; }

    it = array_last(cache->entries);
    if (it->row < row) { return len; }

    l = 0;
    r = len - 1;

    while (l <= r) {
        m  = (l + r) / 2;
        it = array_item(cache->entries, m);

        if (it->row < row) {
            l = m + 1;
        } else if (it->row > row) {
            r = m - 1;
        } else {
            return m;
        }
    }

    return r + 1;
}

static inline _yed_syntax_cache_entry *_yed_syntax_add_to_cache(yed_syntax *syntax, _yed_syntax_cache *cache, u32 row, _yed_syntax_range *range) {
    int                      range_idx;
    int                      idx;
    _yed_syntax_cache_entry *it;
    _yed_syntax_cache_entry  new_entry;

    range_idx = _yed_syntax_get_range_idx(syntax, range);
    if (range_idx == -1) { range_idx = 0; }

    it = array_last(cache->entries);

    /* Fast path for building from scratch. */
    if (it == NULL) {
        new_entry.row       = row;
        new_entry.range_idx = range_idx;
        _yed_syntax_cache_evict_if_full(syntax, cache);
        return array_push(cache->entries, new_entry);
    } else {
        if (it->row < row) {
            new_entry.row       = row;
            new_entry.range_idx = range_idx;
            _yed_syntax_cache_evict_if_full(syntax, cache);
            return array_push(cache->entries, new_entry);
        } else if (it->row == row) {
            it->range_idx = range_idx;
            return it;
        }
    }

    /* Try to insert into an existing entry. */
    it = _yed_syntax_cache_lookup_exact(syntax, cache, row);
    if (it != NULL) {
        it->range_idx = range_idx;
        return it;
    }

    /* Nope, so we might have to evict. */
    _yed_syntax_cache_evict_if_full(syntax, cache);

    idx                 = _yed_syntax_cache_insert_point(syntax, cache, row);
    new_entry.row       = row;
    new_entry.range_idx = range_idx;

    return array_insert(cache->entries, idx, new_entry);
}

static inline void _yed_syntax_build_cache(yed_syntax *syntax, yed_buffer *buffer) {
    u64                start;

    CACHE_IT           it;
    _yed_syntax_cache  new_cache;
    _yed_syntax_cache *cache;
    u32                n_lines;
    u32                bump;
    u32                lim;
    _yed_syntax_range *range;
    int                row;
    yed_line          *line;
    _yed_syntax_range *new_range;

    if (!syntax->finalized || !syntax->needs_state) { return; }

    start = measure_time_now_ms();

    it = tree_lookup(syntax->caches, buffer);

    if (!tree_it_good(it)) {
        _yed_syntax_make_cache(&new_cache, YED_SYN_CACHE_SIZE);
        it = tree_insert(syntax->caches, buffer, new_cache);
    }

    cache = &tree_it_val(it);

    array_clear(cache->entries);

    n_lines = yed_buff_n_lines(buffer);
    bump    = n_lines < cache->size
                ? 1
                : MAX(n_lines / cache->size, 2);
    lim     = 1 + bump;
    range   = syntax->global;
    row     = 1;

    bucket_array_traverse(buffer->lines, line) {
        if (row == n_lines) { break; }

        if (line->visual_width > 0) {
            array_zero_term(line->chars);
            new_range = _yed_syntax_get_line_end_state(syntax, buffer, line, range);
            if (!new_range->one_line) { range = new_range; }
        }

        if (row == lim) {
            _yed_syntax_add_to_cache(syntax, cache, row + 1, range);
            lim += bump;
        }

        row += 1;
    }

    DBG("cache: %llu ms", measure_time_now_ms() - start);
}

static _yed_syntax_range *_yed_syntax_get_start_state(yed_syntax *syntax, yed_buffer *buffer, int row) {
    _yed_syntax_cache       *cache;
    _yed_syntax_cache_entry *it;
    int                      r;
    _yed_syntax_range       *range;
    _yed_syntax_range       *new_range;
    yed_line                *line;

    if (!syntax->finalized || !syntax->needs_state || buffer == NULL || row <= 1) { return syntax->global; }

    if (row <= 1) { return syntax->global; }

    cache = _yed_syntax_get_cache(syntax, buffer);
    it    = _yed_syntax_cache_lookup_nearest(syntax, cache, row);

    if (it == NULL) {
        range = syntax->global;
        r     = 1;
    } else {
        r     = it->row;
        range = *(_yed_syntax_range**)array_item(syntax->ranges, it->range_idx);
    }

    while (r < row) {
        line = yed_buff_get_line(buffer, r);

        if (line->visual_width > 0) {
            array_zero_term(line->chars);
            new_range = _yed_syntax_get_line_end_state(syntax, buffer, line, range);
            if (!new_range->one_line) { range = new_range; }
        }

        r += 1;
    }

    return range;
}

static inline int _yed_syntax_fixup_cache(yed_syntax *syntax, yed_buffer *buffer, _yed_syntax_cache *cache, int row) {
    int                      started_at_top;
    _yed_syntax_cache_entry *it;
    int                      changed;
    int                      count;
    _yed_syntax_range       *start_range;
    yed_line                *line;
    _yed_syntax_range       *end_range;
    int                      end_range_idx;
    _yed_syntax_cache_entry  new_entry;

    if (array_len(cache->entries) == 0) {
        return 0;
    }

    started_at_top = 0;

    it = _yed_syntax_cache_lookup_nearest(syntax, cache, row);
    if (it == NULL) {
        it = _yed_syntax_add_to_cache(syntax, cache, 1, syntax->global);
        started_at_top = 1;
    }

    DBG("fixup: row %d ->", it->row);

    changed = 0;
    count   = 0;
    while (it != array_last(cache->entries)) {
        start_range = *(_yed_syntax_range**)array_item(syntax->ranges, it->range_idx);
        line        = yed_buff_get_line(buffer, it->row);

        array_zero_term(line->chars);
        end_range = _yed_syntax_get_line_end_state(syntax, buffer, line, start_range);

        end_range_idx = _yed_syntax_get_range_idx(syntax, end_range);
        if (end_range_idx == -1) { end_range_idx = 0; }

        if ((it + 1)->row == it->row + 1) {
            /* Cache entry exists. */

            if ((it + 1)->range_idx == end_range_idx) {
                /* The cache entry is correct, therefore all further cache entries are correct, so we're done! */
                goto out;
            }

            /* Update it. */
            it            = it + 1;
            it->range_idx = end_range_idx;
            changed       = 1;
        } else {
            /* Insert a new entry. */
            it = _yed_syntax_add_to_cache(syntax, cache, it->row + 1, end_range);
        }

        /* `it` is the correct next cache entry to use at this point. */
        count += 1;
    }

out:;
    DBG(" %d entries", count);

    return changed | started_at_top;
}

static inline void _yed_syntax_cache_rebuild(yed_syntax *syntax, _yed_syntax_cache *cache, yed_buffer *buffer, int row, int mod_event) {
    yed_line                *line;
    _yed_syntax_cache_entry *cache_entry;
    _yed_syntax_range       *cached_state;
    _yed_syntax_range       *start_state;
    _yed_syntax_range       *end_state;
    int                      mark_dirty;
    _yed_syntax_cache_entry *it;
    int                      idx;

    if (!syntax->finalized) { return; }

    mark_dirty = 0;

    switch (mod_event) {
        case BUFF_MOD_APPEND_TO_LINE:
        case BUFF_MOD_POP_FROM_LINE:
        case BUFF_MOD_INSERT_INTO_LINE:
        case BUFF_MOD_DELETE_FROM_LINE:
        case BUFF_MOD_CLEAR_LINE:
        case BUFF_MOD_SET_LINE:
            line = yed_buff_get_line(buffer, row);
            array_zero_term(line->chars);

            start_state  = _yed_syntax_get_start_state(syntax, buffer, row);
            end_state    = _yed_syntax_get_line_end_state(syntax, buffer, line, start_state);
            cache_entry  = _yed_syntax_cache_lookup_exact(syntax, cache, row + 1);
            cached_state = (cache_entry == NULL)
                            ? NULL
                            : *(_yed_syntax_range**)array_item(syntax->ranges, cache_entry->range_idx);

            if (cached_state != end_state) {
                mark_dirty  = _yed_syntax_fixup_cache(syntax, buffer, cache, row);
            }

            if (array_len(cache->entries) == cache->size - 1) {
                _yed_syntax_cache_evict_one(syntax, cache);
            } else if (array_len(cache->entries) == cache->size) {
                _yed_syntax_cache_evict_one(syntax, cache);
                _yed_syntax_cache_evict_one(syntax, cache);
            }
            _yed_syntax_add_to_cache(syntax, cache, row + 1, end_state);
            _yed_syntax_add_to_cache(syntax, cache, row,     start_state);

            break;

        case BUFF_MOD_ADD_LINE:
        case BUFF_MOD_INSERT_LINE:
            if (yed_buff_n_lines(buffer) == 1) { break; }

            if (row < yed_buff_n_lines(buffer)) {
                array_rtraverse(cache->entries, it) {
                    if (it->row >= row) {
                        it->row += 1;
                    } else {
                        break;
                    }
                }
            }

            cached_state = _yed_syntax_get_start_state(syntax, buffer, row);
            _yed_syntax_add_to_cache(syntax, cache, row, cached_state);
            mark_dirty = _yed_syntax_fixup_cache(syntax, buffer, cache, row + 1);

            break;

        case BUFF_MOD_DELETE_LINE:
            idx = array_len(cache->entries) - 1;
            array_rtraverse(cache->entries, it) {
                if (it->row > row) {
                    it->row -= 1;
                } else {
                    if (it->row == row) {
                        array_delete(cache->entries, idx);
                    }
                    break;
                }
                idx -= 1;
            }


            if (row == 1) {
                _yed_syntax_add_to_cache(syntax, cache, row, syntax->global);
            }
            mark_dirty = _yed_syntax_fixup_cache(syntax, buffer, cache, row - 1);

            break;

        case BUFF_MOD_CLEAR:
            _yed_syntax_remove_cache(syntax, buffer);
            mark_dirty = 1;
            break;
    }

    if (mark_dirty) {
        DBG("DIRTY");
        yed_mark_dirty_frames(buffer);
    }
}


/************************************************************************************/
/*                              Parsing and highlighting                            */
/************************************************************************************/


static inline const char * _yed_syntax_find_next_kwd(yed_syntax *syntax, _yed_syntax_range *range, yed_line *line, const char *start, const char *end, int *len_out, _yed_syntax_attr **attr_out) {
    yed_glyph       *g;
    yed_glyph       *gend;
    int              len;
    int              col;
    int              word_len;
    const char      *word;
    _yed_syntax_kwd *lookup;

    g    = (yed_glyph*)(void*)start;
    gend = (yed_glyph*)(void*)end;

    while (g < gend) {
        len = yed_get_glyph_len(*g);
        if (len > 1) { goto next; }

        if (is_alpha(g->c) || g->c == '_') {
            col      = yed_line_idx_to_col(line, ((void*)g) - ((void*)array_data(line->chars)));
            word_len = _yed_syntax_line_get_word_len(line, col);
            word     = &g->c;

            if (word + word_len > end) { break; }

            /*
             * It's safe to use width here in the lookup even though it's
             * passed to strncmp() as a byte length because only alphanumeric or
             * underscore characters can be in a "word" as understood by this part
             * of the higsyntaxighter.
             */
            lookup = _yed_syntax_kwd_set_lookup(&range->items.kwds, word, word_len);
            if (lookup != NULL) {
                *len_out  = word_len;
                *attr_out = lookup->attr;
                return &g->c;
            }

            len += word_len;
        }

next:;
        g = (yed_glyph*)(((void*)g) + len);
    }

    return NULL;
}

static inline const char * _yed_syntax_find_next_regex_match(yed_syntax *syntax, _yed_syntax_range *range, yed_line *line, const char *start, const char *end, int *len_out, _yed_syntax_attr **attr_out) {
    int                nmatch;
    const char        *match_start;
    _yed_syntax_regex *match_rit;
    regmatch_t         first_match;
    regmatch_t        *m;
    _yed_syntax_regex *rit;
    int                eflags;
    int                err;

    nmatch      = syntax->max_group + 1;
    match_start = NULL;
    match_rit   = NULL;

    memset(&first_match, 0, sizeof(first_match));

    array_traverse(range->items.regs, rit) {
        eflags = (start == array_data(line->chars)) ? 0 : REG_NOTBOL;
        err    = regexec(&rit->reg, start, nmatch, syntax->matches, eflags);

        if (!err) {
            m = syntax->matches + rit->group;

            /* Find the match that occurs first in the string. */
            if (m->rm_so != -1 && start + m->rm_eo <= end) {
                if (match_start == NULL || m->rm_so < first_match.rm_so) {
                    memcpy(&first_match, m, sizeof(first_match));
                    match_start = start + first_match.rm_so;
                    match_rit   = rit;
                }
            }
        }
    }

    if (match_start != NULL) {
        *len_out  = first_match.rm_eo - first_match.rm_so;
        *attr_out = match_rit->attr;
    }

    return match_start;
}

static inline const char * _yed_syntax_find_next_range_start(yed_syntax *syntax, yed_line *line, const char *start, _yed_syntax_range **range_out, int *len_out) {
    const char         *end;
    regmatch_t          match;
    const char         *match_start;
    _yed_syntax_range  *match_range;
    regmatch_t          first_match;
    _yed_syntax_range **rit;
    _yed_syntax_range  *r;
    int                 eflags;
    int                 err;

    end         = array_data(line->chars) + array_len(line->chars);
    match_start = NULL;
    match_range = NULL;

    memset(&first_match, 0, sizeof(first_match));

    array_traverse_from(syntax->ranges, rit, 1) { /* Skip global. */
        r      = *rit;
        eflags = (start == array_data(line->chars)) ? 0 : REG_NOTBOL;
        err    = regexec(&r->start, start, 1, &match, eflags);

        if (!err) {
            /* Find the match that occurs first in the string. */
            if (match.rm_so != -1) {
                if (match_start == NULL || match.rm_so < first_match.rm_so) {
                    memcpy(&first_match, &match, sizeof(first_match));
                    match_start = start + first_match.rm_so;
                    match_range = r;
                }
            }
        }
    }

    if (match_start != NULL) {
        *len_out   = first_match.rm_eo - first_match.rm_so;
        *range_out = match_range;
    }

    return match_start;
}

static inline const char * _yed_syntax_find_range_end(yed_syntax *syntax, _yed_syntax_range *range, yed_line *line, const char *start, int *len_out) {
    const char *end;
    int         nmatch;
    const char *match_start;
    regmatch_t  m;
    int         eflags;
    int         err;
    regex_t    *rit;

    end    = array_data(line->chars) + array_len(line->chars);
    nmatch = syntax->max_group + 1;

    while (start <= end) {
        match_start = NULL;
        eflags      = (start == array_data(line->chars)) ? 0 : REG_NOTBOL;
        err         = regexec(&range->end, start, nmatch, syntax->matches, eflags);

        if (!err) {
            memcpy(&m, syntax->matches, sizeof(m));

            if (m.rm_so != -1 && start + m.rm_eo <= end) {
                /* We found a match for the end. Is it in a skip? */

                array_traverse(range->skips, rit) {
                    eflags = (start == array_data(line->chars)) ? 0 : REG_NOTBOL;
                    err    = regexec(rit, start, nmatch, syntax->matches, eflags);

                    if (!err) {
                        if (m.rm_so >= syntax->matches->rm_so) {
                            /* A skip that comes before the end. Try again. */
                            start = start + syntax->matches->rm_eo;
                            goto next;
                        }
                    }
                }

                *len_out = m.rm_eo - m.rm_so;
                return start + m.rm_so;
            }
        }

        goto out;
next:;
        if (start >= end) { break; }
    }

out:;
    *len_out = 0;

    if (range->one_line) {
        return end;
    }

    return NULL;
}

static inline _yed_syntax_range *_yed_syntax_get_line_end_state(yed_syntax *syntax, yed_buffer *buffer, yed_line *line, _yed_syntax_range *start_range) {
    _yed_syntax_range *range;
    const char        *start;
    const char        *end;
    const char        *str;
    const char        *next_range_start;
    const char        *range_end_start;
    int                range_end_len;
    int                next_range_start_len;
    _yed_syntax_range *next_range;

    range            = start_range;
    start            = array_data(line->chars);
    end              = start + array_len(line->chars);
    str              = start;
    next_range_start = NULL;

    if (range != syntax->global) {
        range_end_start = _yed_syntax_find_range_end(syntax, range, line, str, &range_end_len);

        if (range_end_start == NULL) { goto out; }

        str   = range_end_start + range_end_len;
        range = syntax->global;
    }

    while ((next_range_start = _yed_syntax_find_next_range_start(syntax, line, str, &next_range, &next_range_start_len)) != NULL) {
            range = next_range;

            str = next_range_start + next_range_start_len;

            range_end_start = _yed_syntax_find_range_end(syntax, range, line, str, &range_end_len);
            if (range_end_start == NULL) {
                if (range->one_line) {
                    range = syntax->global;
                }
                break;
            }

            str   = range_end_start + range_end_len;
            range = syntax->global;
    }

out:;
    return range;
}


static inline _yed_syntax_range *_yed_syntax_line(yed_syntax *syntax, yed_line *line, array_t line_attrs, _yed_syntax_range *start_range) {
    _yed_syntax_range *range;
    const char    *str;
    const char    *start;
    const char    *line_end;
    const char    *end;
    const char    *range_end_start;
    int            range_end_len;
    int            cstart;
    int            cend;
    yed_attrs     *ait;
    int            i;
    const char    *next_kwd;
    const char    *next_range_start;
    const char    *next_match;
    _yed_syntax_attr  *next_kwd_attr;
    _yed_syntax_attr  *next_match_attr;
    _yed_syntax_attr  *a;
    int            next_kwd_len;
    int            next_match_len;
    int            next_range_start_len;
    _yed_syntax_range *next_range;
    const char    *first;
    int            first_len;

#define NOT_SEARCHED    ((void*)~(u64)NULL)
#define NEEDS_SEARCH(x) ((x) == NOT_SEARCHED || ((x) != NULL && (x) < str))

    range    = start_range;
    str      = array_data(line->chars);
    start    = str;
    line_end = start + array_len(line->chars);
    end      = line_end;

    if (range != syntax->global) {
        range_end_start = _yed_syntax_find_range_end(syntax, range, line, str, &range_end_len);

        cstart = yed_line_idx_to_col(line, str - start);
        cend   = range_end_start == NULL
                    ? line->visual_width + 1
                    : yed_line_idx_to_col(line, (range_end_start + range_end_len) - start);

        for (i = cstart; i < cend; i += 1) {
            ait = array_item(line_attrs, i - 1);
            yed_combine_attrs(ait, &range->attr->attr);
        }

        if (range_end_start == NULL) {
            return range;
        } else {
            str   = range_end_start + range_end_len;
            range = syntax->global;
        }
    }

    next_kwd         = NOT_SEARCHED;
    next_range_start = NOT_SEARCHED;
    next_match       = NOT_SEARCHED;
    next_kwd_attr    = NULL;
    next_match_attr  = NULL;

    while (str < line_end) {
        /* Want to skip early as often as we can to avoid needless searches/regexecs. */
        if      (next_kwd         == str) { goto set_kwd;   }
        else if (next_range_start == str) { goto set_range; }
        else if (next_match       == str) { goto set_match; }

        if (NEEDS_SEARCH(next_kwd)) {
            next_kwd = _yed_syntax_find_next_kwd(syntax, range, line, str, end, &next_kwd_len, &next_kwd_attr);

            /* If the next keyword is right here, skip range/regex search. */
            if (next_kwd == str) { goto set_kwd; }
        }

        if (range == syntax->global && NEEDS_SEARCH(next_range_start)) {
            next_range_start = _yed_syntax_find_next_range_start(syntax, line, str, &next_range, &next_range_start_len);

            /* If the next range is right here, skip regex search. */
            if (next_range_start == str) { goto set_range; }
        }

        if (NEEDS_SEARCH(next_match)) {
            next_match = _yed_syntax_find_next_regex_match(syntax, range, line, str, end, &next_match_len, &next_match_attr);
        }

        first     = NULL;
        first_len = 0;
        a         = NULL;

        if (next_match >= str) {
set_match:;
            first     = next_match;
            first_len = next_match_len;
            a         = next_match_attr;
        }

        if (range == syntax->global
        &&  next_range_start >= str
        &&  (next_kwd == NULL || next_range_start <= next_kwd)
        &&  (first == NULL || next_range_start <= first)) {
set_range:;
            range = next_range;

            range_end_start = _yed_syntax_find_range_end(syntax, range, line, next_range_start + next_range_start_len, &range_end_len);

            cstart = yed_line_idx_to_col(line, next_range_start - start);
            cend   = range_end_start == NULL
                        ? line->visual_width + 1
                        : yed_line_idx_to_col(line, (range_end_start + range_end_len) - start);
            for (i = cstart; i < cend; i += 1) {
                ait = array_item(line_attrs, i - 1);
                yed_combine_attrs(ait, &next_range->attr->attr);
            }

            end       = range_end_start + range_end_len;
            first     = next_range_start + MAX(next_range_start_len, 1);
            first_len = 0;

            /* Invalidate previously saved searches. */
            next_kwd = next_match = NOT_SEARCHED;
        }

        if (next_kwd >= str && (first == NULL || next_kwd <= first)) {
set_kwd:;
            first     = next_kwd;
            first_len = next_kwd_len;
            a         = next_kwd_attr;
        }

        if (first == NULL) {
            if (range == syntax->global || range_end_start == NULL) { break; } /* Nothing more to do. */

            range = syntax->global;
            str   = end;
            end   = line_end;

            /* Invalidate previously saved searches. */
            next_kwd = next_match = NOT_SEARCHED;
        } else {
            cstart = yed_line_idx_to_col(line, first - start);
            cend   = yed_line_idx_to_col(line, (first + first_len) - start);
            str    = first + first_len;

            if (a != NULL) {
                for (i = cstart; i < cend; i += 1) {
                    ait = array_item(line_attrs, i - 1);
                    yed_combine_attrs(ait, &a->attr);
                }
            }
        }
    }

    return range;

#undef NEEDS_SEARCH
#undef NOT_SEARCHED
}





/************************************************************************************/
/*                                    interface                                     */
/************************************************************************************/


static inline void yed_syntax_start(yed_syntax *syntax) {
    memset(syntax, 0, sizeof(*syntax));

    syntax->attrs      = array_make(_yed_syntax_attr*);
    syntax->attr_stack = array_make(_yed_syntax_attr*);
    syntax->ranges     = array_make(_yed_syntax_range*);

    syntax->global = malloc(sizeof(*syntax->global));
    _yed_syntax_make_range(syntax->global);
    array_push(syntax->ranges, syntax->global);

    syntax->caches = CACHE_TREE_MAKE();
}

static inline void yed_syntax_end(yed_syntax *syntax) {
    syntax->matches = malloc(sizeof(*syntax->matches) * (syntax->max_group + 1));

    syntax->finalized = 1;
}

static inline void yed_syntax_free(yed_syntax *syntax) {
    _yed_syntax_range **rit;
    _yed_syntax_attr  **ait;
    CACHE_IT            it;

    array_traverse(syntax->ranges, rit) {
        _yed_syntax_free_range(*rit);
    }
    array_free(syntax->ranges);

    array_traverse(syntax->attrs, ait) {
        free((*ait)->str);
        free(*ait);
    }
    array_free(syntax->attr_stack);
    array_free(syntax->attrs);

    if (syntax->regex_err_str != NULL) { free(syntax->regex_err_str); }

    if (syntax->matches != NULL) { free(syntax->matches); }

    tree_traverse(syntax->caches, it) {
        _yed_syntax_free_cache(&tree_it_val(it));
    }
    CACHE_TREE_FREE(syntax->caches);
}



static inline void yed_syntax_kwd(yed_syntax *syntax, const char *kwd) {
    _yed_syntax_range *range;
    _yed_syntax_kwd   *k;

    range = _yed_syntax_top_range(syntax);
    k     = _yed_syntax_kwd_set_add(&range->items.kwds, kwd);
    if (k == NULL) { return; }

    k->attr = _yed_syntax_top_attr(syntax);
}

static inline void yed_syntax_attr_push(yed_syntax *syntax, const char *str) {
    _yed_syntax_attr *a;

    a = malloc(sizeof(*a));

    a->str  = strdup(str);
    a->attr = yed_parse_attrs(str);

    array_push(syntax->attrs, a);
    array_push(syntax->attr_stack, a);
}

static inline void yed_syntax_attr_pop(yed_syntax *syntax) {
    if (array_len(syntax->attr_stack) > 0) {
        array_pop(syntax->attr_stack);
    }
}

static inline int _yed_syntax_add_regex(yed_syntax *syntax, const char *pattern, int group) {
    int                err;
    _yed_syntax_regex  r;
    size_t             err_len;
    _yed_syntax_range *range;

    memset(&r, 0, sizeof(r));

    err = regcomp(&r.reg, pattern, REG_EXTENDED);

    if (err) {
        err_len = regerror(err, &r.reg, NULL, 0);
        if (syntax->regex_err_str != NULL) { free(syntax->regex_err_str); }
        syntax->regex_err_str = malloc(err_len);
        regerror(err, &r.reg, syntax->regex_err_str, err_len);
    } else {
        range = _yed_syntax_top_range(syntax);

        r.group = group;
        r.attr  = _yed_syntax_top_attr(syntax);
        array_push(range->items.regs, r);

        if (group > syntax->max_group) {
            syntax->max_group = group;
        }
    }

    return err;
}

static inline int yed_syntax_regex(yed_syntax *syntax, const char *pattern) {
    return _yed_syntax_add_regex(syntax, pattern, 0);
}

static inline int yed_syntax_regex_sub(yed_syntax *syntax, const char *pattern, int group) {
    return _yed_syntax_add_regex(syntax, pattern, group);
}

static inline int yed_syntax_range_start(yed_syntax *syntax, const char *pattern) {
    _yed_syntax_range *range;
    int                err;
    size_t             err_len;

    if (_yed_syntax_top_range(syntax) != syntax->global) { return -1; }

    range = malloc(sizeof(*range));

    _yed_syntax_make_range(range);

    err = regcomp(&range->start, pattern, REG_EXTENDED);

    if (err) {
        err_len = regerror(err, &range->start, NULL, 0);
        if (syntax->regex_err_str != NULL) { free(syntax->regex_err_str); }
        syntax->regex_err_str = malloc(err_len);
        regerror(err, &range->start, syntax->regex_err_str, err_len);
        _yed_syntax_free_range(range);
    } else {
        range->attr = _yed_syntax_top_attr(syntax);
        array_push(syntax->ranges, range);
        syntax->range = range;
    }

    return err;
}

static inline int yed_syntax_range_end(yed_syntax *syntax, const char *pattern) {
    _yed_syntax_range *range;
    int                err;
    size_t             err_len;

    range = _yed_syntax_top_range(syntax);
    if (range == syntax->global) { return -1; }

    err = regcomp(&range->end, pattern, REG_EXTENDED);

    if (err) {
        err_len = regerror(err, &range->end, NULL, 0);
        if (syntax->regex_err_str != NULL) { free(syntax->regex_err_str); }
        syntax->regex_err_str = malloc(err_len);
        regerror(err, &range->end, syntax->regex_err_str, err_len);
        _yed_syntax_free_range(range);
    } else {
        if (!range->one_line) { syntax->needs_state = 1; }
        syntax->range = NULL;
    }

    return err;
}

static inline int yed_syntax_range_skip(yed_syntax *syntax, const char *pattern) {
    _yed_syntax_range *range;
    int                err;
    regex_t            reg;
    size_t             err_len;

    range = _yed_syntax_top_range(syntax);
    if (range == syntax->global) { return -1; }

    err = regcomp(&reg, pattern, REG_EXTENDED);

    if (err) {
        err_len = regerror(err, &reg, NULL, 0);
        if (syntax->regex_err_str != NULL) { free(syntax->regex_err_str); }
        syntax->regex_err_str = malloc(err_len);
        regerror(err, &reg, syntax->regex_err_str, err_len);
    } else {
        array_push(range->skips, reg);
    }

    return err;
}

static inline int yed_syntax_range_one_line(yed_syntax *syntax) {
    _yed_syntax_range *range;

    range = _yed_syntax_top_range(syntax);
    if (range == syntax->global) { return -1; }

    range->one_line = 1;

    return 0;
}

static inline const char * yed_syntax_get_regex_err(yed_syntax *syntax) {
    return syntax->regex_err_str;
}

static inline void yed_syntax_line_event(yed_syntax *syntax, yed_event *event) {
    yed_line          *line;
    array_t            line_attrs;
    _yed_syntax_range *start_range;

    if (!syntax->finalized) { return; }

    line = yed_buff_get_line(event->frame->buffer, event->row);
    if (line == NULL || line->visual_width == 0) { return; }

    line_attrs = event->line_attrs;

    array_zero_term(line->chars);

    start_range = _yed_syntax_get_start_state(syntax, event->frame->buffer, event->row);

    _yed_syntax_line(syntax, line, line_attrs, start_range);
}

static inline void yed_syntax_style_event(yed_syntax *syntax, yed_event *event) {
    _yed_syntax_attr **ait;
    _yed_syntax_attr  *a;

    if (!syntax->finalized) { return; }

    array_traverse(syntax->attrs, ait) {
        a       = *ait;
        a->attr = yed_parse_attrs(a->str);
    }
}

static inline void yed_syntax_buffer_delete_event(yed_syntax *syntax, yed_event *event) {
    CACHE_IT it;

    if (!syntax->finalized || !syntax->needs_state) { return; }

    it = tree_lookup(syntax->caches, event->buffer);

    if (tree_it_good(it)) {
        _yed_syntax_remove_cache(syntax, event->buffer);
    }
}

static inline void yed_syntax_buffer_mod_event(yed_syntax *syntax, yed_event *event) {
    CACHE_IT it;

    if (!syntax->finalized || !syntax->needs_state) { return; }

    it = tree_lookup(syntax->caches, event->buffer);

    if (tree_it_good(it)) {
        _yed_syntax_cache_rebuild(syntax, &tree_it_val(it), event->buffer, event->row, event->buff_mod_event);
    }
}


#undef CACHE_TREE
#undef CACHE_IT
#undef CACHE_TREE_MAKE
#undef CACHE_TREE_FREE
