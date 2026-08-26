# `SelectionMarkerSystem`

**Files:** [`src/systems/SelectionMarkerSystem.h`](../../OpenglGeometry/src/systems/SelectionMarkerSystem.h),
[`.cpp`](../../OpenglGeometry/src/systems/SelectionMarkerSystem.cpp)
**Position in pipeline:** 7th (after `MeshGeneratingSystem`, before `RenderingSystem`)

Owns and drives the **selection-centre marker** — a big yellow screen-facing crosshair that
sits on the median of the current selection.

```cpp
SelectionMarkerSystem::SelectionMarkerSystem(Ref<Scene> scene)
    : m_Scene{ scene }
{
    Archetypes::CreateSelectionCentreMarker(m_Scene.get());   // SelectionCentreTag, hidden
}
```

`CreateSelectionCentreMarker` ([`CursorArchetypeCreation.h`](../../OpenglGeometry/src/archetypes/CursorArchetypeCreation.h))
builds an entity with `SelectionCentreTag`, a `PositionComponent`, a
`ColorComponent = Globals::selectionColor`, `IsInvisibleTag`, and a `MeshComponent` using
`StaticMeshType::Crosshair` + `AvailableShaders::Point` (`RenderingMode::Lines`). Like the 3D
cursor it has no `IdComponent`, so it never shows in the *Shape List*.

`Process()` each frame:

1. Finds the marker via `<SelectionCentreTag, PositionComponent>`.
2. Averages the positions of every `<IsSelectedTag, PositionComponent>` entity.
3. If the count is zero → `AddTag<IsInvisibleTag>()` and stop.
4. Otherwise move the marker's `PositionComponent` to the average and clear `IsInvisibleTag`.

It runs after the GUI systems (so this frame's picking / shape-list selection is already
applied) and before `RenderingSystem` (so the move and the visibility flip are drawn the same
frame). The `Point` shader billboards the crosshair mesh on the view plane and holds it at a
constant screen size — see [renderer/shader-reference.md](../renderer/shader-reference.md#point).

The same `Globals::selectionColor` is what `RenderingSystem` tints `IsSelectedTag` entities
with, so the marker and the highlighted objects match.
