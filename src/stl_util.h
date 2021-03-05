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

#ifndef ___STL_UTIL_H___
#define ___STL_UTIL_H___

#include <stdint.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#ifndef M_PI
#define M_PI 3.141592653589793
#endif

#define EPSILON 0.0001f

typedef struct {
    float xx;
    float xy;
    float xz;
    float xw;
    float yx;
    float yy;
    float yz;
    float yw;
    float zx;
    float zy;
    float zz;
    float zw;
    float tx;
    float ty;
    float tz;
    float tw;
} mat;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} vec;

typedef struct {
    vec min;
    vec max;
} bounds_t;

typedef struct {
    vec normal;
    vec vertices[3];
} facet_t;

inline void write_header(FILE* f, const char* name, uint32_t facet_count, int is_ascii) {
    if (is_ascii) {
        if (name) {
            fprintf(f, "solid %s\n", name);
        } else {
            fprintf(f, "solid \n");
        }
    } else {
        char header[81];
        memset(header, 0x00, 81);
        if (name) {
          strncpy(header, name, 80);
        }
        fwrite(header, 1, 80, f);
        fwrite(&facet_count, 1, 4, f);
    }
}

inline void write_final(FILE* f, const char* name, uint32_t facet_count, int is_ascii) {
    if (is_ascii) {
        fprintf(f, "endsolid %s\n", name);
    } else {
        long offset = ftell(f);
        fseek(f, 80, SEEK_SET);
        fwrite(&facet_count, 1, 4, f);
        fseek(f, offset, SEEK_SET);
    }
}

inline void write_facet(FILE* f, facet_t* facet, int is_ascii) {
    if (is_ascii) {
        fprintf(f, "facet normal %f %f %f\n", facet->normal.x, facet->normal.y, facet->normal.z);
        fprintf(f, "  outer loop\n");
        for (int i = 0; i < 3; i++) {
            fprintf(f, "    vertex %f %f %f\n", facet->vertices[i].x, facet->vertices[i].y, facet->vertices[i].z);
        }
        fprintf(f, "  endloop\n");
        fprintf(f, "endfacet\n");
    } else {
        fwrite(&(facet->normal), 1, 12, f);
        for (int i = 0; i < 3; i++) {
            fwrite(&(facet->vertices[i]), 1, 12, f);
        }
        uint16_t abc = 0;
        fwrite(&abc, 1, 2, f);
    }
}

inline int read_header(FILE* f, char* name, size_t name_length, uint32_t* facet_count, int is_ascii) {
    if (is_ascii) {
        char buffer[4096];
        if (fgets(buffer, 4095, f)) {
            if (strncmp(buffer, "solid ", 6) == 0) {
                if (name) {
                    strncpy(name, buffer + 6, name_length);
                    if (strlen(buffer + 6) >= name_length) {
                        name[name_length - 1] = 0;
                    }
                    for (int i = strlen(name) - 1; i >= 0; i--) {
                        if (isspace(name[i])) {
                            name[i] = 0;
                        } else {
                            break;
                        }
                    }
                }
                return 1;
            }
        }
    } else {
        char header[81];
        memset(header, 0x00, 81);
        if (fread(header, 1, 80, f) == 80) {
            if (name) {
                strncpy(name, header, name_length);
                if (strlen(header) >= name_length) {
                    name[name_length - 1] = 0;
                }
                for (int i = strlen(name) - 1; i >= 0; i--) {
                    if (isspace(name[i])) {
                        name[i] = 0;
                    } else {
                        break;
                    }
                }
            }
            uint32_t tmp = 0;
            if (fread(&tmp, 1, 4, f) == 4) {
                if (facet_count) {
                    *facet_count = tmp;
                } 
                return 1;
            }
        }
    }
    return 0;
}

inline int read_final(FILE* f, int is_ascii) {
    if (is_ascii) {
        char buffer[4096];
        if (fgets(buffer, 4095, f)) {
            if (strncmp(buffer, "endsolid", 8) == 0) {
                return 1;
            }
        }
    } else {
        return 1;
    }
    return 0;
}

