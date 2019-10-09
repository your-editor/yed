all: vimish init

init:
	$(CC) init.c -O0 -g -shared -fPIC -Wall -Werror -Wno-unused-function -o init.so -L.. -lyed -I../src

vimish:
	$(CC) vimish.c -O0 -g -shared -fPIC -Wall -Werror -Wno-unused-function -o vimish.so -L.. -lyed -I../src

clean:
	rm -rf *.so *.so.dSYM
