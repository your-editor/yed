SRC=$(shell find . -name "*.c")
STYLE_SRC=$(shell find styles -name "*.c")
TARGETS=$(patsubst %.c,%.so,$(SRC))
YED_DIR=..

all: $(TARGETS)

# The way style_pack is designed causes a bunch of macro
# redefinition warnings.
# Let's just silence those.
style_pack.so: style_pack.c $(STYLE_SRC) $(LIB_SRC)
	@echo "Compiling plugin $@.."
	@$(CC) $< $(PLUGIN_C_FLAGS) $(cfg) -w -Wno-error -o $@

%.so: %.c $(LIB_SRC)
	@echo "Compiling plugin $@.."
	@$(CC) $< $(PLUGIN_C_FLAGS) $(cfg) -o $@

clean:
	rm -rf $(TARGETS)
