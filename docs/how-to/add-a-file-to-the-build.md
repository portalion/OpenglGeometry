# How to add a file to the build

Source lists are **explicit**, matching the sibling PhysicsSimulation project. Creating a
`.cpp` on disk is not enough — add it to the relevant `CMakeLists.txt`, or you get an
`unresolved external symbol` at link time rather than a compile error.

| File goes in | Add it to |
| --- | --- |
| `OpenglGeometry/src/**` | [`OpenglGeometry/CMakeLists.txt`](../../OpenglGeometry/CMakeLists.txt), in the `add_executable` list |
| `Algebra/src/**` | [`Algebra/CMakeLists.txt`](../../Algebra/CMakeLists.txt), in the `add_library` list |
| `OpenglGeometry/resources/**` | Nothing — the whole tree is copied next to the exe every build |

## Adding a source file

```cmake
add_executable(${PROJECT_NAME}
    "src/main.cpp"
    "src/App.cpp"

    "src/systems/MeshGeneratingSystem.cpp"
    "src/systems/MyNewSystem.cpp"          # ← new, keep it in its folder group
    ...
)
```

Paths are relative to the `CMakeLists.txt`, forward slashes, quoted. The list is grouped by
folder with blank lines between groups — keep that style.

Only `.cpp` files need listing; headers are found through the include directories.

Re-configure afterwards. Visual Studio does this automatically when it notices the file
changed; from the command line:

```bash
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -S . -B out/build/x64-Debug
```

## Header-only additions need nothing

Archetypes (`src/archetypes/*.h`), UI helpers (`src/UI/*.h`) and the interfaces are all
`inline`/header-only. They are picked up through the `src/` include directory with no build
change at all.

## When you need to do more

The project splits third-party code two ways:

- **`Dependencies/`** — git submodules, added with `add_subdirectory` from the root.
- **`vendor/`** — code checked into this repository, built by
  [`vendor/CMakeLists.txt`](../../vendor/CMakeLists.txt).

### A new vendored library

Drop it in `vendor/mylib/` and add a target to `vendor/CMakeLists.txt`. For
something header-only, copy the `entt` pattern:

```cmake
add_library(mylib INTERFACE)

target_include_directories(
	mylib INTERFACE
	${PROJECT_SOURCE_DIR}
)
```

For something that compiles, copy the `imgui` pattern — note `${PROJECT_NAME}` doubles as the
source subdirectory name:

```cmake
project(mylib)

add_library(mylib)

target_include_directories(mylib PUBLIC ${PROJECT_SOURCE_DIR})

target_sources(
	mylib PRIVATE
	${PROJECT_NAME}/a.cpp      # → vendor/mylib/a.cpp
	${PROJECT_NAME}/b.cpp
)
```

Then add it to the app's `target_link_libraries`.

### A new submodule dependency

```bash
git submodule add https://github.com/owner/mylib Dependencies/mylib
cd Dependencies/mylib && git checkout <tag-or-sha> && cd ../..
git add Dependencies/mylib .gitmodules
```

Then `add_subdirectory (Dependencies/mylib)` in the root `CMakeLists.txt`, before the project
subdirectories, and link its target.

Pin an explicit commit or tag. Watch out for dependencies whose own
`cmake_minimum_required` is below 3.5 — CMake 4.x refuses those outright. (The GLEW submodule
is fine: it declares the range form, `VERSION 2.8.12...4.0`.)

### A new sub-project

Create `MyLib/CMakeLists.txt` following `Algebra`:

```cmake
project (MyLib)

add_library(${PROJECT_NAME}
	"src/Thing.cpp"
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

target_include_directories (${PROJECT_NAME} PUBLIC
	${PROJECT_SOURCE_DIR}/src
)
```

then `add_subdirectory (MyLib)` in the root and link it from `OpenglGeometry`.

### A new resource type

Everything under `OpenglGeometry/resources/` is copied recursively, so new shader
subdirectories need no build change. Load them with paths relative to the working directory,
e.g. `"resources/shaders/myGroup/"`.

## Include paths

Include directories are carried by the targets, so there is nothing global to maintain:

| You want | Write | Provided by |
| --- | --- | --- |
| Project header | `#include "scene/Components.h"` | `OpenglGeometry`'s own `src/` include dir |
| Math | `#include "Algebra.h"` | `Algebra` target (PUBLIC) |
| ImGui | `#include <imgui/imgui.h>` | `imgui` target (PUBLIC, exports `vendor/`) |
| EnTT | `#include <entt/entt.hpp>` | `entt` interface target |
| GL loader | `#include <GL/glew.h>` | `libglew_static` (PUBLIC, also brings `GLEW_STATIC`) |
| Windowing | `#include <GLFW/glfw3.h>` | `glfw` target |

Because `src` is an include root, **use project-relative includes, never `../`**:

```cpp
#include "scene/Components.h"      // ✅
#include "../scene/Components.h"   // ❌
```

Both `"quoted"` and `<angled>` forms appear in the codebase for project headers; either
works.
