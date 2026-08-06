# How to add a file to the build

This project uses **MSBuild with an explicit file list**. Creating a `.cpp` on disk does not
add it to the build — you get a linker error (`unresolved external symbol`) rather than a
compile error, which is easy to misdiagnose.

## The easy way

In Visual Studio's Solution Explorer: right-click the project → **Add → New Item…** (or
**Existing Item…**). Visual Studio updates both `.vcxproj` and `.filters`.

Pick the right project:

| File belongs to | Project |
| --- | --- |
| `OpenglGeometry/src/**` | `OpenglGeometry.vcxproj` |
| `Algebra/src/**` | `Algebra.vcxproj` |

## The manual way

If you created the files outside the IDE, edit
[`OpenglGeometry/OpenglGeometry.vcxproj`](../../OpenglGeometry/OpenglGeometry.vcxproj)
directly. There are two relevant `<ItemGroup>`s.

Source files:

```xml
<ItemGroup>
  <ClCompile Include="src\systems\MyNewSystem.cpp" />
  ...
</ItemGroup>
```

Headers:

```xml
<ItemGroup>
  <ClInclude Include="src\systems\MyNewSystem.h" />
  ...
</ItemGroup>
```

Use **backslashes** and paths relative to the `.vcxproj` directory.

Optionally mirror the entry in `OpenglGeometry.filters` so the file shows up in the right
Solution Explorer folder — this affects only the IDE, not the build:

```xml
<ClCompile Include="src\systems\MyNewSystem.cpp">
  <Filter>Source Files</Filter>
</ClCompile>
```

If the solution is open, Visual Studio will prompt to reload the project.

## Which files actually need this

| File type | Needs a `.vcxproj` entry? |
| --- | --- |
| `.cpp` | **Yes** — otherwise it is never compiled and you get link errors |
| `.h` | Not for the build to work, but add it as `<ClInclude>` so it appears in Solution Explorer and in searches |
| Header-only, `inline` (archetypes, `UI/`) | Same as above — build works without it, add it for visibility |
| `.vert` / `.frag` / `.tesc` / `.tese` | No. Shaders are loaded at runtime and copied by the post-build `xcopy`, which copies the whole `resources/` tree |

## Include paths

`OpenglGeometry`'s include directories are:

```
$(ProjectDir)vendor        →  #include <imgui/imgui.h>, <entt/entt.hpp>
$(ProjectDir)src           →  #include "scene/Components.h"
$(SolutionDir)Algebra\src  →  #include "Algebra.h"
$(SolutionDir)Dependencies\include
```

Because `src` is an include root, **use project-relative includes, never `../`**:

```cpp
#include "scene/Components.h"      // ✅
#include "../scene/Components.h"   // ❌
```

Both `"quoted"` and `<angled>` forms appear in the codebase for project headers; either
works.

## Adding a new resource directory

Anything placed under `OpenglGeometry/resources/` is copied recursively next to the
executable by the post-build step:

```
xcopy /Y /I /E "$(ProjectDir)resources" "$(TargetDir)resources"
```

so new shader subdirectories need no build change. Load them with paths relative to the
executable's working directory, e.g. `"resources/shaders/myGroup/"`.

## Adding a whole new project

Rare, but: create the `.vcxproj`, add it to `OpenglGeometry.sln` with configuration mappings
for all four Debug/Release × x64/Win32 combinations, add a project reference from
`OpenglGeometry`, and put its `src` directory on the include path. `Algebra` is the model to
copy — a `StaticLibrary` with `stdcpp20` and the shared `build/$(Platform)/$(Configuration)`
output layout.
