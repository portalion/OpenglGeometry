# `NotificationSystem`

**Files:** [`src/systems/NotificationSystem.h`](../../OpenglGeometry/src/systems/NotificationSystem.h),
[`.cpp`](../../OpenglGeometry/src/systems/NotificationSystem.cpp)
**Position in pipeline:** 1st (first thing that happens each frame)

Converts "something changed" (`ObserverChangedState`) into "these meshes are stale"
(`IsDirtyTag`), walking the dependency graph built by the archetypes.

The concepts behind it are covered in [ecs/change-propagation.md](../ecs/change-propagation.md);
this page documents the system itself.

## `Process()`

```cpp
void NotificationSystem::Process()
{
    for (Entity e : m_Scene->GetAllEntitiesWith<ObserverChangedState, NotificationComponent>())
        NotifyWithNotificationComponent(e);

    for (Entity e : m_Scene->GetAllEntitiesWith<ToBeDestroyedTag, NotificationComponent>())
        NotifyWithNotificationComponent(e);

    for (Entity e : m_Scene->GetAllEntitiesWith<ObserverChangedState, VirtualEntityComponent>())
        NotifyWithVirtualComponent(e);

    for (Entity e : m_Scene->GetAllEntitiesWith<ObserverChangedState>())
        e.RemoveTag<ObserverChangedState>();
}
```

### Pass 1 — changed entities notify dependents

An entity with a `NotificationComponent` holds a list of the entities that consume it
(typically: a point, listing the curves and patches built from it). Every one of them is
tagged dirty.

### Pass 2 — dying entities notify dependents

Same call, but driven by `ToBeDestroyedTag`. This runs *before* `RemovalSystem`, so the
curve that used the point gets a chance to regenerate. It will regenerate on the next frame,
by which time the point is gone — and `CopyValidPointsToVector` prunes it via `IsValid()`.

### Pass 3 — virtual entities dirty their parent

```cpp
void NotificationSystem::NotifyWithVirtualComponent(Entity entity)
{
    auto& parent = entity.GetComponent<VirtualEntityComponent>().realEntity;
    if (parent.IsValid())
        parent.AddTag<IsDirtyTag>();
    else
        entity.AddTag<ToBeDestroyedTag>();
}
```

The `else` branch is orphan cleanup: a helper whose owner disappeared deletes itself. In the
normal case `RemovalSystem` already cascades parent → children, so this is a safety net for
paths that destroy a parent without going through the cascade.

### Pass 4 — clear the input tag

`ObserverChangedState` is a one-frame signal and is cleared unconditionally at the end,
including for entities that had no notification or virtual component.

## `NotifyWithNotificationComponent`

```cpp
void NotificationSystem::NotifyWithNotificationComponent(Entity entity)
{
    auto& notificationList = entity.GetComponent<NotificationComponent>().entitiesToNotify;
    for (auto it = notificationList.begin(); it != notificationList.end(); )
    {
        if (!it->IsValid())
        {
            it = notificationList.erase(it);
            continue;
        }

        it->AddTag<IsDirtyTag>();
        if (it->HasComponent<VirtualEntityComponent>())
            it->AddTag<ObserverChangedState>();

        it++;
    }
}
```

Two behaviours worth calling out:

**Self-pruning.** Dead dependents are erased from the list as they are found. This is why
`NotificationComponent::entitiesToNotify` is a `std::list` — erase-during-iteration is O(1)
and does not invalidate other iterators.

**Re-tagging virtual dependents.** If the dependent is itself a virtual entity (a Bézier
patch, say), it is given `ObserverChangedState` as well, so pass 3 will forward the change
to *its* parent. This is what makes multi-level propagation work:

```
point ──NotificationComponent──▶ patch ──VirtualEntityComponent──▶ surface
```

## When you need to touch this system

Almost never. The mechanism is generic: to make a new shape react to changes, wire the
dependency in its archetype (`AddNotifiersToEntityContainer` / `AddVirtualToEntity`) and
this system handles the rest.

You would extend it only to introduce a genuinely new *kind* of relationship — for example,
a "shape depends on another shape's generated mesh, not on its control points".
