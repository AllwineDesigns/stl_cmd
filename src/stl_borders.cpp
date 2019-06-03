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
#include <unordered_map>
#include <iostream>

#define BUFFER_SIZE 4096

void print_usage() {
    fprintf(stderr, "stl_borders prints how many border edges there are in a given STL file.\n\n");
    fprintf(stderr, "usage: stl_borders [ -e ] [ -i ] [ <input file> ]\n");
    fprintf(stderr, "    Prints how many border edges there are. For a watertight, manifold model, it will be 0.\n");
    fprintf(stderr, "    -e - will use the vertices exactly as stored rather than consolidating vertices that are very close together\n");
    fprintf(stderr, "    -i - ignore degenerate triangles (those with area very near or exactly 0).");
}

struct VertexKey {
  float x;
  float y;
  float z;
  VertexKey() : x(0), y(0), z(0) {}
  VertexKey(float xx, float yy, float zz, bool exact = false) {
    if(exact) {
      x = xx;
      y = yy;
      z = zz;
    } else {
      x = (float)(int)(std::round((double)xx/EPSILON));
      y = (float)(int)(std::round((double)yy/EPSILON));
      z = (float)(int)(std::round((double)zz/EPSILON));
    }
  }

  bool operator==(const VertexKey& k) const {
    return x == k.x && y == k.y && z == k.z;
  }
};

namespace std {
  template <>
  struct hash<VertexKey> {
    std::size_t operator()(const VertexKey& k) const {
      return ((std::hash<float>()(k.x) ^ (std::hash<float>()(k.y) << 1)) >> 1) ^ (hash<float>()(k.z) << 1);
    }
  };
}

struct EdgeKey {
  VertexKey v1;
  VertexKey v2;

  EdgeKey(const VertexKey &a, const VertexKey &b) {
    v1 = a;
    v2 = b;
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
    bool exactKeys = false;
    bool ignoreDegenerateFaces = false;

    while((c = getopt(argc, argv, "ei")) != -1) {
        switch(c) {
            case 'i':
                ignoreDegenerateFaces = true;
                break;
            case 'e':
                exactKeys = true;
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

    std::unordered_map<EdgeKey, int> edgeCounts;

    int ignoredFaces = 0;
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

      if(ignoreDegenerateFaces) {
        vec vec1;
        vec vec2;

        vec1.x = p1.x-p0.x;
        vec1.y = p1.y-p0.y;
        vec1.z = p1.z-p0.z;

        vec2.x = p2.x-p0.x;
        vec2.y = p2.y-p0.y;
        vec2.z = p2.z-p0.z;

        vec cross;

        vec_cross(&vec1, &vec2, &cross);
        float area = .5*vec_magnitude(&cross);
        if(area < EPSILON) {
          ignoredFaces++;
          continue;
        }
      }

      VertexKey v0(p0.x, p0.y, p0.z, exactKeys);
      VertexKey v1(p1.x, p1.y, p1.z, exactKeys);
      VertexKey v2(p2.x, p2.y, p2.z, exactKeys);

      EdgeKey e0(v0,v1);
      EdgeKey e1(v1,v2);
      EdgeKey e2(v2,v0);

      if(edgeCounts.count(e0)) {
        edgeCounts[e0] = edgeCounts[e0] + 1;
      } else {
        edgeCounts[e0] = 1;
      }

      if(edgeCounts.count(e1)) {
        edgeCounts[e1] = edgeCounts[e1] + 1;
      } else {
        edgeCounts[e1] = 1;
      }

      if(edgeCounts.count(e2)) {
        edgeCounts[e2] = edgeCounts[e2] + 1;
      } else {
        edgeCounts[e2] = 1;
      }
    }

    std::unordered_map<EdgeKey, int>::iterator itr = edgeCounts.begin();

    uint32_t borderEdges = 0;
    while(itr != edgeCounts.end()) {
      VertexKey v1 = itr->first.v1;
      VertexKey v2 = itr->first.v2;
      EdgeKey reverseKey(v2, v1);
      if(!edgeCounts.count(reverseKey) || itr->second > edgeCounts[reverseKey]) {
        if(edgeCounts.count(reverseKey)) {
          borderEdges += itr->second-edgeCounts[reverseKey];
        } else {
          borderEdges += itr->second;
        }
      }
      ++itr;
    }

    std::cout << borderEdges << std::endl;

    return 0;
}
