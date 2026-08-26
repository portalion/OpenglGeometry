# Tags

Tags are empty structs in [`src/scene/Tags.h`](../../OpenglGeometry/src/scene/Tags.h). EnTT
stores them without per-entity payload, so they are cheap to add, remove and filter on.

Use `entity.AddTag<T>()` / `entity.RemoveTag<T>()` rather than the component equivalents:
`AddTag` checks `HasComponent` first and is therefore **idempotent**, whereas
`AddComponent<T>()` on an entity that already has `T` trips an EnTT assertion.

```cpp
struct IsDirtyTag { };
struct IsSelectedTag { };
struct ObserverChangedState { };
struct IsInvisibleTag { };
struct ToBeDestroyedTag { };
```

---

## `IsDirtyTag` — "my mesh is stale"

**Set by:** anything that invalidates generated geometry.

- `ShapeInspectorSystem::TorusInspect` when a torus parameter is dragged
- `NotificationSystem` on every entity listed in a changed point's `NotificationComponent`
- `NotificationSystem::NotifyWithVirtualComponent` on a virtual entity's parent
- All the `Archetypes::Add*ToEntity` functions, so a freshly created shape gets a mesh on
  its first frame

**Cleared by:** `MeshGeneratingSystem`, as the *first* statement of each generation loop
body — before doing the work, so an exception or early return still clears it.

**Lifetime:** at most one frame.

---

## `ObserverChangedState` — "one of my observable values changed"

**Set by:** `Observable<T>::operator=` when the assigned value differs from the current one,
and by `NotificationSystem` when propagating through virtual entities.

**Read by:** `NotificationSystem`, which uses it to trigger `NotificationComponent` and
`VirtualEntityComponent` propagation.

**Cleared by:** `NotificationSystem`, at the end of `Process()`, for every entity carrying it.

**Lifetime:** one frame. This is the *input* to the notification step, whereas `IsDirtyTag`
is its *output*. See [change-propagation.md](change-propagation.md).

---

## `IsSelectedTag` — "the user selected me"

**Set / cleared by:**

- `GUI::ShapeList` — clicking a row toggles it; *Select All* / *Deselect All* apply it in bulk
- `GUI::HandleViewportPicking` — left-click / box-drag in the 3D viewport selects points
  (`src/ui/ViewportPicking.cpp`, driven by `GUISystem`)

**Read by:**

- `ShapeInspectorRegistry::Display` — views `<IsSelectedTag>` to decide what to inspect
- `ShapeCreation::GetSelectedPoints` — views `<IsSelectedTag, NotificationComponent>` to
  collect control points for a new curve
- `RenderingSystem` — tints selected entities with `Globals::selectionColor`
- *Remove All Selected* in the shape list, which converts selection into `ToBeDestroyedTag`

**Lifetime:** persists until the user changes the selection.

> Viewport picking only ever tags entities whose `ObjectTypeComponent` is `Point`, and those
> always carry `NameComponent` / `IdComponent` / `NotificationComponent` (via
> `Archetypes::AddShapeToEntity` + `AddPointToEntity`), so the inspector and curve-creation
> consumers stay safe. `ShapeInspectorRegistry` also guards `NameComponent` access anyway.

---

## `IsInvisibleTag` — "don't draw me"

**Set / cleared by:** `ShapeInspectorSystem::VirtualInspect`'s checkbox, which toggles the
visibility of a shape's helper geometry (e.g. a Bézier curve's control polyline).

**Read by:** `RenderingSystem`, as a view exclusion:

```cpp
m_Scene->GetAllEntitiesWith<MeshComponent>(Excluded<IsInvisibleTag>())
```

The entity still exists and its mesh is still regenerated — only the draw call is skipped.

---

## `ToBeDestroyedTag` — "delete me at the start of next frame"

**Set by:**

- *Remove All Selected* in the shape list
- `NotificationSystem::NotifyWithVirtualComponent` when a virtual entity's parent is no
  longer valid (orphan cleanup)
- `RemovalSystem` itself, cascading from a parent to its `IsParentOfVirtualEntitiesComponent`
  children

**Read by:** `RemovalSystem`, which destroys the entity.

Deletion is deferred rather than immediate so that no system is destroying entities in the
middle of another system's view iteration. Note that `NotificationSystem` also runs
`NotifyWithNotificationComponent` for `<ToBeDestroyedTag, NotificationComponent>` entities —
so a point being deleted dirties its dependents one last time, giving curves a chance to
regenerate without it.

---

## Ordering summary

```
frame N       ShapeInspector / Observable      →  ObserverChangedState, IsDirtyTag
frame N+1     NotificationSystem               →  reads ObserverChangedState,
                                                   writes IsDirtyTag, clears ObserverChangedState
              RemovalSystem                    →  consumes ToBeDestroyedTag
              MeshGeneratingSystem             →  consumes IsDirtyTag
              RenderingSystem                  →  respects IsInvisibleTag
```
