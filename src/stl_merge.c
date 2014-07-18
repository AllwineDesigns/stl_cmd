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
    fprintf(stderr, "usage: stl_merge -o <out file> <in file1> <in file2>\n");
    fprintf(stderr, "    Merges two binary stl files into a single file.");
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

    if(errflg || !outflag || argc != 5) {
        print_usage();
        exit(2);
    }

    char* file1 = argv[optind];
    char* file2 = argv[optind+1];

    if(!is_valid_binary_stl(file1)) {
        fprintf(stderr, "file1 is not a binary stl file.\n");
        exit(2);
    }

    if(!is_valid_binary_stl(file2)) {
        fprintf(stderr, "file2 is not a binary stl file.\n");
        exit(2);
    }

    FILE *f1;
    FILE *f2;
    FILE *outf;

    f1 = fopen(file1, "rb");
    if(!f1) {
        fprintf(stderr, "Can't read file1: %s\n", file1);
        exit(2);
    }

    f2 = fopen(file2, "rb");
    if(!f2) {
        fprintf(stderr, "Can't read file2: %s\n", file2);
        exit(2);
    }

    outf = fopen(out_file, "wb");
    if(!outf) {
        fprintf(stderr, "Can't open out file: %s\n", out_file);
        exit(2);
    }

    fseek(f1, 80, SEEK_SET);
    fseek(f2, 80, SEEK_SET);

    uint32_t num_tris1;
    uint32_t num_tris2;

    fread(&num_tris1, 4, 1, f1);
    fread(&num_tris2, 4, 1, f2);

    char header[81] = {0}; // include an extra char for terminating \0 of snprintf
    char base1[50];
    char base2[50];
    strncpy(base1, basename(file1), 50);
    strncpy(base2, basename(file2), 50);
    snprintf(header, 81, "Merged copy of %s and %s", base1, base2);

    uint32_t merged_tris = num_tris1+num_tris2;

    fwrite(header, 80, 1, outf);
    fwrite(&merged_tris, 4, 1, outf);

    char buffer[BUFFER_SIZE];

    int r;
    while((r = fread(buffer, 1, BUFFER_SIZE, f1))) {
        fwrite(buffer, 1, r, outf);
    }
    fclose(f1);

    while((r = fread(buffer, 1, BUFFER_SIZE, f2))) {
        fwrite(buffer, 1, r, outf);
    }
    fclose(f2);
    fclose(outf);

    return 0;
}
