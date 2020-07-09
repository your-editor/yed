char *yed_word_under_cursor(void) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_line   *line;
    int         col, word_len;
    char        c, *word_start, *ret;

    if (!ys->active_frame)    { return NULL; }
    frame = ys->active_frame;

    if (!frame->buffer)       { return NULL; }
    buff = frame->buffer;

    line = yed_buff_get_line(buff, frame->cursor_line);
    if (!line)                { return NULL; }

    col = frame->cursor_col;

    if (col == line->visual_width + 1)     { return NULL; }

    c = ((yed_glyph*)yed_line_col_to_glyph(line, col))->c;

    if (isspace(c))           { return NULL; }

    word_len = 0;

    if (isalnum(c) || c == '_') {
        while (col > 1) {
            c = ((yed_glyph*)yed_line_col_to_glyph(line, col - 1))->c;

            if (!isalnum(c) && c != '_') {
                break;
            }

            col -= 1;
        }
        word_start = array_item(line->chars, yed_line_col_to_idx(line, col));
        c = ((yed_glyph*)yed_line_col_to_glyph(line, col))->c;
        while (col <= line->visual_width
        &&    (isalnum(c) || c == '_')) {

            word_len += 1;
            col      += 1;
            c         = ((yed_glyph*)yed_line_col_to_glyph(line, col))->c;
        }
    } else {
        while (col > 1) {
            c = ((yed_glyph*)yed_line_col_to_glyph(line, col - 1))->c;

            if (isalnum(c) || c == '_' || isspace(c)) {
                break;
            }

            col -= 1;
        }
        word_start = array_item(line->chars, yed_line_col_to_idx(line, col));
        c          = ((yed_glyph*)yed_line_col_to_glyph(line, col))->c;
        while (col <= line->visual_width
        &&    (!isalnum(c) && c != '_' && !isspace(c))) {

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

char * abs_path(char *path, char *buff) {
    char exp_path[4096];

    exp_path[0] = buff[0] = 0;

    expand_path(path, exp_path);
    if (!realpath(exp_path, buff)) {
        return NULL;
    }

    return buff;
}

char * relative_path_if_subtree(char *path, char *buff) {
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
    if (!abs_path(path, a_path)) { return NULL; }

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

char * homeify_path(char *path, char *buff) {
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
        return buff;
    }

    return NULL;
}

char * get_path_ext(char *path) {
    char *ext;

    ext = strrchr(path, '.');

    if (!ext) {
        /* no extension */
    } else {
        ext += 1;
    }

    return ext;
}

char * path_without_ext(char *path) {
    char *ext;
    char *cpy;
    int   len;

    cpy = strdup(path);
    len = strlen(cpy);

    ext = get_path_ext(cpy);

    if (ext) {
        cpy[len - strlen(ext) - 1] = 0;
    }

    return cpy;
}

char *exe_path(char *prg) {
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

int file_exists_in_path(char *path, char *name) {
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

int file_exists_in_PATH(char *name) {
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

void expand_path(char *path, char *buff) {
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

array_t sh_split(char *s) {
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

    while (start < len && isspace(copy[start])) { start += 1; }

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
            &&     !isspace(copy[end + 1])) {
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

        while (start < len && isspace(copy[start])) { start += 1; }
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

    for (;;) {
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
