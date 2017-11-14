BIN_DIR := bin
CMDS := $(addprefix $(BIN_DIR)/,stl_header stl_merge stl_transform stl_count stl_bbox stl_cube stl_empty stl_threads stl_normals)

CC := gcc
CFLAGS=-O3

all: $(CMDS) bin/stl_boolean

$(BIN_DIR)/%: src/%.c src/stl_util.h
	$(CC) $(CFLAGS) $(OUTPUT_OPTION) $<

bin/stl_boolean: src/stl_boolean.cpp
	gcc -lstdc++ src/stl_boolean.cpp -o bin/stl_boolean 

$(CMDS): | $(BIN_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)
