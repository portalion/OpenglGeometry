# Archetypes

[`src/archetypes/`](../OpenglGeometry/src/archetypes/) — header-only, `inline`, all in
`namespace Archetypes`.

An archetype is a **recipe**: a function that attaches the right set of components to an
entity so it becomes a particular kind of shape. There are no shape classes; this is the
closest thing to a constructor.

| File | Contents |
| --- | --- |
| `Archetypes.h` | Convenience header including all the others |
| `SimpleArchetypeCreation.h` | The shared building blocks (`AddShapeToEntity`, `AddVirtualToEntity`, `AddLineToEntity`, `AddNotifiersToEntityContainer`) |
| `PointArchetypeCreation.h` | Point |
| `TorusArchetypeCreation.h` | Torus |
| `PolylineArchetypeCreation.h` | Polyline |
| `BezierCurveArchetypeCreation.h` | Bézier C0 and C2 |
| `InterpolatedBezierCurveArchetypeCreation.h` | Interpolated Bézier |
| `BezierSurfaceArchetypeCreation.h` | Bézier surface (rectangular and cylindrical) |

## The two-layer convention

Every archetype comes in two flavours:

```cpp
// composable: attach this feature to an entity that already exists
inline Entity AddTorusToEntity(Entity entity, Algebra::Vector4 position);

// convenient: create an entity and compose the features
inline Entity CreateTorus(Scene* scene, Algebra::Vector4 position)
{
    auto resultTorus = scene->CreateEntity();
    AddShapeToEntity(resultTorus, "Torus");
    AddTorusToEntity(resultTorus, position);
    return resultTorus;
}
```

`Add*` never creates the entity, so features compose. A Bézier curve is literally
"a line + a Bézier generator + a virtual polyline":

```cpp
AddVirtualToEntity(virtualPolyline, entity);
AddPolylineToEntity(virtualPolyline, pointsBegin, pointsEnd);

AddLineToEntity(entity, pointsBegin, pointsEnd);
entity.AddComponent<BezierLineGenerationComponent>().generationFunction = ...;
```

Both layers return the `Entity` so calls can be chained or the result captured.

`Create*` takes a raw `Scene*` (callers pass `m_Scene.get()`), while `Add*` takes an `Entity`,
which already carries its scene pointer.

---

## Building blocks — `SimpleArchetypeCreation.h`

### `AddShapeToEntity`

```cpp
inline Entity AddShapeToEntity(Entity entity, std::string shapeName)
{
    auto id = entity.AddComponent<IdComponent>().id;
    entity.AddComponent<NameComponent>().name = shapeName + ' ' + std::to_string(id);
    return entity;
}
```

Makes an entity a **user-visible shape**: it gets a unique id and a display name like
`"Torus 7"`, which is what puts it in the *Shape List* panel. Every `Create*` calls this
first. Entities that should stay hidden (the grid, Bézier patches) deliberately do not.

### `AddVirtualToEntity`

```cpp
inline Entity AddVirtualToEntity(Entity entity, Entity parent)
{
    entity.AddComponent<VirtualEntityComponent>().realEntity = parent;

    if (!parent.HasComponent<IsParentOfVirtualEntitiesComponent>())
        parent.AddComponent<IsParentOfVirtualEntitiesComponent>();

    parent.GetComponent<IsParentOfVirtualEntitiesComponent>().virtualEntities.push_back(entity);
    return entity;
}
```

Sets **both sides** of the parent/helper link in one call. Never write one side by hand: the
child side drives change propagation and orphan cleanup, the parent side drives cascade
deletion and the visibility checkbox.

### `AddNotifiersToEntityContainer`

```cpp
template<std::forward_iterator Iter>
    requires std::same_as<std::iter_value_t<Iter>, Entity>
inline Entity AddNotifiersToEntityContainer(Entity parent, std::list<Entity>& entityContainer,
                                            Iter pointsBegin, const Iter& pointsEnd);
```

For each source point: appends `parent` to the point's `NotificationComponent::entitiesToNotify`
and appends the point to `entityContainer`. This is what makes a shape rebuild when its
control points move.

> The skip branch (`!IsValid() || !HasComponent<NotificationComponent>()`) does `continue`
> without advancing the iterator — an infinite loop if any input entity fails the check. See
> [gotchas](gotchas.md).