inline int read_facet(FILE* f, facet_t* facet, int is_ascii) {
    if (!facet) {
        return 0;
    }
    memset(facet, 0x00, sizeof(facet_t));
    long offset = ftell(f);
    if (offset < 0) {
        return 0;
    }
    if (is_ascii) {
        char buffer[4096];
        if (!fgets(buffer, 4095, f)) {
            return 0;
        }
        if (strncmp(buffer, "endsolid", 8) == 0) {
            //We have reached the end, so seek back
            fseek(f, offset, SEEK_SET);
            return 0;
        }
        if (sscanf(buffer, " facet normal %f %f %f", &(facet->normal.x), &(facet->normal.y), &(facet->normal.z)) != 3) {
            return 0;
        }
        if (!fgets(buffer, 4095, f)) {
            return 0;
        }
        char arg1[256], arg2[256];
        if (sscanf(buffer, " %s %s", arg1, arg2) != 2 || strcmp(arg1, "outer") != 0 || strcmp(arg2, "loop") != 0) {
            return 0;
        }
        for (int i = 0; i < 3; i++) {
            if (!fgets(buffer, 4095, f)) {
                return 0;
            }
            if (sscanf(buffer, " vertex %f %f %f", &(facet->vertices[i].x), &(facet->vertices[i].y), &(facet->vertices[i].z)) != 3) {
                return 0;
            }
        }
        if (!fgets(buffer, 4095, f)) {
            return 0;
        }
        if (sscanf(buffer, " %s", arg1) != 1 || strcmp(arg1, "endloop") != 0) {
            return 0;
        }
        if (!fgets(buffer, 4095, f)) {
            return 0;
        }
        if (sscanf(buffer, " %s", arg1) != 1 || strcmp(arg1, "endfacet") != 0) {
            return 0;
        }
    } else {
        if (fread(&(facet->normal), 1, 12, f) != 12) {
            return 0;
        }
        facet->normal.w = 1.0;
        for (int i = 0; i < 3; i++) {
            if (fread(&(facet->vertices[i]), 1, 12, f) != 12) {
                return 0;
            }
            facet->vertices[i].w = 1.0;
        }
        uint16_t abc = 0;
        if (fread(&abc, 1, 2, f) != 2) {
            return 0;
        }
    }
    return 1;
}

inline int is_valid_ascii_stl(FILE* f) {
    if (!f) {
      return 0;
    }
    long offset = ftell(f);
    if (offset < 0) {
        return 0;
    }
    int is_valid = 0;
    if (read_header(f, NULL, 0, NULL, 1)) {
        facet_t facet;
        while (read_facet(f, &facet, 1)) {
            // Do nothing
        }
        if (read_final(f, 1)) {
            is_valid = 1;
        }
    }
    fseek(f, offset, SEEK_SET);
    return is_valid;
}

inline int is_valid_ascii_stl(char* filename) {
    FILE *f;
    f = fopen(filename, "r");
    int is_valid = is_valid_ascii_stl(f);
    if (f) {
      fclose(f);
    }
    return is_valid;
}

inline int is_valid_binary_stl(FILE* f) {
    if (!f) {
      return 0;
    }
    long offset = ftell(f);
    if (offset < 0) {
        return 0;
    }
    int is_valid = 0;
    struct stat st;
    fstat(fileno(f), &st);

    off_t size = st.st_size;
    if (size >= 84) {
        fseek(f, 80, SEEK_SET);

        uint32_t num_tris;
        size_t readBytes = fread(&num_tris, 4, 1, f);
        uint64_t calced_size = 84+(4*12+2)*(uint64_t)num_tris;
        if (size == calced_size) {
            is_valid = 1;
        } else {
          //fprintf(stderr, "    actual size: %10lld\n", size);
          //fprintf(stderr, "  num triangles: %10d\n", num_tris);
          //fprintf(stderr, "calculated size: %10lld\n", calced_size);
        }
    }
    fseek(f, offset, SEEK_SET);
    return is_valid;
}

