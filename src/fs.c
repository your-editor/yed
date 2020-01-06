int yed_get_ft(char *path) {
    char *ext;

    ext = get_path_ext(path);

    if (ext) {
        if (strcmp(ext, "c") == 0
        ||  strcmp(ext, "h") == 0) {
            return FT_C;
        } else if (strcmp(ext, "cpp") == 0
        ||         strcmp(ext, "hpp") == 0) {
            return FT_CXX;
        } else if (strcmp(ext, "sh") == 0) {
            return FT_SH;
        } else if (strcmp(ext, "tex") == 0) {
            return FT_LATEX;
        } else if (strcmp(ext, "bjou") == 0) {
            return FT_BJOU;
        } else if (strcmp(ext, "py") == 0) {
            return FT_PYTHON;
        }
    }

    return FT_TXT;
}
