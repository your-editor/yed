int yed_get_ft(char *path) {
    char *copy;
    char *base;
    char *ext;
    int   ft;

    ft = FT_UNKNOWN;

    copy = strdup(path);
    base = basename(copy);
    ext  = get_path_ext(base);

    if (ext) {
        if (strcmp(ext, "c") == 0
        ||  strcmp(ext, "h") == 0) {
            ft = FT_C;
        } else if (strcmp(ext, "cpp") == 0
        ||         strcmp(ext, "hpp") == 0) {
            ft = FT_CXX;
        } else if (strcmp(ext, "sh") == 0) {
            ft = FT_SH;
        } else if (strcmp(ext, "tex") == 0) {
            ft = FT_LATEX;
        } else if (strcmp(ext, "bjou") == 0) {
            ft = FT_BJOU;
        } else if (strcmp(ext, "py") == 0) {
            ft = FT_PYTHON;
        } else if (strcmp(ext, "yedrc") == 0) {
            ft = FT_YEDRC;
        } else if (strcmp(ext, "jgr") == 0) {
            ft = FT_JGRAPH;
        } else if (strcmp(ext, "gl")   == 0
        ||         strcmp(ext, "glsl") == 0) {
            ft = FT_GLSL;
        } else if (strcmp(ext, "si") == 0) {
            ft = FT_SIMON;
        }
    } else {
        if (strcmp(base, "yedrc") == 0) {
            ft = FT_YEDRC;
        }
    }

    free(copy);

    return ft;
}
