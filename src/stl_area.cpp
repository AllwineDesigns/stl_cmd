/*

Copyright 2018 by Freakin' Sweet Apps, LLC (stl_cmd@freakinsweetapps.com)

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

void print_usage() {
    fprintf(stderr, "stl_area prints surface area of an STL file.\n\n");
    fprintf(stderr, "usage: stl_area <input file>\n");
    fprintf(stderr, "    Prints the total surface area of all the triangles in an STL file.\n");
}

int main(int argc, char** argv) {
    if(argc >= 2) {
        if(strcmp(argv[1], "--help") == 0) {
            print_usage();
            exit(2);
        }
    }

    char *file = argv[1];

    if(!is_valid_binary_stl(file)) {
        fprintf(stderr, "%s is not a binary stl file.\n", file);
        exit(2);
    }

    FILE *f;

    f = fopen(file, "rb");
    if(!f) {
        fprintf(stderr, "Can't read file: %s\n", file);
        exit(2);
    }

    fseek(f, 80, SEEK_SET);

    uint32_t num_tris;
    size_t readBytes = fread(&num_tris, 4, 1, f);

    float area = 0;

    for(int i = 0; i < num_tris; i++) {
        fseek(f, 12, SEEK_CUR); // normal

        vec pt1, pt2, pt3;

        readBytes = fread(&pt1, 1, 12,f);
        readBytes = fread(&pt2, 1, 12,f);
        readBytes = fread(&pt3, 1, 12,f);

        vec a,b;

        vec_sub(&pt2, &pt1, &a);
        vec_sub(&pt3, &pt1, &b);

        vec cross;

        vec_cross(&a, &b, &cross);

        float triArea = vec_magnitude(&cross)*.5;

        area += triArea;

        fseek(f, 2, SEEK_CUR);
    }

    fclose(f);

    printf("%f\n", area);

    return 0;
}
