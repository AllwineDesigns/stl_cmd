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
#include <libgen.h>
#include <string.h>
#include <math.h>
#include "stl_util.h"

// TODO make output file optional, if not specified output
// to stdout. Requires calculating how many tris are necessary
// before outputting any. Currently, we write all our tris, then
// fseek back to byte 80 to set the number of tris in the file. That
// won't work with stdout.

void print_usage() {
    fprintf(stderr, "stl_threads outputs an STL file with male or female threads per the ISO metric screw thread standard.\n\n");
    fprintf(stderr, "usage: stl_threads [ -f ] [ -D <diameter> ] [ -P <pitch> ] [ -a <angle> ]\n"
                    "                   [ -h <height> ] [ -s <segments> ] <output file>\n");
    fprintf(stderr, "    Outputs an stl file with male or female screw threads per the ISO metric \n"
                    "    screw thread standard (http://en.wikipedia.org/wiki/ISO_metric_screw_thread).\n"
                    "\n"
                    "    -f - Outputs female threads (defaults to male).\n"
                    "    -D <diameter> - Changes to major diameter of the threads.\n"
                    "    -P <pitch> - Changes the height of a single thread, aka the pitch per the\n"
                    "                 ISO metric standard.\n"
                    "    -h <height> - Changes the total height of the threads.\n"
                    "    -a <angle> - Changes the thread angle (degrees). The standard (and default)\n"
                    "                 is 60 degrees. For 3D printing this can cause overhang issues \n"
                    "                 as 60 degrees results in a 30 degree angle with the ground\n"
                    "                 plane. Setting to 90 degrees results in a 45 degree angle with\n"
                    "                 the ground plane.\n"
                    "    -s <segments> - Changes the resolution of the generated STL file. More\n"
                    "                    segments yields finer resolution. <segments> is the number\n"
                    "                    of segments to approximate a circle. Defaults to 72 (every\n"
                    "                    5 degrees).\n"
                    "    -o <outer female diameter> - When generating female threads, this is the\n"
                    "                                 outer diameter. Must be greater than <diameter>.\n");
}

void print_normal(vec *p1, vec *p2, vec *p3, int rev) {
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

    printf("%f, %f, %f\n", n1.x, n1.y, n1.z);
}

void write_poly(FILE *f, vec *poly, int num_pts, int rev) {
    for(int i = 1; i < num_pts-1; i++) {
        write_tri(f, &poly[0], &poly[i], &poly[i+1], rev);
    }
}

typedef enum {
    ABOVE,
    BELOW,
    ON
} point_state_t;

point_state_t point_state(vec *pt, vec *p, vec *n) { // return whether pt is above, below or on the plane defined by p and n
    point_state_t state;
    vec tmp;
    float dot;

    vec_sub(pt,p,&tmp);
    dot = vec_dot(&tmp,n);

    if(dot > EPSILON) {
        state = ABOVE;
    } else if(dot < -EPSILON) {
        state = BELOW;
    } else {
        state = ON;
    }

    return state;
}

void tri_normal(vec *p1, vec *p2, vec *p3, int rev, vec *out) {
    if(rev) {
        vec *tmp = p1;
        p1 = p3;
        p3 = tmp;
    }

    vec v1,v2;

    vec_sub(p2,p1,&v1);
    vec_sub(p3,p1,&v2);

    vec_cross(&v1,&v2,out);
    vec_normalize(out,out);
}

