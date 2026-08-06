# Architecture

## Layer diagram

```
                    ┌──────────────────────────────────────────┐
   main.cpp ───────▶│ App                                       │
                    │  • Window (GLFW + GLEW + ImGui)           │
                    │  • Ref<Scene>          (entt::registry)   │
                    │  • Unique<SystemPipeline>                 │
                    └───────────────┬──────────────────────────┘
                                    │ Update() once per frame
                    ┌───────────────▼──────────────────────────┐
                    │ SystemPipeline: ordered list of ISystem   │
                    └───────────────┬──────────────────────────┘
        ┌──────────┬────────────────┼─────────────┬─────────────┐
        ▼          ▼                ▼             ▼             ▼
  Notification  Removal      GUI / Inspector   MeshGenerating  Rendering
   System       System          / Popup           System        System
        │          │                │             │             │
        │          │                │             │             ▼
        │          │                │             │        Renderer ─▶ Shader
        │          │                │             ▼                  ▶ VertexArray
        │          │                │        MeshGenerator::*   (CPU geometry)
        │          │                ▼
        │          │           Archetypes::Create*  (entity assembly)
        └──────────┴────────────────┴──────────── Scene / Entity / Components
```

Cross-cutting: `managers/` (singletons for shaders, static meshes, ids), `Algebra`
(math), `core/Base.h` (`Ref`/`Unique` aliases).

## The frame loop

[`App::Run`](../OpenglGeometry/src/App.cpp) is the only loop in the program:

```cpp
while (running && !window.ShouldClose())
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    systemPipeline->Update();          // ← all application logic happens here

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    window.ProcessFrame();             // swap buffers + poll events
}
```

Two consequences worth internalising:

1. **Everything is inside an ImGui frame.** Systems may call ImGui functions freely, and
   they also *read input through ImGui* (`ImGui::IsMouseDragging`, `ImGui::GetIO().DeltaTime`,
   `ImGui::IsKeyChordPressed`) rather than through GLFW directly. `DragCamera` is the clearest
   example.
2. **ImGui's draw data is submitted after the systems run**, so scene geometry is drawn
   first and the UI composites on top.

## The system pipeline

[`SystemPipeline`](../OpenglGeometry/src/systems/SystemPipeline.cpp) is a `std::vector<Ref<ISystem>>`
constructed once and iterated in order every frame. The order is deliberate:

| # | System | Responsibility |
| --- | --- | --- |
| 1 | `NotificationSystem` | Turns "an entity changed" into `IsDirtyTag` on everything that depends on it |
| 2 | `RemovalSystem` | Destroys entities tagged `ToBeDestroyedTag`, cascading to their virtual children |
| 3 | `GUISystem` | Draws the *Shape List* panel; selection and delete requests |
| 4 | `ShapeInspectorSystem` | Draws per-component editors for selected entities; edits set `IsDirtyTag` |
| 5 | `PopupSystem` | Drives modal popups (shape creation) |
| 6 | `MeshGeneratingSystem` | Rebuilds vertex/index buffers for every `IsDirtyTag` entity, then clears the tag |
| 7 | `RenderingSystem` | Updates the camera, collects uniforms, issues draw calls |

The GUI systems sit *before* mesh generation so that a value the user drags this frame is
already reflected in the mesh drawn this frame. Notification and removal run first so that
mesh generation never touches a destroyed entity.

See [systems/README.md](systems/README.md) for details on each.

## Data flow: how a user edit reaches the screen

Take "user drags the torus radius slider":

```
ShapeInspectorSystem::TorusInspect
    → mutates TorusGenerationComponent.radius
    → entity.AddTag<IsDirtyTag>()
                    │
                    ▼ (same frame, later in the pipeline)
MeshGeneratingSystem::TorusGeneration
    → sees <IsDirtyTag, TorusGenerationComponent>
    → RemoveTag<IsDirtyTag>()
    → MeshGenerator::Torus::GenerateMesh(...)      // pure CPU function
    → ModifyOrCreateMesh(...)                      // uploads into the existing VertexArray
                    │
                    ▼
RenderingSystem
    → for each MeshComponent (not IsInvisibleTag): bind VAO, bind shader,
      apply uniforms, glDrawElements
```

