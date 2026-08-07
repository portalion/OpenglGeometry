# How to add a new shape

The complete path, using a hypothetical **sphere** as the example. Adjust which steps you
need — a shape built from existing control points (like a curve) skips the generation
component entirely, see [the shortcut at the end](#shortcut-a-new-curve-type).

---

## 1. Add a generation component

`src/scene/Components.h`:

```cpp
struct SphereGenerationComponent
{
    float radius = 1.0f;
    unsigned int latitudeSegments  = 16;
    unsigned int longitudeSegments = 32;

    SphereGenerationComponent() = default;
    SphereGenerationComponent(const SphereGenerationComponent& other) = default;
};
```

This component is both the parameter block and the "what kind of shape am I" marker that
`MeshGeneratingSystem` dispatches on. More detail:
[how-to/add-a-new-component.md](add-a-new-component.md).

## 2. Write the mesh generator

`src/meshGenerators/SphereMeshGenerator.h`:

```cpp
#pragma once
#include "Base.h"

namespace MeshGenerator
{
    namespace Sphere
    {
        std::vector<Algebra::Vector4> GenerateVertices(float radius,
                                                       unsigned int latitudeSegments,
                                                       unsigned int longitudeSegments);
        std::vector<uint32_t> GenerateIndices(unsigned int latitudeSegments,
                                              unsigned int longitudeSegments);
        GeneratedMesh<Algebra::Vector4> GenerateMesh(float radius,
                                                     unsigned int latitudeSegments = 16,
                                                     unsigned int longitudeSegments = 32);
    }
}
```

`src/meshGenerators/SphereMeshGenerator.cpp` implements them. Keep it **pure** — no
`Entity`, no OpenGL, no ImGui. Mirror
[`TorusMeshGenerator.cpp`](../../OpenglGeometry/src/meshGenerators/TorusMeshGenerator.cpp),
which is the closest analogue.

Then include the header in
[`MeshGenerators.h`](../../OpenglGeometry/src/meshGenerators/MeshGenerators.h):

```cpp
#include "SphereMeshGenerator.h"
```

**Add the new `.cpp` to the `add_executable` list in `OpenglGeometry/CMakeLists.txt`** — see
[add-a-file-to-the-build.md](add-a-file-to-the-build.md).

## 3. Add a generation pass to `MeshGeneratingSystem`

`src/systems/MeshGeneratingSystem.h`, private section:

```cpp
void SphereGeneration();
```

`src/systems/MeshGeneratingSystem.cpp`:

```cpp
void MeshGeneratingSystem::SphereGeneration()
{
    for (Entity entity : m_Scene->GetAllEntitiesWith<IsDirtyTag, SphereGenerationComponent>())
    {
        entity.RemoveTag<IsDirtyTag>();          // clear the tag FIRST

        const auto& sgc = entity.GetComponent<SphereGenerationComponent>();

        auto generatedMesh = MeshGenerator::Sphere::GenerateMesh(
            sgc.radius, sgc.latitudeSegments, sgc.longitudeSegments);

        ModifyOrCreateMesh(entity, generatedMesh.vertices, generatedMesh.indices,
                           generatedMesh.layout);
    }
}

void MeshGeneratingSystem::Process()
{
    BezierLineGeneration();
    BezierSurfaceGeneration();
    LineGeneration();
    TorusGeneration();
    SphereGeneration();          // ← add the call
}
```

`ModifyOrCreateMesh`'s defaults are `RenderingMode::Lines` and `AvailableShaders::Default`.
Pass different values for triangles or a custom shader:

```cpp
ModifyOrCreateMesh(entity, vertices, indices, layout,
                   RenderingMode::Triangles, { AvailableShaders::Default });
```

## 4. Add the archetype

`src/archetypes/SphereArchetypeCreation.h`:

```cpp
#pragma once
#include "SimpleArchetypeCreation.h"

namespace Archetypes
{
    inline Entity AddSphereToEntity(Entity entity, Algebra::Vector4 position)
    {
        entity.AddTag<IsDirtyTag>();                     // ← without this, no mesh is ever built
        entity.AddComponent<SphereGenerationComponent>();

        entity.AddComponent<PositionComponent>().position = position;
        entity.AddComponent<RotationComponent>();
        entity.AddComponent<ScaleComponent>().scale = { 1.f, 1.f, 1.f };

        return entity;
    }

    inline Entity CreateSphere(Scene* scene, Algebra::Vector4 position)
    {
        auto result = scene->CreateEntity();
        AddShapeToEntity(result, "Sphere");               // ← id + name → appears in the shape list
        AddSphereToEntity(result, position);
        return result;
    }
}
```

Include it from
[`Archetypes.h`](../../OpenglGeometry/src/archetypes/Archetypes.h). Archetypes are
header-only `inline` functions, so there is no `.cpp` at all and nothing to add to the build.

More on the two-layer convention: [archetypes.md](../archetypes.md).

## 5. Add it to the creation menu

`src/UI/popups/ShapeCreation.cpp`, in `Display()`:

```cpp
if (ImGui::MenuItem("Create Sphere##Creation menu"))
{
    Archetypes::CreateSphere(m_Scene.get(), cursorPosition);
}
```

## 6. Add inspector fields

`src/systems/gui/ShapeInspectorSystem.h`:

```cpp
void SphereInspect(Entity entity);
```

`.cpp` — bind in the constructor and implement:

```cpp
Bind<SphereGenerationComponent>(&ShapeInspectorSystem::SphereInspect);

void ShapeInspectorSystem::SphereInspect(Entity entity)
{
    auto& sphere = entity.GetComponent<SphereGenerationComponent>();

    if (ImGui::DragFloat(GUI::GenerateLabel(entity, "Radius").c_str(), &sphere.radius, 0.1f))
        entity.AddTag<IsDirtyTag>();                       // ← mark dirty on every change

    if (GUI::DragUInt(GUI::GenerateLabel(entity, "Latitude Segments").c_str(),
                      &sphere.latitudeSegments, 1.0f, 3, 64))
        entity.AddTag<IsDirtyTag>();
}
```

Details: [how-to/add-an-inspector-field.md](add-an-inspector-field.md).

---

## Checklist

- [ ] Component in `Components.h`
- [ ] Generator in `meshGenerators/`, included from `MeshGenerators.h`
- [ ] Generation pass in `MeshGeneratingSystem`, called from `Process()`
- [ ] Archetype header, included from `Archetypes.h`
- [ ] `IsDirtyTag` set in the archetype
- [ ] `AddShapeToEntity` called (if the shape should be user-visible)
- [ ] Menu item in `ShapeCreation::Display`
- [ ] Inspector handler bound in `ShapeInspectorSystem`

## Common mistakes

| Symptom | Cause |
| --- | --- |
| Shape created but invisible | Forgot `IsDirtyTag` in the archetype, so no `MeshComponent` was ever created |
| Shape not in the *Shape List* | Forgot `AddShapeToEntity` (no `NameComponent`) |
| Editing parameters does nothing | Forgot `entity.AddTag<IsDirtyTag>()` in the inspector handler |
| Unresolved external symbol | The `.cpp` is on disk but not listed in `OpenglGeometry/CMakeLists.txt` |
| Assertion inside EnTT on creation | Used `AddComponent<T>` where the component already exists — use `AddTag`/`AddOrReplaceComponent` |
| Renders as garbage triangles | `RenderingMode` doesn't match how the index buffer was built |

---

## Shortcut: a new curve type

If the shape is just another curve over existing control points, you need **no new
component, no system change and no generator struct** — only a pure function and an
archetype:

1. `meshGenerators/MyCurveGenerator.{h,cpp}` exposing
   `std::vector<Algebra::Vector4> GenerateVertices(const std::vector<Algebra::Vector4>&)`,
   returning a multiple of four Bézier control points.
2. An archetype copying
   [`BezierCurveArchetypeCreation.h`](../../OpenglGeometry/src/archetypes/BezierCurveArchetypeCreation.h),
   with `generationFunction = MeshGenerator::MyCurve::GenerateVertices`.
3. A menu item.

See [geometry/bezier-curves.md](../geometry/bezier-curves.md#adding-a-fourth-curve-type).
