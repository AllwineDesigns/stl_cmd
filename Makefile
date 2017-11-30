prefix?=/usr/local
target=$(DESTDIR)$(prefix)

BIN_DIR := bin
CMDS := $(addprefix $(BIN_DIR)/,stl_header stl_merge stl_transform stl_count stl_bbox stl_cube stl_sphere stl_cylinder stl_cone stl_torus stl_empty stl_threads stl_normals stl_convex)

CC := g++
CFLAGS=-O3 
CCPP := g++
CPPFLAGS=-std=c++0x

all: $(CMDS) bin/stl_boolean

$(BIN_DIR)/%: src/%.cpp src/stl_util.h
	$(CC) $(CFLAGS) $(OUTPUT_OPTION) $<

bin/stl_boolean: src/stl_boolean.cpp src/csgjs/*.cpp src/csgjs/math/*.cpp src/csgjs/math/*.h src/csgjs/*.h
	$(CCPP) $(CFLAGS) $(CPPFLAGS) src/stl_boolean.cpp src/csgjs/*.cpp src/csgjs/math/*.cpp -Isrc -o bin/stl_boolean 

$(CMDS): | $(BIN_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)

install: all
	install -d $(target)/bin
	install $(BIN_DIR)/stl_header $(target)/bin/stl_header
	install $(BIN_DIR)/stl_merge $(target)/bin/stl_merge
	install $(BIN_DIR)/stl_transform $(target)/bin/stl_transform
	install $(BIN_DIR)/stl_count $(target)/bin/stl_count
	install $(BIN_DIR)/stl_bbox $(target)/bin/stl_bbox
	install $(BIN_DIR)/stl_cube $(target)/bin/stl_cube
	install $(BIN_DIR)/stl_sphere $(target)/bin/stl_sphere
	install $(BIN_DIR)/stl_cylinder $(target)/bin/stl_cylinder
	install $(BIN_DIR)/stl_torus $(target)/bin/stl_torus
	install $(BIN_DIR)/stl_empty $(target)/bin/stl_empty
	install $(BIN_DIR)/stl_threads $(target)/bin/stl_threads
	install $(BIN_DIR)/stl_normals $(target)/bin/stl_normals
	install $(BIN_DIR)/stl_boolean $(target)/bin/stl_boolean

uninstall: 
	rm $(target)/bin/stl_header
	rm $(target)/bin/stl_merge
	rm $(target)/bin/stl_transform
	rm $(target)/bin/stl_count
	rm $(target)/bin/stl_bbox
	rm $(target)/bin/stl_cube
	rm $(target)/bin/stl_sphere
	rm $(target)/bin/stl_cylinder
	rm $(target)/bin/stl_torus
	rm $(target)/bin/stl_empty
	rm $(target)/bin/stl_threads
	rm $(target)/bin/stl_normals
	rm $(target)/bin/stl_boolean
