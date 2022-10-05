typedef struct __attribute__((__packed__)) {
    char path[4096];
    char id[32];
} _path_patch_guide;


__attribute__((used))
static
_path_patch_guide
_path_patch_guide_default_plug_dir = {
    { DEFAULT_PLUG_DIR },
    /* rot13 of "default_plug_dir" */
    {'q', 'r', 's', 'n', 'h', 'y', 'g', '_', 'c', 'y', 'h', 't', '_', 'q', 'v', 'e'}
};

__attribute__((used))
static
_path_patch_guide
_path_patch_guide_installed_lib_dir = {
    { INSTALLED_LIB_DIR },
    /* rot13 of "installed_lib_dir" */
    {'v', 'a', 'f', 'g', 'n', 'y', 'y', 'r', 'q', '_', 'y', 'v', 'o', '_', 'q', 'v', 'e'}
};

__attribute__((used))
static
_path_patch_guide
_path_patch_guide_installed_include_dir = {
    { INSTALLED_INCLUDE_DIR },
    /* rot13 of "installed_include_dir" */
    { 'v', 'a', 'f', 'g', 'n', 'y', 'y', 'r', 'q', '_', 'v', 'a', 'p', 'y', 'h', 'q', 'r', '_', 'q', 'v', 'e' }
};

__attribute__((used))
static
_path_patch_guide
_path_patch_guide_installed_share_dir = {
    { INSTALLED_SHARE_DIR },
    /* rot13 of "installed_share_dir" */
    { 'v', 'a', 'f', 'g', 'n', 'y', 'y', 'r', 'q', '_', 'f', 'u', 'n', 'e', 'r', '_', 'q', 'v', 'e' }
};


static inline
const char * default_plug_dir(void) { return _path_patch_guide_default_plug_dir.path; }

static inline
const char * installed_lib_dir(void) { return _path_patch_guide_installed_lib_dir.path; }

static inline
const char * installed_include_dir(void) { return _path_patch_guide_installed_include_dir.path; }

static inline
const char * installed_share_dir(void) { return _path_patch_guide_installed_share_dir.path; }
