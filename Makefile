LIB_SRC=$(shell find src -name "*.c")
LIB_SRC+=$(shell find src -name "*.h")
DRIVER_SRC=src/yed_driver.c
CLEAN_SO=$(shell find . -name "*.so")
CLEAN_DSYM=$(shell find . -name "*.dSYM")
PLUG_SRC=$(shell find plugins -name "*.c")
STYLE_SRC=$(shell find plugins/styles -name "*.c")
PLUG_TARGETS=$(patsubst %.c,%.so,$(PLUG_SRC))

all: lib/libyed.so _yed plugins start
	@echo "Done."

lib/libyed.so: $(LIB_SRC)
	@echo "Compiling libyed.so.."
	@$(CC) src/yed.c $(LIB_C_FLAGS) $(cfg) -o lib/libyed.so
	@echo "Creating include directory.."
	@mkdir -p include/yed
	@cp src/*.h include/yed

_yed: lib/libyed.so $(DRIVER_SRC)
	@echo "Compiling the driver.."
	@$(CC) src/yed_driver.c $(DRIVER_C_FLAGS) $(cfg) -o _yed

.PHONY: plugins
plugins: lib/libyed.so _yed $(PLUG_TARGETS)

# The way style_pack is designed causes a bunch of macro
# redefinition warnings.
# Let's just silence those.
plugins/style_pack.so: plugins/style_pack.c $(STYLE_SRC) lib/libyed.so
	@echo "Compiling plugin $@.."
	@$(CC) $< $(PLUGIN_C_FLAGS) $(cfg) -w -Wno-error -o $@

plugins/%.so: plugins/%.c lib/libyed.so
	@echo "Compiling plugin $@.."
	@$(CC) $< $(PLUGIN_C_FLAGS) $(cfg) -o $@

.PHONY: start
start: lib/libyed.so _yed
	@echo "Compiling the configuration tutorial.."
	@cd share/start && $(CC) init.c $(PLUGIN_C_FLAGS) $(cfg) -o init.so

install:
	@./install.sh

uninstall:
	@./uninstall.sh

clean:
	@./clean.sh
