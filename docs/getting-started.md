# Getting started

## Prerequisites

| Requirement | Notes |
| --- | --- |
| Windows | The project is MSBuild/Visual Studio only; there is no CMake or premake file. |
| Visual Studio 2022 (17.9+) | The projects target platform toolset **v145**. If your VS installs a different toolset, retarget the solution (right-click solution → *Retarget solution*) or edit `<PlatformToolset>` in both `.vcxproj` files. |
| C++20 | Both projects set `<LanguageStandard>stdcpp20`. The code uses concepts, ranges (`std::views::transform`), `std::numbers`, designated initializers and `contains()`. |
| GPU with OpenGL 4.6 | The window requests a 4.6 core profile context and shaders are `#version 460 core` (two Bézier-surface shaders are `#version 430`). Tessellation shaders are required. |

Everything else is vendored in the repository — you do **not** need a package manager.

## Building

```bash
git clone <repo-url>
```

Then:

1. Open `OpenglGeometry.sln` in Visual Studio.
2. Pick a configuration. `x64` is the one that is actually wired up
   (`Dependencies\lib\x64`); `Win32` maps to `Dependencies\lib\Win32`.
3. Build the solution. `Algebra` builds first as a static library, then `OpenglGeometry`
   links against it.

From the command line:

```bash
msbuild OpenglGeometry.sln /p:Configuration=Debug /p:Platform=x64
```

Outputs land in:

```
build/<Platform>/<Configuration>/<ProjectName>/out/     # binaries
build/<Platform>/<Configuration>/<ProjectName>/         # intermediates
```

### Post-build step

`OpenglGeometry.vcxproj` runs a post-build command:

```
xcopy /Y /I /E "$(ProjectDir)resources" "$(TargetDir)resources"
```

This copies `OpenglGeometry/resources/` (the GLSL shaders) next to the executable.
Shaders are loaded at runtime with the **relative** path `resources/shaders/...`, so the
working directory must be the output directory. If you launch the exe from somewhere else,
shader files silently fail to open, the programs link empty, and you get a black viewport —
see [gotchas](gotchas.md).

If you edit a shader and nothing changes, rebuild so the post-build copy runs again (or edit
the copy under `build/.../out/resources/` while iterating).

## Running

Run from Visual Studio (F5), or run the produced `OpenglGeometry.exe` from its own
directory. A console window is attached (`<SubSystem>Console`) and is used for log output:
OpenGL/GLSL version, shader and mesh loading info, and uniform warnings.

In `_DEBUG` builds the ImGui demo window is also shown (`App::Run`).

### Controls

| Input | Action |
| --- | --- |
| Right-drag | Orbit camera (yaw + pitch) |
| Shift + right-drag | Roll camera |
| Middle-drag | Pan camera |
| Mouse wheel | Zoom |
| **Shift + A** | Open the *Shape Creation* popup |

The two ImGui panels are *Shape List* (select / deselect / delete) and *Selected Shapes
Properties* (per-component inspectors). See [UI](ui.md).

## Dependencies

| Library | Where | How it is consumed |
| --- | --- | --- |
| **GLFW 3** | `Dependencies/include/GLFW`, `Dependencies/lib/{x64,Win32}` | `glfw3.lib`, static |
| **GLEW 2.1.0** | `Dependencies/include/GL`, `Dependencies/glew-2.1.0` | `glew32s.lib`, static (`GLEW_STATIC` is defined) |
| **OpenGL** | system | `opengl32.lib` |
| **Dear ImGui** (docking branch) | `OpenglGeometry/vendor/imgui` | Sources compiled directly into the project |
| **EnTT** | `OpenglGeometry/vendor/entt/entt.hpp` | Single-header, header-only |
| **Algebra** | `Algebra/` | In-repo static library project, see [algebra.md](algebra.md) |

Include directories for `OpenglGeometry`:

```
$(ProjectDir)vendor        →  imgui/..., entt/entt.hpp
$(ProjectDir)src           →  "core/Base.h", "scene/Entity.h", ...
$(SolutionDir)Algebra\src  →  "Algebra.h", "Vector4.h", ...
$(SolutionDir)Dependencies\include
```

Because `src` is an include root, headers are included as project-relative paths
(`#include "scene/Components.h"`), not with `../`. Follow that convention in new code.

## Repository layout

```
OpenglGeometry.sln
Algebra/                       Static library: Vector4, Matrix4, Quaternion, helpers
  src/
Dependencies/                  Prebuilt GLFW/GLEW headers + libs
OpenglGeometry/
  OpenglGeometry.vcxproj       All source files are listed here explicitly
  resources/shaders/           GLSL, copied next to the exe post-build
  vendor/                      imgui/, entt/
  deprecated/                  Old raycast-ellipsoid code, NOT in the build
  src/
    main.cpp                   Entry point
    App.{h,cpp}                Owns window, scene and pipeline; runs the frame loop
    core/                      Base.h (Ref/Unique), Globals, Window, DragCamera, Notifier
    interfaces/                ISystem, ICamera, IPopup, IObserver
    scene/                     Scene, Entity, Components, Tags, Observable, registry helper
    systems/                   The per-frame systems, incl. systems/gui/
    archetypes/                Functions that assemble entities into "shapes"
    meshGenerators/            Pure CPU geometry generation (torus, polyline, Bézier)
    managers/                  Singletons: ShaderManager, StaticMeshManager, IdManager
    renderer/                  Shader, ShaderBuilder, VertexArray, buffers, Renderer
    UI/                        ImGui panels and popups
    utils/                     GL error macros, GLEW/ImGui initialisation
build/                         Build output (gitignored)
docs/                          You are here
```

`deprecated/` and `core/Notifier.*` + `interfaces/IObserver.h` are dead code — see
[gotchas](gotchas.md).

## Adding files

**Important:** this is an MSBuild project with an explicit file list. Creating a `.cpp` on
disk is not enough — it must be added to `OpenglGeometry.vcxproj` (and ideally
`OpenglGeometry.filters`). Adding files through the Visual Studio Solution Explorer does
this for you. See [how-to/add-a-file-to-the-build.md](how-to/add-a-file-to-the-build.md).
