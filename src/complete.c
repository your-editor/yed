#include "internal.h"

int compute_common_prefix_len(char **items) {
    return 0;
}

int get_cmd_completion(char *in, char ***out) {
    tree_it(yed_command_name_t, yed_command)   it;
    int                                        in_len,
                                               i,
                                               n_items;
    char                                     **items,
                                              *key;

    in_len  = strlen(in);
    n_items = 0;

    /*
     * Count the number of commands that start with our 'in'
     * string. 'in' might be an empty string, in which case,
     * this method still gives us every command as a completion
     * item (which is what we want).
     */
    it  = tree_geq(ys->commands, in);
    i   = 0;
    key = tree_it_key(it);
    while (strncmp(key, in, in_len) == 0) {
        i += 1;
        tree_it_next(it);
        if (!tree_it_good(it)) {
            break;
        }
        key = tree_it_key(it);
    }

    n_items = i;

    if (n_items) {
        items = malloc(i * sizeof(char*));

        /* Now start collecting the command names. */
        it = tree_geq(ys->commands, in);
        i  = 0;
        while (i < n_items) {
            items[i] = tree_it_key(it);
            tree_it_next(it);
            i += 1;
        }
    } else {
        items = NULL;
    }

    *out = items;
    return n_items;
}

int yed_get_completion(int type, char *in, char ***out, int *common_prefix_len) {
    int    num_items;
    char **items;

    items     = NULL;
    num_items = 0;

    switch (type) {
        case COMPL_CMD:
            num_items = get_cmd_completion(in, &items);
            break;
    }

    if (items && common_prefix_len) {
        *common_prefix_len = compute_common_prefix_len(items);
    }

    if (out) {
        *out = items;
    }

    return num_items;
}
