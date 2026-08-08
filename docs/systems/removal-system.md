# `RemovalSystem`

**Files:** [`src/systems/RemovalSystem.h`](../../OpenglGeometry/src/systems/RemovalSystem.h),
[`.cpp`](../../OpenglGeometry/src/systems/RemovalSystem.cpp)
**Position in pipeline:** 2nd

The single place where entities are actually destroyed.

```cpp
void RemovalSystem::Process()
{
    for (auto entity : m_Scene->GetAllEntitiesWith<ToBeDestroyedTag>())
    {
        if (entity.HasComponent<IsParentOfVirtualEntitiesComponent>())
        {
            auto virtualEntities = entity.GetComponent<IsParentOfVirtualEntitiesComponent>()
                                         .virtualEntities;
            for (auto child : virtualEntities)
                if (child.IsValid())
                    child.AddTag<ToBeDestroyedTag>();
        }
        m_Scene->DestroyEntity(entity);
    }
}
```

## Deferred destruction

Nothing calls `Scene::DestroyEntity` directly. Callers tag the entity instead:

```cpp
entity.AddTag<ToBeDestroyedTag>();
```

Set from three places: the *Remove All Selected* button in the shape list,
`NotificationSystem::NotifyWithVirtualComponent` (orphan cleanup), and this system's own
cascade.

The point of deferral is that destruction invalidates entity handles held all over the
scene — in `LineGenerationComponent::controlPoints`, `NotificationComponent::entitiesToNotify`,
`BezierPatchGenerationComponent::controlPoints`. Doing it at one known point in the frame,
before any system reads those lists for real work, keeps the invalidation window small and
predictable. The lists themselves are pruned lazily by their consumers via `IsValid()`.

## Cascading to virtual children

Deleting a Bézier curve must also delete its control polyline; deleting a surface must
delete its patches. That relationship is recorded in
`IsParentOfVirtualEntitiesComponent::virtualEntities` (set up by
`Archetypes::AddVirtualToEntity`), and the cascade tags each child rather than destroying it
directly.

Because children are *tagged*, not destroyed, a child that is itself a parent of virtual
entities gets its own cascade — but only when the loop reaches it. Whether that happens this
frame or the next depends on view iteration order; either way the whole tree is gone within
a frame or two, and the intermediate states are handled by `IsValid()` checks everywhere
else.

Note the copy in `auto virtualEntities = ...` — the vector is copied before iterating,
because the loop body tags entities and would otherwise be walking a container that another
iteration step could touch.

## Resource cleanup

Destroying an entity destroys its components, which is where GPU and id resources are
released:

- `MeshComponent` holds `Ref<VertexArray>`. When the last reference goes, `~VertexArray`
  calls `glDeleteVertexArrays`, and the owned `VertexBuffer`/`IndexBuffer` destructors call
  `glDeleteBuffers`.
  **Shared static meshes** (`StaticMeshManager`) are *not* freed, because the manager holds
  a reference for the process lifetime — that is the point of the manager.
- `IdComponent`'s destructor returns its id to `IdManager`'s free queue.

## Gotcha: what is not cleaned up

Destroying a point does **not** remove it from the `controlPoints` lists of curves that used
it, or from any `entitiesToNotify` lists. Those entries become invalid handles and are
pruned opportunistically the next time the list is walked
(`CopyValidPointsToVector`, `NotifyWithNotificationComponent`). This is by design, but it
means: **any new code that stores `Entity` must check `IsValid()` before dereferencing.**

One place this is not done today is `MeshGeneratingSystem::BezierSurfaceGeneration`, which
reads patch control points without validity checks — see [gotchas](../gotchas.md).
