#include "internal.h"

void yed_init_vars(void) {
    ys->vars = tree_make_c(yed_var_name_t, yed_var_val_t, strcmp);
}

void yed_set_default_vars(void) {
    yed_set_var("tab-width", "4");
    /* indent-c-disable-bs is not set by default */
}

void yed_set_var(char *var, char *val) {
    tree_it(yed_var_name_t,
            yed_var_val_t)     it;
    char                      *old_val;

    if (!var || !val) {
        return;
    }

    it = tree_lookup(ys->vars, var);

    if (!tree_it_good(it)) {
        tree_insert(ys->vars, strdup(var), strdup(val));
    } else {
        old_val = tree_it_val(it);
        tree_insert(ys->vars, var, strdup(val));
        free(old_val);
    }
}

char *yed_get_var(char *var) {
    tree_it(yed_var_name_t,
            yed_var_val_t)     it;

    if (!var) {
        return NULL;
    }

    it = tree_lookup(ys->vars, var);

    if (!tree_it_good(it)) {
        return NULL;
    }

    return tree_it_val(it);
}

void yed_unset_var(char *var) {
    tree_it(yed_var_name_t,
            yed_var_val_t)       it;
    char                        *old_var,
                                *old_val;

    if (!var) {
        return;
    }

    it = tree_lookup(ys->vars, var);

    if (!tree_it_good(it)) {
        return;
    }

    old_var = tree_it_key(it);
    old_val = tree_it_val(it);

    tree_delete(ys->vars, var);
    free(old_var);
    free(old_val);
}
