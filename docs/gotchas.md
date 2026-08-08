# Known issues and gotchas

Findings from reading the code as of the current `master`. None of these block normal use —
they are the things that will cost you an hour if you hit them without warning.

Nothing here has been changed; this is a description of the code as it stands.

---

## Bugs

### `AddNotifiersToEntityContainer` can loop forever

[`archetypes/SimpleArchetypeCreation.h:36`](../OpenglGeometry/src/archetypes/SimpleArchetypeCreation.h)

```cpp
for (auto it = pointsBegin; it != pointsEnd; )
{
    if (!it->IsValid() || !it->HasComponent<NotificationComponent>())
    {
        continue;                 // ← never advances the iterator
    }
    ...
    it++;
}
```

Any input entity that is invalid or lacks a `NotificationComponent` hangs the application.
It does not trigger today because the only caller (`ShapeCreation::GetSelectedPoints`)
already filters on `<IsSelectedTag, NotificationComponent>`. Passing an unfiltered entity
list to `CreatePolyline` / `CreateBezierC0` / … would hang.

### `PositionComponent::Move` assigns instead of moving

[`scene/Components.h:20`](../OpenglGeometry/src/scene/Components.h)

```cpp
void Move(Algebra::Vector4 offset)
{
    position = offset;            // sets, does not add
}
```

Has no callers today, so nothing is broken — but the name promises `position = position + offset`.

### `VirtualInspect` generates colliding widget IDs

[`systems/gui/ShapeInspectorSystem.cpp:79`](../OpenglGeometry/src/systems/gui/ShapeInspectorSystem.cpp)

```cpp
for (Entity virtualEntity : virtualEntities)
{
    ...
    if (ImGui::Checkbox(GUI::GenerateLabel(entity, "virtual visible").c_str(), &visible))
```

The label is built from `entity` (the parent), not `virtualEntity`, so an entity with several
virtual children renders several checkboxes with the same ImGui ID. Only one behaves. Bézier
curves have exactly one virtual child, so it is invisible today; a Bézier surface has one per
patch.

### `ModifyOrCreateMesh` hard-codes the vertex size on the update path

[`systems/MeshGeneratingSystem.h:38`](../OpenglGeometry/src/systems/MeshGeneratingSystem.h)

```cpp
vertexArray->GetVertexBuffers()[0]->SetData(vertices.data(),
    static_cast<uint32_t>(vertices.size() * sizeof(Algebra::Vector4)));
```

The function is templated on `T` but computes the byte size with `sizeof(Algebra::Vector4)`.
Correct for every current caller; silently wrong for any other vertex type. The creation path
(`VertexArray::CreateWithBuffers`) uses `sizeof(T)` properly.

### `BezierSurfaceGeneration` dereferences entities without validating

[`systems/MeshGeneratingSystem.cpp:118`](../OpenglGeometry/src/systems/MeshGeneratingSystem.cpp)

```cpp
vertices.push_back(patch.GetComponent<BezierPatchGenerationComponent>()
                        .controlPoints[i][j].GetComponent<PositionComponent>().position);
```

No `IsValid()` / `HasComponent()` checks, unlike `CopyValidPointsToVector` which is careful.
Deleting a surface's control point (they are real, selectable entities by default) crashes
here. Every other consumer of stored `Entity` handles guards; this one does not.

### `IdComponent`'s copy constructor double-frees the id

[`scene/Components.h:81`](../OpenglGeometry/src/scene/Components.h)

```cpp
struct IdComponent
{
    const ID id = IdManager::GetInstance().GetNewId();
    IdComponent(const IdComponent& other) = default;
    ~IdComponent() { IdManager::GetInstance().FreeId(id); }
};
```

A copy shares the id, and both destructors push it onto the free queue, so the id can later
be handed out to two entities. No code copies these today.

---

## Sharp edges

### `g_cameraPosition` is always `(0, 0, 0, 1)`

[`systems/RenderingSystem.cpp:33`](../OpenglGeometry/src/systems/RenderingSystem.cpp)

```cpp
sceneContext.CameraPosition = cameraUniforms.Position[3];
```

`cameraUniforms` is filled by `UniformApplier`, which only writes `Position` if the entity has
a `PositionComponent`. The camera entity created in `BaseScene` has none, so `Position` stays
identity and its row 3 is `(0, 0, 0, 1)`.

