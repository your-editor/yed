CFG_DEBUG=-g -O0 -DYED_DO_ASSERTIONS -DYED_DO_LOGGING
CFG_RELEASE=-O3

CFG=$(CFG_DEBUG)
# CFG=$(CFG_RELEASE)

ifeq ($(CC),clang)
	ERR_LIM=-fmax-errors=3
else
	ERR_LIM=-ferror-limit=3
endif

LIB_C_FLAGS=-shared -fPIC -Wall $(ERR_LIM) -Werror -Wno-address-of-packed-member -Wno-unused-function -ldl -lm -lpthread $(CFG)
DRV_C_FLAGS=-Wall $(ERR_LIM) -Werror -Wno-unused-function -ldl -lm -lpthread $(CFG)

all: yed_driver plugs

yed_driver: lib_yed
	$(CC) src/yed_driver.c $(DRV_C_FLAGS) -o _yed

lib_yed:
	$(CC) src/yed.c $(LIB_C_FLAGS) -o libyed.so

plugs:
	cd plugins && ./install.sh

check: lib_yed

install:
	cp _yed /usr/local/bin/yed
	cp libyed.so /usr/local/lib

clean:
	rm -rf _yed libyed.so yed.dSYM libyed.so.dSYM
	cd plugins && $(MAKE) clean