inline int is_valid_binary_stl(char* filename) {
    FILE *f;
    f = fopen(filename, "rb");
    int is_valid = is_valid_binary_stl(f);
    if (f) {
      fclose(f);
    }
    return is_valid;
}

inline void mat_print(mat *m) {
    printf("%f %f %f %f\n", m->xx, m->xy, m->xz, m->xw);
    printf("%f %f %f %f\n", m->yx, m->yy, m->yz, m->yw);
    printf("%f %f %f %f\n", m->zx, m->zy, m->zz, m->zw);
    printf("%f %f %f %f\n", m->tx, m->ty, m->tz, m->tw);
}

inline void init_identity_mat(mat *m) {
    m->xx = 1;
    m->xy = 0;
    m->xz = 0;
    m->xw = 0;

    m->yx = 0;
    m->yy = 1;
    m->yz = 0;
    m->yw = 0;

    m->zx = 0;
    m->zy = 0;
    m->zz = 1;
    m->zw = 0;

    m->tx = 0;
    m->ty = 0;
    m->tz = 0;
    m->tw = 1;
}

inline void init_s_mat(mat *m, float s) {
    m->xx = s;
    m->xy = 0;
    m->xz = 0;
    m->xw = 0;

    m->yx = 0;
    m->yy = s;
    m->yz = 0;
    m->yw = 0;

    m->zx = 0;
    m->zy = 0;
    m->zz = s;
    m->zw = 0;

    m->tx = 0;
    m->ty = 0;
    m->tz = 0;
    m->tw = 1;
}

inline void init_sx_mat(mat *m, float s) {
    m->xx = s;
    m->xy = 0;
    m->xz = 0;
    m->xw = 0;

    m->yx = 0;
    m->yy = 1;
    m->yz = 0;
    m->yw = 0;

    m->zx = 0;
    m->zy = 0;
    m->zz = 1;
    m->zw = 0;

    m->tx = 0;
    m->ty = 0;
    m->tz = 0;
    m->tw = 1;
}

inline void init_sy_mat(mat *m, float s) {
    m->xx = 1;
    m->xy = 0;
    m->xz = 0;
    m->xw = 0;

    m->yx = 0;
    m->yy = s;
    m->yz = 0;
    m->yw = 0;

    m->zx = 0;
    m->zy = 0;
    m->zz = 1;
    m->zw = 0;

    m->tx = 0;
    m->ty = 0;
    m->tz = 0;
    m->tw = 1;
}

inline void init_sz_mat(mat *m, float s) {
    m->xx = 1;
    m->xy = 0;
    m->xz = 0;
    m->xw = 0;

    m->yx = 0;
    m->yy = 1;
    m->yz = 0;
    m->yw = 0;

    m->zx = 0;
    m->zy = 0;
    m->zz = s;
    m->zw = 0;

    m->tx = 0;
    m->ty = 0;
    m->tz = 0;
    m->tw = 1;
}

inline void init_rx_mat(mat *m, float angle) {
    angle *= M_PI/180; // convert to radians

    float cosa = cos(angle);
    float sina = sin(angle);

    m->xx = 1;
    m->xy = 0;
    m->xz = 0;
    m->xw = 0;

    m->yx = 0;
    m->yy = cosa;
    m->yz = sina;
    m->yw = 0;

    m->zx = 0;
    m->zy = -sina;
    m->zz = cosa;
    m->zw = 0;

    m->tx = 0;
    m->ty = 0;
    m->tz = 0;
    m->tw = 1;
}

inline void init_ry_mat(mat *m, float angle) {
    angle *= M_PI/180; // convert to radians

    float cosa = cos(angle);
    float sina = sin(angle);

    m->xx = cosa;
    m->xy = 0;
    m->xz = -sina;
    m->xw = 0;

    m->yx = 0;
    m->yy = 1;
    m->yz = 0;
    m->yw = 0;

    m->zx = sina;
    m->zy = 0;
    m->zz = cosa;
    m->zw = 0;

    m->tx = 0;
    m->ty = 0;
    m->tz = 0;
    m->tw = 1;
}

