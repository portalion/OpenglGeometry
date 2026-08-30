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

## C0 and C2 — one component, two tilings

C0 and C2 surfaces share `BezierSurfaceGenerationComponent`; `ObjectTypeComponent`
(`BezierSurfaceC0` / `BezierSurfaceC2`) is the only thing that tells them apart. The difference
is captured by one struct:

```cpp
struct SurfaceDegree { unsigned int stride; unsigned int seam; };
inline constexpr SurfaceDegree SurfaceDegreeC0{ 3u, 1u };   // patches share 1 row/column
inline constexpr SurfaceDegree SurfaceDegreeC2{ 1u, 3u };   // de Boor net overlaps by 3
```

## Control point count

`CalculateNumberOfPointsForSurface(params, degree)`:

```cpp
overlap = CONTROL_PONTS_PER_EDGE - degree.stride;     // C0: 1   C2: 3
numberOfPointsX = degree.stride * numberOfXPatches + overlap;   // C0: 3n+1   C2: n+3
```

For C0, sharing an edge (`3n + 1`, not `4n`) is what makes the surface C0-continuous. For C2
the control points are a **de Boor net**: adjacent patches overlap by three, `n + 3` points per
axis, and the surface is C2 across every boundary.

## Two grid topologies

### Rectangular — `GenerateRectangularGridOfPoints`

A flat grid in the XY plane, evenly spaced:

```cpp
const float sizeXPerPoint = params.sizeX / (numberOfPointsX - 1);
const float sizeYPerPoint = params.sizeY / (numberOfPointsY - 1);

Algebra::Vector4 offset(i * sizeXPerPoint, j * sizeYPerPoint, 0.f);
```

### Cylindrical — `GenerateCylindricalGridOfPoints`

Points are placed on a circle in XZ (`RotationY`) and stacked along Y:

```cpp
const unsigned int distinctColumns = numberOfPointsX - degree.seam;
const float heightPerPoint = params.sizeY / (numberOfPointsY - 1);
const float anglePerPoint  = 2π / distinctColumns;

Algebra::Vector4 heightOffset(0.f, j * heightPerPoint, 0.f);
Algebra::Vector4 radiusOffset = Algebra::Matrix4::RotationY(anglePerPoint * i) *
                                Algebra::Vector4(params.sizeX, 0.f, 0.f);
```

Here `sizeX` means **radius**, not width, and `sizeY` is the total height.

The seam is closed by aliasing rather than by creating duplicate points — the last
`degree.seam` columns hold the *same entity handles* as the first `degree.seam` (one column for
C0, three for C2):

```cpp
for (unsigned int t = 0; t < degree.seam; t++)
    for (unsigned int j = 0; j < numberOfPointsY; j++)
        result[distinctColumns + t][j] = result[t][j];
```

Moving a seam point therefore moves every alias automatically, keeping the cylinder closed.
Task 07's "seam control points added to the scene only once" falls out of this.

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
              i * degree.stride,      // C0: stride 3 (shared edge)   C2: stride 1 (de Boor)
              j * degree.stride, points);

          AssignPointsToPatch(patch, patch.GetComponent<BezierPatchGenerationComponent>(),
                              pointsForPatch);
      }
}
```

The stride is the whole trick: for C0, patch *(i, j)* starts at grid *(3i, 3j)* and takes a
4×4 block, so its last row/column is the next patch's first. For C2 the stride is 1 — every
patch shares three rows/columns with its neighbour.

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
  destroyed with the surface. Currently unused by the public factories (both C0 and C2 create
  real, authored control points).

## Mesh generation

`MeshGeneratingSystem::BezierSurfaceGeneration` flattens patches into one buffer:

```cpp
const bool isC2 = GetObjectType(entity) == ObjectType::BezierSurfaceC2;

for (auto patchRow : patches)
  for (auto patch : patchRow)
  {
      PatchGrid control;                       // 4×4 positions of this patch
      for (i, j in 0..3) control[i][j] = patch...controlPoints[i][j]...position;

      if (isC2) control = DeBoorToBernstein(control);

      for (i, j in 0..3) { vertices.push_back(control[i][j]); indices.push_back(indice++); }
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
- **C2 surfaces** (`ObjectType::BezierSurfaceC2`) push the same buffer, but each patch's 4×4 de
  Boor block is first converted to a Bézier block on the CPU — `B = A·P·Aᵀ` with the uniform
  cubic B-spline matrix, via `MeshGenerator::BezierSurfaceC2::DeBoorToBernstein`
  ([`BezierSurfaceGenerator.h`](../../OpenglGeometry/src/meshGenerators/BezierSurfaceGenerator.h)).
  `A` is the same matrix `MeshGenerator::BezierCurveC2::GenerateVertices` uses in 1-D. The
  existing surface shader pair evaluates the converted patch unchanged.
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

Archetypes::CreateBezierSurface(scene, params);    // C0
Archetypes::CreateBezierSurfaceC2(scene, params);  // C2 (patches + 3 points per axis)
```

`Archetypes::CreateBezierSurfaceFromControlGrid(scene, grid, samplesU, samplesV, isC2)` rebuilds
a surface from an already-resolved `grid[iu][iv]` of point entities — used by the scene loader.
Wrapping is implicit: if the file repeats point ids, `grid` columns/rows share entities and the
seam aliases itself.

The UI entry points are the **Create** menu (menu bar and the Shift + right-click viewport
context menu), plus the **Shift + A** creation popup (`ShapeCreation::Display`). Each *Bezier
surface...* item calls `GUI::RequestDialog(BezierSurfaceDialogTitle)`; the request is drained by
`GUI::FlushDialogRequest()` at the top of `DrawAllDialogs`. `BezierSurfaceDialog.cpp` draws the
draft — a `GUI::BeginPropertyTable` with a **C0 / C2** segmented control, paired `u`/`v` fields
for patch counts, size and samples (`GUI::PropertyRowUV`), a cylinder checkbox, and a live
`ImDrawList` control-net preview — laid out after `plans/ui-mockups.html`. On confirm it calls
`CreateBezierSurface` or `CreateBezierSurfaceC2` and writes `samplesU`/`samplesV` onto the
generation component.

## Possible extensions

- **Adjustable tessellation** — add int support to `UniformContext`, then expose
  `u_subdivisions`/`v_subdivisions` in the inspector.
- **Patch-boundary continuity constraints for C0** — enforcing G1 would mean constraining the
  control points either side of each shared edge.
- **B-spline basis evaluation shader** — evaluate the C2 de Boor net directly on the GPU
  instead of the per-rebuild CPU `A·P·Aᵀ` conversion.
