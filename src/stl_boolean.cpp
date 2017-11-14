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
#include <string.h>
#include <iostream>

#include "csgjs.cpp"
#include "stl_util.h"

void ReadModel(const char *filename, csgjs_model& model) {
    FILE *f;

    f = fopen(filename, "rb");

    char header[80];
    uint32_t num_tris;
    fread(header, 80, 1, f);
    fread(&num_tris, 4, 1, f);

    uint16_t abc;
    int index = 0;
    for(int i = 0; i < num_tris; i++) {
        csgjs_vertex vertex1;
        csgjs_vertex vertex2;
        csgjs_vertex vertex3;

        fread(&vertex1.normal, 12, 1, f);
        fread(&vertex1.pos, 12, 1, f);
        fread(&vertex2.pos, 12, 1, f);
        fread(&vertex3.pos, 12, 1, f);
        fread(&abc, 2, 1, f);

        vertex2.normal = vertex1.normal;
        vertex3.normal = vertex1.normal;

        model.vertices.push_back(vertex3);
        model.vertices.push_back(vertex2);
        model.vertices.push_back(vertex1);
        model.indices.push_back(index+2);
        model.indices.push_back(index+1);
        model.indices.push_back(index);
        index += 3;
    }
    fclose(f);
}

void WriteModel(const char *filename, csgjs_model& model) {
    FILE *f;

    f = fopen(filename, "wb");

    char header[80] = {0};
    strncpy(header, "Created with stl_boolean", 80);
    fwrite(header, 80, 1, f);

    uint32_t num_tris;

    num_tris = model.indices.size()/3;
    fwrite(&num_tris, 4, 1, f);

    uint16_t abc = 0;

    for(int i = 0; i < num_tris; i++) {
        vec p1,p2,p3;

        int index1 = model.indices[3*i+0];
        int index2 = model.indices[3*i+1];
        int index3 = model.indices[3*i+2];

        p1.x = model.vertices[index1].pos.x;
        p1.y = model.vertices[index1].pos.y;
        p1.z = model.vertices[index1].pos.z;

        p2.x = model.vertices[index2].pos.x;
        p2.y = model.vertices[index2].pos.y;
        p2.z = model.vertices[index2].pos.z;

        p3.x = model.vertices[index3].pos.x;
        p3.y = model.vertices[index3].pos.y;
        p3.z = model.vertices[index3].pos.z;

        write_tri(f, &p1, &p2, &p3, 1);
    }

    fclose(f);
}

void print_usage() {
    fprintf(stderr, "usage: stl_boolean -a <stl file A> -b <stl file B> [ -i ] [ -u ] [ -d ] <output file>\n");
    fprintf(stderr, "    Performs a mesh CSG boolean operation on STL files A and B using BSP trees.\n"
                    "     -i - performs the intersection of A and B\n"
                    "     -u - performs the union of A and B (default)\n"
                    "     -d - performs the difference of A and B\n");
}

int main(int argc, char **argv)
{
    int errflg = 0;
    char *a_file;
    char *b_file;
    int a_set = 0;
    int b_set = 0;

    int unionAB = 0;
    int intersection = 0;
    int difference = 0;

    int c;

    while((c = getopt(argc, argv, "a:b:iud")) != -1) {
        switch(c) {
            case 'a':
                a_set = 1;
                a_file = optarg;
                break;
            case 'b':
                b_set = 1;
                b_file = optarg;
                break;
            case 'i':
                intersection = 1;
                if(unionAB || difference) {
                    fprintf(stderr, "intersection option provided when union or difference already specified.\n");
                    unionAB = 0;
                    difference = 0;
                }
                break;
            case 'u':
                unionAB = 1;
                if(intersection || difference) {
                    fprintf(stderr, "union option provided when intersection or difference already specified.\n");
                    intersection = 0;
                    difference = 0;
                }
                break;
            case 'd':
                difference = 1;
                if(intersection || unionAB) {
                    fprintf(stderr, "difference option provided when intersection or union already specified.\n");
                    intersection = 0;
                    unionAB = 0;
                }
                break;
            case '?':
                fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
                errflg++;
                break;
        }
    }

    if(!unionAB && !intersection && !difference) {
        unionAB = 1;
    }

    if(errflg || !(a_set && b_set) || optind >= argc) {
        print_usage();
        exit(2);
    }

    char *out_filename = argv[optind];

    csgjs_model model_a;
    csgjs_model model_b;

    ReadModel(a_file, model_a);
    ReadModel(b_file, model_b);

    if(unionAB) {
        csgjs_model m = csgjs_union(model_a, model_b);
        WriteModel(out_filename, m);
    }
    if(intersection) {
        csgjs_model m = csgjs_intersection(model_a, model_b);
        WriteModel(out_filename, m);
    }
    if(difference) {
        csgjs_model m = csgjs_difference(model_a, model_b);
        WriteModel(out_filename, m);
    }
}
