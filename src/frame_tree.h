#ifndef __FRAME_TREE_H__
#define __FRAME_TREE_H__

#include "frame.h"
#include "array.h"

struct yed_frame_t;

#define FTREE_VSPLIT (1)
#define FTREE_HSPLIT (2)

typedef struct yed_frame_tree_t {
    float                         top, left, height, width;
    int                           discrete_rows;
    int                           discrete_cols;
    struct yed_frame_tree_t      *parent;
    int                           is_leaf;
    int                           split_kind;
    union {
        struct yed_frame_t       *frame;
        struct yed_frame_tree_t  *child_trees[2];
    };
} yed_frame_tree;

typedef void (*yed_frame_tree_leaf_do_fn_t)(yed_frame_tree*, void*);

void yed_init_frame_trees(void);
void yed_frame_tree_remove_from_tree(yed_frame_tree *tree);
void yed_frame_tree_leaves_do(yed_frame_tree *tree, yed_frame_tree_leaf_do_fn_t fn, void *arg);
yed_frame_tree *yed_frame_tree_add_root(struct yed_frame_t *f);
yed_frame_tree *yed_frame_tree_vsplit(yed_frame_tree *tree);
yed_frame_tree *yed_frame_tree_hsplit(yed_frame_tree *tree);
void yed_frame_tree_delete_leaf(yed_frame_tree *tree);
yed_frame_tree *yed_frame_tree_get_root(yed_frame_tree *tree);
int yed_frame_tree_is_root(yed_frame_tree *tree);
void yed_frame_tree_recursive_readjust(yed_frame_tree *tree);
yed_frame_tree *yed_frame_tree_get_split_leaf_prefer_left_or_topmost(yed_frame_tree *tree);
yed_frame_tree *yed_frame_tree_get_split_leaf_prefer_right_or_bottommost(yed_frame_tree *tree);
void yed_frame_tree_swap_children(yed_frame_tree *tree);
void yed_frame_tree_get_absolute_rect(yed_frame_tree *tree, float *top, float *left, float *height, float *width);
void yed_frame_tree_set_relative_rect(yed_frame_tree *tree, float top, float left, float height, float width);

#endif
