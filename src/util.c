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

    cmd_buff[0] = 0;

    strcat(cmd_buff, "which ");
    strcat(cmd_buff, prg);
    strcat(cmd_buff, " 2>/dev/null");

    path = yed_run_subproc(cmd_buff);

    if (path) {
        if (strlen(path) == 0) {
            free(path);
            return NULL;
        }
    }

    return path;
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

    return r;
}

void free_string_array(array_t array) {
    char **it;

    array_traverse(array, it) {
        free(*it);
    }

    array_free(array);
}
