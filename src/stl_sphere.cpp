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

void print_usage() {
    fprintf(stderr, "stl_sphere outputs an STL file of a single sphere.\n\n");
    fprintf(stderr, "usage: stl_sphere [-r <radius>] [-s <longitudinal segments>] [-t <latitudinal segments>] [ <output file> ]\n");
    fprintf(stderr, "    Outputs an stl file of a sphere with the provided radius and number of segments. ");
    fprintf(stderr, "    If the radius is omitted, it defaults to 1. If longitudinal segments is omitted, it defaults to 32. If latitudinal segments is omitted, it defaults to half the longitudinal segments. If no output file is provided, data is sent to stdout. \n");
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
    int s_segments = 32;
    int t_segments = 16;

    int set_t_segments = 0;

    while((c = getopt(argc, argv, "r:s:t:")) != -1) {
        switch(c) {
            case 'r':
                radius = atof(optarg);
                break;
            case 's':
                s_segments = atoi(optarg);
                break;
            case 't':
                s_segments = atoi(optarg);
                set_t_segments = 1;
                break;
            case '?':
                fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
                errflg++;
                break;
        }
    }

    if(!set_t_segments) {
      t_segments = s_segments >> 1; // divide by 2
    }

    if(t_segments < 3) {
      t_segments = 3;
    }

    if(s_segments < 3) {
      s_segments = 3;
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
    snprintf(header, 81, "Sphere with radius %.4f", radius);

    fwrite(header, 80, 1, outf);

    uint32_t num_tris = 2*s_segments*(t_segments-1);

    fwrite(&num_tris, 4, 1, outf);

    uint16_t abc = 0; // attribute byte count

    for(int i = 0; i < s_segments; i++) {
      float ti = (float)i/s_segments;
      float ti_next = (float)(i+1)/s_segments;
      float theta = ti*M_PI*2;
      float theta_next = ti_next*M_PI*2;

      float cos_theta = cos(theta);
      float sin_theta = sin(theta);

      float cos_theta_next = cos(theta_next);
      float sin_theta_next = sin(theta_next);

      for(int j = 0; j < t_segments; j++) {
        float tj = (float)j/t_segments;
        float tj_next = (float)(j+1)/t_segments;
        float phi = tj*M_PI;
        float phi_next = tj_next*M_PI;

        float cos_phi = cos(phi);
        float sin_phi = sin(phi);

        float cos_phi_next = cos(phi_next);
        float sin_phi_next = sin(phi_next);

        vec p1, p2, p3, p4;
        if(j == 0) {
          p1.x = radius*cos_theta*sin_phi;
          p1.y = radius*sin_theta*sin_phi;
          p1.z = radius*cos_phi;

          p2.x = radius*cos_theta_next*sin_phi_next;
          p2.y = radius*sin_theta_next*sin_phi_next;
          p2.z = radius*cos_phi_next;

          p3.x = radius*cos_theta*sin_phi_next;
          p3.y = radius*sin_theta*sin_phi_next;
          p3.z = radius*cos_phi_next;

          write_tri(outf, &p1, &p2, &p3, 1);
        } else if(j == t_segments-1)  {
          p1.x = radius*cos_theta*sin_phi;
          p1.y = radius*sin_theta*sin_phi;
          p1.z = radius*cos_phi;

          p2.x = radius*cos_theta_next*sin_phi;
          p2.y = radius*sin_theta_next*sin_phi;
          p2.z = radius*cos_phi;

          p3.x = radius*cos_theta*sin_phi_next;
          p3.y = radius*sin_theta*sin_phi_next;
          p3.z = radius*cos_phi_next;

          write_tri(outf, &p1, &p2, &p3, 1);
        } else {
          p1.x = radius*cos_theta*sin_phi;
          p1.y = radius*sin_theta*sin_phi;
          p1.z = radius*cos_phi;

          p2.x = radius*cos_theta_next*sin_phi;
          p2.y = radius*sin_theta_next*sin_phi;
          p2.z = radius*cos_phi;

          p3.x = radius*cos_theta_next*sin_phi_next;
          p3.y = radius*sin_theta_next*sin_phi_next;
          p3.z = radius*cos_phi_next;

          p4.x = radius*cos_theta*sin_phi_next;
          p4.y = radius*sin_theta*sin_phi_next;
          p4.z = radius*cos_phi_next;

          write_quad(outf, &p1, &p2, &p3, &p4, 1);
        }
      }
    }

    return 0;
}
