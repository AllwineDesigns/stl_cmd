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
#include <libgen.h>
#include <string.h>
#include <math.h>
#include "stl_util.h"

#define BUFFER_SIZE 4096

void print_usage() {
    fprintf(stderr, "stl_bbox prints bounding box information about an STL file.\n\n");
    fprintf(stderr, "usage: stl_bbox <input file> [<input file> ...]\n");
    fprintf(stderr, "    Prints bounding box information for the given STL file.\n");
    fprintf(stderr, "    If '-' is given as a file name, stdin is read.\n");
}

int main(int argc, char** argv) {
    if (argc == 1 || (argc >= 2 && strcmp(argv[1], "--help") == 0)) {
        print_usage();
        exit(2);
    }

    int failed = 0;
    for (int arg = 1; arg < argc; arg++) {
        char *file_name = argv[arg];
        FILE* f = (strcmp(file_name, "-") == 0) ? stdin : fopen(file_name, "rb");
        if (!f) {
            fprintf(stderr, "Can't read file: %s\n", file_name);
            failed++;
        } else {
            int is_ascii = is_valid_ascii_stl(f);
            if (!is_ascii && !is_valid_binary_stl(f)) {
                fprintf(stderr, "%s is not an STL file.\n", file_name);
                failed++;
            } else {
                bounds_t b;
                get_bounds(f, &b, is_ascii);
                printf("File: %s Extents: (%f, %f, %f) - (%f, %f, %f)\n", file_name, b.min.x, b.min.y, b.min.z, b.max.x, b.max.y, b.max.z);
                printf("File: %s Dimensions: (%f, %f, %f)\n", file_name, b.max.x-b.min.x, b.max.y-b.min.y, b.max.z-b.min.z);
            }
        }
        if (f && f != stdin) {
            fclose(f);
        }
    }
    if (failed) {
        exit(2);
    }
    return 0;
}