inline void init_rz_mat(mat *m, float angle) {
    angle *= M_PI/180; // convert to radians

    float cosa = cos(angle);
    float sina = sin(angle);

    m->xx = cosa;
    m->xy = sina;
    m->xz = 0;
    m->xw = 0;

    m->yx = -sina;
    m->yy = cosa;
    m->yz = 0;
    m->yw = 0;

    m->zx = 0;
    m->zy = 0;
    m->zz = 1;
    m->zw = 0;

    m->tx = 0;
    m->ty = 0;
    m->tz = 0;
    m->tw = 1;
}

inline void init_tx_mat(mat *m, float t) {
    m->xx = 1;
    m->xy = 0;
    m->xz = 0;
    m->xw = 0;

    m->yx = 0;
    m->yy = 1;
    m->yz = 0;
    m->yw = 0;

    m->zx = 0;
    m->zy = 0;
    m->zz = 1;
    m->zw = 0;

    m->tx = t;
    m->ty = 0;
    m->tz = 0;
    m->tw = 1;
}

inline void init_ty_mat(mat *m, float t) {
    m->xx = 1;
    m->xy = 0;
    m->xz = 0;
    m->xw = 0;

    m->yx = 0;
    m->yy = 1;
    m->yz = 0;
    m->yw = 0;

    m->zx = 0;
    m->zy = 0;
    m->zz = 1;
    m->zw = 0;

    m->tx = 0;
    m->ty = t;
    m->tz = 0;
    m->tw = 1;
}

inline void init_tz_mat(mat *m, float t) {
    m->xx = 1;
    m->xy = 0;
    m->xz = 0;
    m->xw = 0;

    m->yx = 0;
    m->yy = 1;
    m->yz = 0;
    m->yw = 0;

    m->zx = 0;
    m->zy = 0;
    m->zz = 1;
    m->zw = 0;

    m->tx = 0;
    m->ty = 0;
    m->tz = t;
    m->tw = 1;
}

inline void init_inv_s_mat(mat *m, float s) {
    m->xx = 1./s;
    m->xy = 0;
    m->xz = 0;
    m->xw = 0;

    m->yx = 0;
    m->yy = 1./s;
    m->yz = 0;
    m->yw = 0;

    m->zx = 0;
    m->zy = 0;
    m->zz = 1./s;
    m->zw = 0;

    m->tx = 0;
    m->ty = 0;
    m->tz = 0;
    m->tw = 1;
}

inline void init_inv_sx_mat(mat *m, float s) {
    m->xx = 1./s;
    m->xy = 0;
    m->xz = 0;
    m->xw = 0;

    m->yx = 0;
    m->yy = 1;
    m->yz = 0;
    m->yw = 0;

    m->zx = 0;
    m->zy = 0;
    m->zz = 1;
    m->zw = 0;

    m->tx = 0;
    m->ty = 0;
    m->tz = 0;
    m->tw = 1;
}

inline void init_inv_sy_mat(mat *m, float s) {
    m->xx = 1;
    m->xy = 0;
    m->xz = 0;
    m->xw = 0;

    m->yx = 0;
    m->yy = 1./s;
    m->yz = 0;
    m->yw = 0;

    m->zx = 0;
    m->zy = 0;
    m->zz = 1;
    m->zw = 0;

    m->tx = 0;
    m->ty = 0;
    m->tz = 0;
    m->tw = 1;
}

inline void init_inv_sz_mat(mat *m, float s) {
    m->xx = 1;
    m->xy = 0;
    m->xz = 0;
    m->xw = 0;

    m->yx = 0;
    m->yy = 1;
    m->yz = 0;
    m->yw = 0;

    m->zx = 0;
    m->zy = 0;
    m->zz = 1./s;
    m->zw = 0;

    m->tx = 0;
    m->ty = 0;
    m->tz = 0;
    m->tw = 1;
}

