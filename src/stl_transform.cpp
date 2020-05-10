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

// TODO make input and output file optional, if not specified read from
// stdin and write to stdout. Add to other stl_cmds as well. This will
// allow for piping between stl_cmds which could elimate some temporary files:
// stl_cube -w 1 | stl_transform -tx 10 > cube.stl
// stl_cube -w 2 | stl_transform -tx 20 > cube2.stl
// stl_merge *.stl |  transform -s 10 > all_cubes_scaled.stl

void print_usage() {
    fprintf(stderr, "stl_transform performs any number of transformations to an STL file.\n\n");
    fprintf(stderr, "usage: stl_transform [[ <transformation> ] ...] <input file> <output file>\n");
    fprintf(stderr, "    Performs any number of the following transformations in\n");
    fprintf(stderr, "    the order they are listed on the command line:\n");
    fprintf(stderr, "        -rx <angle> - rotates <angle> degrees about the x-axis\n");
    fprintf(stderr, "        -ry <angle> - rotates <angle> degrees about the y-axis\n");
    fprintf(stderr, "        -rz <angle> - rotates <angle> degrees about the z-axis\n");
    fprintf(stderr, "        -s <s> - uniformly scales x, y and z by <s> (cannot be 0)\n");
    fprintf(stderr, "        -sx <x> - scales by <x> in x (cannot be 0)\n");
    fprintf(stderr, "        -sy <y> - scales by <y> in y (cannot be 0)\n");
    fprintf(stderr, "        -sz <z> - scales by <z> in z (cannot be 0)\n");
    fprintf(stderr, "        -tx <x> - translates <x> units in x\n");
    fprintf(stderr, "        -ty <y> - translates <y> units in y\n");
    fprintf(stderr, "        -tz <z> - translates <z> units in z\n");
}

