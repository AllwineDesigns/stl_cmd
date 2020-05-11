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
#include <ctype.h>
#include "stl_util.h"

#define BUFFER_SIZE 4096

void print_usage() {
    fprintf(stderr, "stl_ascii converts a binary STL file to ASCII.\n\n");
    fprintf(stderr, "usage: stl_ascii [<input file> [<output file>] ]\n");
    fprintf(stderr, "    Outputs an ASCII stl file given a binary STL file. ");
    fprintf(stderr, "    If no input file is provided, data is read from stdin. If no output file is provided, data is sent to stdout. \n");
}

int main(int argc, char** argv) {
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        print_usage();
        exit(2);
    }
    FILE *in_file = stdin;
    const char* in_file_name = "stdin";
    FILE *out_file = stdout;
    const char* out_file_name = "stdout";
    if (argc >= 2) {
        in_file_name = argv[1];
        in_file = fopen(in_file_name, "rb");
        if (!in_file) {
            fprintf(stderr, "Can't read from file: %s\n", in_file_name);
        }
    }
    if (argc >= 3) {
        out_file_name = argv[2];
        out_file = fopen(out_file_name, "w");
        if (!out_file) {
            fprintf(stderr, "Can't write to file: %s\n", out_file_name);
            fclose(in_file);
        }
    }
    char name[BUFFER_SIZE];
    memset(name, 0x00, BUFFER_SIZE);
    facet_t facet;
    uint32_t facet_count = 0;

    if (is_valid_ascii_stl(in_file)) {
        read_header(in_file, name, BUFFER_SIZE, NULL, 1);
        write_header(out_file, name, facet_count, 1);
        while (read_facet(in_file, &facet, 1)) {
            write_facet(out_file, &facet, 1);
            facet_count++;
        }
        read_final(in_file, 1);
        write_final(out_file, name, facet_count, 1);
    } else if (is_valid_binary_stl(in_file)) {
        read_header(in_file, name, BUFFER_SIZE, &facet_count, 0);
        write_header(out_file, name, facet_count, 1);
        for (int i = 0; i < facet_count; i++) {
            read_facet(in_file, &facet, 0);
            write_facet(out_file, &facet, 1);
        }
        read_final(in_file, 0);
        write_final(out_file, name, facet_count, 1);
    } else {
        fprintf(stderr, "Invalid STL file: %s\n", in_file_name);
        if (in_file != stdin) {
            fclose(in_file);
        }
        if (out_file != stdout) {
            fclose(out_file);
        }
        exit(2);
    }
    if (in_file != stdin) {
        fclose(in_file);
    }
    if (out_file != stdout) {
        fclose(out_file);
    }
    return 0;
}
