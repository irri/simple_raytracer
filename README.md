# Simple parallel raytracer using OpenMP

Written as an assignment in a class in Tokyo Institute of Technology.

## Build
> clang++ -osimple_raytracer -I/usr/local/include/libpng16 -L/usr/local/lib -lpng -lz main.cpp Ray.cpp 

## Usage
Hard coded scene inside main.cpp.
> ./simple_raytracer

![Rendered sample image](render.png)

