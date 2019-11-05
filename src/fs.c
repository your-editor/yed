#include "internal.h"

int yed_get_ft(char *path) {
    char *ext;

    ext = get_path_ext(path);

    if (ext) {
        if (strcmp(ext, "c") == 0
        ||  strcmp(ext, "h") == 0) {
            return FT_C;
        } else if (strcmp(ext, "sh") == 0) {
            return FT_SH;
        }
    }

    return FT_TXT;
}
