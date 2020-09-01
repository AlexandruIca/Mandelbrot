# Mandelbrot fractal
![Mandelbrot](./media/Sample.gif)

# How to build
Install [conan](https://conan.io/downloads.html) and [CMake](https://cmake.org/download/). After that:
```sh
mkdir build && cd build

conan install .. --build sdl2

cmake ..

cmake --build .
```
It's possible you need to install some libraries(at least on linux) to be able to build this. Conan should give you all info.

# How to use
* Scroll for zoom in/out
* Drag with the mouse to move
* Press `Q/E` to increase/decrease number of iterations
* Press `C` to reset