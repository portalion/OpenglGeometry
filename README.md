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

Open `OpenglGeometry.sln` in Visual Studio 2022 and build `x64`. All dependencies (GLFW,
GLEW, ImGui, EnTT) are vendored — see [getting started](docs/getting-started.md) for details.

## Controls

| Input | Action |
| --- | --- |
| Right-drag | Orbit camera |
| Shift + right-drag | Roll camera |
| Middle-drag | Pan camera |
| Mouse wheel | Zoom |
| **Shift + A** | Shape creation menu |
