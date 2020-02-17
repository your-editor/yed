SRC=$(shell find . -name "*.c")
TARGETS=$(patsubst %.c,%.so,$(SRC))
YED_DIR=..

all: $(TARGETS)

%.so: %.c $(LIB_SRC)
	@echo "Compiling plugin $@.."
	@$(CC) $< $(PLUGIN_C_FLAGS) $(cfg) -o $@

clean:
	rm -rf $(TARGETS)
