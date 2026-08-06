# How to add an inspector field

The *Selected Shapes Properties* panel is driven by
[`ShapeInspectorSystem`](../systems/gui-systems.md#shapeinspectorsystem), which inherits
[`ComponentFunctionRegistry<>`](../ecs/component-function-registry.md). Adding an editor for
a component is two lines plus the handler body.

## 1. Declare the handler

`src/systems/gui/ShapeInspectorSystem.h`, private section:

```cpp
void MyComponentInspect(Entity entity);
```

## 2. Bind it

`src/systems/gui/ShapeInspectorSystem.cpp`, in the constructor:

```cpp
ShapeInspectorSystem::ShapeInspectorSystem(Ref<Scene> scene)
    : m_Scene{ scene }
{
    Bind<PositionComponent>(&ShapeInspectorSystem::PositionInspect);
    ...
    Bind<MyComponent>(&ShapeInspectorSystem::MyComponentInspect);   // ← new
}
```

The handler now runs automatically for any selected entity that has `MyComponent`. There is
no dispatch table to update and no `if` chain to extend.

## 3. Write the handler

```cpp
void ShapeInspectorSystem::MyComponentInspect(Entity entity)
{
    auto& component = entity.GetComponent<MyComponent>();

    if (ImGui::DragFloat(GUI::GenerateLabel(entity, "Some Value").c_str(),
                         &component.someValue, 0.1f))
    {
        entity.AddTag<IsDirtyTag>();
    }
}
```

---

## The three rules

### Always use `GUI::GenerateLabel`

```cpp
GUI::GenerateLabel(entity, "Radius").c_str()      // → "Radius##14"
```

ImGui identifies widgets by their label. Two selected tori both showing "Radius" would share
an ID and fight over the same drag state. The `##` suffix (the entity handle) is hidden from
the user and makes the ID unique. See [ui.md](../ui.md#guigeneratelabel).

If you render several widgets of the same kind *within one entity*, disambiguate further:

```cpp
GUI::GenerateLabel(entity, "Point " + std::to_string(i))
```

### Mark dirty when generated geometry depends on the value

Every ImGui widget returns `true` on the frame it changed:

```cpp
if (ImGui::DragFloat(label, &component.radius, 0.1f))
    entity.AddTag<IsDirtyTag>();
```

Without this the value changes but the mesh never rebuilds.

`MeshGeneratingSystem` runs *after* the inspector in the pipeline, so the change is visible
in the same frame.

### Assign through `Observable`, never mutate in place

For `PositionComponent` (or any future observable field), taking a reference and letting
ImGui write into it bypasses `operator=` and no notification fires. Copy out, edit, assign
back:

```cpp
Algebra::Vector4 tmpPosition = entity.GetComponent<PositionComponent>().position;
auto& position = entity.GetComponent<PositionComponent>().position;

if (ImGui::DragFloat3(GUI::GenerateLabel(entity, "Position").c_str(), &tmpPosition.x, 0.1f))
    position = tmpPosition;
```

See [ecs/change-propagation.md](../ecs/change-propagation.md).

---

## Widget cookbook

```cpp
// float
ImGui::DragFloat(label, &value, 0.1f);
ImGui::DragFloat(label, &value, 0.1f, 0.0f, 100.0f);          // with range

// Vector4 xyz — Algebra::Vector4 is 4 contiguous floats, so &v.x works
ImGui::DragFloat3(label, &vector.x, 0.1f);
ImGui::DragFloat4(label, &vector.x, 0.1f);

// unsigned — ImGui has no DragUInt, use the project helper
GUI::DragUInt(label, &unsignedValue, 1.0f, 3, 64);

// bool
ImGui::Checkbox(label, &flag);

// read-only text
ImGui::Text("Control Points: %zu", controlPoints.size());

// button
if (ImGui::Button(label)) { /* action */ }

// colour (once a colour component exists)
ImGui::ColorEdit4(label, &color.x);
```

## Existing handlers as models

| Pattern you need | Copy from |
| --- | --- |
| Simple float/uint parameters that dirty the mesh | `TorusInspect` |
| Observable value | `PositionInspect` |
| Plain in-place value | `ScaleInspect` |
| Read-only summary of a relationship | `LineInspect` |
| Toggling a tag on related entities | `VirtualInspect` |

## Beware of iterating relationships

`VirtualInspect` shows the trap: it renders one checkbox per child but builds the label from
the *parent* entity, so with more than one child the IDs collide and only one checkbox works.
When rendering a widget per related entity, derive the label from **that** entity:

```cpp
for (Entity virtualEntity : virtualEntities)
{
    if (!virtualEntity.IsValid()) continue;

    bool visible = !virtualEntity.HasComponent<IsInvisibleTag>();
    if (ImGui::Checkbox(GUI::GenerateLabel(virtualEntity, "visible").c_str(), &visible))
    {
        if (visible) virtualEntity.RemoveTag<IsInvisibleTag>();
        else         virtualEntity.AddTag<IsInvisibleTag>();
    }
}
```

(See [gotchas](../gotchas.md) — the current code uses `entity` rather than `virtualEntity`.)

## When the panel is not the right place

`ShapeInspectorSystem` only renders for entities with `IsSelectedTag`. For UI that is not
per-selected-entity, add a helper in `src/UI/` and call it from `GUISystem`, or write a
[new popup](../systems/gui-systems.md#adding-a-popup).
