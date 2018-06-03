#ifndef __CSGJS_UTIL__
#define __CSGJS_UTIL__

#include "csgjs/math/Polygon3.h"
#include "stl_util.h"
#include <vector>
#include <stdio.h>
#include <unordered_map>
#include "csgjs/math/HashKeys.h"

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
    size_t readBytes = fread(&num_tris, 4, 1, f);

    vec p1;
    vec p2;
    vec p3;

    for(int i = 0; i < num_tris; i++) {
        fseek(f, 12, SEEK_CUR); // normal

        readBytes = fread(&p1, 1, 12,f);
        readBytes = fread(&p2, 1, 12,f);
        readBytes = fread(&p3, 1, 12,f);
        fseek(f, 2, SEEK_CUR);

        verts.clear();
        verts.push_back(Vertex(Vector3(p1.x, p1.y, p1.z)));
        verts.push_back(Vertex(Vector3(p2.x, p2.y, p2.z)));
        verts.push_back(Vertex(Vector3(p3.x, p3.y, p3.z)));

        Polygon p(verts);

        if(p.checkIfDegenerateTriangle()) {
//          std::cout << "found degenerate triangle, ignoring" << std::endl;
        } else {
          polys.push_back(Polygon(verts));
        }
    }

    fclose(f);

    return std::move(polys);
  }

  void WriteSTLFile(const char* filename, const std::vector<Polygon> polygons) {
    FILE *outf = fopen(filename, "wb");
    if(!outf) {
      fprintf(stderr, "Can't write to file: %s\n", filename);
    }

    char header[81] = {0};
    snprintf(header, 81, "Created with stl_cmd");
    fwrite(header, 80, 1, outf);

    uint32_t num_tris = 0;
    std::vector<Polygon>::const_iterator itr = polygons.begin();
    while(itr != polygons.end()) {
      num_tris += itr->vertices.size()-2;
      ++itr;
    }

    std::unordered_map<VertexKey, Vector3> vertexLookup;

    fwrite(&num_tris, 4, 1, outf);

    uint16_t abc = 0;

    itr = polygons.begin();
    while(itr != polygons.end()) {
      vec normal;
      vec p0;
      vec p1;

      Vector3 vertex0 = itr->vertices[0].pos;
      Vector3 vertex1 = itr->vertices[1].pos;

      VertexKey key0(vertex0);
      VertexKey key1(vertex1);

      if(vertexLookup.count(key0) > 0) {
        vertex0 = vertexLookup[key0];
      } else {
        vertexLookup[key0] = vertex0;
      }
      if(vertexLookup.count(key1) > 0) {
        vertex1 = vertexLookup[key1];
      } else {
        vertexLookup[key1] = vertex1;
      }

      normal.x = (float)itr->plane.normal.x;
      normal.y = (float)itr->plane.normal.y;
      normal.z = (float)itr->plane.normal.z;

      p0.x = (float)vertex0.x;
      p0.y = (float)vertex0.y;
      p0.z = (float)vertex0.z;

      p1.x = (float)vertex1.x;
      p1.y = (float)vertex1.y;
      p1.z = (float)vertex1.z;

      int numVertices = itr->vertices.size();
      for(int i = 2; i < numVertices; i++) {
        fwrite(&normal, 1, 12, outf);

        vec p2;

        Vector3 vertex2 = itr->vertices[i].pos;
        VertexKey key2(vertex2);
        if(vertexLookup.count(key2) > 0) {
          vertex2 = vertexLookup[key2];
        } else {
          vertexLookup[key2] = vertex2;
        }

        p2.x = (float)vertex2.x;
        p2.y = (float)vertex2.y;
        p2.z = (float)vertex2.z;

        fwrite(&p0, 1, 12, outf);
        fwrite(&p1, 1, 12, outf);
        fwrite(&p2, 1, 12, outf);
        fwrite(&abc, 1, 2,outf);

        p1 = p2;
      }

      ++itr;
    }

    fclose(outf);
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
