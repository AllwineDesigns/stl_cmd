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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openvdb/openvdb.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/math/Transform.h>
#include <openvdb/tools/Composite.h>
#include <iostream>

#include "stl_util.h"

using namespace openvdb::v2_3_0;

void ReadMesh(const char *filename, std::vector<Vec3s>& verts,
                std::vector<Vec3I>& tris) {
    std::cout << "reading mesh" << std::endl;
    FILE *f;

    f = fopen(filename, "rb");

    char header[80];
    uint32_t num_tris;
    fread(header, 80, 1, f);
    fread(&num_tris, 4, 1, f);

    uint16_t abc;
    int index = 0;
    for(int i = 0; i < num_tris; i++) {
        float normal[3];
        float p1[3],p2[3],p3[3];
        fread(normal, 12, 1, f);
        fread(p1, 12, 1, f);
        fread(p2, 12, 1, f);
        fread(p3, 12, 1, f);
        fread(&abc, 2, 1, f);
     //   std::cout << p1[0] << ", " << p1[1] << ", " << p1[2] << std::endl;
     //   std::cout << p2[0] << ", " << p2[1] << ", " << p2[2] << std::endl;
     //   std::cout << p3[0] << ", " << p3[1] << ", " << p3[2] << std::endl;
        verts.push_back(Vec3s(p3[0], p3[1], p3[2]));
        verts.push_back(Vec3s(p2[0], p2[1], p2[2]));
        verts.push_back(Vec3s(p1[0], p1[1], p1[2]));
        tris.push_back(Vec3I(index, index+1, index+2));
        index += 3;
     //   std::cout << i << std::endl;
    }
    std::cout << "done reading mesh" << std::endl;

    fclose(f);
}

