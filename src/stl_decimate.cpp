/*

Copyright 2019 by Allwine Designs, LLC (stl_cmd@allwinedesigns.com)

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
#include "Simplify.h"

#define BUFFER_SIZE 4096

void print_usage() {
    fprintf(stderr, "stl_decimate simplifies the provided STL file using Sven Forstmann's implementation of Fast Quadric Mesh Simplification: https://github.com/sp4cerat/Fast-Quadric-Mesh-Simplification\n\n");
    fprintf(stderr, "usage: stl_decimate [ -t <target triangle count> ] [ -p <percentage of triangles to keep> ] <input file> <output file>\n");
    fprintf(stderr, "    Outputs a simplified version of the input mesh. \n");
    fprintf(stderr, "    -t is used to specifiy an exact triangle count (an integer). \n");
    fprintf(stderr, "    -p is used to specifiy a fraction of the starting triangle count (a floating point number between 0 and 1). \n");
    fprintf(stderr, "    If both -t and -p are used, the smaller number of triangles is used. \n");
    fprintf(stderr, "    If neither are provided, -p defaults to .5. \n");
}

int main(int argc, char** argv) {
    if(argc == 2) {
        if(strcmp(argv[1], "--help") == 0) {
            print_usage();
            exit(2);
        }
    }
    int errflg = 0;
    int c;

    int target_count = -1;
    float percentage = -1;
    bool verbose = false;

    while((c = getopt(argc, argv, "t:p:v")) != -1) {
        switch(c) {
            case 'v':
              verbose = true;
              break;
            case 't':
                target_count = atoi(optarg);
                break;
            case 'p':
                percentage = atof(optarg);
                break;
            case '?':
                fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
                errflg++;
                break;
        }
    }

    if(errflg ||
       optind+1 >= argc) {
        print_usage();
        exit(2);
    }

    Simplify::load_stl(argv[optind]);

    if(percentage > -1 && target_count > -1) {
      target_count = min(percentage*Simplify::triangles.size(), target_count);
    } else if(percentage > -1) {
      target_count = percentage*Simplify::triangles.size();
    } else if(target_count > -1) {
      target_count = min(Simplify::triangles.size(), target_count);
    } else {
      target_count = .5*Simplify::triangles.size();
    }

    Simplify::simplify_mesh(target_count, 7, verbose);
    Simplify::write_stl(argv[optind+1]);

    return 0;
}
