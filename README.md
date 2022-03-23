# PixelEngine
Engine to draw pixels and textures on screen with using OpenGL and GLFW

(work in progress)

Important things to do:
  - optimize functions that are used alot
  - low resolution to mimic old games better
  - small color errors for rendering (because i think it may look kinda cool)
  - add sound later


## Instructions for compiling and using the library
#### (THE PROJECT IS NOT IN ITS COMPLETE STATE SO ALOT CHANGES WILL BE MADE)

- install libraries:

(debian based systems)
```bash
apt install libglew-dev libglfw3-dev
```

(arch based systems)
```bash
pacman -S glew glfw-x11
```
or
```bash
pacman -S glew glfw-wayland
```

- compile

```bash
git clone https://github.com/331uw13/PixelEngine.git
cd PixelEngine
cmake .
make
```
