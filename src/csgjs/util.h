#ifndef __CSGJS_UTIL__
#define __CSGJS_UTIL__

#include "math/Polygon3.h"
#include <vector>
#include <stdio.h>

namespace csgjs {

  std::vector<Polygon> ReadSTLFile(const char* filename);
  void WriteSTLFile(const char* filename, const std::vector<Polygon> polygons);

  unsigned long xorshf96(void);
  int fastRandom(int max);
}

#endif
