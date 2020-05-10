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

#define BUFFER_SIZE 4096

void print_usage() {
    fprintf(stderr, "stl_cylinder outputs an STL file of a single cylinder.\n\n");
    fprintf(stderr, "usage: stl_cylinder [ -r <radius> ] [ -h <height> ] [ -s <segments> ] [ <output file> ]\n");
    fprintf(stderr, "    Outputs an stl file of a cylinder with the provided radius, height and number of segments to approximate a circle.\n");
    fprintf(stderr, "    If the radius or height are omitted, they default to 1. If segments is omitted, it defaults to 32. If no output file is provided, data is sent to stdout. \n");
}

int main(int argc, char** argv) {
    if(argc >= 2) {
        if(strcmp(argv[1], "--help") == 0) {
            print_usage();
            exit(2);
        }
    }
    int errflg = 0;
    int c;

    float radius = 1;
    float height = 1;
    int segments = 32;

    while((c = getopt(argc, argv, "r:h:s:")) != -1) {
        switch(c) {

            case 'r':
                radius = atof(optarg);
                break;
            case 'h':
                height = atof(optarg);
                break;
            case 's':
                segments = atoi(optarg);
                break;
            case '?':
                fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
                errflg++;
                break;
        }
    }

    if(errflg) {
        print_usage();
        exit(2);
    }

    FILE *outf;

    if(optind == argc-1) {
        char *file = argv[optind];

        outf = fopen(file, "wb");
        if(!outf) {
            fprintf(stderr, "Can't write to file: %s\n", file);
            exit(2);
        }
    } else {
        outf = stdout;
    }

    char header[81] = {0};
    snprintf(header, 81, "Cylinder of radius %.4f and height %.4f", radius, height);

    fwrite(header, 80, 1, outf);

                      //    caps          tube
    uint32_t num_tris = 2*(segments-2)+2*segments;

    fwrite(&num_tris, 4, 1, outf);

    uint16_t abc = 0; // attribute byte count


    vec p0;
    vec p1;
    vec p2;
    vec p3;
    for(int i = 0; i < segments; i++) {
      float angle = 2*M_PI*i/segments;
      float angle2 = 2*M_PI*(i+1)/segments;
      if(i == segments-1) {
        angle2 = 0;
      }
      float cosa = cos(angle);
      float sina = sin(angle);

      float cosa2 = cos(angle2);
      float sina2 = sin(angle2);

      p0.x = radius*cosa;
      p0.y = radius*sina;
      p0.z = height*.5;

      p1.x = radius*cosa;
      p1.y = radius*sina;
      p1.z = -height*.5;

      p2.x = radius*cosa2;
      p2.y = radius*sina2;
      p2.z = -height*.5;

      p3.x = radius*cosa2;
      p3.y = radius*sina2;
      p3.z = height*.5;

      write_quad(outf, &p0, &p1, &p2, &p3, 0);
    }

    for(int i = 1; i < segments-1; i++) {
      float angle = 2*M_PI*i/segments;
      float angle2 = 2*M_PI*(i+1)/segments;
      float cosa = cos(angle);
      float sina = sin(angle);

      float cosa2 = cos(angle2);
      float sina2 = sin(angle2);

      p0.x = radius;
      p0.y = 0;
      p0.z = height*.5;

      p1.x = radius*cosa;
      p1.y = radius*sina;
      p1.z = height*.5;

      p2.x = radius*cosa2;
      p2.y = radius*sina2;
      p2.z = height*.5;

      write_tri(outf, &p0, &p1, &p2, 0);

      p0.z = -height*.5;
      p1.z = -height*.5;
      p2.z = -height*.5;

      write_tri(outf, &p0, &p1, &p2, 1);
    }

    return 0;
}
