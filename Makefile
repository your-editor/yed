LIB_SRC=$(shell find src -name "*.c")
LIB_SRC+=$(shell find src -name "*.h")
DRIVER_SRC=src/yed_driver.c
CLEAN_SO=$(shell find . -name "*.so")
CLEAN_DSYM=$(shell find . -name "*.dSYM")

all: libyed.so _yed plugins start
	@echo "Done."

libyed.so: $(LIB_SRC)
	@echo "Compiling libyed.so.."
	@$(CC) src/yed.c $(LIB_C_FLAGS) $(cfg) -o libyed.so
	@echo "Creating include directory.."
	@mkdir -p include/yed
	@cp src/*.h include/yed

_yed: libyed.so $(DRIVER_SRC)
	@echo "Compiling the driver.."
	@$(CC) src/yed_driver.c $(DRIVER_C_FLAGS) $(cfg) -o _yed

.PHONY: plugins
plugins: libyed.so _yed
	@cd plugins && $(MAKE) all

.PHONY: start
start: libyed.so _yed
	@echo "Compiling the configuration tutorial.."
	@cd share/start && $(CC) init.c $(PLUGIN_C_FLAGS) $(cfg) -o init.so

install:
	@./install.sh

uninstall:
	@./uninstall.sh

clean:
	@./clean.sh
