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
#include "stl_util.h"

#define BUFFER_SIZE 4096

void print_usage() {
    fprintf(stderr, "usage: stl_normals [ -c ] [ -r ] [ -v ] <in file> [ <out file> ]\n");
    fprintf(stderr, "    Checks the stored normals against calculated normals based on ordering of vertices.\n"
                    "     -c - if present will ignore the present normal values and calculate them based on the vertex ordering.\n"
                    "     -r - if present will reverse the winding order of the vertices.\n"
                    "     -v - be verboze when printing out differing normals\n");
}

int main(int argc, char** argv) {
    int c;
    int errflg = 0;
    char *out_file;
    char *in_file;
    int calc = 0;
    int reverse = 0;
    int needs_out = 0;
    int verbose = 0;

    if(argc == 1) {
        print_usage();
        exit(2);
    }

    while((c = getopt(argc, argv, "vcr")) != -1) {
        switch(c) {
            case 'c':
                calc = 1;
                needs_out = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'r':
                reverse = 1;
                needs_out = 1;
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

    if(optind < argc) {
        in_file = argv[optind];

        char name[100];
        snprintf(name, sizeof(name), "%s", in_file);

        if(!is_valid_binary_stl(in_file)) {
            fprintf(stderr, "%s is not a binary stl file.\n", name);
            exit(2);
        }

        inf = fopen(in_file, "rb");
        if(!inf) {
            fprintf(stderr, "Can't read file: %s\n", name);
            exit(2);
        }
        fseek(inf, 80, SEEK_SET); // skip header

        if(optind+1 < argc) {
            out_file = argv[optind+1];

            snprintf(name, sizeof(name), "%s", out_file);

            outf = fopen(out_file, "wb");
            if(!outf) {
                fprintf(stderr, "Can't write to file: %s\n", name);
            }
        } else {
            outf = stdout;
        }
    } else {
        print_usage();
        exit(2);
    }

    fseek(inf, 0, SEEK_SET);

    char header[80] = {0};
    fread(header, 1, 80, inf);

    uint16_t zero = 0;
    uint16_t abc;
    uint32_t num_tris;
    fread(&num_tris, 4, 1, inf);

    if(needs_out) {
        fwrite(header, 1, 80, outf);
        fwrite(&num_tris, 1, 4, outf);
    }

    vec p1,p2,p3;
    vec n,cn;
    vec diff;
    p1.w = 1;
    p2.w = 1;
    p3.w = 1;
    n.w = 0;
    cn.w = 0;

    vec d1,d2;
    d1.w = 0;
    d2.w = 0;

    int match = 1;

    if(verbose) {
        fprintf(stderr, "reading %d triangles and normals\n", num_tris);
    }
    for(int i = 0; i < num_tris; i++) {
        fread(&n, 1, 12, inf);
        fread(&p1, 1, 12, inf);
        fread(&p2, 1, 12, inf);
        fread(&p3, 1, 12, inf);
        fread(&abc, 1, 2, inf);

        vec_sub(&p2, &p1, &d1);
        vec_sub(&p3, &p1, &d2);
        vec_cross(&d1, &d2, &cn);
        if(cn.x != 0 ||
            cn.y != 0 ||
             cn.z != 0) {
            vec_normalize(&cn, &cn);
        }
        vec_sub(&cn, &n, &diff);
        if(!(diff.x < EPSILON && diff.x > -EPSILON &&
            diff.y < EPSILON && diff.y > -EPSILON &&
              diff.z < EPSILON && diff.z > -EPSILON)) {
            match = 0;
            if(verbose) {
                fprintf(stderr, "calculated normal %d different than input normal\n", i);
                fprintf(stderr, "calculated: %f, %f, %f\n", cn.x, cn.y, cn.z);
                fprintf(stderr, "input: %f, %f, %f\n", n.x, n.y, n.z);
            }
        }

        if(needs_out) {
            if(calc) {
                if(reverse) {
                    cn.x *= -1;
                    cn.y *= -1;
                    cn.z *= -1;
                }
                fwrite(&cn, 1, 12, outf);
            } else {
                fwrite(&n, 1, 12, outf);
            }

            if(reverse) {
                fwrite(&p3, 1, 12, outf);
                fwrite(&p2, 1, 12, outf);
                fwrite(&p1, 1, 12, outf);
            } else {
                fwrite(&p1, 1, 12, outf);
                fwrite(&p2, 1, 12, outf);
                fwrite(&p3, 1, 12, outf);
            }
            fwrite(&abc, 1, 2, outf);
        }
    }

    if(!needs_out) {
        if(match) {
            fprintf(stderr, "Normals match calculated normals.\n");
        } else {
            fprintf(stderr, "Normals do NOT match calculated normals.\n");
        }
    }

    return 0;
}
