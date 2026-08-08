# The build system

Reference for the CMake setup itself. For *using* it, see
[getting-started.md](getting-started.md); for adding files, see
[how-to/add-a-file-to-the-build.md](how-to/add-a-file-to-the-build.md).

The layout deliberately mirrors the sibling **PhysicsSimulation** project, so the two
repositories can be worked on interchangeably:

- submodules in `Dependencies/`, added with `add_subdirectory`
- vendored third-party code in `vendor/`, with its own `CMakeLists.txt`
- a minimal root `CMakeLists.txt` that does nothing but set C++20 and list subdirectories
- one `project()` per subdirectory, using the `${PROJECT_NAME}` idiom
- explicit source lists, grouped by folder with blank lines
- an always-run `copy_resources` target
- Ninja + `CMakeSettings.json`, building into `out/build/<name>/`

## Files

| File | Responsibility |
| --- | --- |
| [`CMakeLists.txt`](../CMakeLists.txt) | MSVC hot-reload policy, C++20, `add_subdirectory` list. Nothing else. |
| [`Algebra/CMakeLists.txt`](../Algebra/CMakeLists.txt) | `Algebra` static library |
| [`OpenglGeometry/CMakeLists.txt`](../OpenglGeometry/CMakeLists.txt) | `OpenglGeometry` executable, include dirs, resource copy |
| [`vendor/CMakeLists.txt`](../vendor/CMakeLists.txt) | `imgui` and `entt` targets |
| [`CMakeSettings.json`](../CMakeSettings.json) | Visual Studio configurations: `x64-Debug`, `x64-Release` |
| [`.gitmodules`](../.gitmodules) | GLFW and GLEW submodules |

## Dependencies

