# ImEffects

A collection of visual effect widgets for [Dear ImGui](https://github.com/ocornut/imgui).

This repository contains two independent ImGui libraries, each in its own sub-repository:

you can test is with this [Emscripten](https://aiekick.github.io/ImEffects) demo

## [ImGenie](https://github.com/aiekick/ImGenie)

macOS-style **Genie effect** and Linux/Gnome-style **Wobbly windows**.

- Genie appear/disappear animation (Coons patch mesh with cubic Bezier S-curves)
- Wobbly window drag (spring physics on 4 corners)
- 4-side support (Top, Bottom, Left, Right, or Auto)
- Per-window settings, enable/disable each effect independently
- Backend-agnostic capture (you provide FBO callbacks)
- Built-in demo window with save/reset defaults, C API, DLL-ready

See [ImGenie/readme.md](https://github.com/aiekick/ImGenie/blob/master/readme.md) for full documentation.

**Genie effect demo**

![Demo](https://github.com/aiekick/ImEffects/blob/master/doc/ImGenie_genie.gif)

**Wobbly window demo**

![Demo](https://github.com/aiekick/ImEffects/blob/master/doc/ImGenie_wobbly.gif)

## [ImCoolBar](https://github.com/aiekick/ImCoolBar)

macOS Dock-style magnification bar.

- Bubble magnification effect (cos^8 curve) with cursor pinning
- Horizontal and Vertical orientations
- Configurable anchor, size, animation speed, effect strength
- Built-in demo window with save/reset defaults, metrics window, C API, DLL-ready

See [ImCoolBar/readme.md](https://github.com/aiekick/ImCoolBar/blob/master/readme.md) for full documentation.

**Bubble effect demo**

![Demo](https://github.com/aiekick/ImEffects/blob/master/doc/ImCoolBar.gif)

## Demo App

The [ImEffects](https://github.com/aiekick/ImEffects) demo shows an ImCoolBar dock with app icons; clicking icons opens/closes windows with ImGenie animations, and dragging windows shows the wobbly effect.

### Building

Requirements: CMake 3.20+, C++17, OpenGL 3.0+

```bash
git clone --recursive https://github.com/aiekick/ImEffects.git
cd ImEffects
cmake -B build
cmake --build build --config Release
```

The demo binary is output to `./bin/`.

## Dependencies (included in 3rdparty/)

- [Dear ImGui](https://github.com/ocornut/imgui) (docking branch)
- [GLFW](https://www.glfw.org/)
- [GLAD](https://glad.dav1d.de/)
- [stb_image](https://github.com/nothings/stb)

## License

MIT License -- Copyright (c) 2024-2026 Stephane Cuillerdier (aka Aiekick)
