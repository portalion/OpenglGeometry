# Glossary

Terms as they are used *in this codebase*, which sometimes differs from their general
meaning.

| Term | Meaning here |
| --- | --- |
| **Archetype** | A free function that assembles components onto an entity to make it a particular shape. Not EnTT's internal "archetype" concept. `Archetypes::CreateTorus`, `Archetypes::AddPointToEntity`. See [archetypes.md](archetypes.md) |
| **Component** | A plain struct of data attached to an entity. All of them live in [`Components.h`](../OpenglGeometry/src/scene/Components.h) |
| **Control point** | A point entity that a curve or surface is built from. Identified in code by having a `NotificationComponent` |
| **Dirty** | Carrying `IsDirtyTag` — "my mesh must be regenerated this frame" |
| **Entity** | A 16-byte handle (`entt::entity` + `Scene*`), not an object. Copy freely, but check `IsValid()` before using a stored one |
| **Generation component** | A component whose presence tells `MeshGeneratingSystem` what kind of geometry to build: `TorusGenerationComponent`, `LineGenerationComponent`, `BezierSurfaceGenerationComponent`, … |
| **Isoline** | A curve of constant *u* or constant *v* on a surface. The Bézier surface shaders emit isolines rather than triangles, which is why surfaces look like wireframe grids |
| **Layout** | A `BufferLayout` — the description of one vertex's memory, used to configure `glVertexAttribPointer` |
| **Manager** | A singleton owning process-lifetime resources: `ShaderManager`, `StaticMeshManager`, `IdManager`. See [managers.md](managers.md) |
| **Mesh generator** | A pure function under `meshGenerators/` producing vertices/indices from parameters. No ECS, no GL |
| **Notification** | The dependency mechanism: a point's `NotificationComponent` lists everything that must be dirtied when it moves. See [ecs/change-propagation.md](ecs/change-propagation.md) |
| **Observable** | `Observable<T>` — a value wrapper that tags its entity `ObserverChangedState` when assigned a different value. Currently used only by `PositionComponent` |
| **Patch** *(Bézier)* | A 4×4 grid of control points defining one bicubic surface piece. Represented by a virtual entity with `BezierPatchGenerationComponent` |
| **Patch** *(OpenGL)* | A `GL_PATCHES` primitive — a group of `patchSize` vertices handed to the tessellation shader. 4 for curves, 16 for surface patches |
| **Patch size** | `glPatchParameteri(GL_PATCH_VERTICES, n)`, set per shader program via `ShaderBuilder::ChangePatchSize` and applied on `Shader::Bind` |
| **Pipeline** | `SystemPipeline` — the ordered list of systems run once per frame. Nothing to do with the GPU pipeline |
| **Ref / Unique** | Project aliases for `std::shared_ptr` / `std::unique_ptr`, with `CreateRef` / `CreateUnique` factories ([`core/Base.h`](../OpenglGeometry/src/core/Base.h)) |
| **Registry** | The `entt::registry` inside `Scene`. The actual store of all entities and components |
| **Scene context** | `SceneContext` — the per-frame camera uniforms (view, projection, camera position). Becomes the `g_*` uniforms |
| **Entity context** | `EntityContext` — the per-entity transform matrices filled by `UniformApplier`. Becomes `u_modelMatrix` |
| **Uniform context** | `UniformContext` — the stringly-typed name→value maps actually pushed to a shader, matched against its reflected uniform list |
| **Shape** | An entity with a `NameComponent`, i.e. one the user can see and select. Cameras and the cursor count; the grid and Bézier patches do not |
| **System** | A class implementing `ISystem::Process()`, run once per frame by the pipeline. See [systems/README.md](systems/README.md) |
| **Tag** | An empty component used as a marker: `IsDirtyTag`, `IsSelectedTag`, `IsInvisibleTag`, `ToBeDestroyedTag`, `ObserverChangedState`. See [ecs/tags.md](ecs/tags.md) |
| **Virtual entity** | A helper entity owned by another entity — a Bézier curve's control polyline, a surface's patches. Carries `VirtualEntityComponent`; the owner carries `IsParentOfVirtualEntitiesComponent`. Destroyed with its owner, and its changes dirty its owner |
| **View** | A filtered iteration over the registry: `GetAllEntitiesWith<A, B>(Excluded<C>())` |

## Curve terminology

| Term | Meaning |
| --- | --- |
| **C0** | Position-continuous at segment joints; tangents may break. `MeshGenerator::BezierCurveC0` chains cubic Béziers sharing endpoints |
| **C2** | Second-derivative continuous. Implemented as a uniform cubic **B-spline** converted to Bézier form. The user's points are de Boor points, and the curve does *not* pass through them |
| **Interpolated** | A natural cubic spline that **does** pass through every control point, with chord-length parameterisation and zero curvature at the ends |
| **de Boor point** | A B-spline control point (the input for C2 curves), as opposed to a Bézier control point (the output fed to the GPU) |
| **Degree elevation** | Expressing a lower-degree Bézier as a higher-degree one. `FixVertices` uses it to turn a leftover quadratic into a cubic |
| **Chord length** | `|P[i+1] - P[i]|`, used as the parameter interval per spline piece in the interpolated curve |
| **Bernstein basis** | The cubic Bézier weights `(1-t)³, 3t(1-t)², 3t²(1-t), t³`, evaluated in the `.tese` shaders |

## Shader uniform prefixes

| Prefix | Scope | Set by |
| --- | --- | --- |
| `g_` | Scene-wide, constant for the frame | `Renderer::SetSceneContext` |
| `u_` | Per-object | `Renderer::Render` |
| `a_` | Vertex attribute name (in `BufferLayout` only — never reaches GL) | — |
