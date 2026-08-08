# The ECS core

The scene is an [EnTT](https://github.com/skypjack/entt) registry. Entities are ids,
components are plain structs, and behaviour lives in [systems](../systems/README.md).

Files: [`src/scene/`](../../OpenglGeometry/src/scene/)

| File | Purpose |
| --- | --- |
| `Scene.h` / `Scene.cpp` | Owns the `entt::registry`; entity creation/destruction; typed views |
| `Entity.h` / `Entity.cpp` | Lightweight handle (entity id + `Scene*`) with the component API |
| `Components.h` | Every component struct — see [components.md](components.md) |
| `Tags.h` | Empty marker structs — see [tags.md](tags.md) |
| `Observable.h` | Value wrapper that tags its owning entity when it changes |
| `ComponentFunctionRegistry.h` | Type-erased dispatch from component type to a handler |
| `BaseScene.h` / `BaseScene.cpp` | The default starting scene (camera + grid) |

---

## `Scene`

```cpp
class Scene
{
private:
    entt::registry m_Registry;
    void OnPositionCreated(entt::registry&, entt::entity);
public:
    Scene();
    ~Scene();

    Scene(const Scene&) = delete;             // non-copyable: it owns GPU-backed state
    Scene& operator=(const Scene&) = delete;

    Entity CreateEntity();
    void DestroyEntity(Entity entity);

    template<typename... Components> auto GetAllEntitiesWith();
    template<typename... Components, typename... Exclude>
    auto GetAllEntitiesWith(Excluded<Exclude...> exclude);

    friend Entity;
};
```

### Views

`GetAllEntitiesWith<...>()` wraps `entt::registry::view` in a
`std::views::transform` so you iterate `Entity` handles rather than raw `entt::entity`:

```cpp
for (Entity entity : m_Scene->GetAllEntitiesWith<IsDirtyTag, TorusGenerationComponent>())
{
    auto& tgc = entity.GetComponent<TorusGenerationComponent>();
    ...
}
```

Exclusions use the `Excluded<...>` alias (`entt::exclude_t`):

```cpp
// every mesh that is not hidden
m_Scene->GetAllEntitiesWith<MeshComponent>(Excluded<IsInvisibleTag>())

// polylines, but not the ones that are actually Bézier curves
m_Scene->GetAllEntitiesWith<IsDirtyTag, LineGenerationComponent>(Excluded<BezierLineGenerationComponent>())
```

Because the result is a lazy range, `.empty()` works (`GUI::DisplayShapeList`,
`ShapeInspectorSystem::Process` both use it) and you can materialise it with
`std::vector<Entity>(view.begin(), view.end())` when you need to mutate the registry while
iterating — `ShapeCreation::GetSelectedPoints` does exactly this.

> **Mutating the registry while iterating a view is unsafe.** Adding a component that the
> view filters on, or destroying entities, can invalidate iteration. The codebase deals with
> this by deferring: systems add `IsDirtyTag` / `ToBeDestroyedTag` and a *later* system acts
> on it. Follow that pattern rather than destroying entities inline.

### The `PositionComponent` construction hook

```cpp
Scene::Scene()
{
    m_Registry.on_construct<PositionComponent>()
              .connect<&Scene::OnPositionCreated>(this);
}

void Scene::OnPositionCreated(entt::registry& registry, entt::entity entity)
{
    auto& position = registry.get<PositionComponent>(entity);
    position.position.entity = Entity{ entity, this };
}
```

This back-fills the `Observable`'s `entity` field so that assigning to a position can tag
its own entity. Without the hook, `Observable::operator=` would call `AddTag` on a default
constructed `Entity` with a null `Scene*`. If you add another `Observable<T>` member to a
component, you need an equivalent hook — see
[change-propagation.md](change-propagation.md).

---

## `Entity`

A 16-byte value type: `entt::entity m_EntityHandle` + `Scene* m_Scene`. Copy it freely;
it is stored by value inside components (`LineGenerationComponent::controlPoints`,
`BezierPatchGenerationComponent::controlPoints`, …).

| Method | Notes |
| --- | --- |
| `AddComponent<T>(args...)` | `emplace` — **asserts if the component already exists** |
| `AddOrReplaceComponent<T>(args...)` | `emplace_or_replace`, safe to call repeatedly |
| `GetComponent<T>()` | Returns a reference; UB if absent, so guard with `HasComponent` |
| `HasComponent<T>()` | `registry.all_of<T>` |
| `RemoveComponent<T>()` | No-op if absent |
| `AddTag<T>()` | Like `AddComponent<T>()` but checks `HasComponent` first, so it is idempotent |
| `RemoveTag<T>()` | Alias for `RemoveComponent<T>()` |
| `IsValid()` | `registry.valid(handle)` — **always check this for stored entity references** |
| `GetID()` | The raw entity index as a string; used for ImGui `##` label suffixes. *Not* the same as `IdComponent::id` |
| `GetComponentTypes()` | Walks `registry.storage()` and returns `entt::id_type` for every component the entity has. Powers `ComponentFunctionRegistry` |

### Stored entity references can dangle

Components hold `Entity` values (a curve holds its control points; a virtual entity holds
its parent). Entities get destroyed by `RemovalSystem`. Every place that walks such a list
therefore prunes as it goes:

```cpp
for (auto it = pointEntities.begin(); it != pointEntities.end(); )
{
    if (!it->IsValid() || !it->HasComponent<PositionComponent>())
    {
        it = pointEntities.erase(it);
        continue;
    }
    ...
    it++;
}
```

`MeshGeneratingSystem::CopyValidPointsToVector` and
`NotificationSystem::NotifyWithNotificationComponent` both do this. New code that stores
`Entity` must do the same.

---

## `BaseScene`

The starting scene, created by `App`:

```cpp
BaseScene::BaseScene()
{
    // camera entity
    auto camera = CreateEntity();
    auto& cc = camera.AddComponent<CameraComponent>();
    cc.active = true;
    cc.projectionMatrix = Algebra::Matrix4::Projection(aspect, 0.1f, 10000.0f, 3.14f / 2.f);
    cc.cameraHandling = CreateRef<DragCamera>();
    camera.AddComponent<NameComponent>().name = "camera";

    // infinite grid entity
    auto grid = CreateEntity();
    auto& mc = grid.AddComponent<MeshComponent>();
    mc.renderingMode = RenderingMode::Lines;
    mc.shaderTypes  = { AvailableShaders::InfiniteGrid };
    mc.mesh = StaticMeshManager::GetInstance().GetMesh(StaticMeshType::Grid);
}
```

Note the camera gets a `NameComponent`, so it shows up in the *Shape List* panel and can be
selected. The grid does not, so it stays hidden from the UI.

Subclass `Scene` the same way if you want an alternative starting scene, then change the
`CreateRef<BaseScene>()` call in [`App::App`](../../OpenglGeometry/src/App.cpp).
