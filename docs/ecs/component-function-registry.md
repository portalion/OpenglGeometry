# `ComponentFunctionRegistry`

[`src/scene/ComponentFunctionRegistry.h`](../../OpenglGeometry/src/scene/ComponentFunctionRegistry.h)

A small CRTP-flavoured helper that answers: *"for every component this entity happens to
have, run the handler I registered for that component type."*

It replaces what would otherwise be a chain of `if (e.HasComponent<X>()) DoX(e);`.

## The interface

```cpp
template<typename... Args>
class ComponentFunctionRegistry
{
    using ComponentAction = std::function<void(Entity, Args...)>;

private:
    std::unordered_map<entt::id_type, ComponentAction> m_ComponentToFunctionStorage;
    template<typename T> void RegisterFunction(ComponentAction function);

protected:
    template<typename TComponent, typename TClass>
    void Bind(void (TClass::*memberFunc)(Entity, Args...));

public:
    void PerformFunctions(Entity entity, Args... args);
};
```

- `Args...` are extra parameters every handler takes after the `Entity`.
- `Bind<TComponent>(&Derived::Handler)` registers a **member function** of the derived class,
  keyed by `entt::type_hash<TComponent>::value()`.
- `PerformFunctions(entity, args...)` iterates `entity.GetComponentTypes()` (which walks the
  registry's storages) and invokes any registered handler.

Handlers run in whatever order EnTT enumerates storages — **do not rely on ordering between
components.**

## Implementations

### `UniformApplier` — component → uniform contribution

[`src/systems/UniformApplier.cpp`](../../OpenglGeometry/src/systems/UniformApplier.cpp)

```cpp
class UniformApplier : public ComponentFunctionRegistry<EntityContext&>
{
    void PositionApplier(Entity, EntityContext&);
    void RotationApplier(Entity, EntityContext&);
    void ScaleApplier(Entity, EntityContext&);
public:
    UniformApplier();
};

UniformApplier::UniformApplier()
{
    Bind<PositionComponent>(&UniformApplier::PositionApplier);
    Bind<RotationComponent>(&UniformApplier::RotationApplier);
    Bind<ScaleComponent>(&UniformApplier::ScaleApplier);
}
```

Each handler writes one matrix into the shared `EntityContext`:

```cpp
void UniformApplier::PositionApplier(Entity e, EntityContext& ctx)
{
    ctx.Position = Algebra::Matrix4::Translation(e.GetComponent<PositionComponent>().position);
}
```

`EntityContext` starts with all three matrices as identity, so an entity with no
`ScaleComponent` simply contributes an identity scale. `RenderingSystem` then composes
`Position * Rotation * Scale` into `u_modelMatrix`.

`RenderingSystem` uses the same applier for the camera entity, reading
`cameraUniforms.Position[3]` (the translation row of the camera's translation matrix) as
`g_cameraPosition`.

### `ShapeInspectorSystem` — component → ImGui editor

[`src/systems/gui/ShapeInspectorSystem.cpp`](../../OpenglGeometry/src/systems/gui/ShapeInspectorSystem.cpp)

```cpp
class ShapeInspectorSystem : public ComponentFunctionRegistry<>, public ISystem
{
    void PositionInspect(Entity);
    void ScaleInspect(Entity);
    void RotationInspect(Entity);
    void LineInspect(Entity);
    void VirtualInspect(Entity);
    void TorusInspect(Entity);
    ...
};
```

Here `Args...` is empty, so handlers take only the `Entity`. `Process()` calls
`PerformFunctions(entity)` for each selected entity, and every component contributes its own
slice of the properties panel automatically.

## Adding a handler

Two steps, both in the same class:

```cpp
// 1. declare the handler (private)
void MyRegistry::MyComponentHandler(Entity entity /*, Args... */);

// 2. bind it in the constructor
Bind<MyComponent>(&MyRegistry::MyComponentHandler);
```

That is the whole change — no dispatch table to update, no `if` chain to extend.

See [how-to/add-an-inspector-field.md](../how-to/add-an-inspector-field.md) for the inspector
case and [renderer/shaders.md](../renderer/shaders.md) for the uniform case.

## Costs and caveats

- `PerformFunctions` calls `Entity::GetComponentTypes()`, which iterates **all** storages in
  the registry and tests `contains()` for each. That is O(number of component types) per
  entity per call, plus a heap allocation for the returned vector. It runs once per rendered
  entity per frame in `RenderingSystem`. Fine at this scale; worth remembering if entity
  counts grow.
- Only one handler per component type per registry — a second `Bind` for the same component
  overwrites the first silently.
- Handlers are stored as `std::function`, capturing `this`. The registry must therefore not
  be copied or moved after construction. Both current users are long-lived members.
