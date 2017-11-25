#ifndef __CSGJS_UTIL__
#define __CSGJS_UTIL__

#include "math/Polygon3.h"
#include "stl_util.h"
#include <vector>
#include <stdio.h>

namespace csgjs {

  std::vector<Polygon> ReadSTLFile(const char* filename) {
    std::vector<Polygon> polys;
    std::vector<Vertex> verts;
    verts.reserve(3);

    FILE *f;

    f = fopen(filename, "rb");
    if(!f) {
        fprintf(stderr, "Can't read file: %s\n", filename);
        exit(2);
    }

    fseek(f, 80, SEEK_SET);

    uint32_t num_tris;
    fread(&num_tris, 4, 1, f);

    vec p1;
    vec p2;
    vec p3;

    for(int i = 0; i < num_tris; i++) {
        fseek(f, 12, SEEK_CUR); // normal

        fread(&p1, 1, 12,f);
        fread(&p2, 1, 12,f);
        fread(&p3, 1, 12,f);
        fseek(f, 2, SEEK_CUR);

        verts.clear();
        verts.push_back(Vertex(Vector3(p1.x, p1.y, p1.z)));
        verts.push_back(Vertex(Vector3(p2.x, p2.y, p2.z)));
        verts.push_back(Vertex(Vector3(p3.x, p3.y, p3.z)));

        polys.push_back(Polygon(verts));
    }

    fclose(f);

    return std::move(polys);
  }

// from https://stackoverflow.com/questions/1640258/need-a-fast-random-generator-for-c
  static unsigned long x=123456789, y=362436069, z=521288629;

  unsigned long xorshf96(void) {          //period 2^96-1
  unsigned long t;
      x ^= x << 16;
      x ^= x >> 5;
      x ^= x << 1;

     t = x;
     x = y;
     y = z;
     z = t ^ x ^ y;

    return z;
  }

  int fastRandom(int max) {
    return xorshf96() % max;
  }

}

#endif
