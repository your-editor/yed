char * yed_word_at_point(yed_frame *frame, int row, int col) {
    yed_buffer *buff;
    yed_line   *line;
    int         word_len;
    char        c, *word_start, *ret;

    if (frame == NULL || frame->buffer == NULL) { return NULL; }

    buff = frame->buffer;

    line = yed_buff_get_line(buff, row);
    if (!line) { return NULL; }

    if (col == line->visual_width + 1) { return NULL; }

    c = ((yed_glyph*)yed_line_col_to_glyph(line, col))->c;

    if (is_space(c)) { return NULL; }

    word_len = 0;

    if (is_alnum(c) || c == '_') {
        while (col > 1) {
            c = ((yed_glyph*)yed_line_col_to_glyph(line, col - 1))->c;

            if (!is_alnum(c) && c != '_') {
                break;
            }

            col -= 1;
        }
        word_start = array_item(line->chars, yed_line_col_to_idx(line, col));
        c = ((yed_glyph*)yed_line_col_to_glyph(line, col))->c;
        while (col <= line->visual_width
        &&    (is_alnum(c) || c == '_')) {

            word_len += 1;
            col      += 1;
            c         = ((yed_glyph*)yed_line_col_to_glyph(line, col))->c;
        }
    } else {
        while (col > 1) {
            c = ((yed_glyph*)yed_line_col_to_glyph(line, col - 1))->c;

            if (is_alnum(c) || c == '_' || is_space(c)) {
                break;
            }

            col -= 1;
        }
        word_start = array_item(line->chars, yed_line_col_to_idx(line, col));
        c          = ((yed_glyph*)yed_line_col_to_glyph(line, col))->c;
        while (col <= line->visual_width
        &&    (!is_alnum(c) && c != '_' && !is_space(c))) {

            word_len += 1;
            col      += 1;
            c         = ((yed_glyph*)yed_line_col_to_glyph(line, col))->c;
        }
    }

    ret = malloc(word_len + 1);
    memcpy(ret, word_start, word_len);
    ret[word_len] = 0;

    return ret;
}

char *yed_word_under_cursor(void) {
    yed_frame  *frame;

    if (!ys->active_frame)    { return NULL; }
    frame = ys->active_frame;

    return yed_word_at_point(frame, frame->cursor_line, frame->cursor_col);
}

