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

// TODO Add options for layouting out merged files in a row or grid
// rather than just merging.

void print_usage() {
    fprintf(stderr, "usage: stl_merge [ -o <out file> ] [ <in file1> ... ]\n");
    fprintf(stderr, "    Merges binary stl files into a single file. If no out file is provided, data is output to stdout.\n");
}

int main(int argc, char** argv) {
    int c;
    int errflg = 0;
    char *out_file;
    int outflag = 0;

    while((c = getopt(argc, argv, "o:")) != -1) {
        switch(c) {
            case 'o':
                outflag = 1;
                out_file = optarg;
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

    uint32_t num_tris = 0;

    for(int i = optind; i < argc; i++) {
        char* file = argv[i];

        char name[100];
        snprintf(name, sizeof(name), "%s", file);

        if(!is_valid_binary_stl(file)) {

            fprintf(stderr, "%s is not a binary stl file.\n", name);
            exit(2);
        }

        FILE *f;
        f = fopen(file, "rb");
        if(!f) {
            fprintf(stderr, "Can't read file: %s\n", name);
            exit(2);
        }
        fseek(f, 80, SEEK_SET);

        uint32_t nt;
        fread(&nt, 4, 1, f);

        num_tris += nt;
        fclose(f);
    }


    FILE *outf;

    if(outflag) {
        outf = fopen(out_file, "wb");

        char name[100];
        snprintf(name, sizeof(name), "%s", out_file);

        if(!outf) {
            fprintf(stderr, "Can't open out file: %s\n", name);
            exit(2);
        }
    } else {
        outf = stdout;
    }

    char header[81] = {0}; // include an extra char for terminating \0 of snprintf
    char base1[50];
    char base2[50];
    snprintf(header, 81, "Merged using stl_merge.");

    fwrite(header, 80, 1, outf);
    fwrite(&num_tris, 4, 1, outf);

    char buffer[BUFFER_SIZE];

    for(int i = optind; i < argc; i++) {
        char* file = argv[i];

        char name[100];
        snprintf(name, sizeof(name), "%s", file);

        FILE *f;
        f = fopen(file, "rb");
        if(!f) {
            fprintf(stderr, "Can't read file: %s\n", name);
            exit(2);
        }
        fseek(f, 84, SEEK_SET);

        int r;
        while((r = fread(buffer, 1, BUFFER_SIZE, f))) {
            fwrite(buffer, 1, r, outf);
        }
        fclose(f);
    }

    return 0;
}
