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
    fprintf(stderr, "stl_count prints the number of triangles in an STL file.\n\n");
    fprintf(stderr, "usage: stl_count [ <input file> ]\n");
    fprintf(stderr, "    Prints the number of triangles in the provided binary STL file. If no input file is specified, data is read from stdin.\n");
}

int main(int argc, char** argv) {
    if(argc >= 2) {
        if(strcmp(argv[1], "--help") == 0) {
            print_usage();
            exit(2);
        }
    }
    if(argc == 2) {
        FILE *f;
        char* filename = argv[1];

        if(!is_valid_binary_stl(filename)) {
            fprintf(stderr, "stl_count only accepts binary stl files.\n");
            exit(2);
        }
        f = fopen(filename, "r+b");
        if(!f) {
            fprintf(stderr, "Can't read file: %s\n", filename);
            exit(2);
        }
        fseek(f, 80, SEEK_SET);

        uint32_t num_tris;

        if(fread(&num_tris, 1, 4, f) < 4) {
            fprintf(stderr, "invalid binary stl file\n");
            exit(2);
        }

        fclose(f);
        printf("%d\n", num_tris);
    } else if(argc > 2) {
        print_usage();
        exit(2);
    } else {
        off_t bights_read = 0;
        while(bights_read < 80 && getc(stdin) != EOF) {
            bights_read++;
        }
        if(bights_read < 80) {
            fprintf(stderr, "invalid binary stl file\n");
            exit(2);
        }
        // got through header

        bights_read = 0;
        uint8_t buff[4];

        int c;
        // now read num_tris
        while(bights_read < 4 && (c = getc(stdin)) != EOF) {
            buff[bights_read] = c;
            bights_read++;
        }

        if(bights_read < 4) {
            fprintf(stderr, "invalid binary stl file\n");
            exit(2);
        }

        uint32_t num_tris = *((uint32_t*)buff);

        // read the rest of the file
        while(bights_read < num_tris*16 && getc(stdin) != EOF) {
            bights_read++;
        }

        if(bights_read < num_tris*16) {
            fprintf(stderr, "invalid binary stl file\n");
            exit(2);
        }

        printf("%d\n", num_tris);
    }

    return 0;
}