// taken from stl_threads, should probably be factored out to a shared header/library or something
void write_tri(FILE *f,
                vec *p1, 
                vec *p2, 
                vec *p3, int rev) {
    if(rev) {
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

// taken from stl_threads, should probably be factored out to a shared header/library or something
void write_quad(FILE *f,
                vec *p1, 
                vec *p2, 
                vec *p3, 
                vec *p4,int rev) {
    //write two triangles 1,2,3 and 1,3,4
    if(rev) {
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

void print_usage() {
    fprintf(stderr, "usage: stl_boolean -a <stl file A> -b <stl file B> [ -i ] [ -u ] [ -d ] [ -v <voxel size> ] <output file>\n");
    fprintf(stderr, "    Performs a CSG boolean operation on STL files A and B. First, converts each STL file to sparse volume\n"
                    " level sets using voxel size set by -v (defaults to .5), then performs the CSG operation and outputs a\n"
                    " high resolution mesh with the result (mesh simplification is recommended, someone want to write stl_decimate?).\n"
                    "     -i - performs the intersection of A and B\n"
                    "     -u - performs the union of A and B (default)\n"
                    "     -d - performs the difference of A and B\n");
}

int main(int argc, char **argv)
{
    int errflg = 0;
    char *a_file;
    char *b_file;
    int a_set = 0;
    int b_set = 0;

    int unionAB = 0;
    int intersection = 0;
    int difference = 0;

    float voxel_size = .5;

    int c;

    while((c = getopt(argc, argv, "a:b:iudv:")) != -1) {
        switch(c) {
            case 'a':
                a_set = 1;
                a_file = optarg;
                break;
            case 'b':
                b_set = 1;
                b_file = optarg;
                break;
            case 'i':
                intersection = 1;
                if(unionAB || difference) {
                    fprintf(stderr, "intersection option provided when union or difference already specified.\n");
                    unionAB = 0;
                    difference = 0;
                }
                break;
            case 'u':
                unionAB = 1;
                if(intersection || difference) {
                    fprintf(stderr, "union option provided when intersection or difference already specified.\n");
                    intersection = 0;
                    difference = 0;
                }
                break;
            case 'd':
                difference = 1;
                if(intersection || unionAB) {
                    fprintf(stderr, "difference option provided when intersection or union already specified.\n");
                    intersection = 0;
                    unionAB = 0;
                }
                break;
            case 'v':
                voxel_size = atof(optarg);
                break;
            case '?':
                fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
                errflg++;
                break;
        }
    }

    if(!unionAB && !intersection && !difference) {
        unionAB = 1;
    }

    if(errflg || !(a_set && b_set) || optind >= argc) {
        print_usage();
        exit(2);
    }

    char *out_filename = argv[optind];

    // Initialize the OpenVDB library.  This must be called at least
    // once per program and may safely be called multiple times.
    openvdb::initialize();
    
    std::vector<Vec3s> vertices;
    std::vector<Vec3I> triangles;

    ReadMesh(a_file, vertices, triangles);

    std::vector<Vec3s> vertices_b;
    std::vector<Vec3I> triangles_b;

    ReadMesh(b_file, vertices_b, triangles_b);

    openvdb::math::Transform::Ptr transform = openvdb::math::Transform::createLinearTransform(voxel_size);

    std::cout << "Converting A to levelset..." << std::endl;
    openvdb::FloatGrid::Ptr grid = tools::meshToLevelSet<openvdb::FloatGrid>(*transform, vertices, triangles,float(openvdb::LEVEL_SET_HALF_WIDTH));
    std::cout << "Conversion complete." << std::endl;

    std::cout << "Converting B to levelset..." << std::endl;
    openvdb::FloatGrid::Ptr grid_b = tools::meshToLevelSet<openvdb::FloatGrid>(*transform, vertices_b, triangles_b,float(openvdb::LEVEL_SET_HALF_WIDTH));
    std::cout << "Conversion complete." << std::endl;

    if(unionAB) {
        std::cout << "Performing union..." << std::endl;
        tools::csgUnion(*grid, *grid_b, true);
    }
    if(intersection) {
        std::cout << "Performing intersection..." << std::endl;
        tools::csgIntersection(*grid, *grid_b, true);
    }
    if(difference) {
        std::cout << "Performing difference..." << std::endl;
        tools::csgDifference(*grid, *grid_b, true);
    }

    std::vector<Vec3s> out_verts;
    std::vector<Vec3I> out_tris;
    std::vector<Vec4I> out_quads;

    std::cout << "Converting resulting level set to mesh..." << std::endl;

//    tools::volumeToMesh<openvdb::FloatGrid>(*grid, out_verts, out_tris, out_quads, 0, 1);
    tools::volumeToMesh<openvdb::FloatGrid>(*grid, out_verts, out_quads, 0);
    std::cout << "Conversion complete." << std::endl;

    std::cout << "Writing mesh..." << std::endl;
    FILE *f;

    f = fopen("out.stl", "wb");

    char header[80] = {0};
    strncpy(header, "Created with stl_boolean", 80);
    fwrite(header, 80, 1, f);

    uint32_t num_tris;

    num_tris = out_tris.size()+out_quads.size()*2;
    fwrite(&num_tris, 4, 1, f);

    uint16_t abc = 0;

    for(int i = 0; i < out_tris.size(); i++) {
        vec p1,p2,p3;

        p1.x = out_verts[out_tris[i].x()].x();
        p1.y = out_verts[out_tris[i].x()].y();
        p1.z = out_verts[out_tris[i].x()].z();

        p2.x = out_verts[out_tris[i].y()].x();
        p2.y = out_verts[out_tris[i].y()].y();
        p2.z = out_verts[out_tris[i].y()].z();

        p3.x = out_verts[out_tris[i].z()].x();
        p3.y = out_verts[out_tris[i].z()].y();
        p3.z = out_verts[out_tris[i].z()].z();

        write_tri(f, &p1, &p2, &p3, 1);
    }

    for(int i = 0; i < out_quads.size(); i++) {
        vec p1,p2,p3,p4;

        p1.x = out_verts[out_quads[i].x()].x();
        p1.y = out_verts[out_quads[i].x()].y();
        p1.z = out_verts[out_quads[i].x()].z();

        p2.x = out_verts[out_quads[i].y()].x();
        p2.y = out_verts[out_quads[i].y()].y();
        p2.z = out_verts[out_quads[i].y()].z();

        p3.x = out_verts[out_quads[i].z()].x();
        p3.y = out_verts[out_quads[i].z()].y();
        p3.z = out_verts[out_quads[i].z()].z();

        p4.x = out_verts[out_quads[i].w()].x();
        p4.y = out_verts[out_quads[i].w()].y();
        p4.z = out_verts[out_quads[i].w()].z();

        write_quad(f, &p1, &p2, &p3, &p4, 1);
    }

    fclose(f);
    std::cout << "All done!" << std::endl;
}
