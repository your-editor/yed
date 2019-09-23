#include "yed.h"

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>

typedef struct {
	void                *handle;
	struct yed_state_t* (*_init)(int, char**);
	void                (*_fini)(struct yed_state_t*);
    int                 (*_pump)(void);
    struct yed_state_t* (*_get_state)(void);
    void                (*_set_state)(struct yed_state_t*);
} yed_lib_t;

yed_lib_t yed_lib;
struct yed_state_t *state;

int load_yed_lib(void);
void call_yed_fini(void);


int main(int argc, char **argv) {
	int                 status;
    struct yed_state_t *state;

	if (load_yed_lib() != 0) {
        return 1;
    }

	state = yed_lib._init(argc, argv);
    atexit(call_yed_fini);

	if (!state)    { return 1; }

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

int load_yed_lib(void) {

#define LOAD_YED_FN(fn) do {                                    \
	yed_lib._##fn = dlsym(yed_lib.handle, "yed_" #fn);          \
	if (yed_lib._##fn == NULL) {                                \
		printf("[yed]! could not load symbol 'yed_%s'\n", #fn); \
	}                                                           \
} while (0)

    if (yed_lib.handle) {
        state = yed_lib._get_state();
        dlclose(yed_lib.handle);
    }

	yed_lib.handle = dlopen("./libyed.so", RTLD_NOW);

	if (yed_lib.handle == NULL) {
		printf("[yed]! could not load 'libyed.so'\n");
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
