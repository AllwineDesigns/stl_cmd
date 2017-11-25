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
#include <iostream>

#include "csgjs/CSG.h"
#include "csgjs/Trees.h"
#include "csgjs/util.h"

using namespace csgjs;

int main(int argc, char **argv) {
  std::vector<Polygon> p = {
    Polygon({ 
      Vertex(Vector3(0,0,1)), 
      Vertex(Vector3(1,0,1)), 
      Vertex(Vector3(1,1,1)), 
      Vertex(Vector3(0,1,1)) }),
    Polygon({ 
      Vertex(Vector3(0,0,0)), 
      Vertex(Vector3(0,1,0)), 
      Vertex(Vector3(1,1,0)), 
      Vertex(Vector3(1,0,0)) }),
    Polygon({ 
      Vertex(Vector3(0,0,0)), 
      Vertex(Vector3(0,0,1)), 
      Vertex(Vector3(0,1,1)), 
      Vertex(Vector3(0,1,0)) }),
    Polygon({ 
      Vertex(Vector3(1,0,0)), 
      Vertex(Vector3(1,1,0)), 
      Vertex(Vector3(1,1,1)), 
      Vertex(Vector3(1,0,1)) }),
    Polygon({ 
      Vertex(Vector3(0,0,0)), 
      Vertex(Vector3(1,0,0)), 
      Vertex(Vector3(1,0,1)), 
      Vertex(Vector3(0,0,1)) }),
    Polygon({ 
      Vertex(Vector3(0,1,0)), 
      Vertex(Vector3(0,1,1)), 
      Vertex(Vector3(1,1,1)), 
      Vertex(Vector3(1,1,0)) })
  };

//  std::vector<Polygon> stl_file = ReadSTLFile("model.stl");
  std::vector<Polygon> stl_file = ReadSTLFile("torus.stl");

//  CSG csg(stl_file);
//  CSG csg2 = csg.transform(Matrix4x4::translate(-2,0,0));

//  CSG both = csg.csgUnion(csg2);

//  std::cout << csg << std::endl;
//  std::cout << csg2 << std::endl;
//  std::cout << both << std::endl;

  Tree tree(stl_file);
//  Tree tree(p);

  std::cout << tree << std::endl;
}
