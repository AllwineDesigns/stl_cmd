/*

Copyright 2014 by Freakin' Sweet Apps, LLC (stl_cmd@freakinsweetapps.com)
Copyright 2026 by Orion Elenzil

Modified 2026-10-02 by Orion Elenzil:
Adapted from existing source to implement the stl_type command.

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
    fprintf(stderr, "stl_type prints whether an STL file is 'ascii' or 'binary'.\n\n");
    fprintf(stderr, "usage: stl_type [ <input file> ]\n");
    fprintf(stderr, "    Prints 'ascii' or 'binary' depending on the provided STL file.\n");
    fprintf(stderr, "    If no input file is specified, data is read from stdin.\n");
    fprintf(stderr, "    If the provided file is not a valid STL, prints an error and returns non-zero.\n");
}

int main(int argc, char** argv) {
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        print_usage();
        exit(2);
    }
    FILE *in_file = stdin;
    const char* in_file_name = "stdin";
    if (argc >= 2) {
        in_file_name = argv[1];
        in_file = fopen(in_file_name, "rb");
        if (!in_file) {
            fprintf(stderr, "Can't read from file: %s\n", in_file_name);
            exit(2);
        }
    }
    char name[BUFFER_SIZE];
    memset(name, 0x00, BUFFER_SIZE);
    facet_t facet;
    uint32_t facet_count = 0;

    if (is_valid_ascii_stl(in_file)) {
        printf("ascii\n");
    } else if (is_valid_binary_stl(in_file)) {
        printf("binary\n");
    } else {
        fprintf(stderr, "Invalid STL file: %s\n", in_file_name);
        if (in_file != stdin) {
            fclose(in_file);
        }
        exit(2);
    }

    exit(0);
}
