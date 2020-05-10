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
#include <string.h>
#include <iostream>

#include "csgjs/CSG.h"
#include "csgjs/util.h"

void print_usage() {
    fprintf(stderr, "stl_boolean performs CSG operations on two STL files.\n\n");
    fprintf(stderr, "usage: stl_boolean -a <stl file A> -b <stl file B> [ -i ] [ -u ] [ -d ] <output file>\n");
    fprintf(stderr, "    Performs a mesh CSG boolean operation on STL files A and B using BSP trees.\n"
                    "     -i - performs the intersection of A and B\n"
                    "     -u - performs the union of A and B (default)\n"
                    "     -d - performs the difference of A and B\n");
}

int main(int argc, char **argv)
{
    if(argc >= 2) {
        if(strcmp(argv[1], "--help") == 0) {
            print_usage();
            exit(2);
        }
    }
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

    csgjs::CSG A(csgjs::ReadSTLFile(a_file));
    csgjs::CSG B(csgjs::ReadSTLFile(b_file));

    if(unionAB) {
      csgjs::CSG csg = A.csgUnion(B);
      csg.canonicalize();
      csg.makeManifold();
      csgjs::WriteSTLFile(out_filename, csg.toPolygons());
    }
    if(intersection) {
      csgjs::CSG csg = A.csgIntersect(B);
      csg.canonicalize();
      csg.makeManifold();
      csgjs::WriteSTLFile(out_filename, csg.toPolygons());
    }
    if(difference) {
      csgjs::CSG csg = A.csgSubtract(B);
      csg.canonicalize();
      csg.makeManifold();
      csgjs::WriteSTLFile(out_filename, csg.toPolygons());
    }
}
