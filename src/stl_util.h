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
#include <sys/stat.h>

int is_valid_binary_stl(char* filename) {
    FILE *f;
    f = fopen(filename, "rb");

    if(f) {
        struct stat st;
        fstat(fileno(f), &st);

        off_t size = st.st_size;
        if(size >= 84) {
            fseek(f, 80, SEEK_SET);

            uint32_t num_tris;
            fread(&num_tris, 4, 1, f);
            uint64_t calced_size = 84+(4*12+2)*num_tris;
            if(size != calced_size) {
//                fprintf(stderr, "    actual size: %10lld\n", size);
//                fprintf(stderr, "  num triangles: %10d\n", num_tris);
//                fprintf(stderr, "calculated size: %10lld\n", calced_size);
                return 0;
            } 
        } else {
            return 0;
        }
    }

    return 1;
}
