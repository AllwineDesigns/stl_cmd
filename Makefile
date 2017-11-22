BIN_DIR := bin
CMDS := $(addprefix $(BIN_DIR)/,stl_header stl_merge stl_transform stl_count stl_bbox stl_cube stl_sphere stl_empty stl_threads stl_normals)

CC := gcc
CFLAGS=-O3
CPPFLAGS=-lstdc++

all: $(CMDS) bin/stl_boolean bin/stl_csg

$(BIN_DIR)/%: src/%.c src/stl_util.h
	$(CC) $(CFLAGS) $(OUTPUT_OPTION) $<

bin/stl_csg: src/stl_csg.cpp src/csgjs/*.cpp src/csgjs/math/*.cpp src/csgjs/math/*.h src/csgjs/*.h
	g++ -std=c++11 src/csgjs/*.cpp src/csgjs/math/*.cpp src/stl_csg.cpp -Isrc -o bin/stl_csg


bin/stl_boolean: src/stl_boolean.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) src/stl_boolean.cpp -o bin/stl_boolean 

$(CMDS): | $(BIN_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)
