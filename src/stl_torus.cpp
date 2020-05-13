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
    fprintf(stderr, "stl_torus outputs an STL file of a single torus.\n\n");
    fprintf(stderr, "usage: stl_torus [ -o <outer radius> ] [ -i <inner radius> ] [ -s <segments> ] [ -c <cross sectional segments> ] [ <output file> ]\n");
    fprintf(stderr, "    Outputs an stl file of a torus with the provided outer radius, inner radius and number of segments.\n");
    fprintf(stderr, "    If the inner radius is omitted, it defaults to .5. If the outer radius is omitted, it defaults to 1.\n");
    fprintf(stderr, "    If segments is omitted, it defaults to 32. If cross sectional segments is omitted, it defaults to half the segments.\n");
    fprintf(stderr, "    If no output file is provided, data is sent to stdout. \n");
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

    float innerRadius = .5;
    float outerRadius = 1;
    int segments = 32;
    int minorSegments = 16;

    int minorSegmentsSet = 0;

    while((c = getopt(argc, argv, "i:o:s:c:")) != -1) {
        switch(c) {
            case 'i':
                innerRadius = atof(optarg);
                break;
            case 'o':
                outerRadius = atof(optarg);
                break;
            case 's':
                segments = atoi(optarg);
                break;
            case 'c':
                minorSegments = atoi(optarg);
                minorSegmentsSet = 1;
                break;
            case '?':
                fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
                errflg++;
                break;
        }
    }

    if(!minorSegmentsSet) {
      minorSegments = segments/2;
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
    snprintf(header, 81, "Torus with inner radius %.4f and outer radius %.4f", innerRadius, outerRadius);

    fwrite(header, 80, 1, outf);

    uint32_t num_tris = 2*segments*minorSegments;

    fwrite(&num_tris, 4, 1, outf);

    uint16_t abc = 0; // attribute byte count

    vec p0;
    vec p1;
    vec p2;
    vec p3;
    float majorRadius = (outerRadius+innerRadius)*.5;
    float minorRadius = (outerRadius-innerRadius)*.5;
    for(int i = 0; i < segments; i++) {
      float i_angle = 2*M_PI*i/segments;
      float i_angle2 = 2*M_PI*(i+1)/segments;
      if(i == segments-1) {
        i_angle2 = 0;
      }
      float i_cosa = cos(i_angle);
      float i_sina = sin(i_angle);

      float i_cosa2 = cos(i_angle2);
      float i_sina2 = sin(i_angle2);

      for(int j = 0; j < minorSegments; j++) {
        float j_angle = 2*M_PI*j/minorSegments;
        float j_angle2 = 2*M_PI*(j+1)/minorSegments;
        if(j == minorSegments-1) {
          j_angle2 = 0;
        }
        float j_cosa = cos(j_angle);
        float j_sina = sin(j_angle);

        float j_cosa2 = cos(j_angle2);
        float j_sina2 = sin(j_angle2);

        p0.x = i_cosa*(majorRadius+minorRadius*j_cosa);
        p0.y = i_sina*(majorRadius+minorRadius*j_cosa);
        p0.z = minorRadius*j_sina;

        p1.x = i_cosa2*(majorRadius+minorRadius*j_cosa);
        p1.y = i_sina2*(majorRadius+minorRadius*j_cosa);
        p1.z = minorRadius*j_sina;

        p2.x = i_cosa2*(majorRadius+minorRadius*j_cosa2);
        p2.y = i_sina2*(majorRadius+minorRadius*j_cosa2);
        p2.z = minorRadius*j_sina2;

        p3.x = i_cosa*(majorRadius+minorRadius*j_cosa2);
        p3.y = i_sina*(majorRadius+minorRadius*j_cosa2);
        p3.z = minorRadius*j_sina2;

        write_quad(outf, &p0, &p1, &p2, &p3, 0);
      }
    }

    return 0;
}
