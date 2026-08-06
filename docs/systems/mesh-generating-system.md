# `MeshGeneratingSystem`

**Files:** [`src/systems/MeshGeneratingSystem.h`](../../OpenglGeometry/src/systems/MeshGeneratingSystem.h),
[`.cpp`](../../OpenglGeometry/src/systems/MeshGeneratingSystem.cpp)
**Position in pipeline:** 6th (after all UI, before rendering)

The bridge between the ECS and the pure geometry functions in
[`meshGenerators/`](../geometry/README.md). It finds every entity tagged `IsDirtyTag`,
regenerates its vertex/index data, and uploads it.

## `Process()`

```cpp
void MeshGeneratingSystem::Process()
{
    BezierLineGeneration();
    BezierSurfaceGeneration();
    LineGeneration();
    TorusGeneration();
}
```

Four independent passes, one per shape family. They are mutually exclusive by construction —
each views a different component combination — so the call order between them does not
matter.

Every pass follows the same shape:

```cpp
for (Entity entity : m_Scene->GetAllEntitiesWith<IsDirtyTag, /* generation component */>())
{
    entity.RemoveTag<IsDirtyTag>();      // ← cleared first, before any work
    ...gather inputs...
    auto generated = MeshGenerator::X::GenerateMesh(...);
    ModifyOrCreateMesh(entity, generated.vertices, generated.indices, generated.layout, ...);
}
```

Clearing the tag first means a generator that throws or returns early does not leave the
entity permanently dirty (which would burn a regeneration every frame forever).

## The four passes

### `TorusGeneration`

View: `<IsDirtyTag, TorusGenerationComponent>`

Reads the four torus parameters, calls `MeshGenerator::Torus::GenerateMesh`, uploads with
the default rendering mode (`Lines`) and the default shader (`AvailableShaders::Default`).
The torus is drawn as a wireframe of radial and tubular rings.

### `LineGeneration`

View: `<IsDirtyTag, LineGenerationComponent>` **excluding** `BezierLineGenerationComponent`

This exclusion is the mechanism that separates a plain polyline from a Bézier curve — both
carry `LineGenerationComponent`, only the curve carries the Bézier component as well.

Control-point positions are gathered by `CopyValidPointsToVector` and passed to
`MeshGenerator::Polyline::GenerateMesh`, which emits `GL_LINES` index pairs.

### `BezierLineGeneration`

View: `<IsDirtyTag, LineGenerationComponent, BezierLineGenerationComponent>`

```cpp
auto& controlPoints    = entity.GetComponent<LineGenerationComponent>().controlPoints;
const auto& generator  = entity.GetComponent<BezierLineGenerationComponent>().generationFunction;

std::vector<Algebra::Vector4> positions = CopyValidPointsToVector(controlPoints);

auto vertices = generator(positions);
auto indices  = GenerateLineIndices(vertices.size());   // 0,1,2,3,...

ModifyOrCreateMesh(entity, vertices, indices, bezierShaderLayout,
                   RenderingMode::Patches, { AvailableShaders::BezierCurveC0 });
```

The stored `generationFunction` is what makes C0, C2 and interpolated curves different — the
system itself is agnostic. Output is a flat list of Bézier control points in groups of four,
drawn as `GL_PATCHES` with `patchSize == 4`, so the tessellation shader evaluates each cubic
segment on the GPU. Indices are simply `0..n-1`.

All three curve variants use the `BezierCurveC0` shader program; the name refers to the
shader files, not to curve continuity.

### `BezierSurfaceGeneration`

View: `<IsDirtyTag, BezierSurfaceGenerationComponent>`

```cpp
for (auto patchRow : patches)
    for (auto patch : patchRow)
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
            {
                vertices.push_back(patch.GetComponent<BezierPatchGenerationComponent>()
                                        .controlPoints[i][j]
                                        .GetComponent<PositionComponent>().position);
                indices.push_back(indice++);
            }
```

Flattens every patch into 16 consecutive vertices. Drawn as `GL_PATCHES` with
`patchSize == 16`, with **two** shader programs
(`BezierSurfaceHorizontal` + `BezierSurfaceVertical`) so the same buffer produces isolines in
both parametric directions — the classic wireframe look.

> This pass does not validate the patch or point entities before dereferencing them. See
> [gotchas](../gotchas.md).

## Helpers

### `CopyValidPointsToVector`

```cpp
std::vector<Algebra::Vector4> CopyValidPointsToVector(std::list<Entity>& pointEntities);
```

Copies positions out of a control-point list and **prunes invalid entries from the list as a
side effect** (hence the non-const reference and the `std::list`). Also forces `w = 1.f` on
every position, so a point whose `w` drifted still transforms correctly.

This is where deleted control points silently disappear from curves.

### `GenerateLineIndices`

Produces `0, 1, 2, … n-1` — a pass-through index buffer for geometry that is already in draw
order. Used by the Bézier passes. (Note `MeshGenerator::Polyline::GenerateIndices` is
different: it emits *pairs* for `GL_LINES`.)

### `ModifyOrCreateMesh`

```cpp
template<typename T>
void ModifyOrCreateMesh(Entity e,
                        std::vector<T>& vertices,
                        std::vector<uint32_t>& indices,
                        const BufferLayout& layout,
                        const RenderingMode& mode = RenderingMode::Lines,
                        std::initializer_list<AvailableShaders> shaderTypes = { AvailableShaders::Default });
```

Header-only template. Two paths:

- **Entity already has a `MeshComponent`** → reuse the existing `VertexArray`, calling
  `SetData` / `SetLayout` on vertex buffer 0 and `SetIndices` on the index buffer. No GL
  object is recreated, so a slider drag re-uploads without churning handles.
- **No `MeshComponent`** → `VertexArray::CreateWithBuffers` builds VAO + VBO + IBO and the
  component is added.

Either way `renderingMode` and `shaderTypes` are overwritten from the arguments.

> The reuse path computes the upload size as `vertices.size() * sizeof(Algebra::Vector4)`
> rather than `sizeof(T)`. All current callers use `T = Algebra::Vector4`, so it works, but
> it will silently corrupt uploads for any other vertex type. See [gotchas](../gotchas.md).

## Adding a new generated shape

1. Add a `XGenerationComponent` holding the parameters ([how-to](../how-to/add-a-new-component.md)).
2. Write a pure generator under `meshGenerators/` returning
   `GeneratedMesh<Algebra::Vector4>` ([geometry](../geometry/README.md)).
3. Add an `XGeneration()` private method here viewing `<IsDirtyTag, XGenerationComponent>`,
   and call it from `Process()`.
4. Add an archetype that attaches the component and `IsDirtyTag`
   ([archetypes](../archetypes.md)).
5. Optionally bind an inspector handler ([how-to](../how-to/add-an-inspector-field.md)).

The full walkthrough is in [how-to/add-a-new-shape.md](../how-to/add-a-new-shape.md).
