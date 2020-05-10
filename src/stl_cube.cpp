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

void print_usage() {
    fprintf(stderr, "stl_cube outputs an STL file of a single cube.\n\n");
    fprintf(stderr, "usage: stl_cube [-w <width>] [ <output file> ]\n");
    fprintf(stderr, "    Outputs an stl file of a cube with the provided width. ");
    fprintf(stderr, "    If the width is omitted, it defaults to 1. If no output file is provided, data is sent to stdout. \n");
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

    float width = 1;

    while((c = getopt(argc, argv, "w:")) != -1) {
        switch(c) {
            case 'w':
                width = atof(optarg);
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
    snprintf(header, 81, "Cube of width %.4f", width);

    fwrite(header, 80, 1, outf);

    uint32_t num_tris = 12;

    fwrite(&num_tris, 4, 1, outf);

    uint16_t abc = 0; // attribute byte count

    float tris[12][3][3] = {
        // Top
        {
           {  1,  1,  1  },
           {  1,  1, -1  },
           { -1,  1, -1  }
        },
        {
           { -1,  1, -1  },
           { -1,  1,  1  },
           {  1,  1,  1  }
        },
        // Bottom
        {
           { -1, -1,  1  },
           {  1, -1, -1  },
           {  1, -1,  1  }
        },
        {
           { -1, -1,  1  },
           { -1, -1, -1  },
           {  1, -1, -1  }
        },
        // Right
        {
           {  1, -1,  1  },
           {  1, -1, -1  },
           {  1,  1, -1  }
        },
        {
           {  1,  1, -1  },
           {  1,  1,  1  },
           {  1, -1,  1  }
        },
        // Left
        {
           { -1, -1,  1  },
           { -1,  1,  1  },
           { -1,  1, -1  }
        },
        {
           { -1,  1, -1  },
           { -1, -1, -1  },
           { -1, -1,  1  }
        },
        // Near
        {
           {  1,  1,  1  },
           { -1,  1,  1  },
           { -1, -1,  1  }
        },
        {
           { -1, -1,  1  },
           {  1, -1,  1  },
           {  1,  1,  1  }
        },
        // Far
        {
           {  1, -1, -1  },
           { -1, -1, -1  },
           { -1,  1, -1  }
        },
        {
           { -1,  1, -1  },
           {  1,  1, -1  },
           {  1, -1, -1  }
        }
    };

    float normals[12][3] = {
        // Top
        {  0,  1,  0  },
        {  0,  1,  0  },
        // Bottom
        {  0, -1,  0  },
        {  0, -1,  0  },
        // Right
        {  1,  0,  0  },
        {  1,  0,  0  },
        // Left
        { -1,  0,  0  },
        { -1,  0,  0  },
        // Near
        {  0,  0,  1  },
        {  0,  0,  1  },
        // Far
        {  0,  0, -1  },
        {  0,  0, -1  }
    };

    for(int i = 0; i < num_tris; i++) {
        fwrite(normals[i], 1, 12, outf);
        for(int j = 0; j < 3; j++) {
            for(int k = 0; k < 3; k++) {
                tris[i][j][k] *= .5*width;
            }
        }
        fwrite(tris[i], 1, 36, outf);
        fwrite(&abc, 1, 2,outf);
    }

    return 0;
}
