BIN_DIR := bin
CMDS := $(addprefix $(BIN_DIR)/,stl_header stl_merge stl_transform stl_count stl_bbox stl_cube stl_empty stl_threads)
CC := gcc
CFLAGS=-O3

all: $(CMDS)

$(BIN_DIR)/%: src/%.c src/stl_util.h
	$(CC) $(CFLAGS) $(OUTPUT_OPTION) $<

$(CMDS): | $(BIN_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)