inline void init_inv_rx_mat(mat *m, float angle) {
    angle *= M_PI/180; // convert to radians

    float cosa = cos(angle);
    float sina = sin(angle);

    m->xx = 1;
    m->yx = 0;
    m->zx = 0;
    m->tx = 0;

    m->xy = 0;
    m->yy = cosa;
    m->zy = sina;
    m->ty = 0;

    m->xz = 0;
    m->yz = -sina;
    m->zz = cosa;
    m->tz = 0;

    m->xw = 0;
    m->yw = 0;
    m->zw = 0;
    m->tw = 1;
}

inline void init_inv_ry_mat(mat *m, float angle) {
    angle *= M_PI/180; // convert to radians

    float cosa = cos(angle);
    float sina = sin(angle);

    m->xx = cosa;
    m->yx = 0;
    m->zx = -sina;
    m->tx = 0;

    m->xy = 0;
    m->yy = 1;
    m->zy = 0;
    m->ty = 0;

    m->xz = sina;
    m->yz = 0;
    m->zz = cosa;
    m->tz = 0;

    m->xw = 0;
    m->yw = 0;
    m->zw = 0;
    m->tw = 1;
}

inline void init_inv_rz_mat(mat *m, float angle) {
    angle *= M_PI/180; // convert to radians

    float cosa = cos(angle);
    float sina = sin(angle);

    m->xx = cosa;
    m->yx = sina;
    m->zx = 0;
    m->tx = 0;

    m->xy = -sina;
    m->yy = cosa;
    m->zy = 0;
    m->ty = 0;

    m->xz = 0;
    m->yz = 0;
    m->zz = 1;
    m->tz = 0;

    m->xw = 0;
    m->yw = 0;
    m->zw = 0;
    m->tw = 1;
}

inline void init_inv_tx_mat(mat *m, float t) {
    m->xx = 1;
    m->xy = 0;
    m->xz = 0;
    m->xw = 0;

    m->yx = 0;
    m->yy = 1;
    m->yz = 0;
    m->yw = 0;

    m->zx = 0;
    m->zy = 0;
    m->zz = 1;
    m->zw = 0;

    m->tx = -t;
    m->ty = 0;
    m->tz = 0;
    m->tw = 1;
}

inline void init_inv_ty_mat(mat *m, float t) {
    m->xx = 1;
    m->xy = 0;
    m->xz = 0;
    m->xw = 0;

    m->yx = 0;
    m->yy = 1;
    m->yz = 0;
    m->yw = 0;

    m->zx = 0;
    m->zy = 0;
    m->zz = 1;
    m->zw = 0;

    m->tx = 0;
    m->ty = -t;
    m->tz = 0;
    m->tw = 1;
}

inline void init_inv_tz_mat(mat *m, float t) {
    m->xx = 1;
    m->xy = 0;
    m->xz = 0;
    m->xw = 0;

    m->yx = 0;
    m->yy = 1;
    m->yz = 0;
    m->yw = 0;

    m->zx = 0;
    m->zy = 0;
    m->zz = 1;
    m->zw = 0;

    m->tx = 0;
    m->ty = 0;
    m->tz = -t;
    m->tw = 1;
}

inline void mat_transpose(mat *m, mat *out) {
    out->xx = m->xx;
    out->xy = m->yx;
    out->xz = m->zx;
    out->xw = m->tx;

    out->yx = m->xy;
    out->yy = m->yy;
    out->yz = m->zy;
    out->yw = m->ty;

    out->zx = m->xz;
    out->zy = m->yz;
    out->zz = m->zz;
    out->zw = m->tz;

    out->tx = m->xw;
    out->ty = m->yw;
    out->tz = m->zw;
    out->tw = m->tw;
}

