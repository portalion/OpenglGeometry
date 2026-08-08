# Components

All components live in one file: [`src/scene/Components.h`](../../OpenglGeometry/src/scene/Components.h).
They are plain aggregates with a defaulted default-constructor and copy-constructor.

Empty marker types are documented separately in [tags.md](tags.md).

---

## Transform

### `PositionComponent`

```cpp
struct PositionComponent
{
    Observable<Algebra::Vector4> position;
    void Move(Algebra::Vector4 offset);
};
```

The only component whose value is wrapped in [`Observable`](change-propagation.md): assigning
to `position` tags the owning entity with `ObserverChangedState`, which is how dependents
learn to rebuild. `Scene` installs an `on_construct<PositionComponent>` hook to give the
`Observable` a back-reference to its entity.

Reads convert implicitly to `const Algebra::Vector4&`, so `Vector4 p = pc.position;` works.
Writes must go through `operator=` — do **not** take a reference to the inner value and mutate
it, or the change is never observed. The correct pattern (from `ShapeInspectorSystem`):

```cpp
Algebra::Vector4 tmp = entity.GetComponent<PositionComponent>().position;  // copy out
auto& position = entity.GetComponent<PositionComponent>().position;
if (ImGui::DragFloat3(label, &tmp.x, 0.1f))
    position = tmp;                                                        // assign back
```

> `Move()` currently **assigns** rather than adds (`position = offset`). See [gotchas](../gotchas.md).

*Consumed by:* `UniformApplier::PositionApplier` (→ `u_modelMatrix`),
`MeshGeneratingSystem` (control-point positions), `ShapeInspectorSystem::PositionInspect`.

### `RotationComponent`

```cpp
struct RotationComponent
{
    Algebra::Quaternion rotation;
    void Rotate(Algebra::Quaternion rotation);   // accumulates: this = this * r, then normalise
};
```

*Consumed by:* `UniformApplier::RotationApplier` (→ `rotation.ToMatrix()` into the model
matrix), `ShapeInspectorSystem::RotationInspect`.

> The inspector drags `rotation.x/y/z` (raw quaternion components) with `DragFloat3`, which
> is not a well-behaved rotation UI. See [gotchas](../gotchas.md).

### `ScaleComponent`

```cpp
struct ScaleComponent { Algebra::Vector4 scale; };
```

*Consumed by:* `UniformApplier::ScaleApplier` (→ `Matrix4::DiagonalScaling(x, y, z)`),
`ShapeInspectorSystem::ScaleInspect`.

The three transform components are independent: an entity may have any subset. Missing ones
simply contribute an identity matrix, because `EntityContext` initialises all three to
`Matrix4::Identity()` and the applier only runs for components that exist.

---

## Identity

### `IdComponent`

```cpp
struct IdComponent
{
    const ID id = IdManager::GetInstance().GetNewId();
    ~IdComponent() { IdManager::GetInstance().FreeId(id); }
};
```

A process-unique `long long` handed out by [`IdManager`](../managers.md), recycled on
destruction. Used to build human-readable names (`"Torus 7"`). Distinct from
`Entity::GetID()`, which returns the raw EnTT index and is what ImGui labels use.

> The destructor frees the id, but the copy constructor is defaulted, so copying an
> `IdComponent` produces two objects that will both free the same id. In practice components
> are not copied around, but be aware of it.

### `NameComponent`

```cpp
struct NameComponent { std::string name; };
```

The display name. **This component is what makes an entity appear in the *Shape List*
panel** (`GUI::DisplayShapeList` views `<NameComponent>`), and `ShapeInspectorSystem`
unconditionally reads it for any selected entity. So: give an entity a `NameComponent` if
and only if it should be user-visible and selectable.

Set by `Archetypes::AddShapeToEntity`, which formats `"<shapeName> <id>"`.

---

## Rendering

### `MeshComponent`

```cpp
struct MeshComponent
{
    Ref<VertexArray> mesh;
    std::vector<AvailableShaders> shaderTypes;
    RenderingMode renderingMode = RenderingMode::Triangles;
};
```

The one component `RenderingSystem` requires. `shaderTypes` is a *list*: the same mesh is
drawn once per shader. Bézier surfaces exploit this — they are drawn with
`BezierSurfaceHorizontal` and `BezierSurfaceVertical` to produce a cross-hatched isoline
grid from a single vertex buffer.

`renderingMode` maps directly onto the GL primitive (`GL_TRIANGLES`, `GL_LINES`,
`GL_PATCHES`). Anything tessellated must use `Patches`.

Created either directly (points, cursor, grid — using a shared mesh from
`StaticMeshManager`) or by `MeshGeneratingSystem::ModifyOrCreateMesh` (generated geometry).

### `CameraComponent`

```cpp
struct CameraComponent
{
    bool active = false;
    Algebra::Matrix4 projectionMatrix;
    Algebra::Matrix4 viewMatrix;
    Ref<ICamera> cameraHandling;
};
```