| Library | Where | How |
| --- | --- | --- |
| **GLFW** | `Dependencies/glfw` | git submodule, `add_subdirectory` → target `glfw` |
| **GLEW** | `Dependencies/glew` | git submodule ([Perlmint/glew-cmake](https://github.com/Perlmint/glew-cmake)), `add_subdirectory` → target `libglew_static` |
| **Dear ImGui** | `vendor/imgui` | vendored in-tree, built by `vendor/CMakeLists.txt` |
| **EnTT** | `vendor/entt` | vendored in-tree, INTERFACE target |
| **OpenGL** | system | transitive through `libglew_static` |

The split is the point: **`Dependencies/` is for things git manages, `vendor/` is for things
checked into this repository.** Both submodules are pinned to the **same commits
PhysicsSimulation uses**, so the two projects compile against identical dependency versions.

```bash
git submodule update --init --recursive
```

`libglew_static` carries `GLEW_STATIC` and its include directory as `PUBLIC` properties, so
linking it is all that is required.

## Target graph

```
OpenglGeometry (executable)
├── Algebra         static     PUBLIC include: Algebra/src
├── imgui           static     PUBLIC include: vendor
│                              links glfw + libglew_static
├── entt            interface  PUBLIC include: vendor
├── glfw            static     submodule
└── libglew_static  static     submodule; PUBLIC GLEW_STATIC + include dir
```

`vendor/CMakeLists.txt` uses PhysicsSimulation's trick of naming the project after the library
so `${PROJECT_NAME}` doubles as the source subdirectory:

```cmake
project(imgui)

target_sources(
	imgui PRIVATE
	${PROJECT_NAME}/imgui.cpp      # → vendor/imgui/imgui.cpp
	...
)
```

Because the include directory is `vendor/` itself, `#include <imgui/imgui.h>` and
`#include <entt/entt.hpp>` both resolve unchanged from before the move.

## Resource copying

```cmake
add_custom_target(copy_resources ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${PROJECT_SOURCE_DIR}/resources
	${PROJECT_BINARY_DIR}/resources
	COMMENT "Copying resources into binary directory")

add_dependencies(${PROJECT_NAME} copy_resources)
```

A custom target with `ALL` and no `OUTPUT` runs on **every** build, so an edited shader always
reaches the executable. `${PROJECT_BINARY_DIR}` is `out/build/<name>/OpenglGeometry`, which is
also where the executable lands — exactly what the runtime's relative `resources/shaders/...`
paths need.

## Deviations from PhysicsSimulation

**1. Vendored code lives in `vendor/` at the repository root.** PhysicsSimulation calls it
`Libs` and nests it as `PhysicsSimulation/Libs/`, which ties the vendored code to one
project. Hoisting it to the root puts both third-party directories at the same level —
`Dependencies/` for submodules, `vendor/` for committed code — and lets a second project (or
`Algebra`) link `imgui` or `entt` without reaching into the application's subtree.

Only the `add_subdirectory (vendor)` call moved, from `OpenglGeometry/CMakeLists.txt` to the
root. `vendor/CMakeLists.txt` is unchanged: it uses `${PROJECT_SOURCE_DIR}`, which follows the
directory containing the `project()` call.

**2. ImGui is vendored rather than a submodule** — same as PhysicsSimulation, which also keeps
its (docking-branch) ImGui vendored in-tree. The only difference is content: PhysicsSimulation
also vendors ImPlot; this project also has `imgui_stdlib.cpp`. Both are flat layouts with the
backends next to `imgui.cpp`.

**3. `Algebra` is a separate library target.** PhysicsSimulation keeps everything in one
executable. This project has an independent math library that predates the CMake migration,
so it stays its own `add_subdirectory`. It follows the same per-project idiom.

**4. `CMakeSettings.json` has a Release configuration.** PhysicsSimulation defines only
`x64-Debug`. `x64-Release` is added here because the project is regularly built both ways.

**5. Lowercase `resources/`.** PhysicsSimulation uses `Resources/`. The shader loader hard-codes
`"resources/shaders/"`, so renaming would mean a source change for no gain.

## Known rough edges

**The root `CMakeLists.txt` has no `project()` call**, matching both PhysicsSimulation and
Duck. CMake therefore emits two developer warnings on every configure:

```
CMake Warning (dev) in CMakeLists.txt:
  No project() command is present.
```

Harmless — CMake substitutes `project(Project)`. To silence it, add one line after
`cmake_minimum_required`:

```cmake
project (OpenglGeometry)
```

**GLEW builds more than needed.** Like PhysicsSimulation, the glew submodule is added with no
options, so it also builds `libglew_shared` (`glew-sharedd.dll`), `glewinfo` and `visualinfo`,
none of which are used. Setting `ONLY_LIBS ON` and `glew-cmake_BUILD_SHARED OFF` before
`add_subdirectory (Dependencies/glew)` would trim them.

**`CMakeSettings.json` is Visual Studio-only and superseded.** Microsoft's replacement is
`CMakePresets.json`, which VS, VS Code, CLion and the `cmake` CLI all understand. This project
uses `CMakeSettings.json` to match PhysicsSimulation; the equivalent preset file is a
drop-in swap if you ever want CLI/`--preset` support back. Command-line builds work either
way, they just need the arguments spelled out — see
[getting-started.md](getting-started.md#building).

**One pre-existing compiler warning**, unrelated to the build system:

```
StaticMeshManager.cpp(114): warning C4267: 'argument': conversion from 'size_t' to 'uint32_t'
```

That is `indices.push_back(indices.size())` in the grid mesh loop. Benign at these sizes.

## Migration notes (from MSBuild)

| Was | Now |
| --- | --- |
| `OpenglGeometry.sln` + 2 `.vcxproj` + 2 `.filters` | 4 `CMakeLists.txt` + `CMakeSettings.json` |
| `Dependencies/` prebuilt libs (gitignored, so clean clones could not build) | `Dependencies/` git submodules built from source |
| `OpenglGeometry/vendor/{imgui,entt}` | `vendor/{imgui,entt}` — same name, hoisted to the root |
| Post-build `xcopy /Y /I /E` | `copy_resources` custom target |
| `GLEW_STATIC` in `<PreprocessorDefinitions>` | Inherited from `libglew_static` |
| `$(SolutionDir)Algebra\src` on the include path | `PUBLIC` include directory on the `Algebra` target |
| `<PlatformToolset>v145` | Ninja + `cl.exe` from the MSVC environment |
| Output to `build/$(Platform)/$(Configuration)/$(ProjectName)/out/` | `out/build/<name>/OpenglGeometry/` |
| `Win32` configurations (broken — no 32-bit `glfw3.lib` was present) | x64 only; add an `x86` entry to `CMakeSettings.json` if needed |
| `#ifdef _DEBUG` | Unchanged — MSVC still defines it for Debug builds |

Behaviour that deliberately did **not** change: C++20, console subsystem, static GLEW/GLFW
linkage, the `_DEBUG` ImGui demo window, and the runtime shader paths.

## Things you might want next

- **`build/`** may still hold stale MSBuild output from before the migration; it is inert and
  gitignored.
- **Warnings**: CMake's MSVC default is `/W3`. `target_compile_options(${PROJECT_NAME} PRIVATE /W4)`
  would surface more, with some noise from signed/unsigned comparisons in the mesh generators.
- **Tests**: there is no test target. `Algebra` is the natural first candidate — a pure static
  library with no GL dependency.
- **Sharing code across projects**: OpenglGeometry, PhysicsSimulation and Duck now all build
  GLFW and GLEW the same way, so their engine/math layers could be factored into a shared
  library.
