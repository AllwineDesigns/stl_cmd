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
    fprintf(stderr, "stl_zero centres an STL file.\n\n");
    fprintf(stderr, "usage: stl_zero [-base] [<input file> [<output file>] ]\n");
    fprintf(stderr, "    Centre an STL file around the origin. ");
    fprintf(stderr, "    If no input file is provided, data is read from stdin. If no output file is provided, data is sent to stdout. \n");
    fprintf(stderr, "        -base - If this is specified, set the lowest point in the design to z = 0. \n");
}

int main(int argc, char** argv) {
    FILE *in_file = stdin;
    const char* in_file_name = "stdin";
    FILE *out_file = stdout;
    const char* out_file_name = "stdout";
    int do_base = 0;
    int done_flags = 0;

    for (int arg = 1; arg < argc; arg++) {
        if (!done_flags) {
            if (argv[arg][0] == '-') {
                if (strcmp(argv[arg], "--help") == 0) {
                    print_usage();
                    exit(2);
                } else if (strcmp(argv[arg], "--") == 0) {
                    done_flags = 1;
                    continue;
                } else if (strcmp(argv[arg], "-base") == 0) {
                    do_base = 1;
                    continue;
                } else {
                    fprintf(stderr, "Unrecognized argument: %s\n", argv[arg]);
                    print_usage();
                    exit(2);
                }
            } else {
                done_flags = 1;
            }
        }

        if (in_file == stdin) {
            in_file_name = argv[arg];
            in_file = fopen(in_file_name, "rb");
            if (!in_file) {
                fprintf(stderr, "Can't read from file: %s\n", in_file_name);
                exit(2);
            }
        } else if (out_file == stdout) {
            out_file_name = argv[arg];
            out_file = fopen(out_file_name, "w");
            if (!out_file) {
                fprintf(stderr, "Can't write to file: %s\n", out_file_name);
                fclose(in_file);
                exit(2);
            }
        } else {
            fprintf(stderr, "Too many arguments!\n");
            print_usage();
            if (in_file != stdin) {
                fclose(in_file);
            }
            if (out_file != stdout) {
                fclose(out_file);
            }
            exit(2);
        }
    }

    int failed = 0;
    int is_ascii = is_valid_ascii_stl(in_file);
    if (!is_ascii && !is_valid_binary_stl(in_file)) {
        fprintf(stderr, "%s is not an STL file.\n", in_file_name);
        failed++;
    } else {
        bounds_t b;
        get_bounds(in_file, &b, is_ascii);
        double x_centre = (b.min.x + b.max.x) / 2.0;
        double y_centre = (b.min.y + b.max.y) / 2.0;
        double z_centre = (b.min.z + b.max.z) / 2.0;
        vec translation;
        translation.x = -x_centre;
        translation.y = -y_centre;
        translation.z = do_base ? -b.min.z : -z_centre;
        translation.w = 0;

        fseek(in_file, 0, SEEK_SET);
        char name[BUFFER_SIZE];
        memset(name, 0x00, BUFFER_SIZE);
        facet_t facet;
        facet_t translated_facet;
        uint32_t facet_count = 0;

        read_header(in_file, name, BUFFER_SIZE, &facet_count, is_ascii);
        write_header(out_file, name, facet_count, is_ascii);
        for (int i = 0; is_ascii || i < facet_count; i++) {
            if (read_facet(in_file, &facet, is_ascii)) {
                translated_facet.normal = facet.normal;
                for (int j = 0; j < 3; j++) {
                    vec_add(&facet.vertices[j], &translation, &translated_facet.vertices[j]); 
                }
                write_facet(out_file, &translated_facet, is_ascii);
                if (is_ascii) {
                    facet_count++;
                }
            } else {
                break;
            }
        }
        read_final(in_file, is_ascii);
        write_final(out_file, name, facet_count, is_ascii);
    }
    if (in_file != stdin) {
        fclose(in_file);
    }
    if (out_file != stdout) {
        fclose(out_file);
    }
    if (failed) {
        exit(2);
    }
    return 0;
}
