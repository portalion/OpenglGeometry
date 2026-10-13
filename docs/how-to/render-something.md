# How to render something

The shortest path from "I have some vertices" to "they are on screen". The worked example
draws a loaded CAM tool path as a line strip from
[`CAMGUISystem::LoadPath`](../../OpenglGeometry/src/systems/gui/CAMGUISystem.cpp).

If you want a full user-facing modelling shape (shape list, inspector, creation menu,
serialization), follow [add-a-new-shape.md](add-a-new-shape.md) instead.

---

## What the renderer needs

[`RenderingSystem`](../../OpenglGeometry/src/systems/RenderingSystem.cpp) draws **every
entity that has a `MeshComponent`** and no `IsInvisibleTag`. Nothing else is required.

```cpp
struct MeshComponent
{
    Ref<VertexArray> mesh;                     // GPU buffers (vertices + indices)
    std::vector<AvailableShaders> shaderTypes; // drawn once per shader in the list
    RenderingMode renderingMode;               // Triangles, Lines or Patches
};
```

Everything else is optional and is turned into shader uniforms by
[`UniformApplier`](../../OpenglGeometry/src/renderer/UniformApplier.cpp) when present:

| Component / tag | Effect | Default when missing |
| --- | --- | --- |
| `PositionComponent` | translation | identity |
| `RotationComponent` | rotation | identity |
| `ScaleComponent` | scale | identity |
| `ColorComponent` | colour | `Globals::defaultPointsColor` |
| `IsSelectedTag` | draws in `Globals::selectionColor` | — |
| `IsTransparentTag` | drawn after opaque meshes, without depth test | — |
| `IsInvisibleTag` | not drawn at all | — |

---

## Example: draw a tool path

### 1. Keep a handle to the entity

`src/systems/gui/CAMGUISystem.h`:

```cpp
#include <scene/Entity.h>

class CAMGUISystem : public ISystem
{
private:
    // ...
    Entity m_PathEntity;

    void UploadPath(std::vector<Algebra::Vector4>& points);
    // ...
};
```

### 2. Create the mesh once, update it afterwards

`src/systems/gui/CAMGUISystem.cpp`:

```cpp
#include <meshGenerators/PolylineMeshGenerator.h>
#include <renderer/VertexArray.h>
#include <scene/Components.h>

void CAMGUISystem::UploadPath(std::vector<Algebra::Vector4>& points)
{
    // Line segments 0-1, 1-2, 2-3, ... for RenderingMode::Lines
    auto indices = MeshGenerator::Polyline::GenerateIndices(static_cast<unsigned int>(points.size()));

    const BufferLayout layout = { { ShaderDataType::Float4, "position" } };

    if (!m_PathEntity.IsValid())
    {
        m_PathEntity = m_Scene->CreateEntity();
        m_PathEntity.AddComponent<ColorComponent>().color = Algebra::Vector4(0.2f, 0.8f, 1.f, 1.f);

        auto& mesh = m_PathEntity.AddComponent<MeshComponent>();
        mesh.mesh = VertexArray::CreateWithBuffers(points, indices, layout);
        mesh.shaderTypes = { AvailableShaders::Default };
        mesh.renderingMode = RenderingMode::Lines;
        return;
    }

    // Reloading: reuse the existing GPU buffers
    auto& vertexArray = m_PathEntity.GetComponent<MeshComponent>().mesh;
    vertexArray->GetVertexBuffers()[0]->SetData(points.data(),
        static_cast<uint32_t>(points.size() * sizeof(Algebra::Vector4)));
    vertexArray->GetIndexBuffer()->SetIndices(indices.data(),
        static_cast<uint32_t>(indices.size()));
}
```

### 3. Call it from `LoadPath`

```cpp
void CAMGUISystem::LoadPath(const std::string& file)
{
    std::vector<Algebra::Vector4> points;

    // ... parse `file` into points; w must be 1 ...
    // points.push_back(Algebra::Vector4(x, y, z, 1.f));

    if (points.size() < 2)
    {
        Logger::Warning("Path {} has fewer than 2 points", file);
        return;
    }

    UploadPath(points);
}
```

