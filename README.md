# OpenglGeometry

![obraz](https://github.com/user-attachments/assets/a0dc21de-451f-4d91-8aaa-e024db9b6531)

An interactive OpenGL 4.6 editor for parametric geometry — points, tori, polylines, Bézier
curves (C0, C2, interpolated) and Bézier surfaces — built on an EnTT entity-component-system
core with Dear ImGui for the interface.

**📖 [Full documentation →](docs/README.md)**

Quick links: [Getting started](docs/getting-started.md) ·
[Architecture](docs/architecture.md) ·
[Add a new shape](docs/how-to/add-a-new-shape.md) ·
[Known issues](docs/gotchas.md)

## Build

Requires Windows and Visual Studio 2022 or 2026 with the *Desktop development with C++*
workload (which supplies CMake and Ninja).

```bash
git clone --recurse-submodules <repo-url>
```

Then open the folder in Visual Studio and pick the `x64-Debug` configuration, or from a
Developer Command Prompt:

```bash
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -S . -B out/build/x64-Debug
```

```bash
cmake --build out/build/x64-Debug
```

GLFW and GLEW are git submodules under `Dependencies/`; ImGui and EnTT are vendored in
`OpenglGeometry/Libs/`. The executable and its shaders land in
`out/build/<name>/OpenglGeometry/`.

See [getting started](docs/getting-started.md) for details and
[build system](docs/build-system.md) for how the CMake files are put together.

## Controls

| Input | Action |
| --- | --- |
| Right-drag | Orbit camera |
| Shift + right-drag | Roll camera |
| Middle-drag | Pan camera |
| Mouse wheel | Zoom |
| **Shift + A** | Shape creation menu |
