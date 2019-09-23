CFG_DEBUG=-g -O0 -DYED_DO_ASSERTIONS -DYED_DO_LOGGING
CFG_RELEASE=-O3

# CFG=$(CFG_DEBUG)
CFG=$(CFG_RELEASE)

ERR_LIM=-fmax-errors=3
# ERR_LIM=-ferror-limit=3

LIB_C_FLAGS=-shared -fPIC -Wall $(ERR_LIM) -Werror -Wno-unused-function -ldl $(CFG)
DRV_C_FLAGS=-Wall $(ERR_LIM) -Werror -Wno-unused-function -ldl $(CFG)

all: yed_driver

yed_driver: lib_yed
	$(CC) src/yed_driver.c $(DRV_C_FLAGS) -o yed

lib_yed:
	$(CC) src/yed.c $(LIB_C_FLAGS) -o libyed.so

check: lib_yed

clean:
	rm -rf
