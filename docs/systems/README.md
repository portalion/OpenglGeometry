# Systems

A system is anything implementing [`ISystem`](../../OpenglGeometry/src/interfaces/ISystem.h):

```cpp
class ISystem
{
public:
    virtual ~ISystem() = default;
    virtual void Process() = 0;
};
```

Systems hold a `Ref<Scene>` and are constructed once. `Process()` is called once per frame.

## `SystemPipeline`

[`src/systems/SystemPipeline.cpp`](../../OpenglGeometry/src/systems/SystemPipeline.cpp)

```cpp
SystemPipeline::SystemPipeline(Ref<Scene> scene, Viewport& viewport)
{
    m_Scene = scene;
    m_Systems =
    {
        CreateRef<NotificationSystem>(scene),
        CreateRef<RemovalSystem>(scene),

        CreateRef<GUISystem>(scene, viewport),   // draws panels + shape-list/inspector, viewport picking, cursor
        CreateRef<PopupSystem>(scene),

        CreateRef<CurveBernsteinSystem>(scene),  // C2 curves: keep the virtual Bézier points in step
        CreateRef<MeshGeneratingSystem>(scene),
        CreateRef<SelectionMarkerSystem>(scene),
        CreateRef<RenderingSystem>(scene, viewport)
    };
}

void SystemPipeline::Update()
{
    for (auto& system : m_Systems)
        system->Process();
}
```

Owned by `App` as a `Unique<SystemPipeline>`, rebuilt if the scene is replaced.

## Order matters

| # | System | Reads | Writes | Doc |
| --- | --- | --- | --- | --- |
| 1 | `NotificationSystem` | `ObserverChangedState`, `ToBeDestroyedTag`, `NotificationComponent`, `VirtualEntityComponent` | `IsDirtyTag`, `ToBeDestroyedTag`; clears `ObserverChangedState` | [→](notification-system.md) |
| 2 | `RemovalSystem` | `ToBeDestroyedTag`, `IsParentOfVirtualEntitiesComponent` | destroys entities | [→](removal-system.md) |
| 3 | `GUISystem` | `NameComponent`, `CameraComponent`, `ObjectTypeComponent`/`PositionComponent` (viewport picking) | `IsSelectedTag`, `ToBeDestroyedTag` | [→](gui-systems.md) |
| 4 | `ShapeInspectorSystem` | `IsSelectedTag` + every bound component | component values, `IsDirtyTag`, `IsInvisibleTag` | [→](gui-systems.md) |
| 5 | `PopupSystem` | keyboard, `IsSelectedTag` | creates entities | [→](gui-systems.md) |
| — | `CurveBernsteinSystem` | `CurveHelpersComponent` + `LineGenerationComponent` (C2 curves) | virtual Bézier-point entities + positions (past the `Observable`); converts a dragged Bézier point back into a de Boor move; `IsInvisibleTag`, `IsDirtyTag` | [→](../geometry/bezier-curves.md#bézier-points-of-a-c2-curve) |
| 6 | `MeshGeneratingSystem` | `IsDirtyTag` + generation components | `MeshComponent`, GPU buffers; clears `IsDirtyTag` | [→](mesh-generating-system.md) |
| 7 | `SelectionMarkerSystem` | `IsSelectedTag` + `PositionComponent` | the selection-centre marker's `PositionComponent` / `IsInvisibleTag` | [→](selection-marker-system.md) |
| 8 | `RenderingSystem` | `CameraComponent`, `MeshComponent` | GL draw calls | [→](rendering-system.md) |

The rationale, in one line each:

- **Notification before removal** — a point being deleted still gets to dirty its dependents,
  so curves rebuild without it instead of holding a dangling reference into a draw call.
- **Removal before the GUI** — panels never list an entity that is already logically gone.
- **GUI before mesh generation** — an edit made this frame is visible this frame.
- **Mesh generation before rendering** — obviously; and it means `RenderingSystem` never
  sees a half-built `VertexArray`.

Inserting a system in the wrong place produces a one-frame lag rather than a crash, which
makes ordering bugs subtle. When in doubt, put pure-logic systems near the top and anything
that touches the GPU near the bottom.

## Conventions

- Constructor takes `Ref<Scene>`; store it as `m_Scene`.
- `Process()` is the only public method. Everything else is private helpers.
- Never destroy entities or add view-filtered components while iterating a view of that
  same filter — defer via a tag.
- ImGui calls are allowed anywhere (the whole pipeline runs inside an ImGui frame), but keep
  them in the `systems/gui/` systems by convention.

To add one, see [how-to/add-a-new-system.md](../how-to/add-a-new-system.md).
