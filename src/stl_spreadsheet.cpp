/*

Copyright 2017 by Freakin' Sweet Apps, LLC (stl_cmd@freakinsweetapps.com)

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
#include <iostream>

void print_usage() {
    fprintf(stderr, "usage: stl_spreadsheet <input file>\n");
    fprintf(stderr, "    Prints vertex and normal information for every triangle in STL file in a tab delimited format that can be opened as a spreadsheet.\n");
}

int main(int argc, char** argv) {
    if(argc != 2) {
        print_usage();
        exit(2);
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
    fread(&num_tris, 4, 1, f);

    vec normal;
    vec point;

    std::cout << "normal.x\tnormal.y\tnormal.z\tpoint1.x\tpoint1.y\tpoint1.z\tpoint2.x\tpoint2.y\tpoint2.z\tpoint3.x\tpoint3.y\tpoint3.z" << std::endl;

    for(int i = 0; i < num_tris; i++) {
      fread(&normal, 1, 12,f);

      std::cout << normal.x << "\t" << normal.y << "\t" << normal.z;

      for(int j = 0; j < 3; j++) {
        fread(&point, 1, 12,f);
        std::cout << "\t" << point.x << "\t" << point.y << "\t" << point.z;
      }
      std::cout << std::endl;
      fseek(f, 2, SEEK_CUR);
    }

    fclose(f);

    return 0;
}