This matters because `infiniteGrid.vert/.frag` use `g_cameraPosition` for grid scaling and
distance fade, and `bezierLine/default.tesc` uses it for tessellation density. Both currently
behave as if the camera never moves. `DragCamera` tracks its own position internally
(`GetPosition()`), it just never reaches the uniform.

### The projection matrix is never updated on resize

[`App.cpp:56`](../OpenglGeometry/src/App.cpp)

```cpp
void App::HandleResize()
{
    float newWidth  = static_cast<float>(window.GetWidth() - Globals::rightInterfaceWidth);
    float newHeight = static_cast<float>(window.GetHeight());
    float aspect    = newWidth / newHeight;      // computed, then discarded
}
```

`CameraComponent::projectionMatrix` is set once in `BaseScene` from the *starting* dimensions.
Resizing the window updates the GL viewport but not the aspect ratio, so the scene stretches.

Similarly `DragCamera::HandleRotations` divides deltas by `Globals::startingSceneWidth/Height`
rather than the current size, so rotation sensitivity does not follow resizes.

### Two cursor entities exist

Both [`GUISystem`](../OpenglGeometry/src/systems/gui/GUISystem.cpp) and
[`ShapeCreation`](../OpenglGeometry/src/UI/popups/ShapeCreation.cpp) create their own
"Cursor" entity with identical setup. Two rows appear in the *Shape List*, two gizmos render
at the origin, and it is `ShapeCreation`'s copy whose position determines where new shapes
appear — so moving the other one has no effect on shape creation.

### `IndexBuffer` uploads through `GL_ARRAY_BUFFER`

[`renderer/IndexBuffer.cpp:8`](../OpenglGeometry/src/renderer/IndexBuffer.cpp)

```cpp
glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
```

The data lands in the correct buffer object (the target is only a binding point) and `Bind()`
correctly uses `GL_ELEMENT_ARRAY_BUFFER`, so rendering works. But the upload clobbers whatever
was bound to `GL_ARRAY_BUFFER`, which is a latent hazard for any future code that batches
uploads.

### Only `vec4` and `mat4` uniforms can be set

[`renderer/Shader.cpp:139`](../OpenglGeometry/src/renderer/Shader.cpp)

`Shader::ApplyContext` switches on `GL_FLOAT_VEC4` and `GL_FLOAT_MAT4` only. Any other type
falls through with no warning and keeps its GLSL default.

The concrete consequence: `bezierSurface/defaultHorizontal.tesc`'s `u_subdivisions` and
`v_subdivisions` are `int` and permanently 32. `Shader::SetUniformVec1i` exists but is never
called. Fix sketched in [renderer/shaders.md](renderer/shaders.md#adding-a-new-uniform-type).

Also, the "missing uniform" warning in the `GL_FLOAT_MAT4` branch prints `GL_FLOAT_VEC4` as
the type — copy-paste, harmless but misleading when reading the console.

### Shader files that fail to open are silent

[`renderer/ShaderBuilder.cpp:9`](../OpenglGeometry/src/renderer/ShaderBuilder.cpp)

```cpp
std::ifstream stream(filepath);
std::string line; std::stringstream ss;
while (getline(stream, line)) ss << line << '\n';
return ss;
```

No `is_open()` check. A wrong path yields an empty source, which compiles to an empty shader;
the program links and renders nothing. If geometry disappears after a shader change, check
the path first.

`Shader::CreateShader` likewise never checks `GL_LINK_STATUS` or `GL_VALIDATE_STATUS`.

### `ShaderManager::GetShader` / `StaticMeshManager::GetMesh` use `operator[]`

Requesting an unregistered enum value default-constructs a null pointer in the map rather
than failing. A null shader is caught by `Renderer::Render`'s early return (invisible
geometry); a null mesh is **not** checked and crashes at `m_ActualMesh->Bind()`.

### Depth testing is never enabled

[`core/Window.cpp:66`](../OpenglGeometry/src/core/Window.cpp) enables only blending. The depth
buffer is cleared every frame but never tested, so geometry is drawn in registry order. Fine
for the current all-wireframe rendering; must be enabled before drawing solid surfaces.

### GL objects are destroyed after `glfwTerminate()`

`~App` calls `glfwTerminate()`, but `ShaderManager` and `StaticMeshManager` are function-local
statics destroyed at process exit, i.e. *after* `main` returns. Their destructors call
`glDeleteProgram` / `glDeleteVertexArrays` / `glDeleteBuffers` with no current context —
undefined behaviour that happens to be harmless on typical drivers.

