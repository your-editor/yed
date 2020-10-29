void yed_init_ft(void) {
    ys->ft_array = array_make(char*);
}

int yed_make_ft(char *name) {
    char **it;
    char  *dup;
    array_traverse(ys->ft_array, it) {
        if (strcmp(*it, name) == 0) {
            return FT_ERR_TAKEN;
        }
    }
    dup = strdup(name);
    array_push(ys->ft_array, dup);
    return array_len(ys->ft_array) - 1;
}

void yed_delete_ft(char *name) {
    int ft;

    ft = yed_get_ft(name);

    if (ft == FT_ERR_NOT_FOUND) {
        return;
    }

    free(*(char**)array_item(ys->ft_array, ft));
    array_delete(ys->ft_array, ft);
}

int yed_get_ft(char *name) {
    int    idx;
    char **it;

    idx = 0;
    array_traverse(ys->ft_array, it) {
        if (strcmp(*it, name) == 0) {
            return idx;
        }
        idx += 1;
    }

    return FT_ERR_NOT_FOUND;
}

char * yed_get_ft_name(int ft) {
    if (ft < 0 || array_len(ys->ft_array) <= ft) {
        return NULL;
    }

    return *(char**)array_item(ys->ft_array, ft);
}
