#include "yed.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>

static yed_lib_t yed_lib;
static struct yed_state_t *state;

int load_yed_lib(void);
void call_yed_fini(void);


int main(int argc, char **argv) {
    int status;

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
        } else if (status == YED_RELOAD) {
            if (load_yed_lib() != 0) {
                return 1;
            }
        }
    }
}


void call_yed_fini(void)    { yed_lib._fini(state); }

/*
 * If some doofus decides to take load libyed.so from
 * a plugin for some crazy reason, they will remove our
 * ability to do any dynamic reloading because they've
 * grabbed another reference to the library.
 * As long as that other reference exists, the system
 * won't unload our mapped image...
 *
 * So, this function will clear out essentially acquire
 * _all_ of the references to the library once by one
 * and remove them so that the ref count eventually goes
 * to zero and the library is unloaded.
 *
 * This works, but it will probably screw up whoever took
 * the additional reference. Tough.
 */
void force_yed_unload(void *handle) {
    void *try_handle;

    while ((try_handle = dlopen("libyed.so", RTLD_NOW | RTLD_NOLOAD))) {
        dlclose(try_handle);
        dlclose(handle);
    }
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

    yed_lib.handle = dlopen("libyed.so", RTLD_NOW | RTLD_LOCAL);

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
