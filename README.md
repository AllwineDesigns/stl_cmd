stl_cmd
=======

The goal of each stl_cmd is to provide a simple command line interface for manipulating binary STL files. stl_cmd aims to be easy to set up, and is geared towards teaching basic terminal usage and programming skills in the 3D printing space.

Getting started
---------------

    git clone https://github.com/frknsweetapps/stl_cmd.git
    cd stl_cmd
    make

The stl_cmds will be compiled and placed in the bin/ directory in the root of the stl_cmd repo. Add it to your path and you can perform the following commands.

STL Commands
------------

This list is rather short for now, but hopefully will grow over time.

### stl_header

    stl_header [-s <header>] [-o <output file>] <input file>

Prints or sets the data in the header section of a binary STL file. The header section is rarely used, but can store a small amount of data (80 characters). Copyright info or a very brief description are some possibilities.

### stl_count

    stl_count [ <input file> ]

Prints the number of triangles in the provided binary STL file. If no input file is provided, data is read from stdin.

### stl_bbox

    stl_bbox <input file>

Prints bounding box information about the provided binary STL file.

### stl_empty

    stl_empty <output file>

Outputs an empty binary STL file. Can be useful to initialize an empty STL file when merging several files together.

### stl_cube

    stl_cube [ -w <width> ] [ <output file> ]

Outputs a binary STL file of a cube with the provided width. If no output file is provided, data is sent to stdout.

### stl_merge

    stl_merge [ -o <output file> ] [ <input file> ... ]

Combines binary STL files into a single one. If no output file is provided, data is written to stdout.

### stl_transform

    stl_transform [[ <transformation> ] ...] <input file> <output file>

Performs any number of transformations in the order listed on the command line. Transformations include:

    -rx <angle> - rotates <angle> degrees about the x-axis
    -ry <angle> - rotates <angle> degrees about the y-axis
    -rz <angle> - rotates <angle> degrees about the z-axis
    -s <s> - uniformly scales x, y and z by <s> (cannot be 0)
    -sx <x> - scales by <x> in x (cannot be 0)
    -sy <y> - scales by <y> in y (cannot be 0)
    -sz <z> - scales by <z> in z (cannot be 0)
    -tx <x> - translates <x> units in x
    -ty <y> - translates <y> units in y
    -tz <z> - translates <z> units in z

### stl_threads

    stl_threads [ -f ] [ -D <diameter> ] [ -P <pitch> ] [ -a <angle> ] 
                [ -h <height> ] [ -s <segments> ] <output file>

Outputs an stl file with male or female screw threads per the [ISO metric
screw thread standard](http://en.wikipedia.org/wiki/ISO_metric_screw_thread).

    -f            - Outputs female threads (defaults to male).
    -D <diameter> - Changes to major diameter of the threads.
    -P <pitch>    - Changes the height of a single thread, aka the pitch per 
                    the ISO metric standard.
    -h <height>   - Changes the total height of the threads.
    -a <angle>    - Changes the thread angle (degrees). The standard (and 
                    default) is 60 degrees. For 3D printing this can cause 
                    overhang issues as 60 degrees results in a 30 degree 
                    angle with the ground plane. Setting to 90 degrees 
                    results in a 45 degree angle with the ground plane.
    -s <segments> - Changes the resolution of the generated STL file. More 
                    segments yields finer resolution. <segments> is the 
                    number of segments to approximate a circle. Defaults to 
                    72 (every 5 degrees).

Future commands
---------------

These are ideas for future commands that may make it into the stl_cmd suite.

### stl_sphere 

    Generate an STL file with a single sphere in it.

### stl_cylinder 

    Generate an STL file with a single cylinder in it.

### stl_cone 

    Generate an STL file with a single cone in it.

### stl_boolean 

    Combine two STL files using boolean operations.

### stl_twist

    Deform an STL file by twisting it.

### stl_bend

    Deform an STL file by bending it.

### stl_noise

    Deform an STL file by displacing vertices using noise.

Teaching
--------

The goal of this project is to be a resource for teaching terminal usage and some basic programming concepts in the 3D printing space. Imagine an assignment which involves building a brick wall. Students would need to use a combination of stl_cube, stl_transform and stl_merge. The commands could be combined in a bash or &lt;insert favorite scripting language&gt; script with for and while loops, could accept input and use conditionals to affect the attributes of the wall. 

The terminal is an important tool to learn when programming, but can be boring to learn when just making text based programs. stl_cmd aims to make the intro level terminal usage and programming more interesting by creating 3D printable models. As more commands are added more creative assignments are possible. I hope to grow the suite of commands included in stl_cmd with that goal in mind. 

Copyright 2014 Freakin' Sweet Apps, LLC (stl_cmd@freakinsweetapps.com)
