# Getting started

## Prerequisites

| Requirement | Notes |
| --- | --- |
| Windows | The build is Windows/MSVC only. Nothing in the CMake files is deliberately platform-specific, but nothing else has been tested, and `utils/GlCall.h` uses the MSVC-only `__debugbreak()`. |
| Visual Studio 2022 or 2026 | Any edition, with the *Desktop development with C++* workload. This also supplies CMake and Ninja. |
| C++20 | Set globally in the root `CMakeLists.txt`. The code uses concepts, ranges (`std::views::transform`), `std::numbers`, designated initializers and `contains()`. |
| Git | GLFW and GLEW are submodules. |
| GPU with OpenGL 4.6 | The window requests a 4.6 core profile context and shaders are `#version 460 core` (two Bézier-surface shaders are `#version 430`). Tessellation shaders are required. |

## Building

Clone **with submodules**:

```bash
git clone --recurse-submodules <repo-url>
```

If you already cloned without them:

```bash
git submodule update --init --recursive
```

### From Visual Studio

**File → Open → Folder…** and pick the repository root. Visual Studio reads
`CMakeSettings.json` and offers the `x64-Debug` and `x64-Release` configurations. Select
`OpenglGeometry.exe` as the startup item and press F5.

### From the command line

Run from a **Developer Command Prompt** (Ninja and `cl.exe` must be on `PATH`):

```bash
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -S . -B out/build/x64-Debug
```

```bash
cmake --build out/build/x64-Debug
```

Swap `Debug` for `Release` and `x64-Debug` for `x64-Release` to match the other
configuration. `CMakeSettings.json` is read only by Visual Studio, so the CLI needs the
generator and build type spelled out.

### Output layout

```
out/build/<name>/
  OpenglGeometry/
    OpenglGeometry.exe
    resources/shaders/       re-copied on every build
    Libs/imgui.lib
  Algebra/Algebra.lib
  Dependencies/              built GLFW and GLEW
```

The executable and its `resources/` tree sit in the same directory, which is what the runtime
shader paths require.

### Resources are copied, not referenced

A `copy_resources` target mirrors `OpenglGeometry/resources/` next to the executable on
**every** build, so an edited shader always reaches the running program:

```cmake
add_custom_target(copy_resources ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${PROJECT_SOURCE_DIR}/resources
	${PROJECT_BINARY_DIR}/resources
	COMMENT "Copying resources into binary directory")
```

Shaders are loaded at runtime with the **relative** path `resources/shaders/...`, so the
working directory must be the directory containing the executable. If you run the exe from
somewhere else, shader files silently fail to open, the programs link empty, and you get a
black viewport — see [gotchas](gotchas.md).

There is still no hot reload: rebuild and restart to see a shader change.

## Running

Press F5 in Visual Studio, or run `out/build/<name>/OpenglGeometry/OpenglGeometry.exe` from
its own directory. A console window is attached and used for log output: OpenGL/GLSL version,
shader and mesh loading info, and uniform warnings. A healthy startup looks like:

```
OpenGL Version: 4.6.0 - Build 30.0.100.9864
GLSL Version: 4.60 - Build 30.0.100.9864
INFO: Loading Meshes
INFO: Loaded Meshes - Elasped time: 1ms
INFO: Loading Shaders
INFO: Loaded Shaders
```

In Debug builds the ImGui demo window is also shown (`App::Run`, guarded by `_DEBUG`).

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

| Library | Where | How it is obtained |
| --- | --- | --- |
| **GLFW** | `Dependencies/glfw` | git submodule, built from source → target `glfw` |
| **GLEW** | `Dependencies/glew` | git submodule ([Perlmint/glew-cmake](https://github.com/Perlmint/glew-cmake)) → target `libglew_static` |
| **Dear ImGui** (docking branch) | `OpenglGeometry/Libs/imgui` | vendored in-tree → target `imgui` |
| **EnTT** | `OpenglGeometry/Libs/entt` | vendored in-tree → INTERFACE target `entt` |
| **OpenGL** | system | transitive through `libglew_static` |
| **Algebra** | `Algebra/` | in-repo static library, see [algebra.md](algebra.md) |

The rule of thumb: **`Dependencies/` for submodules, `Libs/` for code committed to this
repository.** Both submodules are pinned to the same commits as the sibling
**PhysicsSimulation** project. See [build-system.md](build-system.md) for the full rationale.

### Include paths

Every include path is carried by a target, so there is nothing global to maintain:

| You want | Write | Provided by |
| --- | --- | --- |
| Project header | `#include "scene/Components.h"` | `OpenglGeometry`'s own `src/` include dir |
| Math | `#include "Algebra.h"` | `Algebra` target (PUBLIC) |
| ImGui | `#include <imgui/imgui.h>` | `imgui` target (PUBLIC, exports `Libs/`) |
| EnTT | `#include <entt/entt.hpp>` | `entt` interface target |
| GL loader | `#include <GL/glew.h>` | `libglew_static` (PUBLIC, also brings `GLEW_STATIC`) |
| Windowing | `#include <GLFW/glfw3.h>` | `glfw` target |

## Repository layout

```
CMakeLists.txt                 Root: C++20, MSVC hot reload, add_subdirectory list
CMakeSettings.json             Visual Studio configurations (x64-Debug, x64-Release)
.gitmodules                    GLFW and GLEW
Dependencies/
  glfw/                        submodule
  glew/                        submodule
Algebra/
  CMakeLists.txt               Static library target
  src/                         Vector4, Matrix4, Quaternion, helpers
OpenglGeometry/
  CMakeLists.txt               Executable, include dirs, resource copy
  Libs/
    CMakeLists.txt             imgui and entt targets
    imgui/                     vendored (docking branch, flat layout)
    entt/                      vendored single header
  resources/shaders/           GLSL, copied next to the exe
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
out/                           Build output (gitignored)
docs/                          You are here
```

`deprecated/` and `core/Notifier.*` + `interfaces/IObserver.h` are dead code — see
[gotchas](gotchas.md).

## Adding files

Source lists are **explicit**. A new `.cpp` must be added to the relevant `CMakeLists.txt` —
see [how-to/add-a-file-to-the-build.md](how-to/add-a-file-to-the-build.md).
