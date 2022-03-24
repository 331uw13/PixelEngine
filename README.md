PixelEngine
===========

An engine that draws pixels and textures on screen using OpenGL and GLFW.

Work in progress.

## Features yet to be added:

- Optimize frequently used functions

- Low resolution to mimic old games better

- Small color errors for rendering (because I think it may look kinda cool)

- Add sound later

## Instructions for building and using the engine

**The project is not in its complete state, so many changes could be introduced.**

1. Install required dependencies

    - For Debian-based systems:

        ```bash
        apt install libglew-dev libglfw3-dev
        ```

    - For Arch-based systems:

        ```bash
        pacman -S glew glfw-x11
        ```

        Or:

        ```bash
        pacman -S glew glfw-wayland
        ```

2. Clone the repository

    ```bash
    git clone https://github.com/331uw13/PixelEngine.git
    cd PixelEngine
    ```

3. Build and run the project

    ```bash
    cmake .
    make
    ./testing/a.out
    ```