char * abs_path(const char *path, char *buff) {
    char exp_path[4096];
    int  exp_len;
    u64  i;
    u64  k;
    u64  j;

    exp_path[0] = buff[0] = 0;

    expand_path(path, exp_path);
    exp_len = strlen(exp_path);

    if (exp_len == 0) { goto out; }

    if (exp_path[0] != '/') {
        strcat(buff, ys->working_dir);
        strcat(buff, "/");
    }

    strcat(buff, exp_path);

    /* Move to the beginning of the string */
    i = 0;
    k = 0;

    /* Replace backslashes with forward slashes */
    while (buff[i] != '\0') {
        /* Forward slash or backslash separator found? */
        if (buff[i] == '/' || buff[i] == '\\') {
            buff[k++] = '/';
            while (buff[i] == '/' || buff[i] == '\\')
                i++;
        } else {
            buff[k++] = buff[i++];
        }
    }

    /* Properly terminate the string with a NULL character */
    buff[k] = '\0';

    /* Move back to the beginning of the string */
    i = 0;
    j = 0;
    k = 0;

    /* Parse the entire string */
    do {
        /* Forward slash separator found? */
        if (buff[i] == '/' || buff[i] == '\0') {
            /* "." element found? */
            if ((i - j) == 1 && !strncmp (buff + j, ".", 1)) {
                /* Check whether the pathname is empty? */
                if (k == 0) {
                    if (buff[i] == '\0') {
                        buff[k++] = '.';
                    } else if (buff[i] == '/' && buff[i + 1] == '\0') {
                        buff[k++] = '.';
                        buff[k++] = '/';
                    }
                } else if (k > 1) {
                    /* Remove the final slash if necessary */
                    if (buff[i] == '\0')
                        k--;
                }
            }
            /* ".." element found? */
            else if ((i - j) == 2 && !strncmp (buff + j, "..", 2)) {
                /* Check whether the pathname is empty? */
                if (k == 0) {
                    buff[k++] = '.';
                    buff[k++] = '.';

                    /* Append a slash if necessary */
                    if (buff[i] == '/')
                        buff[k++] = '/';
                } else if (k > 1) {
                    /* Search the path for the previous slash */
                    for (j = 1; j < k; j++) {
                        if (buff[k - j - 1] == '/')
                            break;
                    }

                    /* Slash separator found? */
                    if (j < k) {
                        if (!strncmp (buff + k - j, "..", 2)) {
                            buff[k++] = '.';
                            buff[k++] = '.';
                        } else {
                            k = k - j - 1;
                        }

                        /* Append a slash if necessary */
                        if (k == 0 && buff[0] == '/')
                            buff[k++] = '/';
                        else if (buff[i] == '/')
                            buff[k++] = '/';
                    }
                    /* No slash separator found? */
                    else {
                        if (k == 3 && !strncmp (buff, "..", 2)) {
                            buff[k++] = '.';
                            buff[k++] = '.';

                            /* Append a slash if necessary */
                            if (buff[i] == '/')
                                buff[k++] = '/';
                        } else if (buff[i] == '\0') {
                            k = 0;
                            buff[k++] = '.';
                        } else if (buff[i] == '/' && buff[i + 1] == '\0') {
                            k = 0;
                            buff[k++] = '.';
                            buff[k++] = '/';
                        } else {
                            k = 0;
                        }
                    }
                }
            } else {
                /* Copy directory name */
                memmove (buff + k, buff + j, i - j);
                /* Advance write pointer */
                k += i - j;

                /* Append a slash if necessary */
                if (buff[i] == '/')
                    buff[k++] = '/';
            }

            /* Move to the next token */
            while (buff[i] == '/')
                i++;
            j = i;
        }
        else if (k == 0) {
            while (buff[i] == '.' || buff[i] == '/') {
                 j++,i++;
            }
        }
    } while (buff[i++] != '\0');

    /* Properly terminate the string with a NULL character */
    buff[k] = '\0';

out:;
    return buff;
}

char * relative_path_if_subtree(const char *path, char *buff) {
    int  is_subtree;
    char a_path[4096];
    int  cwd_len;

    buff[0] = 0;

    is_subtree = (path[0] != '/' && path[0] != '~');

    /*
     * If the path is already relative, then
     * we're done.
     */
    if (is_subtree) {
        strcat(buff, path);
        return buff;
    }

    /* Do expansion. */
    abs_path(path, a_path);

    /*
     * If the absolute path puts us in a subtree of
     * the current directory, then we can turn it into
     * a relative path by lopping off the current directory
     * portion of the path.
     */
    cwd_len = strlen(ys->working_dir);

    if (cwd_len >= strlen(a_path)) { goto abs; }

    if (strncmp(a_path, ys->working_dir, cwd_len) == 0) {
        strcat(buff, a_path + cwd_len + 1);
        return buff;
    }

abs:
    /* Fail safe. */
    strcat(buff, a_path);
    return buff;
}

char * homeify_path(const char *path, char *buff) {
    int   len,
          home_len;
    char *home;

    len      = strlen(path);
    home     = getenv("HOME");
    home_len = strlen(home);

    if (!home) { return NULL; }

    if (strncmp(path, home, home_len) == 0) {
        buff[0] = '~';
        memcpy(buff + 1, path + home_len, len - home_len + 1);
    } else {
        memcpy(buff, path, len + 1);
    }

    return buff;
}

const char * get_path_ext(const char *path) {
    char *ext;

    ext = strrchr(path, '.');

    if (!ext) {
        /* no extension */
    } else {
        ext += 1;
    }

    return ext;
}

const char * get_path_basename(const char *path) {
    char *slash;

    slash = strrchr(path, '/');

    if (!slash) {
        return path;
    } else {
        slash += 1;
    }

    return slash;
}