### `GLCall` is active in release builds

[`utils/GlCall.h`](../OpenglGeometry/src/utils/GlCall.h) — `__debugbreak()` is not compiled
out and is MSVC-specific. The macro also expands to multiple statements, so it must never be
used as the body of an unbraced `if`.

### Panning accelerates the longer you drag

[`core/DragCamera.cpp:21`](../OpenglGeometry/src/core/DragCamera.cpp) — `HandleRotations`
calls `ImGui::ResetMouseDragDelta` after each step so deltas are per-frame, but
`HandleTranslation` does not. The direction is normalised, so speed is constant per frame,
but the drag delta keeps growing and is re-applied every frame while the button is held.

### Rotation is edited as raw quaternion components

[`systems/gui/ShapeInspectorSystem.cpp:52`](../OpenglGeometry/src/systems/gui/ShapeInspectorSystem.cpp)

```cpp
auto& rotation = entity.GetComponent<RotationComponent>().rotation;
ImGui::DragFloat3(GUI::GenerateLabel(entity, "Rotation").c_str(), &rotation.x, 0.1f);
```

Dragging `x/y/z` of a quaternion without touching `w` and without renormalising is not a
usable rotation UI. `Quaternion::CreateFromEulerAngles` exists and would be the basis of a
better one. Note also that `Quaternion`'s layout is `(w, x, y, z)`, so `&rotation.x` starts at
the *second* float — which is what makes `DragFloat3` reach x/y/z rather than w/x/y.

### `2 * 3.14f` instead of τ

[`meshGenerators/TorusMeshGenerator.cpp:19`](../OpenglGeometry/src/meshGenerators/TorusMeshGenerator.cpp)
and `BaseScene`'s FOV use a hard-coded `3.14f`. The ~0.05% error leaves a hairline seam where
the torus wraps. `BezierSurfaceArchetypeCreation.h` correctly uses `std::numbers::pi_v<float>`.

---

## Dead code

| What | Where | Notes |
| --- | --- | --- |
| `Notifier`, `IObserver` | `core/Notifier.*`, `interfaces/IObserver.h` | Superseded by the ECS notification mechanism. Zero references. `Notifier.cpp` is still compiled |
| `deprecated/ellipsoid/` | `OpenglGeometry/deprecated/` | Old CPU raycasting code. Not listed in `OpenglGeometry/CMakeLists.txt`; would not compile (includes headers at paths that no longer exist) |
| `App::Render/HandleInput/Update/DisplayParameters/CreateShape` | `App.h` | Declared, never defined |
| `App::ScreenToNDC`, `App::GetClickedPoint` | `App.cpp` | Half-finished picking; `GetClickedPoint` computes NDC and a threshold, then returns. No callers |
| `App::showGrid` | `App.h` | Never read |
| `Globals::startingCameraPosition` | `core/Globals.cpp` | `DragCamera` is constructed with its default `(0,0,0,1)` instead |
| `Globals::defaultPointsColor`, `defaultMiddlePointColor` | `core/Globals.cpp` | `Renderer::Render` hard-codes `u_color` |
| `StaticMeshType::Cube` | `managers/StaticMeshManager.cpp` | Built at startup, never requested |
| `Shader::SetUniformVec1i`, `SetUniformVec2f` | `renderer/Shader.cpp` | Never called — `ApplyContext` handles only vec4/mat4 |
| `maxFadeDistance` | `resources/shaders/infiniteGrid.frag` | Declared, unused |
| `d0`, `d2`, `d3` | `resources/shaders/bezierLine/default.tesc` | Computed, only `d1` feeds the tessellation level |

---

## Inconsistencies worth normalising

- `resources/shaders/bezierSurface/defaultVertical.*` is `#version 430`; everything else is
  `#version 460 core`.
- `BufferLayout` attribute names are `"position"` in some places and `"a_Position"` in others.
  Names never reach GL, so this is cosmetic — but it suggests two different conventions.
- Some systems forward-declare `Scene`/`Entity` in their headers, others include the full
  headers.
- Include style mixes `#include "scene/Scene.h"` and `#include <scene/Scene.h>` for project
  headers.
- `CONTROL_PONTS_PER_EDGE` in `BezierSurfaceArchetypeCreation.h` is misspelled ("PONTS").
- `Matrix4::Projection(float aspect, float lastZ, float firstZ, float fov)` — the caller
  passes near as `lastZ` and far as `firstZ`. Read the call site, not the names.
