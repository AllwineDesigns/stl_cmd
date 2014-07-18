all: bin/stl_header bin/stl_merge bin/stl_transform

bin:
	mkdir bin

bin/stl_header: src/stl_header.c src/stl_util.h bin
	gcc src/stl_header.c -o bin/stl_header

bin/stl_merge: src/stl_merge.c src/stl_util.h bin
	gcc src/stl_merge.c -o bin/stl_merge

bin/stl_transform: src/stl_transform.c src/stl_util.h bin
	gcc src/stl_transform.c -o bin/stl_transform

clean:
	rm -rf bin