inline void mat_mult(mat *m1, mat *m2, mat *out) {
    out->xx = m1->xx*m2->xx+m1->xy*m2->yx+m1->xz*m2->zx+m1->xw*m2->tx;
    out->xy = m1->xx*m2->xy+m1->xy*m2->yy+m1->xz*m2->zy+m1->xw*m2->ty;
    out->xz = m1->xx*m2->xz+m1->xy*m2->yz+m1->xz*m2->zz+m1->xw*m2->tz;
    out->xw = m1->xx*m2->xw+m1->xy*m2->yw+m1->xz*m2->zw+m1->xw*m2->tw;

    out->yx = m1->yx*m2->xx+m1->yy*m2->yx+m1->yz*m2->zx+m1->yw*m2->tx;
    out->yy = m1->yx*m2->xy+m1->yy*m2->yy+m1->yz*m2->zy+m1->yw*m2->ty;
    out->yz = m1->yx*m2->xz+m1->yy*m2->yz+m1->yz*m2->zz+m1->yw*m2->tz;
    out->yw = m1->yx*m2->xw+m1->yy*m2->yw+m1->yz*m2->zw+m1->yw*m2->tw;

    out->zx = m1->zx*m2->xx+m1->zy*m2->yx+m1->zz*m2->zx+m1->zw*m2->tx;
    out->zy = m1->zx*m2->xy+m1->zy*m2->yy+m1->zz*m2->zy+m1->zw*m2->ty;
    out->zz = m1->zx*m2->xz+m1->zy*m2->yz+m1->zz*m2->zz+m1->zw*m2->tz;
    out->zw = m1->zx*m2->xw+m1->zy*m2->yw+m1->zz*m2->zw+m1->zw*m2->tw;

    out->tx = m1->tx*m2->xx+m1->ty*m2->yx+m1->tz*m2->zx+m1->tw*m2->tx;
    out->ty = m1->tx*m2->xy+m1->ty*m2->yy+m1->tz*m2->zy+m1->tw*m2->ty;
    out->tz = m1->tx*m2->xz+m1->ty*m2->yz+m1->tz*m2->zz+m1->tw*m2->tz;
    out->tw = m1->tx*m2->xw+m1->ty*m2->yw+m1->tz*m2->zw+m1->tw*m2->tw;
}

inline void vec_mat_mult(vec *v, mat *m, vec *out) {
    out->x = v->x*m->xx+v->y*m->yx+v->z*m->zx+v->w*m->tx;
    out->y = v->x*m->xy+v->y*m->yy+v->z*m->zy+v->w*m->ty;
    out->z = v->x*m->xz+v->y*m->yz+v->z*m->zz+v->w*m->tz;
    out->w = v->x*m->xw+v->y*m->yw+v->z*m->zw+v->w*m->tw;
}

inline float vec_magnitude(vec *v) {
    return sqrt(v->x*v->x+v->y*v->y+v->z*v->z);
}

inline void vec_normalize(vec *v, vec *out) {
    float invmag = 1./vec_magnitude(v);
    out->x = v->x*invmag;
    out->y = v->y*invmag;
    out->z = v->z*invmag;
}

inline int vec_equals(vec *v, vec *v2) {
    return fabs(v->x-v2->x) < EPSILON &&
           fabs(v->y-v2->y) < EPSILON &&
           fabs(v->z-v2->z) < EPSILON;
}

inline int vec_equals_exact(vec *v, vec *v2) {
    return v->x-v2->x == 0 &&
           v->y-v2->y == 0 &&
           v->z-v2->z == 0;
}

inline void vec_copy(vec *v, vec *out) {
    out->x = v->x;
    out->y = v->y;
    out->z = v->z;
    out->w = v->w;
}

inline void mat_copy(mat *m1, mat *out) {
    out->xx = m1->xx;
    out->xy = m1->xy;
    out->xz = m1->xz;
    out->xw = m1->xw;

    out->yx = m1->yx;
    out->yy = m1->yy;
    out->yz = m1->yz;
    out->yw = m1->yw;

    out->zx = m1->zx;
    out->zy = m1->zy;
    out->zz = m1->zz;
    out->zw = m1->zw;

    out->tx = m1->tx;
    out->ty = m1->ty;
    out->tz = m1->tz;
    out->tw = m1->tw;
}

