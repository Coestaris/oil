# oil

**OpenGL Image Library** is a small C library for loading images and working with pixels and textures. It was written for Zomboid2.0 and other projects, including Raytracer.

- `png.h` exposes PNG loading from a file or a memory buffer.
- `bmp.h` exposes bitmap creation and saving.
- `graphics.h` contains pixel access, drawing primitives, and OpenGL texture helpers. Texture uploads require a current OpenGL context.

## Build and run the conversion demo

Install a C compiler, CMake, OpenGL and FreeGLUT development libraries, and zlib. From the repository root, build the demo, place a PNG named `menu.png` in the working directory, and run it.

```sh
cmake -S . -B build
cmake --build build
./build/oil
```

The demo loads `menu.png` and writes `image.bmp` as a 24-bit bitmap. It overwrites an existing file with that name.

## Use in a C project

Include `oil.h` and compile the image and graphics sources into your target. The CMake target in this repository builds the demo executable. For an embedding example, see [Raytracer](https://github.com/Coestaris/Raytracer), which includes oil as a submodule.

Distributed under the [MIT License](LICENSE).
