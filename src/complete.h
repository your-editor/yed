#ifndef __COMPLETE_H__
#define __COMPLETE_H__


#define COMPL_ERR_NO_ERR   (0)
#define COMPL_ERR_NO_COMPL (1)
#define COMPL_ERR_NO_MATCH (2)

typedef struct yed_completion_results_t {
    array_t strings;
    int     common_prefix_len;
} yed_completion_results;

void yed_init_completions(void);
void yed_set_default_completions(void);
int yed_complete(char *compl_name, char *string, yed_completion_results *results);
int yed_complete_multiple(int n, char **compl_names, char *string, yed_completion_results *results);

void yed_set_completion(char *name, yed_completion comp);
void yed_unset_completion(char *name);
yed_completion yed_get_completion(char *name);

#define FN_BODY_FOR_COMPLETE_FROM_TREE(_str, _t, _it, _res, _stat)      \
{                                                                       \
int   _len;                                                             \
char *_key, *_cpy;                                                      \
                                                                        \
(_stat) = COMPL_ERR_NO_ERR;                                             \
if ((_str) == NULL) {                                                   \
    (_stat) = COMPL_ERR_NO_MATCH;                                       \
    goto _out;                                                          \
}                                                                       \
                                                                        \
_len  = strlen((_str));                                                 \
(_it) = tree_gtr((_t), (_str));                                         \
                                                                        \
if (!tree_it_good((_it))) {                                             \
    (_stat) = COMPL_ERR_NO_MATCH;                                       \
    goto _out;                                                          \
}                                                                       \
                                                                        \
array_clear((_res)->strings);                                           \
array_grow_if_needed_to((_res)->strings, tree_len((_t)));               \
                                                                        \
_key = tree_it_key((_it));                                              \
while (strncmp(_key, (_str), _len) == 0) {                              \
    _cpy = strdup(_key);                                                \
    array_push((_res)->strings, _cpy);                                  \
    tree_it_next((_it));                                                \
    if (!tree_it_good((_it))) { break; }                                \
    _key = tree_it_key((_it));                                          \
}                                                                       \
                                                                        \
_out:;                                                                  \
}

#define FN_BODY_FOR_COMPLETE_FROM_ARRAY(_str, _n, _ptr, _res, _stat)    \
{                                                                       \
tree(str_t, empty_t)     _t_;                                           \
tree_it(str_t, empty_t)  _it_;                                          \
int                      _i_;                                           \
char                    *_key_;                                         \
                                                                        \
_t_ = tree_make(str_t, empty_t);                                        \
                                                                        \
for (_i_ = 0; _i_ < (_n); _i_ += 1) {                                   \
    _it_ = tree_lookup(_t_, (_ptr)[_i_]);                               \
    if (!tree_it_good(_it_)) {                                          \
        tree_insert(_t_, strdup((_ptr)[_i_]), (empty_t){});             \
    }                                                                   \
}                                                                       \
                                                                        \
FN_BODY_FOR_COMPLETE_FROM_TREE((_str), (_t_), (_it_), (_res), (_stat)); \
                                                                        \
while (tree_len(_t_)) {                                                 \
    _it_  = tree_begin(_t_);                                            \
    _key_ = tree_it_key(_it_);                                          \
    tree_delete(_t_, _key_);                                            \
    free(_key_);                                                        \
}                                                                       \
tree_free(_t_);                                                         \
}

#endif
