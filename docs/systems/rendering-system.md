# `RenderingSystem`

**Files:** [`src/systems/RenderingSystem.h`](../../OpenglGeometry/src/systems/RenderingSystem.h),
[`.cpp`](../../OpenglGeometry/src/systems/RenderingSystem.cpp)
**Position in pipeline:** 7th (last)

Updates the active camera, gathers uniforms, and issues one draw call per
(mesh × shader) pair.

```cpp
class RenderingSystem : public ISystem
{
    Ref<Scene>     m_Scene;
    Ref<Renderer>  m_Renderer;
    UniformApplier m_UniformApplier;
public:
    RenderingSystem(Ref<Scene>);
    void Process();
};
```

## `Process()` — camera phase

```cpp
SceneContext sceneContext;

for (Entity entity : m_Scene->GetAllEntitiesWith<CameraComponent>())
{
    auto& cameraComponent = entity.GetComponent<CameraComponent>();
    if (!cameraComponent.active) continue;

    cameraComponent.cameraHandling->HandleInput(cameraComponent);   // writes viewMatrix

    EntityContext cameraUniforms;
    m_UniformApplier.PerformFunctions(entity, cameraUniforms);

    sceneContext.CameraPosition   = cameraUniforms.Position[3];
    sceneContext.ProjectionMatrix = cameraComponent.projectionMatrix;
    sceneContext.ViewMatrix       = cameraComponent.viewMatrix;
}

m_Renderer->SetSceneContext(sceneContext);
```

Notes:

- **Camera input is processed here**, not in a dedicated input system. `ICamera::HandleInput`
  reads the ImGui IO state and writes `cameraComponent.viewMatrix`. See
  [core.md](../core.md#icamera--dragcamera).
- `cameraUniforms.Position[3]` is row 3 of the camera entity's translation matrix, i.e. its
  world position — but only if the camera entity has a `PositionComponent`. The camera in
  `BaseScene` does not, so this is the identity matrix's last row, `(0,0,0,1)`.
  `g_cameraPosition` is consequently constant, which matters because the infinite-grid and
  Bézier-curve shaders use it for fading and tessellation density. See
  [gotchas](../gotchas.md).
- If several cameras are `active`, the last one visited wins. There is no ordering guarantee.

## `Process()` — draw phase

```cpp
for (Entity entity : m_Scene->GetAllEntitiesWith<MeshComponent>(Excluded<IsInvisibleTag>()))
{
    auto& meshComponent = entity.GetComponent<MeshComponent>();

    EntityContext context;
    m_UniformApplier.PerformFunctions(entity, context);   // fills Position/Rotation/Scale

    m_Renderer->SetMesh(meshComponent.mesh);
    for (auto shaderType : meshComponent.shaderTypes)
    {
        m_Renderer->SetShader(shaderType);
        m_Renderer->Render(meshComponent.renderingMode, context);
    }
}
```

- Only `MeshComponent` is required. Transform components are optional; the
  [`UniformApplier`](../ecs/component-function-registry.md) contributes identity for
  whatever is missing.
- `IsInvisibleTag` is excluded at the view level, so hidden helper geometry costs nothing.
- The inner loop over `shaderTypes` is what lets a Bézier surface be drawn twice from one
  buffer (horizontal + vertical isolines).
- There is no sorting, no batching and no depth pre-pass — entities are drawn in registry
  order.

## What the `Renderer` does with this

See [renderer/README.md](../renderer/README.md) for the full picture. Briefly:

```cpp
void Renderer::Render(RenderingMode mode, const EntityContext& context)
{
    UniformContext uniformContext = sceneContext;                      // g_* uniforms
    uniformContext.Matrix4Uniforms["u_modelMatrix"] =
        context.Position * context.Rotation * context.Scale;
    uniformContext.Vector4Uniforms["u_color"] = Algebra::Vector4(1.f, 0.2f, 0.f, 1.f);

    m_ActualShader->ApplyContext(uniformContext);
    m_ActualMesh->Bind();
    glDrawElements(static_cast<GLenum>(mode),
                   m_ActualMesh->GetIndexBuffer()->GetCount(),
                   GL_UNSIGNED_INT, nullptr);
}
```

The model matrix is composed **translation × rotation × scale** in that order, so an object
is scaled first, then rotated, then translated — the usual convention.

`u_color` is hard-coded to orange for every entity. There is no colour component yet; adding
one is a natural extension (see [how-to/add-a-new-component.md](../how-to/add-a-new-component.md)).

## Notable omissions

These are not bugs so much as things that simply have not been built yet, and the most
likely places to extend:

- **No depth test enabled.** `Window::SetupGLFWFunctions` enables blending only; `glClear`
  clears the depth buffer but `GL_DEPTH_TEST` is never enabled. Geometry is drawn in
  registry order, which is fine for wireframes but will look wrong for solid surfaces.
- **No frustum culling or sorting.**
- **No per-entity colour, line width, or material.**
- **No render targets / picking pass.** `App::GetClickedPoint` and `App::ScreenToNDC` exist
  as stubs from the previous architecture and are not wired to anything.
