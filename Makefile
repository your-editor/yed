CFG_DEBUG=-g -O0 -DYED_DO_ASSERTIONS -DYED_DO_LOGGING
CFG_RELEASE=-O3

CFG=$(CFG_DEBUG)
# CFG=$(CFG_RELEASE)

ifeq ($(CC),clang)
	ERR_LIM=-fmax-errors=3
else
	ERR_LIM=-ferror-limit=3
endif

LIB_C_FLAGS=-shared -fPIC -Wall $(ERR_LIM) -Werror -Wno-unused-function -ldl -lm $(CFG)
DRV_C_FLAGS=-Wall $(ERR_LIM) -Werror -Wno-unused-function -ldl -lm $(CFG)

all: yed_driver vimish

yed_driver: lib_yed
	$(CC) src/yed_driver.c $(DRV_C_FLAGS) -o yed

lib_yed:
	$(CC) src/yed.c $(LIB_C_FLAGS) -o libyed.so

check: lib_yed vimish

vimish:
	$(CC) vimish.c -O0 -g -shared -fPIC -Wall $(ERR_LIM) -Werror -Wno-unused-function -o vimish.so -L. -lyed

clean:
	rm -rf yed libyed.so yed.dSYM libyed.so.dSYM
