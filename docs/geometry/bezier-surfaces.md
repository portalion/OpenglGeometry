# Bézier surfaces

A Bézier surface is a grid of **bicubic patches**, each defined by 4×4 control points, sharing
edges with its neighbours. Unlike the curves, there is no CPU generator — the entire
construction lives in
[`BezierSurfaceArchetypeCreation.h`](../../OpenglGeometry/src/archetypes/BezierSurfaceArchetypeCreation.h),
and evaluation happens entirely in the tessellation shaders.

## Entity structure

```
surface entity                                          ← the only entity with a mesh
 ├─ BezierSurfaceGenerationComponent { bezierPatches[x][y] }
 ├─ IsParentOfVirtualEntitiesComponent
 ├─ IsDirtyTag
 │
 ├─ patch entity (virtual)  × numberOfXPatches · numberOfYPatches
 │    ├─ BezierPatchGenerationComponent { controlPoints[4][4] }
 │    └─ VirtualEntityComponent → surface
 │
 └─ control points  (real entities by default, or virtual if createVirtual == true)
      ├─ PositionComponent, NotificationComponent, MeshComponent
      └─ NotificationComponent lists the patches that use it
```

Patches are pure data holders — they carry no mesh. The surface gathers every patch's control
points into one buffer each time it is dirtied.

## Parameters

```cpp
struct BezierSurfaceCreationParameters
{
    bool  isCylinder = false;
    float sizeX = 4;
    float sizeY = 4;
    unsigned int numberOfXPatches = 1;
    unsigned int numberOfYPatches = 1;
    Algebra::Vector4 startingPosition;
};

const unsigned int CONTROL_PONTS_PER_EDGE = 4;
```

## Control point count

Adjacent patches share an edge, so a strip of *n* patches needs `3n + 1` points, not `4n`:

```cpp
numberOfPointsX = (CONTROL_PONTS_PER_EDGE - 1) * numberOfXPatches + 1;   // 3n + 1
numberOfPointsY = (CONTROL_PONTS_PER_EDGE - 1) * numberOfYPatches + 1;
```

Sharing is what makes the surface C0-continuous across patch boundaries.

## Two grid topologies

### Rectangular — `GenerateRectangularGridOfPoints`

A flat grid in the XY plane, evenly spaced:

```cpp
const float sizeXPerPoint = params.sizeX / (numberOfPointsX - 1);
const float sizeYPerPoint = params.sizeY / (numberOfPointsY - 1);

Algebra::Vector4 offset(i * sizeXPerPoint, j * sizeYPerPoint, 0.f);
```

### Cylindrical — `GenerateCylindricalGridOfPoints`

Points are placed on a circle in XY and extruded along Z:

```cpp
const float heightPerPoint = params.sizeY / (params.numberOfYPatches * 3);
const float anglePerPoint  = 2π / (numberOfPointsX - 1);

Algebra::Vector4 heightOffset(0.f, 0.f, j * heightPerPoint);
Algebra::Vector4 radiusOffset = Algebra::Matrix4::RotationZ(anglePerPoint * i) *
                                Algebra::Vector4(params.sizeX, 0.f, 0.f);
```

Here `sizeX` means **radius**, not width, and `sizeY` is the total height.

The seam is closed by aliasing rather than by creating duplicate points — the last column
holds the *same entity handles* as the first:

```cpp
for (unsigned int i = 0; i < numberOfPointsY; i++)
    result[numberOfPointsX - 1][i] = result[0][i];
```

Moving a seam point therefore moves both sides of the seam automatically, keeping the
cylinder closed.

## Assembling patches

```cpp
void FillBezierComponent(...)
{
    result.bezierPatches = std::vector<std::vector<Entity>>(
        numberOfXPatches, std::vector<Entity>(numberOfYPatches));

    for (i in patches X)
      for (j in patches Y)
      {
          auto patch = CreateVirtualPatch(scene, surface);
          result.bezierPatches[i][j] = patch;

          auto pointsForPatch = CreateLinearVectorFrom2D(
              i * (CONTROL_PONTS_PER_EDGE - 1),      // ← stride of 3, not 4: shared edges
              j * (CONTROL_PONTS_PER_EDGE - 1), points);

          AssignPointsToPatch(patch, patch.GetComponent<BezierPatchGenerationComponent>(),
                              pointsForPatch);
      }
}
```

The stride of 3 is the whole trick: patch *(i, j)* starts at grid position *(3i, 3j)* and
takes a 4×4 block, so its last row/column is the next patch's first.

`AssignPointsToPatch` also registers the dependency in each point:

```cpp
if (point.HasComponent<NotificationComponent>())
    point.GetComponent<NotificationComponent>().entitiesToNotify.push_back(patch);
patchPoints[x][y] = point;
```

So moving a point dirties its patch, which (being virtual) dirties the surface — see
[ecs/change-propagation.md](../ecs/change-propagation.md).

## Real vs virtual control points

```cpp
inline Entity AddBezierSurfaceToEntity(Entity entity, Scene* scene,
                                       BezierSurfaceCreationParameters bezierParams,
                                       bool createVirtual = false);
```

- `createVirtual == false` (the default, used by `CreateBezierSurface`) — control points are
  real entities created via `Archetypes::CreatePoint`. They get a `NameComponent`, appear in
  the shape list, and are individually selectable and editable.
- `createVirtual == true` — control points are owned by the surface: no name, not listed,
  destroyed with the surface. This is the mode a future C2 surface would want, where control
  points are derived rather than authored.

## Mesh generation

`MeshGeneratingSystem::BezierSurfaceGeneration` flattens patches into one buffer:

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

ModifyOrCreateMesh(entity, vertices, indices, bezierShaderLayout,
                   RenderingMode::Patches,
                   { AvailableShaders::BezierSurfaceHorizontal,
                     AvailableShaders::BezierSurfaceVertical });
```

- 16 consecutive vertices per patch, **row-major** — the order the evaluation shader expects.
- Points shared between patches are duplicated in the buffer. That is required: each
  `GL_PATCHES` primitive must carry its own 16 vertices.
- **Two shaders on one mesh.** The buffer is drawn twice, once for isolines along *u* and once
  along *v*, producing the wireframe grid.
- The subdivision level is fixed at 32×32 by the GLSL defaults, because the
  `u_subdivisions`/`v_subdivisions` uniforms are `int` and cannot be set from
  `UniformContext` — see [renderer/shaders.md](../renderer/shaders.md).

> This loop does not check `IsValid()` on the patch or point entities before dereferencing.
> See [gotchas](../gotchas.md).

## Creating one

```cpp
Archetypes::BezierSurfaceCreationParameters params;
params.numberOfXPatches = 3;
params.numberOfYPatches = 2;
params.sizeX = 6.f;
params.sizeY = 4.f;
params.startingPosition = cursorPosition;
params.isCylinder = false;

Archetypes::CreateBezierSurface(scene, params);
```

**There is currently no UI for this** — `ShapeCreation::Display` has no surface entry, because
the parameters need a small dialog rather than a one-click menu item. Adding it is a natural
next step: a popup with two `DragInt`s, two `DragFloat`s and a checkbox, calling
`CreateBezierSurface` on confirm.

## Possible extensions

- **C2 surfaces** — a de Boor control net converted per patch, mirroring `BezierCurveC2`.
  The `createVirtual` flag exists for exactly this.
- **Adjustable tessellation** — add int support to `UniformContext`, then expose
  `u_subdivisions`/`v_subdivisions` in the inspector.
- **Patch-boundary continuity constraints** — currently patches are only C0; enforcing G1
  would mean constraining the control points either side of each shared edge.