int main(int argc, char** argv) {
    if(argc >= 2) {
        if(strcmp(argv[1], "--help") == 0) {
            print_usage();
            exit(2);
        }
    }
    int errflg = 0;
    int did_scale = 0;

    mat tmp;
    mat tmp2;
    mat combined;
    mat inv_combined;
    mat inv_transpose;

    init_identity_mat(&combined);
    init_identity_mat(&inv_combined);

    int index;
    float arg;
    // TODO better arg handling so better errors can be displayed
    for(index = 1; index < argc; index++) {
        if(strcmp("-rx", argv[index]) == 0) {
           index++;
           if(index >= argc) break;
           arg = atof(argv[index]);
           init_rx_mat(&tmp, arg);
           mat_copy(&combined, &tmp2);
           mat_mult(&tmp2, &tmp, &combined);

           init_inv_rx_mat(&tmp, arg);
           mat_copy(&inv_combined, &tmp2);
           mat_mult(&tmp, &tmp2, &inv_combined);
        } else if(strcmp("-ry", argv[index]) == 0) {
           index++;
           if(index >= argc) break;
           arg = atof(argv[index]);
           init_ry_mat(&tmp, arg);
           mat_copy(&combined, &tmp2);
           mat_mult(&tmp2, &tmp, &combined);

           init_inv_ry_mat(&tmp, arg);
           mat_copy(&inv_combined, &tmp2);
           mat_mult(&tmp, &tmp2, &inv_combined);
        } else if(strcmp("-rz", argv[index]) == 0) {
           index++;
           if(index >= argc) break;
           arg = atof(argv[index]);
           init_rz_mat(&tmp, arg);
           mat_copy(&combined, &tmp2);
           mat_mult(&tmp2, &tmp, &combined);

           init_inv_rz_mat(&tmp, arg);
           mat_copy(&inv_combined, &tmp2);
           mat_mult(&tmp, &tmp2, &inv_combined);
        } else if(strcmp("-s", argv[index]) == 0) {
           index++;
           if(index >= argc) break;
           did_scale = 1;
           arg = atof(argv[index]);
           if(arg == 0) {
               errflg++;
           } else {
               init_s_mat(&tmp, arg);
               mat_copy(&combined, &tmp2);
               mat_mult(&tmp2, &tmp, &combined);

               init_inv_s_mat(&tmp, arg);
               mat_copy(&inv_combined, &tmp2);
               mat_mult(&tmp, &tmp2, &inv_combined);
           }
        } else if(strcmp("-sx", argv[index]) == 0) {
           index++;
           if(index >= argc) break;
           did_scale = 1;
           arg = atof(argv[index]);
           if(arg == 0) {
               errflg++;
           } else {
               init_sx_mat(&tmp, arg);
               mat_copy(&combined, &tmp2);
               mat_mult(&tmp2, &tmp, &combined);

               init_inv_sx_mat(&tmp, arg);
               mat_copy(&inv_combined, &tmp2);
               mat_mult(&tmp, &tmp2, &inv_combined);
           }
        } else if(strcmp("-sy", argv[index]) == 0) {
           index++;
           if(index >= argc) break;
           did_scale = 1;
           arg = atof(argv[index]);
           if(arg == 0) {
               errflg++;
           } else {
               init_sy_mat(&tmp, arg);
               mat_copy(&combined, &tmp2);
               mat_mult(&tmp2, &tmp, &combined);

               init_inv_sy_mat(&tmp, arg);
               mat_copy(&inv_combined, &tmp2);
               mat_mult(&tmp, &tmp2, &inv_combined);
           }
        } else if(strcmp("-sz", argv[index]) == 0) {
           index++;
           if(index >= argc) break;
           did_scale = 1;
           arg = atof(argv[index]);
           if(arg == 0) {
               errflg++;
           } else {
               init_sz_mat(&tmp, arg);
               mat_copy(&combined, &tmp2);
               mat_mult(&tmp2, &tmp, &combined);

               init_inv_sz_mat(&tmp, arg);
               mat_copy(&inv_combined, &tmp2);
               mat_mult(&tmp, &tmp2, &inv_combined);
           }
        } else if(strcmp("-tx", argv[index]) == 0) {
           index++;
           if(index >= argc) break;
           arg = atof(argv[index]);
           init_tx_mat(&tmp, arg);
           mat_copy(&combined, &tmp2);
           mat_mult(&tmp2, &tmp, &combined);

           init_inv_tx_mat(&tmp, arg);
           mat_copy(&inv_combined, &tmp2);
           mat_mult(&tmp, &tmp2, &inv_combined);
        } else if(strcmp("-ty", argv[index]) == 0) {
           index++;
           if(index >= argc) break;
           arg = atof(argv[index]);
           init_ty_mat(&tmp, arg);
           mat_copy(&combined, &tmp2);
           mat_mult(&tmp2, &tmp, &combined);

           init_inv_ty_mat(&tmp, arg);
           mat_copy(&inv_combined, &tmp2);
           mat_mult(&tmp, &tmp2, &inv_combined);
        } else if(strcmp("-tz", argv[index]) == 0) {
           index++;
           if(index >= argc) break;
           arg = atof(argv[index]);
           init_tz_mat(&tmp, arg);
           mat_copy(&combined, &tmp2);
           mat_mult(&tmp2, &tmp, &combined);

           init_inv_tz_mat(&tmp, arg);
           mat_copy(&inv_combined, &tmp2);
           mat_mult(&tmp, &tmp2, &inv_combined);
        } else {
            break;
        }
    }

    if(errflg || index+1 >= argc) {
        print_usage();
        exit(2);
    }

    char *file = argv[index];
    char *outfile = argv[index+1];

    if(!is_valid_binary_stl(file)) {
        fprintf(stderr, "%s is not a binary stl file.\n", file);
        exit(2);
    }

    mat_transpose(&inv_combined, &inv_transpose);

    FILE *f;
    FILE *outf;

    f = fopen(file, "rb");
    if(!f) {
        fprintf(stderr, "Can't read file: %s\n", file);
        exit(2);
    }

    outf = fopen(outfile, "wb");
    if(!outf) {
        fprintf(stderr, "Can't write to file: %s\n", outfile);
        exit(2);
    }

    fseek(f, 80, SEEK_SET);

    uint32_t num_tris;

    size_t readBytes = fread(&num_tris, 4, 1, f);

    char header[81] = {0}; // include an extra char for terminating \0 of snprintf
    snprintf(header, 81, "Transformed copy of %s", basename(file));

    fwrite(header, 80, 1, outf);
    fwrite(&num_tris, 4, 1, outf);

    uint16_t abc = 0; // attribute byte count

    vec tmp_vec;
    vec normal;
    vec point;

    normal.w = 0;
    point.w = 1;

    for(int i = 0; i < num_tris; i++) {
        readBytes = fread(&normal, 1, 12,f);

        vec_mat_mult(&normal, &inv_transpose, &tmp_vec);
        if(did_scale) {
            vec_normalize(&tmp_vec, &normal);
            fwrite(&normal, 1, 12, outf);
        } else {
            fwrite(&tmp_vec, 1, 12, outf);
        }

        for(int j = 0; j < 3; j++) {
            readBytes = fread(&point, 1, 12,f);
            vec_mat_mult(&point, &combined, &tmp_vec);
            fwrite(&tmp_vec, 1, 12, outf);
        }
        readBytes = fread(&abc, 1, 2,f);
        fwrite(&abc, 1, 2,outf);
    }

    fclose(f);
    fclose(outf);

    return 0;
}