// returns true if triangle intersects plane, int1 and int2 will be set
// to the intersection points
// returns false if all points are above or below the plane
int write_sliced_tri(FILE *f,
                      vec *p1,
                      vec *p2,
                      vec *p3,
                      int rev,
                      vec *p,  // point that lies in the slicing plane
                      vec *n,   // normal of slicing plane, keep points in the direction of normal
                      vec *int1, // output, intersecting pt1, returned only if intersection occurs
                      vec *int2, // output, intersecting pt2, returned only if intersection occurs
                      int *tris) // output, number of tris written
                      {
    vec trin;
    tri_normal(p1,p2,p3,rev,&trin);

    point_state_t state1;
    point_state_t state2;
    point_state_t state3;

    state1 = point_state(p1,p,n);
    state2 = point_state(p2,p,n);
    state3 = point_state(p3,p,n);

    if((state1 == ABOVE || state1 == ON) &&
       (state2 == ABOVE || state2 == ON) &&
       (state3 == ABOVE || state3 == ON)) {
        // all points are above or on plane, write tri normally
        write_tri(f,p1,p2,p3,rev);
        *tris = 1;
        return 0;
    } else if((state1 == BELOW || state1 == ON) &&
              (state2 == BELOW || state2 == ON) &&
              (state3 == BELOW || state3 == ON)) {
        // all points are below or on plane, cull entire tri
        *tris = 0;
        return 0;
    } else {
        // triangle intersects plane

        if(state1 == ON) {
            vec tmp1,tmp2;
            float r;

            vec_copy(p1, int1);

            vec_sub(p2,p3,&tmp1);
            vec_sub(p,p3,&tmp2);
            r = vec_dot(n,&tmp2)/vec_dot(n,&tmp1);

            int2->x = p3->x + r*tmp1.x;
            int2->y = p3->y + r*tmp1.y;
            int2->z = p3->z + r*tmp1.z;

            if(state2 == ABOVE) {
                write_tri(f,p1,p2,int2,rev);
            } else {
                write_tri(f,p1,int2,p3,rev);
            }
            *tris = 1;
            return 1;
        }

        if(state2 == ON) {
            vec tmp1,tmp2;
            float r;

            vec_copy(p2, int1);

            vec_sub(p1,p3,&tmp1);
            vec_sub(p,p3,&tmp2);
            r = vec_dot(n,&tmp2)/vec_dot(n,&tmp1);

            int2->x = p3->x + r*tmp1.x;
            int2->y = p3->y + r*tmp1.y;
            int2->z = p3->z + r*tmp1.z;

            if(state1 == ABOVE) {
                write_tri(f,p1,p2,int2,rev);
            } else {
                write_tri(f,p2,p3,int2,rev);
            }
            *tris = 1;
            return 1;
        }

        if(state3 == ON) {
            vec tmp1,tmp2;
            float r;

            vec_copy(p3, int2);

            vec_sub(p1,p2,&tmp1);
            vec_sub(p,p2,&tmp2);
            r = vec_dot(n,&tmp2)/vec_dot(n,&tmp1);

            int1->x = p2->x + r*tmp1.x;
            int1->y = p2->y + r*tmp1.y;
            int1->z = p2->z + r*tmp1.z;

            if(state1 == ABOVE) {
                write_tri(f,p1,int1,p3,rev);
            } else {
                write_tri(f,int1,p2,p3,rev);
            }
            *tris = 1;
            return 1;
        }

        if((state1 == ABOVE && state2 == ABOVE) ||
            (state1 == BELOW && state2 == BELOW)) {
            vec tmp1,tmp2,tmp3;
            float r1,r2;

            vec_sub(p2,p3,&tmp1);
            vec_sub(p1,p3,&tmp2);
            vec_sub(p,p3,&tmp3);

            r1 = vec_dot(n,&tmp3)/vec_dot(n,&tmp1);
            r2 = vec_dot(n,&tmp3)/vec_dot(n,&tmp2);

            int1->x = p3->x + r1*tmp1.x;
            int1->y = p3->y + r1*tmp1.y;
            int1->z = p3->z + r1*tmp1.z;

            int2->x = p3->x + r2*tmp2.x;
            int2->y = p3->y + r2*tmp2.y;
            int2->z = p3->z + r2*tmp2.z;

            if(state3 == ABOVE) {
                write_tri(f,int1,p3,int2,rev);
                *tris = 1;
            } else {
                write_quad(f,p1,p2,int1,int2,rev);
                *tris = 2;
            }
            return 1;
        }

        if((state2 == ABOVE && state3 == ABOVE) ||
            (state2 == BELOW && state3 == BELOW)) {
            vec tmp1,tmp2,tmp3;
            float r1,r2;

            vec_sub(p2,p1,&tmp1);
            vec_sub(p3,p1,&tmp2);
            vec_sub(p,p1,&tmp3);

            r1 = vec_dot(n,&tmp3)/vec_dot(n,&tmp1);
            r2 = vec_dot(n,&tmp3)/vec_dot(n,&tmp2);

            int1->x = p1->x + r1*tmp1.x;
            int1->y = p1->y + r1*tmp1.y;
            int1->z = p1->z + r1*tmp1.z;

            int2->x = p1->x + r2*tmp2.x;
            int2->y = p1->y + r2*tmp2.y;
            int2->z = p1->z + r2*tmp2.z;

            if(state1 == ABOVE) {
                write_tri(f,p1,int1,int2,rev);
                *tris = 1;
            } else {
                write_quad(f,int1,p2,p3,int2,rev);
                *tris = 2;
            }
            return 1;
        }

        if((state1 == ABOVE && state3 == ABOVE) ||
            (state1 == BELOW && state3 == BELOW)) {
            vec tmp1,tmp2,tmp3;
            float r1,r2;

            vec_sub(p1,p2,&tmp1);
            vec_sub(p3,p2,&tmp2);
            vec_sub(p,p2,&tmp3);

            r1 = vec_dot(n,&tmp3)/vec_dot(n,&tmp1);
            r2 = vec_dot(n,&tmp3)/vec_dot(n,&tmp2);

            int1->x = p2->x + r1*tmp1.x;
            int1->y = p2->y + r1*tmp1.y;
            int1->z = p2->z + r1*tmp1.z;

            int2->x = p2->x + r2*tmp2.x;
            int2->y = p2->y + r2*tmp2.y;
            int2->z = p2->z + r2*tmp2.z;

            if(state2 == ABOVE) {
                write_tri(f,int1,p2,int2,rev);
                *tris = 1;
            } else {
                write_quad(f,p1,int1,int2,p3,rev);
                *tris = 2;
            }
            return 1;
        }
    }


    *tris = 0;
    return 0;
}

