![StaticAnalysis](https://github.com/AlexandruIca/Mandelbrot/workflows/StaticAnalysis/badge.svg)
![Linux](https://github.com/AlexandruIca/Mandelbrot/workflows/Linux/badge.svg)

# Mandelbrot fractal
![Mandelbrot](./media/Sample.gif)

# Download
For linux you can download the binary [here](https://github.com/AlexandruIca/Mandelbrot/releases/tag/refs%2Fheads%2Fmaster).

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
