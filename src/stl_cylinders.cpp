/*

Copyright 2020 by Allwine Designs (stl_cmd@allwinedesigns.com)

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
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <math.h>
#include "stl_util.h"

#define BUFFER_SIZE 4096

void print_usage() {
    fprintf(stderr, "stl_cylinders outputs an STL file of a single cylinder.\n\n");
    fprintf(stderr, "usage: stl_cylinders [ -r <radius> ] [ -s <segments> ] <input file> <output file>\n");
    fprintf(stderr, "    Outputs an stl file of a cylinder with the provided radius, height and number of segments to approximate a circle.\n");
    fprintf(stderr, "    If the radius or height are omitted, they default to 1. If segments is omitted, it defaults to 32. If no output file is provided, data is sent to stdout. \n");
}

void write_cylinder(FILE *outf, const vec &pt1, const vec &pt2, float radius, int segments) {
  vec p0;
  vec p1;
  vec p2;
  vec p3;

  vec dp;
  vec x;
  vec y;
  vec z;

  dp.x = pt2.x-pt1.x;
  dp.y = pt2.y-pt1.y;
  dp.z = pt2.z-pt1.z;

  vec_normalize(&dp, &z);

  vec up = { 0, 1, 0 };
  vec_cross(&up, &z, &x);

  float magX = vec_magnitude(&x);

  if(magX < .0001) {
    x.x = 1;
    x.y = 0;
    x.z = 0;

    z.x = 0;
    z.y = 1;
    z.z = 0;
  } else {
    vec_normalize(&x, &x);
  }
  vec_cross(&z, &x, &y);

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

    float circleX = radius*cosa*x.x+radius*sina*y.x;
    float circleY = radius*cosa*x.y+radius*sina*y.y;
    float circleZ = radius*cosa*x.z+radius*sina*y.z;

    float circleX2 = radius*cosa2*x.x+radius*sina2*y.x;
    float circleY2 = radius*cosa2*x.y+radius*sina2*y.y;
    float circleZ2 = radius*cosa2*x.z+radius*sina2*y.z;

    p0.x = circleX+pt2.x;
    p0.y = circleY+pt2.y;
    p0.z = circleZ+pt2.z;

    p1.x = circleX+pt1.x;
    p1.y = circleY+pt1.y;
    p1.z = circleZ+pt1.z;

    p2.x = circleX2+pt1.x;
    p2.y = circleY2+pt1.y;
    p2.z = circleZ2+pt1.z;

    p3.x = circleX2+pt2.x;
    p3.y = circleY2+pt2.y;
    p3.z = circleZ2+pt2.z;

    write_quad(outf, &p0, &p1, &p2, &p3, 0);
  }

  for(int i = 1; i < segments-1; i++) {
    float angle = 2*M_PI*i/segments;
    float angle2 = 2*M_PI*(i+1)/segments;
    float cosa = cos(angle);
    float sina = sin(angle);

    float cosa2 = cos(angle2);
    float sina2 = sin(angle2);

    float circleX = radius*cosa*x.x+radius*sina*y.x;
    float circleY = radius*cosa*x.y+radius*sina*y.y;
    float circleZ = radius*cosa*x.z+radius*sina*y.z;

    float circleX2 = radius*cosa2*x.x+radius*sina2*y.x;
    float circleY2 = radius*cosa2*x.y+radius*sina2*y.y;
    float circleZ2 = radius*cosa2*x.z+radius*sina2*y.z;

    p0.x = radius*x.x+pt2.x;
    p0.y = radius*x.y+pt2.y;
    p0.z = radius*x.z+pt2.z;

    p1.x = circleX+pt2.x;
    p1.y = circleY+pt2.y;
    p1.z = circleZ+pt2.z;

    p2.x = circleX2+pt2.x;
    p2.y = circleY2+pt2.y;
    p2.z = circleZ2+pt2.z;

    write_tri(outf, &p0, &p1, &p2, 0);

    p0.x = radius*x.x+pt1.x;
    p0.y = radius*x.y+pt1.y;
    p0.z = radius*x.z+pt1.z;

    p1.x = circleX+pt1.x;
    p1.y = circleY+pt1.y;
    p1.z = circleZ+pt1.z;

    p2.x = circleX2+pt1.x;
    p2.y = circleY2+pt1.y;
    p2.z = circleZ2+pt1.z;

    write_tri(outf, &p0, &p1, &p2, 1);
  }
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
    int segments = 32;

    while((c = getopt(argc, argv, "r:h:s:")) != -1) {
        switch(c) {

            case 'r':
                radius = atof(optarg);
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

    FILE *inf;
    FILE *outf;

    if(optind == argc-2) {
        char *file = argv[optind];

        inf = fopen(file, "rb");
        if(!inf) {
            fprintf(stderr, "Can't read file: %s\n", file);
            exit(2);
        }

        file = argv[optind+1];

        outf = fopen(file, "wb");
        if(!outf) {
            fprintf(stderr, "Can't write to file: %s\n", file);
            exit(2);
        }
    } else {
      print_usage();
      exit(2);
    }

    char header[81] = {0};
    snprintf(header, 81, "stl_cylinders");

    fwrite(header, 80, 1, outf);

    uint32_t num_cylinders;
    size_t readBytes = fread(&num_cylinders, 4, 1, inf);

                      //    caps          tube
    uint32_t num_tris = num_cylinders*(2*(segments-2)+2*segments);

    fwrite(&num_tris, 4, 1, outf);

    uint16_t abc = 0; // attribute byte count

    vec p1;
    vec p2;

    for(uint32_t i = 0; i < num_cylinders; i++) {
      readBytes = fread(&p1.x, 4, 1, inf);
      readBytes = fread(&p1.y, 4, 1, inf);
      readBytes = fread(&p1.z, 4, 1, inf);
      readBytes = fread(&p2.x, 4, 1, inf);
      readBytes = fread(&p2.y, 4, 1, inf);
      readBytes = fread(&p2.z, 4, 1, inf);
      write_cylinder(outf, p1, p2, radius, segments);
    }

    return 0;
}

