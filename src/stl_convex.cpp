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
#include <libgen.h>
#include <cmath>
#include "stl_util.h"
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <iostream>

#define BUFFER_SIZE 4096

void print_usage() {
    fprintf(stderr, "stl_convex prints whether an STL file is a convex polyhedron.\n\n");
    fprintf(stderr, "usage: stl_convex [ -v ] [ <input file> ]\n");
    fprintf(stderr, "    Prints whether the input file is a convex polyhedron. If no input file is specified, data is read from stdin. If -v is specified, prints out the Euler characteristic in addition to whether the mesh is convex.\n");
    fprintf(stderr, "The Euler characteristic of a polyhedral surface is defined as V - E + F, where V is the number of vertices, E is the number of edges, and F is the number of faces. All convex polyhedra will have an Euler characteristic of 2.\n");
}

#define EPS .000001
struct VertexKey {
  int x;
  int y;
  int z;
  VertexKey() : x(0), y(0), z(0) {}
  VertexKey(float xx, float yy, float zz) {
    x = (int)(std::round((double)xx/EPS));
    y = (int)(std::round((double)yy/EPS));
    z = (int)(std::round((double)zz/EPS));
  }

  bool operator==(const VertexKey& k) const {
    return x == k.x && y == k.y && z == k.z;
  }
};

namespace std {
  template <>
  struct hash<VertexKey> {
    std::size_t operator()(const VertexKey& k) const {
      return ((std::hash<int>()(k.x) ^ (std::hash<int>()(k.y) << 1)) >> 1) ^ (hash<int>()(k.z) << 1);
    }
  };
}

struct EdgeKey {
  VertexKey v1;
  VertexKey v2;

  EdgeKey(const VertexKey &a, const VertexKey &b) {
    if(a.x < b.x) {
      v1 = a;
      v2 = b;
    } else if(a.x > b.x) {
      v2 = a;
      v1 = b;
    } else {
      if(a.y < b.y) {
        v1 = a;
        v2 = b;
      } else if(a.y > b.y) {
        v2 = a;
        v1 = b;
      } else {
        if(a.z < b.z) {
          v1 = a;
          v2 = b;
        } else if(a.z > b.z) {
          v2 = a;
          v1 = b;
        } else {
          v1 = a;
          v2 = b;
        }
      }
    }
  }

  bool operator==(const EdgeKey &k) const {
    return (v1 == k.v1 && v2 == k.v2);
  }
};

namespace std {
  template<>
  struct hash<EdgeKey> {
    std::size_t operator()(const EdgeKey& k) const {
      return ((std::hash<VertexKey>()(k.v1) ^ (std::hash<VertexKey>()(k.v2) << 1)) >> 1);
    }
  };
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

    int verbose = 0;

    while((c = getopt(argc, argv, "v")) != -1) {
        switch(c) {
            case 'v':
                verbose = 1;
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

    FILE *f;

    if(optind == argc-1) {
        char *file = argv[optind];

        f = fopen(file, "rb");
        if(!f) {
            fprintf(stderr, "Can't read file: %s\n", file);
            exit(2);
        }
    } else {
        f = stdin;
    }

    fseek(f, 80, SEEK_SET);
    uint32_t num_tris;
    uint16_t abc = 0; // attribute byte count

    size_t readBytes = fread(&num_tris, 4, 1, f);

    std::unordered_map<VertexKey,std::vector<vec>> verts2normals;
    std::unordered_map<VertexKey,std::vector<vec>> verts2edges;
    std::unordered_set<EdgeKey> edgesSeen;

    for(int i = 0; i < num_tris; i++) {
      vec normal;
      vec p0;
      vec p1;
      vec p2;

      readBytes = fread(&normal, 1, 12,f);
      readBytes = fread(&p0, 1, 12,f);
      readBytes = fread(&p1, 1, 12,f);
      readBytes = fread(&p2, 1, 12,f);
      readBytes = fread(&abc, 1, 2,f);

      VertexKey v0(p0.x, p0.y, p0.z);
      VertexKey v1(p1.x, p1.y, p1.z);
      VertexKey v2(p2.x, p2.y, p2.z);

      verts2normals[v0].push_back(normal);
      verts2normals[v1].push_back(normal);
      verts2normals[v2].push_back(normal);

      vec e;

      e.x = p1.x-p0.x;
      e.y = p1.y-p0.y;
      e.z = p1.z-p0.z;

      verts2edges[v0].push_back(e);

      e.x = p2.x-p0.x;
      e.y = p2.y-p0.y;
      e.z = p2.z-p0.z;

      verts2edges[v0].push_back(e);

      e.x = p0.x-p1.x;
      e.y = p0.y-p1.y;
      e.z = p0.z-p1.z;

      verts2edges[v1].push_back(e);

      e.x = p2.x-p1.x;
      e.y = p2.y-p1.y;
      e.z = p2.z-p1.z;

      verts2edges[v1].push_back(e);

      e.x = p0.x-p2.x;
      e.y = p0.y-p2.y;
      e.z = p0.z-p2.z;

      verts2edges[v2].push_back(e);

      e.x = p1.x-p2.x;
      e.y = p1.y-p2.y;
      e.z = p1.z-p2.z;

      verts2edges[v2].push_back(e);

      edgesSeen.insert(EdgeKey(v0, v1));
      edgesSeen.insert(EdgeKey(v1, v2));
      edgesSeen.insert(EdgeKey(v2, v0));
    }

    int V = verts2normals.size();
    int E = edgesSeen.size();
    int F = num_tris;

    int euler_characteristic = V - E + F;

    if(verbose) {
      std::cout << "V = " << V << std::endl;
      std::cout << "E = " << E << std::endl;
      std::cout << "F = " << F << std::endl;
      std::cout << "V - E + F = " << euler_characteristic << std::endl;
    }

    if(euler_characteristic == 2) {
      if(verbose) {
        std::cout << "Euler characteristic of 2... possibly convex" << std::endl;
      }

      std::unordered_map<VertexKey, std::vector<vec>>::iterator itr = verts2edges.begin();
      while(itr != verts2edges.end()) {
        std::vector<vec>::iterator nItr = verts2normals[itr->first].begin();
        while(nItr != verts2normals[itr->first].end()) {
          std::vector<vec>::iterator vItr = itr->second.begin();
          while(vItr != itr->second.end()) {
            if(vec_dot(&(*vItr), &(*nItr)) > EPSILON) {
              if(verbose) {
                std::cout << "found vertex with non convex edge" << std::endl;
              }
              std::cout << "not convex" << std:: endl;
              exit(0);
            }
            ++vItr;
          }
          ++nItr;
        }
        ++itr;

      }

      std::cout << "convex" << std:: endl;
    } else {
      if(verbose) {
        std::cout << "Euler characteristic of " << euler_characteristic << "..." << std::endl;
      }
      std::cout << "not convex" << std:: endl;
    }

    return 0;
}