That's all. `RenderingSystem` is already in the CAM pipeline, so the path is drawn on the
next frame.

---

## Choosing the pieces

**Rendering mode.** It must match how the indices are built:

| Mode | Indices | Use for |
| --- | --- | --- |
| `RenderingMode::Lines` | pairs `a b`, one per segment | paths, wireframes, grids |
| `RenderingMode::Triangles` | triples `a b c` | solid surfaces (e.g. a heightmap / material block) |
| `RenderingMode::Patches` | groups for tessellation shaders | Bézier curves and surfaces only |

**Shader.** `AvailableShaders::Default` takes one `vec4 position` attribute and a flat
colour, which suits most CAM visuals. If you need normals, lighting or per-vertex data,
add a shader ([add-a-new-shader.md](add-a-new-shader.md)) and give the `BufferLayout` the
matching attributes, e.g.:

```cpp
const BufferLayout layout =
{
    { ShaderDataType::Float4, "position" },
    { ShaderDataType::Float4, "normal" },
};
```

The vertex type passed to `CreateWithBuffers` must then be a struct with exactly that
layout, and `SetData` must use `sizeof` of that struct.

**Moving it around.** Add a `PositionComponent` (and `RotationComponent` / `ScaleComponent`)
and change its value each frame instead of rebuilding the mesh. That's the cheap way to
animate a cutter along the path:

```cpp
m_CutterEntity.GetComponent<PositionComponent>().position = currentToolPosition;
```

**Built-in shapes.** For simple markers you can borrow a ready mesh from
[`StaticMeshManager`](../../OpenglGeometry/src/managers/StaticMeshManager.h) instead of
building one, as the cursor does in
[`CursorArchetypeCreation.h`](../../OpenglGeometry/src/archetypes/CursorArchetypeCreation.h):

```cpp
mesh.mesh = StaticMeshManager::GetInstance().GetMesh(StaticMeshType::Crosshair);
```

---

## Things to know in CAM mode

- **Both modes share one `Scene`.** `ModeManager` gives the modelling and CAM pipelines the
  same scene, so an entity you create in CAM mode is also drawn in modelling mode (and the
  other way round). If that's unwanted, give CAM entities a tag of your own (in
  [`Tags.h`](../../OpenglGeometry/src/scene/Tags.h)) and add/remove `IsInvisibleTag` on them
  when the mode changes.
- **No `IdComponent` / `NameComponent` = not a "shape".** The entity above has neither, so
  it doesn't show in the modelling Shape List, can't be selected or deleted there and isn't
  saved with the scene. That's usually what you want for CAM visuals.
- **File → New / Open won't remove it.** `Scene::Clear()` only destroys entities with an
  `IdComponent`, so the path stays on screen after a scene load. Destroy it yourself if
  that matters, and keep checking `m_PathEntity.IsValid()` before use, as `UploadPath` does.
- **No `MeshGeneratingSystem` in the CAM pipeline.** The `IsDirtyTag` + generation-component
  route from [add-a-new-shape.md](add-a-new-shape.md) only works if you add that system to
  `CAMSystemPipeline`. For CAM data that changes only on load, uploading directly (as above)
  is simpler.
- **No `RemovalSystem` either.** To delete an entity in CAM mode call
  `m_Scene->DestroyEntity(entity)` directly; `ToBeDestroyedTag` does nothing there.

## Common mistakes

| Symptom | Cause |
| --- | --- |
| Nothing drawn | Points have `w = 0` (the shader multiplies by the model matrix; use `w = 1`) |
| Nothing drawn | `shaderTypes` is empty, so the draw loop never runs |
| Random segments / garbage | `RenderingMode` doesn't match the index layout |
| Old path still visible after reload | Created a new entity instead of updating `m_PathEntity` |
| Crash after destroying the path | Used a destroyed entity; check `IsValid()` first |
| Path also visible in modelling mode | Shared scene, see above |