int main(int argc, char** argv) {
    if(argc >= 2) {
        if(strcmp(argv[1], "--help") == 0) {
            print_usage();
            exit(2);
        }
    }
    int c;
    int errflg = 0;

    int male = 1;
    float D = 9.0;        // default value, can be changed as a command line argument
    float P = 2;          // default value, can be changed as a command line argument
    float theta = M_PI/3; // 60 degrees, the standard, can be changed as a command line argument

    float screwHeight = 20; // height of entire screw (without a head)
                            // TODO maybe add ability to add a head
    float outerDiameter = -1;

    int segments = 72; // number of segments to approximate a circle,
                        // higher the number, higher the resolution
                        // (and file size) of the stl file

    while((c = getopt(argc, argv, "fP:D:a:h:s:o:")) != -1) {
        switch(c) {
            case 'f':
                male = 0;
                break;
            case 'P':
                P = atof(optarg);
                break;
            case 'D':
                D = atof(optarg);
                break;
            case 'a':
                theta = atof(optarg)*M_PI/180;
                break;
            case 'h':
                screwHeight = atof(optarg);
                break;
            case 's':
                segments = atoi(optarg);
                break;
            case 'o':
                outerDiameter = atof(optarg);
                break;
            case '?':
                fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
                errflg++;
                break;
        }
    }

    if(outerDiameter <= D) {
        outerDiameter = D+1;
    }

    if(errflg || optind >= argc) {
        print_usage();
        exit(2);
    }

    char *file = argv[optind];

    FILE *outf;

    outf = fopen(file, "wb");
    if(!outf) {
        fprintf(stderr, "Can't write to file: %s\n", file);
        exit(2);
    }

    char header[81] = {0};
    // TODO add some settings summary to header string
    snprintf(header, 81, "Created with stl_threads.");

    fwrite(header, 80, 1, outf);

    // writing 0 to num_tris for now, will update it at the end.
    uint32_t num_tris = 0;
    fwrite(&num_tris, 4, 1, outf);

    // ISO metric screw thread standard designates screw threads
    // as M followed by diameter D, a multiplication sign, and then
    // the pitch P (e.g. M8x1.25). Other standards designate standard
    // pitches for a given diameter (e.g. M8 means the same as M8x1.25).
    //
    // See http://en.wikipedia.org/wiki/ISO_metric_screw_thread
    //
    // The standard assumes a theta of 60 degrees, but we allow
    // theta to change, for ease of manufacturing. Certain 3D printers
    // may not be able to print without drooping at 60 degrees.

    int female = !male;

    // equations vary from wikipedia because we're not assuming a 60 degree theta
    // and we're allowing cutting off different amounts than H/8
    // and H/4 from the tip and troughs

    float tantheta_2 = tan(theta/2);
    float H = P/(2*tantheta_2);
    float Htip = H/8;
    float Htrough = H/4;
    float Hdiff = H-Htip-Htrough;

    float Pdiff = Hdiff*tantheta_2;
    float Ptip = 2*Htip*tantheta_2;
    float Ptrough = 2*Htrough*tantheta_2;

    float Dmin = D-2*Hdiff;
    float Dmin_2 = Dmin/2;
    float D_2 = D/2;
    float fD = outerDiameter/2;


    /*
    // my ascii representation of image at
    // (http://en.wikipedia.org/wiki/ISO_metric_screw_thread)
    // with my own added variables

                |<--H->|
                |      |
    |           |      |
    |-------------. pt5|   ------   // pt5 is the same as pt1 one cycle later
    |    ^      |/|    |      ^
    |    |      . |    |     Ptrough
    |    |       \|    |      v
    |    |        . pt4|   ------
    |    |         \   |
    |    |        ( \  |
    |            (   . pt3  ------
    |    P      (    |\        ^
    |          theta | .       Ptip
    |    |      (    |/        v
    |    |       (   . pt2  ------
    |    |        ( /          ^
    |    v         / |         Pdiff
    |-------------. pt1     ------
    |            /|  |
    |           . |  |
    |<---Dmin/2-->|  |
    |                |
    |                |
    |<-----D/2------>|
    */

    vec pt1,pt2,pt3,pt4,pt5;

    pt1.x = Dmin_2;
    pt1.y = 0;
    pt1.z = 0;
    pt1.w = 1;

    pt2.x = D_2;
    pt2.y = 0;
    pt2.z = Pdiff;
    pt2.w = 1;

    pt3.x = D_2;
    pt3.y = 0;
    pt3.z = Pdiff+Ptip;
    pt3.w = 1;

    pt4.x = Dmin_2;
    pt4.y = 0;
    pt4.z = 2*Pdiff+Ptip;
    pt4.w = 1;

    pt5.x = Dmin_2; // not used, instead pt1 one full cycle later is used
    pt5.y = 0;      // to avoid floating point errors
    pt5.z = P;
    pt5.w = 1;

    int total_segments = (screwHeight/P-1)*segments;

    vec origin = {0};
    vec top = {0};
    top.z = screwHeight;

    vec up = {0};
    up.z = 1;

    vec down = {0};
    down.z = -1;

    float anginc = (float)360/segments;
    vec sliced8Int;
    int needsExtraTris = 0;

    for(int i = -segments; i < total_segments+segments; i++) {
        vec p1,p2,p3,p4,p5;
        vec p1n,p2n,p3n,p4n,p5n;

        vec ob1,ob1n;
        vec ot1,ot1n;

        float ango = (float)360*((i+segments)%segments)/segments;
        float angno = (float)360*((i+segments)%segments+1)/segments;

        float cosao = cos(ango*M_PI/180);
        float sinao = sin(ango*M_PI/180);

        float cosano = cos(angno*M_PI/180);
        float sinano = sin(angno*M_PI/180);

        ob1.x = fD*cosao;
        ob1.y = fD*sinao;
        ob1.z = 0;

        ob1n.x = fD*cosano;
        ob1n.y = fD*sinano;
        ob1n.z = 0;

        ot1.x = fD*cosao;
        ot1.y = fD*sinao;
        ot1.z = screwHeight;

        ot1n.x = fD*cosano;
        ot1n.y = fD*sinano;
        ot1n.z = screwHeight;

        float ang = (float)360*i/segments;
        float angn = (float)360*(i+1)/segments;
        float angc = (float)360*(i+segments)/segments;
        float angnc = (float)360*(i+1+segments)/segments;

        float cosa = cos(ang*M_PI/180);
        float sina = sin(ang*M_PI/180);

        float cosan = cos(angn*M_PI/180);
        float sinan = sin(angn*M_PI/180);

        float cosanc = cos(angnc*M_PI/180);
        float sinanc = sin(angnc*M_PI/180);

        float cosac = cos(angc*M_PI/180);
        float sinac = sin(angc*M_PI/180);

        float z = P*ang/360;
        float zn = P*angn/360;
        float zc = P*angc/360;
        float znc = P*angnc/360;

        mat t;
        mat tn;
        mat tc;
        mat tnc;

        t.xx = cosa;
        t.xy = sina;
        t.xz = 0;
        t.xw = 0;

        t.yx = -sina;
        t.yy = cosa;
        t.yz = 0;
        t.yw = 0;

        t.zx = 0;
        t.zy = 0;
        t.zz = 1;
        t.zw = 0;

        t.tx = 0;
        t.ty = 0;
        t.tz = z;
        t.tw = 1;

        tn.xx = cosan;
        tn.xy = sinan;
        tn.xz = 0;
        tn.xw = 0;

        tn.yx = -sinan;
        tn.yy = cosan;
        tn.yz = 0;
        tn.yw = 0;

        tn.zx = 0;
        tn.zy = 0;
        tn.zz = 1;
        tn.zw = 0;

        tn.tx = 0;
        tn.ty = 0;
        tn.tz = zn;
        tn.tw = 1;

        tnc.xx = cosanc;
        tnc.xy = sinanc;
        tnc.xz = 0;
        tnc.xw = 0;

        tnc.yx = -sinanc;
        tnc.yy = cosanc;
        tnc.yz = 0;
        tnc.yw = 0;

        tnc.zx = 0;
        tnc.zy = 0;
        tnc.zz = 1;
        tnc.zw = 0;

        tnc.tx = 0;
        tnc.ty = 0;
        tnc.tz = znc;
        tnc.tw = 1;

        tc.xx = cosac;
        tc.xy = sinac;
        tc.xz = 0;
        tc.xw = 0;

        tc.yx = -sinac;
        tc.yy = cosac;
        tc.yz = 0;
        tc.yw = 0;

        tc.zx = 0;
        tc.zy = 0;
        tc.zz = 1;
        tc.zw = 0;

        tc.tx = 0;
        tc.ty = 0;
        tc.tz = zc;
        tc.tw = 1;

        vec_mat_mult(&pt1, &t, &p1);
        vec_mat_mult(&pt2, &t, &p2);
        vec_mat_mult(&pt3, &t, &p3);
        vec_mat_mult(&pt4, &t, &p4);
        vec_mat_mult(&pt1, &tc, &p5); // using pt1 with a transform of 1
                                      // full cycle around to avoid
                                      // floating point roundoff errors

        vec_mat_mult(&pt1, &tn, &p1n);
        vec_mat_mult(&pt2, &tn, &p2n);
        vec_mat_mult(&pt3, &tn, &p3n);
        vec_mat_mult(&pt4, &tn, &p4n);
        vec_mat_mult(&pt1, &tnc, &p5n); // using pt1 with a transform of 1
                                        // full cycle around to avoid
                                        // floating point roundoff errors

        if(i < 0) {
            int wrote_outer_tri = 0;

            vec int1,int2;
            int tris_written;
            if(write_sliced_tri(outf, &p1,&p1n,&p2n,female,&origin,&up,&int1,&int2,&tris_written)) {
                if(male) {
                    write_tri(outf,&int2,&origin,&int1,0);
                    num_tris += 1;
                } else {
                    if(wrote_outer_tri) {
                        write_tri(outf,&int2,&int1,&ob1,0);
                        num_tris += 1;
                    } else {
                        write_tri(outf,&int1,&ob1n,&ob1,0);
                        write_tri(outf,&int2,&int1,&ob1,0);
                        wrote_outer_tri = 1;
                        num_tris += 2;
                    }
                }
            }
            num_tris += tris_written;

            if(write_sliced_tri(outf, &p1,&p2n,&p2,female,&origin,&up,&int1,&int2,&tris_written)) {
                if(male) {
                    write_tri(outf,&int2,&origin,&int1,female);
                    num_tris += 1;
                } else {
                    if(wrote_outer_tri) {
                        write_tri(outf,&int2,&int1,&ob1,0);
                        num_tris += 1;
                    } else {
                        write_tri(outf,&int1,&ob1n,&ob1,0);
                        write_tri(outf,&int2,&int1,&ob1,0);
                        wrote_outer_tri = 1;
                        num_tris += 2;
                    }
                }
            }
            num_tris += tris_written;

            if(write_sliced_tri(outf, &p2,&p2n,&p3n,female,&origin,&up,&int1,&int2,&tris_written)) {
                if(male) {
                    write_tri(outf,&int2,&origin,&int1,female);
                    num_tris += 1;
                } else {
                    if(wrote_outer_tri) {
                        write_tri(outf,&int2,&int1,&ob1,0);
                        num_tris += 1;
                    } else {
                        write_tri(outf,&int1,&ob1n,&ob1,0);
                        write_tri(outf,&int2,&int1,&ob1,0);
                        wrote_outer_tri = 1;
                        num_tris += 2;
                    }
                }
            }
            num_tris += tris_written;


            if(write_sliced_tri(outf, &p2,&p3n,&p3,female,&origin,&up,&int1,&int2,&tris_written)) {
                if(male) {
                    write_tri(outf,&int2,&origin,&int1,female);
                    num_tris += 1;
                } else {
                    if(wrote_outer_tri) {
                        write_tri(outf,&int2,&int1,&ob1,0);
                        num_tris += 1;
                    } else {
                        write_tri(outf,&int1,&ob1n,&ob1,0);
                        write_tri(outf,&int2,&int1,&ob1,0);
                        wrote_outer_tri = 1;
                        num_tris += 2;
                    }
                }
            }
            num_tris += tris_written;

            if(write_sliced_tri(outf, &p3,&p3n,&p4n,female,&origin,&up,&int1,&int2,&tris_written)) {
                if(male) {
                    write_tri(outf,&int2,&origin,&int1,female);
                    num_tris += 1;
                } else {
                    if(wrote_outer_tri) {
                        write_tri(outf,&int2,&int1,&ob1,0);
                        num_tris += 1;
                    } else {
                        write_tri(outf,&int1,&ob1n,&ob1,0);
                        write_tri(outf,&int2,&int1,&ob1,0);
                        wrote_outer_tri = 1;
                        num_tris += 2;
                    }
                }
            }
            num_tris += tris_written;

            if(write_sliced_tri(outf, &p3,&p4n,&p4,female,&origin,&up,&int1,&int2,&tris_written)) {
                if(male) {
                    write_tri(outf,&int2,&origin,&int1,female);
                    num_tris += 1;
                } else {
                    if(wrote_outer_tri) {
                        write_tri(outf,&int2,&int1,&ob1,0);
                        num_tris += 1;
                    } else {
                        write_tri(outf,&int1,&ob1n,&ob1,0);
                        write_tri(outf,&int2,&int1,&ob1,0);
                        wrote_outer_tri = 1;
                        num_tris += 2;
                    }
                }
            }
            num_tris += tris_written;

            if(write_sliced_tri(outf, &p4,&p4n,&p5n,female,&origin,&up,&int1,&int2,&tris_written)) {
                if(male) {
                    write_tri(outf,&int2,&origin,&int1,female);
                    num_tris += 1;
                } else {
                    if(wrote_outer_tri) {
                        write_tri(outf,&int2,&int1,&ob1,0);
                        num_tris += 1;
                    } else {
                        write_tri(outf,&int1,&ob1n,&ob1,0);
                        write_tri(outf,&int2,&int1,&ob1,0);
                        wrote_outer_tri = 1;
                        num_tris += 2;
                    }
                }
            }
            num_tris += tris_written;

            if(write_sliced_tri(outf, &p4,&p5n,&p5,female,&origin,&up,&int1,&int2,&tris_written)) {
                if(male) {
                    write_tri(outf,&int2,&origin,&int1,female);
                    num_tris += 1;
                } else {
                    if(wrote_outer_tri) {
                        write_tri(outf,&int2,&int1,&ob1,0);
                        num_tris += 1;
                    } else {
                        write_tri(outf,&int1,&ob1n,&ob1,0);
                        write_tri(outf,&int2,&int1,&ob1,0);
                        wrote_outer_tri = 1;
                        num_tris += 2;
                    }
                }
            }
            num_tris += tris_written;

        } else if(i >= total_segments) {
            int wrote_outer_tri = 0;
            vec int1,int2;
            int tris_written;
            if(write_sliced_tri(outf, &p1,&p1n,&p2n,female,&top,&down,&int1,&int2,&tris_written)) {
                if(male) {
                    write_tri(outf,&int1,&top,&int2,female);
                    num_tris += 1;
                } else {
                    if(wrote_outer_tri) {
                        write_tri(outf,&int2,&int1,&ot1,1);
                        num_tris += 1;
                    } else {
                        write_tri(outf,&int1,&ot1n,&ot1,1);
                        write_tri(outf,&int2,&int1,&ot1,1);
                        wrote_outer_tri = 1;
                        num_tris += 2;
                    }
                }

                if(i == total_segments+segments-1 && needsExtraTris) {
                    write_tri(outf, &int1,&p1n,&sliced8Int,female);
                    num_tris += 1;

                    if(male) {
                        write_tri(outf, &int1,&sliced8Int,&top,female);
                        num_tris += 1;
                    } else {
                        write_tri(outf, &int1,&sliced8Int,&ot1n,female);
                        num_tris += 1;
                    }
                }
            }
            num_tris += tris_written;

            if(write_sliced_tri(outf, &p1,&p2n,&p2,female,&top,&down,&int1,&int2,&tris_written)) {
                if(male) {
                    write_tri(outf,&int1,&top,&int2,female);
                    num_tris += 1;
                } else {
                    if(wrote_outer_tri) {
                        write_tri(outf,&int2,&int1,&ot1,1);
                        num_tris += 1;
                    } else {
                        write_tri(outf,&int1,&ot1n,&ot1,1);
                        write_tri(outf,&int2,&int1,&ot1,1);
                        wrote_outer_tri = 1;
                        num_tris += 2;
                    }
                }
            }
            num_tris += tris_written;

            if(write_sliced_tri(outf, &p2,&p2n,&p3n,female,&top,&down,&int1,&int2,&tris_written)) {
                if(male) {
                    write_tri(outf,&int1,&top,&int2,female);
                    num_tris += 1;
                } else {
                    if(wrote_outer_tri) {
                        write_tri(outf,&int2,&int1,&ot1,1);
                        num_tris += 1;
                    } else {
                        write_tri(outf,&int1,&ot1n,&ot1,1);
                        write_tri(outf,&int2,&int1,&ot1,1);
                        wrote_outer_tri = 1;
                        num_tris += 2;
                    }
                }
            }
            num_tris += tris_written;


            if(write_sliced_tri(outf, &p2,&p3n,&p3,female,&top,&down,&int1,&int2,&tris_written)) {
                if(male) {
                    write_tri(outf,&int1,&top,&int2,female);
                    num_tris += 1;
                } else {
                    if(wrote_outer_tri) {
                        write_tri(outf,&int2,&int1,&ot1,1);
                        num_tris += 1;
                    } else {
                        write_tri(outf,&int1,&ot1n,&ot1,1);
                        write_tri(outf,&int2,&int1,&ot1,1);
                        wrote_outer_tri = 1;
                        num_tris += 2;
                    }
                }
            }
            num_tris += tris_written;

            if(write_sliced_tri(outf, &p3,&p3n,&p4n,female,&top,&down,&int1,&int2,&tris_written)) {
                if(male) {
                    write_tri(outf,&int1,&top,&int2,female);
                    num_tris += 1;
                } else {
                    if(wrote_outer_tri) {
                        write_tri(outf,&int2,&int1,&ot1,1);
                        num_tris += 1;
                    } else {
                        write_tri(outf,&int1,&ot1n,&ot1,1);
                        write_tri(outf,&int2,&int1,&ot1,1);
                        wrote_outer_tri = 1;
                        num_tris += 2;
                    }
                }
            }
            num_tris += tris_written;

            if(write_sliced_tri(outf, &p3,&p4n,&p4,female,&top,&down,&int1,&int2,&tris_written)) {
                if(male) {
                    write_tri(outf,&int1,&top,&int2,female);
                    num_tris += 1;
                } else {
                    if(wrote_outer_tri) {
                        write_tri(outf,&int2,&int1,&ot1,1);
                        num_tris += 1;
                    } else {
                        write_tri(outf,&int1,&ot1n,&ot1,1);
                        write_tri(outf,&int2,&int1,&ot1,1);
                        wrote_outer_tri = 1;
                        num_tris += 2;
                    }
                }
            }
            num_tris += tris_written;

            if(write_sliced_tri(outf, &p4,&p4n,&p5n,female,&top,&down,&int1,&int2,&tris_written)) {
                if(male) {
                    write_tri(outf,&int1,&top,&int2,female);
                    num_tris += 1;
                } else {
                    if(wrote_outer_tri) {
                        write_tri(outf,&int2,&int1,&ot1,1);
                        num_tris += 1;
                    } else {
                        write_tri(outf,&int1,&ot1n,&ot1,1);
                        write_tri(outf,&int2,&int1,&ot1,1);
                        wrote_outer_tri = 1;
                        num_tris += 2;
                    }
                }
            }
            num_tris += tris_written;

            if(write_sliced_tri(outf, &p4,&p5n,&p5,female,&top,&down,&int1,&int2,&tris_written)) {
                if(male) {
                    write_tri(outf,&int1,&top,&int2,female);
                    num_tris += 1;
                } else {
                    if(wrote_outer_tri) {
                        write_tri(outf,&int2,&int1,&ot1,1);
                        num_tris += 1;
                    } else {
                        write_tri(outf,&int1,&ot1n,&ot1,1);
                        write_tri(outf,&int2,&int1,&ot1,1);
                        wrote_outer_tri = 1;
                        num_tris += 2;
                    }
                }
                if(i == total_segments && !vec_equals(&int2,&p5)) {
                    needsExtraTris = 1;
                    vec_copy(&int2,&sliced8Int);
                }
            }
            num_tris += tris_written;
        } else {
            write_quad(outf, &p1,&p1n,&p2n,&p2,female);
            write_quad(outf, &p2,&p2n,&p3n,&p3,female);
            write_quad(outf, &p3,&p3n,&p4n,&p4,female);
            write_quad(outf, &p4,&p4n,&p5n,&p5,female);
            num_tris += 8;
        }
    }

    if(female) {
        for(int i = 0; i < segments; i++) {
            vec ob1,ob1n;
            vec ot1,ot1n;

            float ang = (float)360*i/segments;
            float angn = (float)360*(i+1)/segments;

            float cosa = cos(ang*M_PI/180);
            float sina = sin(ang*M_PI/180);

            float cosan = cos(angn*M_PI/180);
            float sinan = sin(angn*M_PI/180);

            ob1.x = fD*cosa;
            ob1.y = fD*sina;
            ob1.z = 0;

            ob1n.x = fD*cosan;
            ob1n.y = fD*sinan;
            ob1n.z = 0;

            ot1.x = fD*cosa;
            ot1.y = fD*sina;
            ot1.z = screwHeight;

            ot1n.x = fD*cosan;
            ot1n.y = fD*sinan;
            ot1n.z = screwHeight;

            write_quad(outf,&ob1,&ob1n,&ot1n,&ot1,0);
            num_tris += 2;
        }
    }

    fseek(outf, 80, SEEK_SET);
    fwrite(&num_tris, 4, 1, outf);

    fclose(outf);

    return 0;
}
