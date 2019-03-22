/*

Copyright 2018 by Freakin' Sweet Apps, LLC (stl_cmd@freakinsweetapps.com)

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
#include "csgjs/util.h"
#include "csgjs/Trees.h"
#include "csgjs/math/Matrix4x4.h"
#include "csgjs/math/HashKeys.h"
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <cstring>

#ifndef M_PI
#define M_PI 3.141592653589
#endif

using namespace csgjs;

void print_usage() {
    fprintf(stderr, "stl_flat orients an STL file so the plane with the largest surface area of triangles that doesn't have any triangles in front of it is facing down.\n\n");
    fprintf(stderr, "usage: stl_flat <input file> [ <output file> ]\n");
    fprintf(stderr, "    Orients the STL file so the plane with the largest surface area of triangles that doesn't have any triangles in front of it is facing down.\n");
}

typedef struct {
    std::unordered_map<PlaneKey, csgjs_real> area;
    bool operator() (PlaneKey a, PlaneKey b) {
      return area[a] > area[b];
    }
} SortPlanesByArea;

int main(int argc, char** argv) {
    if(argc >= 2) {
        if(strcmp(argv[1], "--help") == 0) {
            print_usage();
            exit(2);
        }
    }

    char *in_filename;
    if(argc > 1) {
      in_filename = argv[1];
    } else {
      print_usage();
      exit(2);
    }

    char *out_filename;
    if(argc > 2) {
      out_filename = argv[2];
    } else {
      print_usage();
      exit(2);
    }

    SortPlanesByArea areaSorter;

    std::vector<Polygon> polys = ReadSTLFile(in_filename);

    std::vector<Polygon>::iterator polyItr = polys.begin();
    while(polyItr != polys.end()) {
      PlaneKey key(polyItr->plane);

      Vector3 a = polyItr->vertices[1].pos-polyItr->vertices[0].pos;
      Vector3 b = polyItr->vertices[2].pos-polyItr->vertices[0].pos;
      
      areaSorter.area[key] += a.cross(b).length()*.5;

      ++polyItr;
    }

    std::vector<PlaneKey> planes;
    std::unordered_map<PlaneKey, csgjs_real>::iterator aItr = areaSorter.area.begin();
    while(aItr != areaSorter.area.end()) {
      planes.push_back(aItr->first);
      ++aItr;
    }

    std::sort(planes.begin(), planes.end(), areaSorter);

    // once quickhull is implemented for use in stl_hull, using the convex hull rather than the
    // actual model would probably go much faster for large models
    // potentially using an unordered_map to look up planes in the convex hull rather than needing the 
    // binary space partitioning tree at all
    Tree tree(polys);

    std::vector<PlaneKey>::iterator pItr = planes.begin();
    while(pItr != planes.end()) {
      if(!tree.hasPolygonsInFront(pItr->plane)) {
        std::cout << "Orienting to plane with total surface area of " << areaSorter.area[*pItr] << std::endl;
        csgjs_real dotWithNegativeZ = pItr->plane.normal.dot(Vector3(0,0,-1));
        if(1-dotWithNegativeZ < EPS && 1-dotWithNegativeZ > NEG_EPS) {
          // plane is already aligned, no transform necessary
          WriteSTLFile(out_filename, polys);
          exit(0);
        } else if(-1-dotWithNegativeZ < EPS && -1-dotWithNegativeZ > NEG_EPS) {
          // plane is oriented 180 degrees 
          Vector3 axis = Vector3(1,0,0);
          csgjs_real angle = M_PI;

          Matrix4x4 xform = Matrix4x4::rotate(axis, angle);
          std::vector<Polygon> transformedPolys;
          std::vector<Polygon>::iterator polyItr = polys.begin();
          while(polyItr != polys.end()) {
            transformedPolys.push_back(polyItr->transform(xform));
            ++polyItr;
          }
          WriteSTLFile(out_filename, transformedPolys);
          exit(0);
        } else {
          // plane needs to be aligned
          Vector3 axis = pItr->plane.normal.cross(Vector3(0,0,-1)).unit();
          csgjs_real angle = acos(pItr->plane.normal.dot(Vector3(0,0,-1)));

          Matrix4x4 xform = Matrix4x4::rotate(axis, angle);
          std::vector<Polygon> transformedPolys;
          std::vector<Polygon>::iterator polyItr = polys.begin();
          while(polyItr != polys.end()) {
            transformedPolys.push_back(polyItr->transform(xform));
            ++polyItr;
          }
          WriteSTLFile(out_filename, transformedPolys);
          exit(0);
        }
      }
      ++pItr;
    }

    return 0;
}
