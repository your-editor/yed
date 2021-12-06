#include "yed.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>

char lib_path[4096];

static yed_lib_t           yed_lib;
static struct yed_state_t *state;

int load_yed_lib(void);
void call_yed_fini(void);

#include "whereami.h"

int main(int argc, char **argv) {
    int   status;
    char *ld_lib_path;
    char *new_ld_lib_path;
    int   exe_path_length;
    char *exe_path;

#ifdef __APPLE__
    #define LIB_PATH_ENV_VAR "DYLD_LIBRARY_PATH"
#else
    #define LIB_PATH_ENV_VAR "LD_LIBRARY_PATH"
#endif

    if (getenv("_YED_DRIVER_CHLD") == NULL) {
        if (strlen(INSTALLED_LIB_DIR)) {
            ld_lib_path      = getenv(LIB_PATH_ENV_VAR);
            new_ld_lib_path  = malloc((ld_lib_path ? strlen(ld_lib_path) : 0) + 1 + strlen(INSTALLED_LIB_DIR) + 1);
            *new_ld_lib_path = 0;
            strcat(new_ld_lib_path, INSTALLED_LIB_DIR);
            if (ld_lib_path != NULL) {
                strcat(new_ld_lib_path, ":");
                strcat(new_ld_lib_path, ld_lib_path);
            }

            exe_path_length = wai_getExecutablePath(NULL, 0, NULL);
            if (exe_path_length == -1) {
                exe_path = argv[0];
            } else {
                exe_path        = malloc(exe_path_length + 1);
                wai_getExecutablePath(exe_path, exe_path_length, NULL);
                exe_path[exe_path_length] = 0;
            }

            setenv(LIB_PATH_ENV_VAR, new_ld_lib_path, 1);
            setenv("_YED_DRIVER_CHLD", "1", 1);
            execvp(exe_path, argv);
            return 1;
        }
    } else {
        if (strlen(INSTALLED_LIB_DIR)) {
            strcat(lib_path, INSTALLED_LIB_DIR);
            strcat(lib_path, "/");
        }
        strcat(lib_path, "libyed.so");

        if (load_yed_lib() != 0) {
            return 1;
        }

        state = yed_lib._init(&yed_lib, argc, argv);

        if (!state)    { return 1; }

        atexit(call_yed_fini);

        while (1) {
            status = yed_lib._pump();

            if (status == YED_QUIT) {
                break;
            }

#ifdef CAN_RELOAD_CORE
            else if (status == YED_RELOAD_CORE) {
                if (load_yed_lib() != 0) {
                    return 1;
                }
            }
#endif

        }
    }
}


void call_yed_fini(void)    { yed_lib._fini(state); }

/*
 * If some doofus decides to load libyed.so from
 * a plugin for some crazy reason, they will remove our
 * ability to do any dynamic reloading because they've
 * grabbed another reference to the library.
 * As long as that other reference exists, the system
 * won't unload our mapped image...
 *
 * So, this function will essentially acquire
 * _all_ of the references to the library one by one
 * and remove them so that the ref count eventually goes
 * to zero and the library is unloaded.
 *
 * This works, but it will probably screw up whoever took
 * the additional reference.
 */
void force_yed_unload(void *handle) {
#ifdef CAN_RELOAD_CORE
    void *try_handle;

    while ((try_handle = dlopen(lib_path, RTLD_NOW | RTLD_NOLOAD))) {
        dlclose(try_handle);
        dlclose(handle);
    }
#else
    dlclose(handle);
#endif
}

int load_yed_lib(void) {
#define LOAD_YED_FN(fn) do {                                    \
    yed_lib._##fn = dlsym(yed_lib.handle, "yed_" #fn);          \
    if (yed_lib._##fn == NULL) {                                \
        printf("[yed]! could not load symbol 'yed_%s'\n", #fn); \
    }                                                           \
} while (0)

    if (yed_lib.handle) {
        state = yed_lib._get_state();
        force_yed_unload(yed_lib.handle);
    }

    yed_lib.handle = dlopen(lib_path, RTLD_NOW | RTLD_LOCAL);

    if (yed_lib.handle == NULL) {
        printf("[yed]! could not load 'libyed.so'\n%s\n", dlerror());
        return 1;
    }

    LOAD_YED_FN(init);
    LOAD_YED_FN(fini);
    LOAD_YED_FN(pump);
    LOAD_YED_FN(get_state);
    LOAD_YED_FN(set_state);

    yed_lib._set_state(state);

    return 0;
}
