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
    fprintf(stderr, "usage: stl_empty <output file>\n");
    fprintf(stderr, "    Outputs an empty stl file. ");
}

int main(int argc, char** argv) {
    if(argc != 2) {
        print_usage();
        exit(2);
    }

    char *file = argv[1];
    FILE *outf;

    outf = fopen(file, "wb");
    if(!outf) {
        fprintf(stderr, "Can't write to file: %s\n", file);
        exit(2);
    }

    char header[81] = {0};
    snprintf(header, 81, "Empty STL file");

    fwrite(header, 80, 1, outf);

    uint32_t num_tris = 0;

    fwrite(&num_tris, 4, 1, outf);

    fclose(outf);

    return 0;
}
