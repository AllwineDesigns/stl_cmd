C++ port of CSG.js
==================
This a C++ port of CSG.js used in OpenJSCAD (https://github.com/jscad/csg.js). OpenJSCAD's CSG.js has significant performance
improvements over the original CSG.js (https://github.com/evanw/csg.js/), which was ported to C++ here (https://github.com/dabroz/csgjs-cpp).
The speed of OpenJSCAD's CSG.js implmented in JavaScript is faster than the original C++ port, so the idea is
to get those gains and more by implementing OpenJSCAD's CSG.js in C++.

So far just the bare minimum is implemented, with the ability to do CSG operations without the retessellation feature of CSG.js (reduces
the number of polygons in the end result). The stl_boolean command leverages this implementation to be able to do CSG operations on
two STL files. Without the overhead of JavaScript, a single operation runs very quickly.
