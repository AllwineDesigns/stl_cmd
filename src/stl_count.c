/*

Copyright 2014 by Freakin' Sweet Apps, LLC (stl_cmd@freakinsweetapps.com)

    This file is part of stl_cmd.

    stl_cmd is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "stl_util.h"

#define BUFFER_SIZE 4096

void print_usage() {
    fprintf(stderr, "usage: stl_count <input file>\n");
    fprintf(stderr, "    Prints the number of triangles in the provided binary STL file.\n");
}

int main(int argc, char** argv) {
    if(argc != 2) {
        print_usage();
        exit(2);
    }

    char* filename = argv[1];

    if(!is_valid_binary_stl(filename)) {
        fprintf(stderr, "stl_count only accepts binary stl files.\n");
        exit(2);
    }

    FILE *f;
    f = fopen(filename, "r+b");
    if(!f) {
        fprintf(stderr, "Can't read file: %s\n", filename);
        exit(2);
    }

    fseek(f, 80, SEEK_SET);

    uint32_t num_tris;

    fread(&num_tris, 1, 4, f);

    fclose(f);

    printf("%s has %d triangles.\n", basename(filename), num_tris);

    return 0;
}
