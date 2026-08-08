# Change propagation

This is the least obvious mechanism in the codebase and the one most likely to bite you.
It answers: *when the user drags a point, how does every curve that uses it get rebuilt?*

## The pieces

| Piece | Role |
| --- | --- |
| `Observable<T>` | Detects that a value actually changed, tags its entity `ObserverChangedState` |
| `Scene::OnPositionCreated` | Gives each `Observable` a back-reference to its entity |
| `NotificationComponent` | "These entities depend on me" |
| `VirtualEntityComponent` | "I am a helper of this entity" |
| `NotificationSystem` | Converts `ObserverChangedState` into `IsDirtyTag` on dependents |
| `IsDirtyTag` | "My mesh must be regenerated" |
| `MeshGeneratingSystem` | Consumes `IsDirtyTag`, rebuilds buffers |

## `Observable<T>`

[`src/scene/Observable.h`](../../OpenglGeometry/src/scene/Observable.h)

```cpp
template <typename T>
class Observable
{
public:
    T value;
    Entity entity;

    inline T& operator=(const T& newValue)
    {
        if (value != newValue)
        {
            value = newValue;
            entity.AddTag<ObserverChangedState>();
        }
        return value;
    }

    inline operator const T& () const { return value; }
};
```

Three things to note:

1. **Only assignment is observed.** The implicit conversion returns `const T&`, so you
   cannot accidentally mutate through it — but you *can* copy the value out, mutate the
   copy, and forget to assign it back. Every write must be an assignment to the
   `Observable` itself.
2. **Equality-gated.** `T` must have `operator!=` (`Algebra::Vector4` does, via
   `operator==`). Assigning the same value is a no-op and produces no notification, so
   dragging a slider without moving it costs nothing.
3. **It needs `entity` to be valid.** A default-constructed `Entity` has a null `Scene*`,
   and `AddTag` would dereference it. Hence the hook below.

### The construction hook

```cpp
// Scene.cpp
Scene::Scene()
{
    m_Registry.on_construct<PositionComponent>()
              .connect<&Scene::OnPositionCreated>(this);
}

void Scene::OnPositionCreated(entt::registry& registry, entt::entity entity)
{
    registry.get<PositionComponent>(entity).position.entity = Entity{ entity, this };
}
```

EnTT fires `on_construct` whenever the component is emplaced, so this covers every code path
that adds a position.

> **If you add a new `Observable<T>` field to any component, you must connect an equivalent
> `on_construct` hook for that component in `Scene::Scene()` (and disconnect it in
> `~Scene()`), or the first assignment will crash.**

## `NotificationSystem`

[`src/systems/NotificationSystem.cpp`](../../OpenglGeometry/src/systems/NotificationSystem.cpp) runs
first in the pipeline and does four passes:

```cpp
void NotificationSystem::Process()
{
    // 1. changed entities notify their dependents
    for (Entity e : m_Scene->GetAllEntitiesWith<ObserverChangedState, NotificationComponent>())
        NotifyWithNotificationComponent(e);

    // 2. entities about to be destroyed also notify their dependents one last time
    for (Entity e : m_Scene->GetAllEntitiesWith<ToBeDestroyedTag, NotificationComponent>())
        NotifyWithNotificationComponent(e);

    // 3. changed virtual entities dirty their parent
    for (Entity e : m_Scene->GetAllEntitiesWith<ObserverChangedState, VirtualEntityComponent>())
        NotifyWithVirtualComponent(e);

    // 4. clear the input tag
    for (Entity e : m_Scene->GetAllEntitiesWith<ObserverChangedState>())
        e.RemoveTag<ObserverChangedState>();
}
```

`NotifyWithNotificationComponent` prunes dead entries, tags each dependent `IsDirtyTag`,
and — if the dependent is itself virtual — also tags it `ObserverChangedState` so the change
keeps travelling up the hierarchy on the next frame:

```cpp
it->AddTag<IsDirtyTag>();
if (it->HasComponent<VirtualEntityComponent>())
    it->AddTag<ObserverChangedState>();
```

`NotifyWithVirtualComponent` dirties the parent, or marks the virtual entity for destruction
if the parent is already gone:

```cpp
auto& parent = entity.GetComponent<VirtualEntityComponent>().realEntity;
if (parent.IsValid()) parent.AddTag<IsDirtyTag>();
else                  entity.AddTag<ToBeDestroyedTag>();
```

## Worked example: moving a control point of a Bézier surface

A surface's control point notifies its **patch** (a virtual entity), and the patch's change
must reach the **surface**, which is what actually owns the mesh.

```
frame N     user drags point P
            PositionComponent.position = newValue
            → P gets ObserverChangedState

frame N+1   NotificationSystem pass 1:
              P has NotificationComponent listing patch Q
              → Q gets IsDirtyTag
              → Q has VirtualEntityComponent, so Q also gets ObserverChangedState
            pass 3: (Q was tagged during pass 1 — whether it is picked up in this same
                     Process() call or the next one depends on view iteration; either way
                     it resolves within a frame or two)
              Q.realEntity == surface S → S gets IsDirtyTag
            pass 4: ObserverChangedState cleared

frame N+1/2 MeshGeneratingSystem::BezierSurfaceGeneration
              S has IsDirtyTag → re-reads every patch's 16 control points → new VBO
```

Note that the patch entity itself never gets a mesh; `IsDirtyTag` on it is harmless because
no generation loop views `<IsDirtyTag, BezierPatchGenerationComponent>`.

## Wiring dependencies

You never construct these links by hand — use the archetype helpers:

```cpp
// "each of these points should notify `parent` when it moves,
//  and `parent` should remember them in `entityContainer`"
Archetypes::AddNotifiersToEntityContainer(parent, entityContainer, pointsBegin, pointsEnd);

// "this entity is a helper of `parent`" — sets both sides of the link
Archetypes::AddVirtualToEntity(entity, parent);
```

`AddLineToEntity` calls the first for you; `AddVirtualToEntity` is called by the Bézier
archetypes for their control polylines and patches.

## Rules of thumb

- **Never regenerate geometry directly from a UI callback.** Set `IsDirtyTag` and let
  `MeshGeneratingSystem` do it. That keeps regeneration to once per frame regardless of how
  many things changed.
- **Never delete an entity directly.** Set `ToBeDestroyedTag`.
- **If a shape depends on other entities, give those entities a `NotificationComponent`**
  and register the shape in it, otherwise the shape will never update.
- **If a shape is a helper of another shape, make it virtual**, so it is destroyed with its
  parent and its edits reach the parent.
- **A change is visible at most one frame later.** Do not add a "force refresh" path to work
  around that; it is not perceptible at 60 fps.