char * path_without_ext(const char *path) {
    const char *ext;
    char       *cpy;
    int         len;

    cpy = strdup(path);
    len = strlen(cpy);

    ext = get_path_ext(cpy);

    if (ext) {
        cpy[len - strlen(ext) - 1] = 0;
    }

    return cpy;
}

static char config_path_buff[4096];
static int  has_config_path;

const char * get_config_path(void) {
    char *dir;

    if (!has_config_path) {
        dir = NULL;

        if ((dir = getenv("YED_CONFIG_DIR")) != NULL) {
            expand_path(dir, config_path_buff);
        } else if ((dir = getenv("XDG_CONFIG_HOME")) != NULL) {
            expand_path(dir, config_path_buff);
            strcat(config_path_buff, "/yed");
        } else {
            expand_path("~/.config/yed", config_path_buff);
        }

        has_config_path = 1;
    }

    return config_path_buff;
}

char * get_config_item_path(const char *item) {
    char buff[4096];

    snprintf(buff, sizeof(buff), "%s/%s", get_config_path(), item);

    return strdup(buff);
}

char *exe_path(const char *prg) {
    char  cmd_buff[256];
    char *path;
    int   len;
    int   status;

    cmd_buff[0] = 0;

    strcat(cmd_buff, "which ");
    strcat(cmd_buff, prg);
    strcat(cmd_buff, " 2>/dev/null");

    path = yed_run_subproc(cmd_buff, &len, &status);

    if (path) {
        if (strlen(path) == 0) {
            free(path);
            return NULL;
        }
    }

    return path;
}

int file_exists_in_path(const char *path, const char *name) {
    char *_path, *next;
    int   found;
    int   has_next;
    char  buff[1024];

    if (!name || !path || strlen(path) == 0) {
        return 0;
    }

    found = 0;
    _path = strdup(path);
    path  = _path;

    do {
        has_next = 0;

        if ((next = strchr(path, ':'))) {
            *next     = 0;
            next     += 1;
            has_next  = 1;
        }

        if (strlen(path) == 0) { continue; }

        sprintf(buff, "%s/%s", path, name);
        if (access(buff, F_OK) == 0) {
            found = 1;
            goto out;
        }
    } while ((path = next), has_next);


out:
    free(_path);
    return found;
}

int file_exists_in_PATH(const char *name) {
    char *path;

    path = getenv("PATH");
    return file_exists_in_path(path, name);
}

int perc_subst(char *pattern, char *subst, char *buff, int buff_len) {
    int   i,
          new_len,
          patt_len,
          subst_len;
    char  c,
          last,
         *buff_p;

    i         = 0;
    new_len   = 0;
    patt_len  = strlen(pattern);
    subst_len = strlen(subst);
    buff[0]   = 0;
    buff_p    = buff;

    while (i < patt_len) {
        c = pattern[i];

        if (c == '\\') {
            /* handled later */
        } else if (c == '%') {
            if (last == '\\') {
                new_len += 1;
                if (new_len >= buff_len) {
                    return -1;
                }

                *(buff_p++) = '%';
            } else {
                new_len += subst_len;
                if (new_len >= buff_len) {
                    return -1;
                }

                *buff_p = 0;
                strcat(buff, subst);
                buff_p += subst_len;
            }
        } else {
            if (last == '\\') {
                new_len += 1;
                if (new_len >= buff_len) {
                    return -1;
                }
                *(buff_p++) = '\\';
            }
            new_len += 1;
            if (new_len >= buff_len) {
                return -1;
            }
            *(buff_p++) = c;
        }

        last  = c;
        i    += 1;
    }

    if (last == '\\') {
        *(buff_p++) = '\\';
    }
    *buff_p = 0;

    return new_len;
}

void expand_path(const char *path, char *buff) {
    int   len,
          i,
          home_len;
    char *home,
         *buff_p;
    char  c;

    len      = strlen(path);
    home     = getenv("HOME");
    home_len = strlen(home);

    if (!home) {
        memcpy(buff, path, len + 1);
        return;
    }

    buff_p = buff;

    for (i = 0; i < len; i += 1) {
        c = path[i];

        if (c == '~') {
            *buff_p = 0;
            strcat(buff, home);
            buff_p += home_len;
        } else {
            *buff_p  = c;
            buff_p  += 1;
        }
    }
    *buff_p = 0;
}

