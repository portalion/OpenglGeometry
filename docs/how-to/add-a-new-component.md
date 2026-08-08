# How to add a new component

Components go in one file:
[`src/scene/Components.h`](../../OpenglGeometry/src/scene/Components.h). No `.cpp`, nothing
to register, nothing to add to the build.

## The template

```cpp
struct MyComponent
{
    float someValue = 1.0f;
    std::vector<Entity> relatedEntities;

    MyComponent() = default;
    MyComponent(const MyComponent& other) = default;
};
```

The two defaulted constructors are the house style — EnTT needs the component to be
constructible and (for some storage operations) copyable. Give members in-class default
values rather than writing a constructor body.

Use it:

```cpp
auto& c = entity.AddComponent<MyComponent>();      // asserts if it already exists
c.someValue = 2.0f;

if (entity.HasComponent<MyComponent>())
    entity.GetComponent<MyComponent>().someValue = 3.0f;

entity.RemoveComponent<MyComponent>();
```

Then view on it:

```cpp
for (Entity e : m_Scene->GetAllEntitiesWith<MyComponent>())            { ... }
for (Entity e : m_Scene->GetAllEntitiesWith<MyComponent, IsDirtyTag>()) { ... }
for (Entity e : m_Scene->GetAllEntitiesWith<MyComponent>(Excluded<IsInvisibleTag>())) { ... }
```

---

## Tags

An empty struct is a tag. Put it in
[`src/scene/Tags.h`](../../OpenglGeometry/src/scene/Tags.h) instead:

```cpp
struct MyMarkerTag { };
```

and use `AddTag<T>()` / `RemoveTag<T>()`, which are idempotent — unlike `AddComponent`, which
asserts on a duplicate. See [ecs/tags.md](../ecs/tags.md).

---

## If the component holds `Entity` references

They can dangle: `RemovalSystem` destroys entities and does not clean up references held
elsewhere. Every consumer must guard:

```cpp
for (auto it = relatedEntities.begin(); it != relatedEntities.end(); )
{
    if (!it->IsValid())
    {
        it = relatedEntities.erase(it);
        continue;
    }
    // ... use *it ...
    it++;
}
```

Use `std::list` if you will prune during iteration (that is why
`LineGenerationComponent::controlPoints` and `NotificationComponent::entitiesToNotify` are
lists).

If the referenced entities should also *notify* your component when they change, register
the dependency:

```cpp
Archetypes::AddNotifiersToEntityContainer(myEntity, myComponent.relatedEntities,
                                          pointsBegin, pointsEnd);
```

See [ecs/change-propagation.md](../ecs/change-propagation.md).

---

## If the component holds an observable value

To have other entities react to a change, wrap the field:

```cpp
struct MyComponent
{
    Observable<Algebra::Vector4> someVector;
    ...
};
```

**You must also connect a construction hook**, or the first assignment dereferences a null
`Scene*`. In `Scene::Scene()`:

```cpp
m_Registry.on_construct<MyComponent>().connect<&Scene::OnMyComponentCreated>(this);
```

and in `~Scene()`:

```cpp
m_Registry.on_construct<MyComponent>().disconnect<&Scene::OnMyComponentCreated>(this);
```

with:

```cpp
void Scene::OnMyComponentCreated(entt::registry& registry, entt::entity entity)
{
    registry.get<MyComponent>(entity).someVector.entity = Entity{ entity, this };
}
```

Mirror how `PositionComponent` is handled in
[`Scene.cpp`](../../OpenglGeometry/src/scene/Scene.cpp).

Remember the read/write asymmetry: reading converts to `const T&`, so writes must be
assignments to the `Observable` itself.

```cpp
Algebra::Vector4 tmp = c.someVector;    // copy out
tmp.x += 1.f;                           // edit the copy
c.someVector = tmp;                     // assign back → notification fires
```

---

## Wiring the component into behaviour

A component does nothing on its own. Pick the consumers you need:

| Goal | Where to hook in |
| --- | --- |
| Regenerate geometry when it changes | A new pass in [`MeshGeneratingSystem`](../systems/mesh-generating-system.md) viewing `<IsDirtyTag, MyComponent>` |
| Contribute a uniform | `Bind<MyComponent>(&UniformApplier::MyApplier)` in [`UniformApplier`](../ecs/component-function-registry.md) |
| Show an editor | `Bind<MyComponent>(&ShapeInspectorSystem::MyInspect)` — [how-to](add-an-inspector-field.md) |
| Anything per-frame | A [new system](add-a-new-system.md) |
| Get attached to a shape | An [archetype](../archetypes.md) |

## Naming conventions

| Suffix | Meaning | Examples |
| --- | --- | --- |
| `…GenerationComponent` | Parameters for generated geometry; dispatched on by `MeshGeneratingSystem` | `TorusGenerationComponent`, `BezierSurfaceGenerationComponent` |
| `…Component` | Everything else | `PositionComponent`, `MeshComponent`, `NameComponent` |
| `…Tag` / `Is…Tag` | Empty marker | `IsDirtyTag`, `ToBeDestroyedTag` |