Now take "user drags a point that a Bézier curve depends on":

```
ShapeInspectorSystem::PositionInspect
    → PositionComponent.position = newValue
    → Observable::operator= detects the value actually changed
    → entity.AddTag<ObserverChangedState>()
                    │
                    ▼ (NEXT frame, because notification runs first in the pipeline)
NotificationSystem
    → point has NotificationComponent listing dependent entities
    → each dependent gets IsDirtyTag
    → the ObserverChangedState tag is cleared
                    │
                    ▼
MeshGeneratingSystem::BezierLineGeneration → new vertex buffer
```

This one-frame delay is inherent to the ordering and is invisible in practice. The full
mechanism is documented in [ecs/change-propagation.md](ecs/change-propagation.md) — it is
the least obvious part of the codebase and worth reading before touching anything that
depends on control points.

## Key design decisions

### Shapes are entities, not classes

There is no `Torus` class. A torus is an entity carrying `IdComponent`, `NameComponent`,
`TorusGenerationComponent`, `PositionComponent`, `RotationComponent`, `ScaleComponent` and
`IsDirtyTag`. The assembly is done by free functions in
[`archetypes/`](../OpenglGeometry/src/archetypes/), split into two layers:

- `AddXToEntity(entity, ...)` — attaches the components for feature X to an existing entity.
- `CreateX(scene, ...)` — creates an entity and composes several `AddXToEntity` calls.

This lets you compose features (a Bézier curve *is* a line *plus* a Bézier generation
function *plus* a virtual polyline child). See [archetypes.md](archetypes.md).

### Behaviour is dispatched from component type, not from a vtable

[`ComponentFunctionRegistry<Args...>`](ecs/component-function-registry.md) maps
`entt::type_hash<Component>` to a member function. `UniformApplier` and
`ShapeInspectorSystem` both derive from it: given an entity, they iterate the entity's
actual component types and run the registered handler for each. Adding a new inspectable or
uniform-contributing component is a one-line `Bind<T>(&Class::Handler)` in a constructor.

### Geometry generation is pure and separate from ECS

Everything under [`meshGenerators/`](../OpenglGeometry/src/meshGenerators/) takes plain
`std::vector<Algebra::Vector4>` in and returns vertices/indices out. No entities, no OpenGL.
`MeshGeneratingSystem` is the only glue between ECS and those functions. This makes the
math independently testable and readable. See [geometry/README.md](geometry/README.md).

### Curves and surfaces are evaluated on the GPU

The CPU only produces *control points* in the right order and layout. Tessellation control
and evaluation shaders do the actual Bézier evaluation, with the tessellation level chosen
from screen-space segment length and camera distance. That is why the rendering mode for
those meshes is `RenderingMode::Patches` and why the shader programs carry a `patchSize`
(4 for curves, 16 for surface patches). See [renderer/shaders.md](renderer/shaders.md).

### Uniforms are pushed from a context, driven by shader reflection

`Shader` reflects its active uniforms at link time (`glGetActiveUniform`). At draw time the
`Renderer` fills a `UniformContext` (name → value maps for `Vector4` and `Matrix4`) and calls
`Shader::ApplyContext`, which walks the reflected uniforms and sets the ones the context
provides. Adding a uniform to a shader therefore requires adding the matching entry to the
context, otherwise the shader keeps its GLSL default and a warning is printed every frame.

### Naming conventions

| Convention | Example |
| --- | --- |
| `m_` prefix for private members | `m_Scene`, `m_Registry`, `m_Systems` |
| `g_` prefix for scene-wide uniforms | `g_viewMatrix`, `g_projectionMatrix`, `g_cameraPosition` |
| `u_` prefix for per-object uniforms | `u_modelMatrix`, `u_color` |
| `I` prefix for interfaces | `ISystem`, `ICamera`, `IPopup` |
| `Ref<T>` / `Unique<T>` instead of raw `std::shared_ptr` / `unique_ptr` | `core/Base.h` |
| ImGui labels always carry `##<id>` | `GUI::GenerateLabel(entity, "Position")` |