array_t sh_split(const char *s) {
    array_t  r;
    char    *copy,
            *sub,
            *sub_p;
    char     c, prev;
    int      len,
             start,
             end,
             q,
             sub_len,
             i;

    r     = array_make(char*);
    copy  = strdup(s);
    len   = strlen(copy);
    start = 0;
    end   = 0;
    prev  = 0;

    while (start < len && is_space(copy[start])) { start += 1; }

    while (start < len) {
        c   = copy[start];
        q   = 0;
        end = start;

        if (c == '#' && prev != '\\') {
            break;
        } else if (c == '"') {
            start += 1;
            prev   = copy[end];
            while (end + 1 < len
            &&    (copy[end + 1] != '"' || prev == '\\')) {
                end += 1;
                prev = copy[end];
            }
            q = 1;
        } else if (c == '\'') {
            start += 1;
            prev   = copy[end];
            while (end + 1 < len
            &&    (copy[end + 1] != '\'' || prev == '\\')) {
                end += 1;
                prev = copy[end];
            }
            q = 1;
        } else {
            while (end + 1 < len
            &&     !is_space(copy[end + 1])) {
                end += 1;
            }
        }

        sub_len = end - start + 1;
        if (q && sub_len == 0 && start == len) {
            sub    = malloc(2);
            sub[0] = copy[end];
            sub[1] = 0;
        } else {
            sub   = malloc(sub_len + 1);
            sub_p = sub;
            for (i = 0; i < sub_len; i += 1) {
                c = copy[start + i];
                if (c == '\\'
                &&  i < sub_len - 1
                &&  (copy[start + i + 1] == '"'
                || copy[start + i + 1] == '\''
                || copy[start + i + 1] == '#')) {
                    continue;
                }
                *sub_p = c;
                sub_p += 1;
            }
            *sub_p = 0;
        }

        array_push(r, sub);

        end  += q;
        start = end + 1;

        while (start < len && is_space(copy[start])) { start += 1; }
    }

    free(copy);

    return r;
}

void free_string_array(array_t array) {
    char **it;

    array_traverse(array, it) {
        free(*it);
    }

    array_free(array);
}

array_t copy_string_array(array_t array) {
    array_t   new;
    char    **it;

    new = array_make(char*);

    array_copy(new, array);

    array_traverse(new, it) {
        *it = strdup(*it);
    }

    return new;
}

char * last_strstr(const char *haystack, const char *needle) {
    char *result, *p;

    if (*needle == '\0') {
        return (char *) haystack;
    }

    result = NULL;

    for (;;) {
        p = strstr(haystack, needle);

        if (p == NULL) { break; }

        result   = p;
        haystack = p + 1;
    }

    return result;
}

char * last_strnstr(const char *haystack, const char *needle, size_t len) {
    char *result, *p, *end;

    if (*needle == '\0') {
        return (char *) haystack;
    }

    result = NULL;
    end    = (char*)haystack + len;

    while (end - haystack) {
        p = strnstr(haystack, needle, end - haystack);

        if (p == NULL) { break; }

        result   = p;
        haystack = p + 1;
    }

    return result;
}

#ifdef NEED_STRNSTR
char *strnstr(const char *haystack, const char *needle, size_t len) {
    int    i;
    size_t needle_len;

    if ((needle_len = strnlen(needle, len)) == 0) {
        return (char *)haystack;
    }

    for (i = 0; i <= (int)(len-needle_len); i += 1) {
        if ((*haystack == *needle)
        &&  (strncmp(haystack, needle, needle_len)) == 0) {
            return (char *)haystack;
        }

        haystack += 1;
    }

    return NULL;
}
#endif

int rect_intersect(int top_a, int bottom_a, int left_a, int right_a,
                   int top_b, int bottom_b, int left_b, int right_b) {

    return (left_a <= right_b && right_a >= left_b
            &&
            top_a <= bottom_b && bottom_a >= top_b);
}