### `AddLineToEntity`

```cpp
inline Entity AddLineToEntity(Entity entity, Iter pointsBegin, const Iter& pointsEnd)
{
    auto& controlPoints = entity.AddComponent<LineGenerationComponent>().controlPoints;
    AddNotifiersToEntityContainer(entity, controlPoints, pointsBegin, pointsEnd);
    return entity;
}
```

"This entity is driven by an ordered list of points." Used by polylines and by every curve.

### The iterator constraint

All the point-consuming archetypes are templated on the iterator:

```cpp
template<std::forward_iterator Iter>
    requires std::same_as<std::iter_value_t<Iter>, Entity>
```

So they accept a `std::vector<Entity>`, a `std::list<Entity>`, or any other forward range of
entities. `ShapeCreation` passes `std::vector<Entity>::iterator`.

---

## The shape recipes

### Point

```cpp
inline Entity AddPointToEntity(Entity entity, Algebra::Vector4 startingPosition)
{
    entity.AddComponent<PositionComponent>().position = startingPosition;
    entity.AddComponent<NotificationComponent>();
    auto& meshComponent = entity.AddComponent<MeshComponent>();
    meshComponent.mesh = StaticMeshManager::GetInstance().GetMesh(StaticMeshType::Square);
    meshComponent.shaderTypes.push_back(AvailableShaders::Default);
    return entity;
}
```

Points share one static square mesh (0.1 units, XY-aligned) — no per-point geometry is
generated. The `NotificationComponent` is added even before anything depends on the point;
it is both the "I can notify" capability and the marker that identifies control points
(`<IsSelectedTag, NotificationComponent>`).

### Torus

```cpp
entity.AddTag<IsDirtyTag>();                       // generate on the first frame
entity.AddComponent<TorusGenerationComponent>();   // default 1.0 / 0.2 / 16 / 32
entity.AddComponent<PositionComponent>().position = position;
entity.AddComponent<RotationComponent>();
entity.AddComponent<ScaleComponent>().scale = { 1.f, 1.f, 1.f };
```

The only shape with a full transform. Note `IsDirtyTag` — without it the entity would have no
`MeshComponent` and would never be drawn.

### Polyline

```cpp
entity.AddTag<IsDirtyTag>();
AddLineToEntity(entity, pointsBegin, pointsEnd);
```

No transform: the geometry *is* the world-space control point positions.

### Bézier C0 / C2

Covered in [geometry/bezier-curves.md](geometry/bezier-curves.md#how-the-archetypes-differ).
Both create a virtual control polyline; the only difference between them is which
`generationFunction` is stored.

### Interpolated Bézier

Same, minus the virtual polyline.

### Bézier surface

Covered in [geometry/bezier-surfaces.md](geometry/bezier-surfaces.md). This is the only
archetype with substantial logic of its own — grid generation, patch assembly, seam closing.

---

## Writing a new archetype

```cpp
#pragma once
#include "SimpleArchetypeCreation.h"

namespace Archetypes
{
    inline Entity AddMyShapeToEntity(Entity entity, Algebra::Vector4 position)
    {
        entity.AddTag<IsDirtyTag>();                        // if it has generated geometry
        entity.AddComponent<MyShapeGenerationComponent>();
        entity.AddComponent<PositionComponent>().position = position;
        return entity;
    }

    inline Entity CreateMyShape(Scene* scene, Algebra::Vector4 position)
    {
        auto result = scene->CreateEntity();
        AddShapeToEntity(result, "My Shape");               // id + name → visible in the list
        AddMyShapeToEntity(result, position);
        return result;
    }
}
```

Then include the header from `Archetypes.h` and add a menu item in
[`ShapeCreation::Display`](systems/gui-systems.md#shapecreation).

Checklist:

- `AddShapeToEntity` if the shape should be user-visible.
- `IsDirtyTag` if it has generated geometry, otherwise it never gets a mesh.
- `AddLineToEntity` / `AddNotifiersToEntityContainer` if it depends on other entities.
- `AddVirtualToEntity` for any helper geometry you create alongside it.
- Headers only — the archetypes are `inline` and have no `.cpp`, so nothing to add to the
  build files to touch.

Full walkthrough: [how-to/add-a-new-shape.md](how-to/add-a-new-shape.md).