`RenderingSystem` iterates all `CameraComponent`s and uses the ones with `active == true`
(if several are active, the last one wins). `cameraHandling` is a strategy object
implementing [`ICamera`](../core.md#icamera--dragcamera); the system calls
`cameraHandling->HandleInput(cameraComponent)` each frame, and the implementation writes
back into `viewMatrix`.

`projectionMatrix` is set once in `BaseScene` and is **not** currently updated on window
resize — see [gotchas](../gotchas.md).

---

## Generation (what shape am I?)

These components are the "type tag" of a shape: `MeshGeneratingSystem` dispatches on them.

### `TorusGenerationComponent`

```cpp
struct TorusGenerationComponent
{
    float radius = 1.0f;
    float tubeRadius = 0.2f;
    unsigned int radialSegments = 16;
    unsigned int tubularSegments = 32;
};
```

Editable in the inspector (`TorusInspect`); every edit sets `IsDirtyTag`.
Consumed by `MeshGenerator::Torus::GenerateMesh`. See [geometry](../geometry/README.md).

### `LineGenerationComponent`

```cpp
struct LineGenerationComponent { std::list<Entity> controlPoints; };
```

An ordered list of point entities. A `std::list` is used because entries are erased during
iteration when they become invalid.

Alone → the entity is a **polyline** (`MeshGeneratingSystem::LineGeneration`, which excludes
entities that also have `BezierLineGenerationComponent`).

### `BezierLineGenerationComponent`

```cpp
struct BezierLineGenerationComponent
{
    std::function<std::vector<Algebra::Vector4>(const std::vector<Algebra::Vector4>&)>
        generationFunction;
};
```

Combined with `LineGenerationComponent` → the entity is a **Bézier curve**. The stored
function turns control points into the flat list of Bézier control points the tessellation
shader consumes, and is what distinguishes the curve variants:

| Archetype | `generationFunction` |
| --- | --- |
| Bézier C0 | `MeshGenerator::BezierCurveC0::GenerateVertices` |
| Bézier C2 (B-spline) | `MeshGenerator::BezierCurveC2::GenerateVertices` |
| Interpolated Bézier | `MeshGenerator::InterpolatedBezierCurve::GenerateVertices` |

Adding a fourth curve type means writing one pure function and one archetype — no new
component, no system change. See [geometry/bezier-curves.md](../geometry/bezier-curves.md).

### `BezierPatchGenerationComponent`

```cpp
struct BezierPatchGenerationComponent
{
    std::array<std::array<Entity, 4>, 4> controlPoints;
};
```

A single 4×4 Bézier patch. Patches are *virtual* entities (they have
`VirtualEntityComponent` pointing at the surface) and have no mesh of their own — the
surface gathers their control points.

### `BezierSurfaceGenerationComponent`

```cpp
struct BezierSurfaceGenerationComponent
{
    std::vector<std::vector<Entity>> bezierPatches;   // [x][y] grid of patch entities
};
```

The surface entity. `MeshGeneratingSystem::BezierSurfaceGeneration` flattens every patch's
16 control points into one buffer and draws it with `patchSize == 16`.
See [geometry/bezier-surfaces.md](../geometry/bezier-surfaces.md).

---

## Relationships

### `NotificationComponent`

```cpp
struct NotificationComponent { std::list<Entity> entitiesToNotify; };
```

"Who depends on me." Points get this component (`AddPointToEntity`), and when a shape is
built from points, the shape entity is appended to each point's list
(`Archetypes::AddNotifiersToEntityContainer`). `NotificationSystem` walks the list and
marks every listed entity `IsDirtyTag`.

It doubles as a **marker for "this entity is a point"**:
`ShapeCreation::GetSelectedPoints` views `<IsSelectedTag, NotificationComponent>` to find
the selected points that can serve as control points.

### `VirtualEntityComponent`

```cpp
struct VirtualEntityComponent { Entity realEntity; };
```

"I am a helper owned by `realEntity`." Virtual entities are the control polylines drawn
alongside Bézier curves, the patch entities of a surface, and (optionally) the surface's own
control points. When a virtual entity changes it dirties its parent; when its parent is gone
it marks itself `ToBeDestroyedTag`.

### `IsParentOfVirtualEntitiesComponent`

```cpp
struct IsParentOfVirtualEntitiesComponent { std::vector<Entity> virtualEntities; };
```

The inverse link. Used by `RemovalSystem` to cascade deletion, and by
`ShapeInspectorSystem::VirtualInspect` to render the "show/hide helpers" checkbox.

Both sides are wired up together by `Archetypes::AddVirtualToEntity`; never set one without
the other.

---

## Quick reference: which components make which shape

| Shape | Components (beyond `IdComponent` + `NameComponent`) |
| --- | --- |
| Point | `PositionComponent`, `NotificationComponent`, `MeshComponent` (shared square) |
| Torus | `TorusGenerationComponent`, `PositionComponent`, `RotationComponent`, `ScaleComponent`, `IsDirtyTag` |
| Polyline | `LineGenerationComponent`, `IsDirtyTag` |
| Bézier C0 / C2 | `LineGenerationComponent`, `BezierLineGenerationComponent`, `IsDirtyTag`, `IsParentOfVirtualEntitiesComponent` (control polyline) |
| Interpolated Bézier | `LineGenerationComponent`, `BezierLineGenerationComponent`, `IsDirtyTag` (no control polyline) |
| Bézier surface | `BezierSurfaceGenerationComponent`, `IsDirtyTag`, `IsParentOfVirtualEntitiesComponent` (patches) |
| Bézier patch (virtual) | `BezierPatchGenerationComponent`, `VirtualEntityComponent` |
| Camera | `CameraComponent`, `NameComponent` |
| Grid | `MeshComponent` only |
| Cursor | `PositionComponent`, `MeshComponent` |
