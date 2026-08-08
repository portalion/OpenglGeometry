# How to add a new system

## 1. Create the files

`src/systems/MySystem.h`:

```cpp
#pragma once
#include "core/Base.h"
#include "interfaces/ISystem.h"

class Scene;
class Entity;

class MySystem : public ISystem
{
private:
    Ref<Scene> m_Scene;

    void SomeHelper(Entity entity);
public:
    MySystem(Ref<Scene> scene);

    void Process() override;
};
```

`src/systems/MySystem.cpp`:

```cpp
#include "MySystem.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/Components.h"

MySystem::MySystem(Ref<Scene> scene)
    : m_Scene{ scene }
{
}

void MySystem::Process()
{
    for (Entity entity : m_Scene->GetAllEntitiesWith<SomeComponent>())
    {
        // ...
    }
}
```

Forward-declare `Scene`/`Entity` in the header and include them in the `.cpp` — that is the
convention used by `NotificationSystem` and `RenderingSystem`, and it keeps compile times
down. (`RemovalSystem` and `MeshGeneratingSystem` include the full headers; both styles
exist.)

GUI systems go in `src/systems/gui/` instead.

## 2. Add both files to the build

See [add-a-file-to-the-build.md](add-a-file-to-the-build.md). Skipping this gives you an
unresolved-external linker error, not a compile error.

## 3. Register it in the pipeline

`src/systems/SystemPipeline.cpp`:

```cpp
#include "MySystem.h"

SystemPipeline::SystemPipeline(Ref<Scene> m_Scene)
{
    this->m_Scene = m_Scene;

    m_Systems =
    {
        CreateRef<NotificationSystem>(m_Scene),
        CreateRef<RemovalSystem>(m_Scene),

        CreateRef<GUISystem>(m_Scene),
        CreateRef<ShapeInspectorSystem>(m_Scene),
        CreateRef<PopupSystem>(m_Scene),

        CreateRef<MySystem>(m_Scene),           // ← position matters, see below

        CreateRef<MeshGeneratingSystem>(m_Scene),
        CreateRef<RenderingSystem>(m_Scene)
    };
}
```

## Choosing the position

Systems run top to bottom, once per frame. Place yours according to what it produces and
consumes:

| Your system… | Put it… |
| --- | --- |
| produces `IsDirtyTag` from some input | before `MeshGeneratingSystem` |
| reacts to `ObserverChangedState` | before `NotificationSystem` clears it (i.e. first), or read `IsDirtyTag` afterwards instead |
| destroys entities (via `ToBeDestroyedTag`) | anywhere; `RemovalSystem` picks it up next frame |
| reads freshly generated meshes | after `MeshGeneratingSystem` |
| draws ImGui | anywhere, but conventionally with the other GUI systems |
| issues GL draw calls | after `RenderingSystem`, or extend that system instead |

Getting it wrong usually costs one frame of latency rather than crashing, which makes
ordering mistakes easy to miss. See [systems/README.md](../systems/README.md#order-matters).

## Rules

**Never destroy entities directly.**

```cpp
entity.AddTag<ToBeDestroyedTag>();     // RemovalSystem handles it
```

**Never mutate a view's filter while iterating it.** Adding a component the view filters on,
or destroying entities, can invalidate iteration. Either defer via a tag, or materialise
first:

```cpp
auto view = m_Scene->GetAllEntitiesWith<SomeComponent>();
std::vector<Entity> entities(view.begin(), view.end());
for (Entity e : entities) { /* safe to mutate */ }
```

**Check `IsValid()` on any stored `Entity`.**

**Set `IsDirtyTag` rather than regenerating geometry yourself.**

## If your system dispatches per component type

Inherit `ComponentFunctionRegistry` as well and bind handlers in the constructor — you get
per-component dispatch for free:

```cpp
class MySystem : public ComponentFunctionRegistry<>, public ISystem
{
    void HandleFoo(Entity entity);
    void HandleBar(Entity entity);
public:
    MySystem(Ref<Scene> scene)
        : m_Scene{ scene }
    {
        Bind<FooComponent>(&MySystem::HandleFoo);
        Bind<BarComponent>(&MySystem::HandleBar);
    }

    void Process() override
    {
        for (Entity e : m_Scene->GetAllEntitiesWith<IsSelectedTag>())
            PerformFunctions(e);
    }
};
```

`ShapeInspectorSystem` is the working example. See
[ecs/component-function-registry.md](../ecs/component-function-registry.md).

## If your system needs state or owned entities

Create entities in the constructor and keep the handle as a member — `GUISystem` does this
for the 3D cursor:

```cpp
MySystem::MySystem(Ref<Scene> scene)
    : m_Scene(scene)
{
    m_Helper = m_Scene->CreateEntity();
    Archetypes::AddShapeToEntity(m_Helper, "Helper");
    m_Helper.AddComponent<PositionComponent>();
}
```

Remember such an entity can still be deleted by the user through the shape list, so
re-validate in `Process()` if that would break you:

```cpp
if (!m_Helper.IsValid()) { /* recreate */ }
```