inline void vec_cross(vec *a, vec *b, vec *out) {
    out->x = a->y*b->z-a->z*b->y;
    out->y = a->z*b->x-a->x*b->z;
    out->z = a->x*b->y-a->y*b->x;
}

inline void vec_add(vec *a, vec *b, vec *out) {
    out->x = a->x+b->x;
    out->y = a->y+b->y;
    out->z = a->z+b->z;
}

inline void vec_sub(vec *a, vec *b, vec *out) {
    out->x = a->x-b->x;
    out->y = a->y-b->y;
    out->z = a->z-b->z;
}

inline float vec_dot(vec *a, vec *b) {
    return a->x*b->x+a->y*b->y+a->z*b->z;
}

inline void write_tri(FILE *f,
                vec *p1,
                vec *p2,
                vec *p3, int rev) {
    if (rev) {
        vec *tmp = p1;
        p1 = p3;
        p3 = tmp;
    }

    vec n1;
    vec v1,v2;

    vec_sub(p2,p1,&v1);
    vec_sub(p3,p1,&v2);

    vec_cross(&v1,&v2,&n1);
    vec_normalize(&n1,&n1);

    uint16_t abc = 0;

    fwrite(&n1, 1, 12, f);
    fwrite(p1, 1, 12, f);
    fwrite(p2, 1, 12, f);
    fwrite(p3, 1, 12, f);
    fwrite(&abc,1,  2, f);
}

inline void write_quad(FILE *f,
                vec *p1,
                vec *p2,
                vec *p3,
                vec *p4,int rev) {
    //write two triangles 1,2,3 and 1,3,4
    if (rev) {
        vec *tmp = p1;
        p1 = p4;
        p4 = tmp;

        tmp = p2;
        p2 = p3;
        p3 = tmp;
    }

    vec n1,n2;

    vec v1,v2,v3;

    vec_sub(p2,p1,&v1);
    vec_sub(p3,p1,&v2);
    vec_sub(p4,p1,&v3);

    vec_cross(&v1,&v2,&n1);
    vec_normalize(&n1,&n1);

    vec_cross(&v2,&v3,&n2);
    vec_normalize(&n2,&n2);

    uint16_t abc = 0;

    fwrite(&n1, 1, 12, f);
    fwrite(p1, 1, 12, f);
    fwrite(p2, 1, 12, f);
    fwrite(p3, 1, 12, f);
    fwrite(&abc,1,  2, f);

    fwrite(&n2, 1, 12, f);
    fwrite(p1, 1, 12, f);
    fwrite(p3, 1, 12, f);
    fwrite(p4, 1, 12, f);
    fwrite(&abc,1,  2, f);
}

inline void get_facet_bounds(facet_t *facet, bounds_t* b, int init) {
    if (init) {
        b->min = facet->vertices[0];
        b->max = facet->vertices[0];
    }
    for (int i = init ? 1 : 0; i < 3; i++) {
        if (b->min.x > facet->vertices[i].x) b->min.x = facet->vertices[i].x;
        if (b->min.y > facet->vertices[i].y) b->min.y = facet->vertices[i].y;
        if (b->min.z > facet->vertices[i].z) b->min.z = facet->vertices[i].z;

        if (b->max.x < facet->vertices[i].x) b->max.x = facet->vertices[i].x;
        if (b->max.y < facet->vertices[i].y) b->max.y = facet->vertices[i].y;
        if (b->max.z < facet->vertices[i].z) b->max.z = facet->vertices[i].z;
    }
}

inline void get_bounds(FILE *f, bounds_t* b, int is_ascii) {
    uint32_t facet_count = 0;
    memset(b, 0x00, sizeof(bounds_t));
    read_header(f, NULL, 0, &facet_count, is_ascii);
    for (int i = 0; is_ascii || i < facet_count; i++) {
        facet_t facet;
        if (read_facet(f, &facet, is_ascii)) {
            get_facet_bounds(&facet, b, i == 0);
        } else {
            break;
        }
    }
    read_final(f, is_ascii);
}

#endif
