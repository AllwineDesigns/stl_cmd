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
#include "stl_util.h"

#define BUFFER_SIZE 4096

void print_usage() {
    fprintf(stderr, "usage: stl_header [-s <header>] [-o <output file>] <input file>\n");
    fprintf(stderr, "    If both -s and -o flags are specified <input file> is copied to <output\n");
    fprintf(stderr, "    file> and its header is set to <header>. If -o is not specified and -s is\n");
    fprintf(stderr, "    then the header is set on the input file. If neither -o nor -s is specified,\n");
    fprintf(stderr, "    the header of the model is output to the terminal.\n");
}

int main(int argc, char** argv) {
    if(argc >= 2) {
        if(strcmp(argv[1], "--help") == 0) {
            print_usage();
            exit(2);
        }
    }
    int c;
    int errflg = 0;
    char *set_header;
    char *out_file;
    int setflag = 0;
    int outflag = 0;

    while((c = getopt(argc, argv, "s:o:")) != -1) {
        switch(c) {
            case 'o':
                outflag = 1;
                out_file = optarg;
                break;
            case 's':
                setflag = 1;
                set_header = optarg;
                break;
            case '?':
                fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
                errflg++;
                break;
        }
    }

    if(errflg || optind >= argc) {
        print_usage();
        exit(2);
    }

    char* filename = argv[optind];
    char header[80];

    if(!is_valid_binary_stl(filename)) {
        fprintf(stderr, "stl_header only accepts binary stl files.\n");
        exit(2);
    }

    FILE *f;
    f = fopen(filename, "r+b");
    if(!f) {
        fprintf(stderr, "Can't read file: %s\n", filename);
        exit(2);
    }

    if(setflag && outflag) {
        strncpy(header, set_header, 80);
        FILE *outf;
        outf = fopen(out_file, "wb");
        if(!outf) {
            fprintf(stderr, "Can't write to file: %s\n", out_file);
            exit(2);
        }
        fwrite(header, 80, 1, outf);
        fseek(f, 80, SEEK_SET);

        char buffer[BUFFER_SIZE];

        int r;
        while((r = fread(buffer, 1, BUFFER_SIZE, f))) {
            fwrite(buffer, 1, r, outf);
        }
        fclose(f);
        fclose(outf);
    } else if(setflag) {
        strncpy(header, set_header, 80);
        fwrite(header, 80, 1, f);
        fclose(f);
    } else {
        size_t readBytes = fread(header, 80, 1, f);
        fclose(f);

        for(int i = 0; i < 80; i++) {
            printf("%c", header[i]);
        }
        printf("\n");
    }

    return 0;
}
