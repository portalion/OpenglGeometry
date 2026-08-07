# Geometry generation

[`src/meshGenerators/`](../../OpenglGeometry/src/meshGenerators/) holds **pure functions**:
they take `std::vector<Algebra::Vector4>` (or plain scalars) and return vertices and indices.
No entities, no components, no OpenGL. This makes them the easiest part of the codebase to
read, change and reason about.

`MeshGeneratingSystem` is the only caller. See
[systems/mesh-generating-system.md](../systems/mesh-generating-system.md).

| File | Namespace | Produces |
| --- | --- | --- |
| `Base.h` | `MeshGenerator` | `GeneratedMesh<VertexType>` |
| `TorusMeshGenerator.*` | `MeshGenerator::Torus` | Full mesh (wireframe torus) |
| `PolylineMeshGenerator.*` | `MeshGenerator::Polyline` | Full mesh (line segments) |
| `BezierCurveGenerator.*` | `MeshGenerator::BezierCurveC0`, `::BezierCurveC2` | Bézier control points |
| `InterpolatedBezierCurveGenerator.*` | `MeshGenerator::InterpolatedBezierCurve` | Bézier control points |
| `MeshGenerators.h` | — | Convenience header including all of the above |

Sub-pages: [Bézier curves](bezier-curves.md) · [Bézier surfaces](bezier-surfaces.md)

---

## `GeneratedMesh`

[`Base.h`](../../OpenglGeometry/src/meshGenerators/Base.h)

```cpp
namespace MeshGenerator
{
    template <typename VertexType>
    struct GeneratedMesh
    {
        std::vector<VertexType> vertices;
        std::vector<uint32_t>   indices;
        BufferLayout layout = { { ShaderDataType::Float4, "position" } };
    };
}
```

The layout defaults to a single `vec4` position, which is what every generator uses. Override
it if a generator emits interleaved data (normals, colours).

## Two kinds of generator

**Mesh generators** return a complete `GeneratedMesh` (vertices *and* an index buffer laid out
for a specific primitive type). Torus and polyline are these.

**Vertex generators** return only `std::vector<Algebra::Vector4>` — a flat list of Bézier
control points to be fed to a tessellation shader. The caller synthesises trivial
`0..n-1` indices. All three curve types are these, and their signature is exactly

```cpp
std::vector<Algebra::Vector4> (const std::vector<Algebra::Vector4>&)
```

which is why they are interchangeable through
`BezierLineGenerationComponent::generationFunction`.

---

## Torus

[`TorusMeshGenerator.cpp`](../../OpenglGeometry/src/meshGenerators/TorusMeshGenerator.cpp)

```cpp
Algebra::Vector4 GetPoint(float angleTube, float angleRadius, float radius, float tubeRadius)
{
    return Algebra::Matrix4::RotationY(angleRadius) *
           Algebra::Vector4(radius + tubeRadius * cosf(angleTube),
                            tubeRadius * sinf(angleTube), 0.f, 1.f);
}
```

A point on the tube circle in the XY plane, swept around the Y axis. The torus therefore lies
in the XZ plane with Y as its axis.

`GenerateVertices` walks `radialSegments × tubularSegments` and stores them row-major
(`index = tubularSegments * i + j`).

`GenerateIndices` emits `GL_LINES` pairs in both directions, wrapping with `%`:

```cpp
indices.push_back(tubularSegments * i + j);          // along the tube
indices.push_back(tubularSegments * i + jNext);
indices.push_back(tubularSegments * i + j);          // around the ring
indices.push_back(tubularSegments * iNext + j);
```

That is 4 indices per vertex → `4 · radialSegments · tubularSegments` indices total, drawn as
a wireframe. There are no triangles and no normals; rendering a solid torus would need a
different index generator.

`2 * 3.14f` is used for τ rather than `std::numbers::pi_v<float>`, which is a small
inaccuracy (≈0.05%) that leaves a hairline seam where the ring wraps.

Parameters come from `TorusGenerationComponent`, editable in the inspector with segment counts
clamped to `[3, 64]`.

---

## Polyline

[`PolylineMeshGenerator.cpp`](../../OpenglGeometry/src/meshGenerators/PolylineMeshGenerator.cpp)

```cpp
std::vector<uint32_t> GenerateIndices(unsigned int verticesSize)
{
    for (int i = 0; i < (int)verticesSize - 1; i++)
    {
        indices.push_back(i);
        indices.push_back(i + 1);
    }
}
```

Vertices pass through unchanged; indices form consecutive `GL_LINES` pairs. The `(int)` cast
matters: without it, `verticesSize - 1` on an empty list would wrap around.

Used both for standalone polylines and for the control polylines drawn alongside Bézier
curves.

> Do not confuse this with `MeshGeneratingSystem::GenerateLineIndices`, which produces
> `0,1,2,…,n-1` for `GL_PATCHES` rather than pairs.

---

## Adding a generator

1. Create `meshGenerators/MyShapeGenerator.{h,cpp}` following the namespace pattern:

```cpp
#pragma once
#include "Base.h"

namespace MeshGenerator
{
    namespace MyShape
    {
        GeneratedMesh<Algebra::Vector4> GenerateMesh(/* parameters */);
    }
}
```

2. Add the header to [`MeshGenerators.h`](../../OpenglGeometry/src/meshGenerators/MeshGenerators.h).
3. Call it from a new pass in `MeshGeneratingSystem`.

Add the new `.cpp` to the `add_executable` list in `OpenglGeometry/CMakeLists.txt`
([how-to](../how-to/add-a-file-to-the-build.md)).

Keep the function pure — no `Entity`, no GL calls. That separation is what makes this layer
easy to test and to reuse (the Bézier vertex generators are reused verbatim by three
different archetypes).

Full walkthrough: [how-to/add-a-new-shape.md](../how-to/add-a-new-shape.md).
