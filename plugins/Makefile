CFG_DEBUG=-g -O0 -DYED_DO_ASSERTIONS -DYED_DO_LOGGING
CFG_RELEASE=-O3

CFG=$(CFG_DEBUG)
# CFG=$(CFG_RELEASE)

all: indent_c syntax_c vimish init

init:
	$(CC) init.c $(CFG) -shared -fPIC -Wall -Werror -Wno-unused-function -o init.so -L.. -lyed -I../src

vimish:
	$(CC) vimish.c $(CFG) -shared -fPIC -Wall -Werror -Wno-unused-function -o vimish.so -L.. -lyed -I../src

syntax_c:
	$(CC) syntax_c.c $(CFG) -shared -fPIC -Wall -Werror -Wno-unused-function -o syntax_c.so -L.. -lyed -I../src

indent_c:
	$(CC) indent_c.c $(CFG) -shared -fPIC -Wall -Werror -Wno-unused-function -o indent_c.so -L.. -lyed -I../src

clean:
	rm -rf *.so *.so.dSYM
